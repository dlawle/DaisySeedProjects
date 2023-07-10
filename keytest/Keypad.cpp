/*
file Keypad.cpp
Copyright (C) 2023  Shegy sekerev@seznam.cz

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "Keypad.h"
#include <stdlib.h>
#include "stm32h7xx_hal.h"

// default constructor
Key::Key()
{
	kchar = NO_KEY;
	kcode = -1;
	kstate = KeyState::IDLE;
	stateChanged = false;
}

// constructor
Key::Key(char userKeyChar)
{
	kchar = userKeyChar;
	kcode = -1;
	kstate = KeyState::IDLE;
	stateChanged = false;
}

void Key::key_update(char userKeyChar, KeyState userState, bool userStatus)
{
	kchar = userKeyChar;
	kstate = userState;
	stateChanged = userStatus;
}

// <<constructor>> Allows custom keymap, pin configuration, and keypad sizes.
Keypad::Keypad(char *userKeymap, const char *rowPins[], const char *colPins[], unsigned char numRows, unsigned char numCols)
{
	for (int i = 0; i < numRows; i++)
	{
		rPorts[i] = rowPins[i][0] & 0x0F;
		rPins[i] = atoi(rowPins[i] + 1);
	}
	for (int i = 0; i < numCols; i++)
	{
		cPorts[i] = colPins[i][0] & 0x0F;
		cPins[i] = atoi(colPins[i] + 1);
	}

	_rowPins = rPins;
	_columnPins = cPins;
	sizeKpd.rows = numRows;
	sizeKpd.columns = numCols;
	keymap = userKeymap;
	keypadEventListener = 0;
	startTime = 0;
	single_key = false;
	setDebounceTime(10);
	setHoldTime(500);
}
// Returns a single key only. Retained for backwards compatibility.
char Keypad::getKey()
{
	single_key = true;

	if (getKeys() && key[0].stateChanged && (key[0].kstate == KeyState::PRESSED))
		return key[0].kchar;

	single_key = false;
	return NO_KEY;
}
// Populate the key list.
bool Keypad::getKeys()
{
	bool keyActivity = false;
	// Limit how often the keypad is scanned. This makes the loop() run 10 times as fast.
	if ((HAL_GetTick() - startTime) > debounceTime)
	{
		scanKeys();
		keyActivity = updateList();
		startTime = HAL_GetTick();
	}
	return keyActivity;
}
// Private : Hardware scan
void Keypad::scanKeys()
{
	// bitMap stores ALL the keys that are being pressed.
	for (unsigned char c = 0; c < sizeKpd.columns; c++)
	{
		HAL_GPIO_WritePin((GPIO_TypeDef *)(GPIO_BASE + (0x400 * cPorts[c])), 0x1 << _columnPins[c], GPIO_PIN_RESET);

		for (unsigned char r = 0; r < sizeKpd.rows; r++)
		{
			bitWrite(bitMap[r], c, !HAL_GPIO_ReadPin((GPIO_TypeDef *)(GPIO_BASE + (0x400 * rPorts[r])), 0x1 << _rowPins[r])); // keypress is active low so invert to high.
		}
		// Set pin to high impedance input. Effectively ends column pulse.
		HAL_GPIO_WritePin((GPIO_TypeDef *)(GPIO_BASE + (0x400 * cPorts[c])), 0x1 << _columnPins[c], GPIO_PIN_SET);
	}
}
// Manage the list without rearranging the keys. Returns true if any keys on the list changed state.
bool Keypad::updateList()
{
	bool anyActivity = false;
	// Delete any IDLE keys
	for (unsigned char i = 0; i < LIST_MAX; i++)
	{
		if (key[i].kstate == KeyState::IDLE)
		{
			key[i].kchar = NO_KEY;
			key[i].kcode = -1;
			key[i].stateChanged = false;
		}
	}
	// Add new keys to empty slots in the key list.
	for (unsigned char r = 0; r < sizeKpd.rows; r++)
	{
		for (unsigned char c = 0; c < sizeKpd.columns; c++)
		{
			bool button = bitRead(bitMap[r], c);
			char keyChar = keymap[r * sizeKpd.columns + c];
			int keyCode = r * sizeKpd.columns + c;
			int idx = findInList(keyCode);
			// Key is already on the list so set its next state.
			if (idx > -1)
				nextKeyState(idx, button);
			// Key is NOT on the list so add it.
			if ((idx == -1) && button)
			{
				for (unsigned char i = 0; i < LIST_MAX; i++)
				{
					if (key[i].kchar == NO_KEY)
					{		// Find an empty slot or don't add key to list.
						key[i].kchar = keyChar;
						key[i].kcode = keyCode;
						key[i].kstate = KeyState::IDLE;	// Keys NOT on the list have an initial state of IDLE.
						nextKeyState(i, button);
						break;	// Don't fill all the empty slots with the same key.
					}
				}
			}
		}
	}
	// Report if the user changed the state of any key.
	for (unsigned char i = 0; i < LIST_MAX; i++)
	{
		if (key[i].stateChanged)
			anyActivity = true;
	}
	return anyActivity;
}

// Private
// This function is a state machine but is also used for debouncing the keys.
void Keypad::nextKeyState(unsigned char idx, bool button)
{
	key[idx].stateChanged = false;

	switch (key[idx].kstate)
	{
	case KeyState::IDLE:
		if (button == GPIO_PIN_SET)
		{
			transitionTo(idx, KeyState::PRESSED);
			holdTimer = HAL_GetTick();
		}		// Get ready for next HOLD state.
		break;

	case KeyState::PRESSED:
		if ((HAL_GetTick() - holdTimer) > holdTime)	// Waiting for a key HOLD...
			transitionTo(idx, KeyState::HOLD);
		else if (button == GPIO_PIN_RESET)				// or for a key to be RELEASED.
			transitionTo(idx, KeyState::RELEASED);
		break;

	case KeyState::HOLD:
		if (button == GPIO_PIN_RESET)
			transitionTo(idx, KeyState::RELEASED);
		break;

	case KeyState::RELEASED:
		transitionTo(idx, KeyState::IDLE);
		break;
	}
}
// New in 2.1
bool Keypad::isPressed(char keyChar)
{
	for (unsigned char i = 0; i < LIST_MAX; i++)
	{
		if (key[i].kchar == keyChar)
			if ((key[i].kstate == KeyState::PRESSED) && key[i].stateChanged)
				return true;
	}
	return false;	// Not pressed.
}

// Search by character for a key in the list of active keys.
// Returns -1 if not found or the index into the list of active keys.
int Keypad::findInList(char keyChar)
{
	for (unsigned char i = 0; i < LIST_MAX; i++)
	{
		if (key[i].kchar == keyChar)
			return i;
	}
	return -1;
}

// Search by code for a key in the list of active keys.
// Returns -1 if not found or the index into the list of active keys.
int Keypad::findInList(int keyCode)
{
	for (unsigned char i = 0; i < LIST_MAX; i++)
	{
		if (key[i].kcode == keyCode)
			return i;
	}
	return -1;
}

// New in 2.0
char Keypad::waitForKey()
{
	char waitKey = NO_KEY;
	while ((waitKey = getKey()) == NO_KEY);	// Block everything while waiting for a keypress.
	return waitKey;
}

// Backwards compatibility function.
KeyState Keypad::getState()
{
	return key[0].kstate;
}

// The end user can test for any changes in state before deciding
// if any variables, etc. needs to be updated in their code.
bool Keypad::keyStateChanged()
{
	return key[0].stateChanged;
}

// The number of keys on the key list, key[LIST_MAX], equals the number
// of bytes in the key list divided by the number of bytes in a Key object.
unsigned char Keypad::numKeys()
{
	return sizeof(key) / sizeof(Key);
}

// Minimum debounceTime is 1 mS. Any lower *will* slow down the loop().
void Keypad::setDebounceTime(uint16_t debounce)
{
	if (debounce < 1)
		debounceTime = 1;
	else
		debounceTime = debounce;
}

void Keypad::setHoldTime(uint16_t hold)
{
	holdTime = hold;
}

void Keypad::addEventListener(void (*listener)(char))
{
	keypadEventListener = listener;
}

void Keypad::transitionTo(unsigned char idx, KeyState nextState)
{
	key[idx].kstate = nextState;
	key[idx].stateChanged = true;

	// Sketch used the getKey() function.
	// Calls keypadEventListener only when the first key in slot 0 changes state.
	if (single_key)
	{
		if ((keypadEventListener != 0) && (idx == 0))
			keypadEventListener(key[0].kchar);
	}
	// Sketch used the getKeys() function.
	// Calls keypadEventListener on any key that changes state.
	else if (keypadEventListener != 0)
		keypadEventListener(key[idx].kchar);
}
