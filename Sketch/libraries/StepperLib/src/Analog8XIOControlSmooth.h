////////////////////////////////////////////////////////
/*
  This file is part of CNCLib - A library for stepper motors.

  Copyright (c) 2013-2017 Herbert Aitenbichler

  CNCLib is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  CNCLib is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  http://www.gnu.org/licenses/
*/
////////////////////////////////////////////////////////

#pragma once

////////////////////////////////////////////////////////

template <pin_t PWMPIN, pin_t DIRPIN, uint16_t delayMs>
class CAnalog8XIOControlSmooth
{
public:

	void Init(int16_t level=0)		// init and set default value
	{
		_currentlevel = 0;
		MySetLevel(level);
#ifndef REDUCED_SIZE
		_level = level;
#endif
	}

	void On(int16_t level)					// Set level and turn on
	{
#ifndef REDUCED_SIZE
		_level = level;
#endif
		MySetLevel(level);
	}

	void OnMax()							// turn on at max level, same as On(255)
	{
		On(255);
	}

#ifndef REDUCED_SIZE
	void On()								// turn on at specified level (see Level property)
	{
		MySetLevel(_level);
	}
#endif

	void Off()								// turn off, use On() to switch on at same value
	{
		MySetLevel(0);
	}

	bool IsOn() const
	{
		return _iolevel != 0;
	}

#ifndef REDUCED_SIZE
	void SetLevel(int16_t level)
	{
		_level = level;
	}

	int16_t GetLevel() const
	{
		return _level;
	}
#endif

	int16_t GetIOLevel() const
	{
		return _iolevel;
	}

	int16_t GetCurrentIOLevel() const
	{
		return _currentlevel;
	}

	void Poll()
	{
		unsigned long milli;
		if (_currentlevel != _iolevel && (milli=millis()) >= _nexttime)
		{
			_nexttime = milli + delayMs;
			if (_currentlevel > _iolevel)
				_currentlevel--;
			else
				_currentlevel++;
			CHAL::digitalWrite(DIRPIN, _currentlevel >= 0);
			CHAL::analogWrite8(PWMPIN, (uint8_t) _currentlevel);
		}
	}

private:

	unsigned long _nexttime;		// time to modify level

#ifndef REDUCED_SIZE
	int16_t	_level;					// value if "enabled", On/Off will switch between 0..level
#endif

	int16_t	_currentlevel;			// used for analogWrite
	int16_t	_iolevel;				// current level


	void MySetLevel(int16_t level)
	{
		_iolevel = level;
		_nexttime = 0;
	}
};

////////////////////////////////////////////////////////
