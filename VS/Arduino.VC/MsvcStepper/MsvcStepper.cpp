////////////////////////////////////////////////////////
/*
  This file is part of CNCLib - A library for stepper motors.

  Copyright (c) 2013-2018 Herbert Aitenbichler

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

#define _CRT_SECURE_NO_WARNINGS

#include <Stdio.h>
#include <StdLib.h>

#include <Arduino.h>
#include <avr/interrupt.h>
#include "MsvcStepper.h"

////////////////////////////////////////////////////////////

CMsvcStepper::CMsvcStepper()
{
	_isReferenceMove = false;
	DelayOptimization = true;;
	SplitFile = true;
	UseSpeedSign = false;
	_TimerEvents = NULL;
	CacheSize = _STORETIMEVALUES;
	_oldCacheSize = -1;
}

////////////////////////////////////////////////////////////

void CMsvcStepper::Init()
{
	__super::Init();
}

////////////////////////////////////////////////////////////

void CMsvcStepper::OnWait(EnumAsByte(EWaitType) wait)
{
	__super::OnWait(wait);
	DoISR();

	if (wait == MovementQueueFull && CanQueueMovement())	// doISR has finsihed move
	{
		static enum EDest
		{
			ToAcc,
			ToRun,
			ToDec,
		} dest = ToAcc;

		SMovement& mv = _movements._queue.Head();
		while (!mv.IsFinished())
		{
			DoISR();
			if (mv.IsDownMove())
				break;
		}
	}
}

////////////////////////////////////////////////////////////

void CMsvcStepper::DoISR()
{
	HandleInterrupt();
}

////////////////////////////////////////////////////////////

void CMsvcStepper::HandleIdle()
{
	if (IsBusy())
		DoISR();

	static unsigned long lasttimerinterrupt = 0;

	if (lasttimerinterrupt + 100 < millis())
	{
		lasttimerinterrupt = millis();
		if (CHAL::_TimerEvent0)
			CHAL::_TimerEvent0();
	}
}

////////////////////////////////////////////////////////////

void CMsvcStepper::OnStart()
{
	_refMovestart = 0;
	__super::OnStart();
}

////////////////////////////////////////////////////////////

void CMsvcStepper::OnIdle(unsigned long idletime)
{
	__super::OnIdle(idletime);
}

////////////////////////////////////////////////////////////

uint8_t CMsvcStepper::GetReferenceValue(uint8_t referenceid)
{
	uint8_t refhitvalue = _pod._referenceHitValue[referenceid];
	uint8_t refoffvalue = _pod._referenceHitValue[referenceid] == LOW ? HIGH : LOW;

	if (!_isReferenceMove || referenceid != _isReferenceId)
	{
		return refoffvalue;
	}

	_referenceMoveSteps++;

	return (_referenceMoveSteps / 16) % 2 == 0 ? refhitvalue : refoffvalue;
}

////////////////////////////////////////////////////////////

bool CMsvcStepper::MoveReference(axis_t axis, uint8_t referenceid, bool toMin, steprate_t vMax, sdist_t maxdist, sdist_t distToRef, sdist_t distIfRefIsOn)
{
	_referenceMoveSteps = 15;
	_isReferenceMove = true;
	_isReferenceId = referenceid;
	bool ret = __super::MoveReference(axis, referenceid, toMin, vMax, maxdist, distToRef, distIfRefIsOn);
	_isReferenceMove = false;
	return ret;
}

////////////////////////////////////////////////////////////

void CMsvcStepper::StartTimer(timer_t timerB)
{
	timerB += TIMEROVERHEAD;
	_TimerEvents[_eventIdx].TimerValues = timerB;
	__super::StartTimer(timerB);

};

////////////////////////////////////////////////////////////

void CMsvcStepper::SetIdleTimer()
{
	__super::SetIdleTimer();
}

////////////////////////////////////////////////////////////

void CMsvcStepper::StepRequest(bool isr)
{
	_refMovestart++;
	__super::StepRequest(isr);
}

////////////////////////////////////////////////////////////

void CMsvcStepper::StepBegin(const SStepBuffer* stepbuffer)
{
	_TimerEvents[_eventIdx].Steps = stepbuffer->_steps;
	_TimerEvents[_eventIdx].Count = stepbuffer->_count;
	int multiplier = stepbuffer->DirStepCount;
	for (int i = 0; i < NUM_AXIS; i++)
	{
		_TimerEvents[_eventIdx].Axis[i].MoveAxis = 0;
		_TimerEvents[_eventIdx].Axis[i].Distance = stepbuffer->_distance[i];
		_TimerEvents[_eventIdx].Axis[i].Multiplier = multiplier % 8;
		multiplier = multiplier / 16;
	}
	strcpy_s(_TimerEvents[_eventIdx].MSCInfo, stepbuffer->_spMSCInfo);
}

////////////////////////////////////////////////////////////
/*
void  CStepper::Step(axis_t axis , bool  directionUp , uint8_t count )
{
	// CStepper:: is abstract => for call of base class in derived class!
}
*/
////////////////////////////////////////////////////////////

void CMsvcStepper::Step(const uint8_t steps[NUM_AXIS], axisArray_t directionUp, bool /* isSameDirection */)
{
	for (axis_t axis=0 ; axis< NUM_AXIS; axis++)
	{
		_TimerEvents[_eventIdx].Axis[axis].MoveAxis = (directionUp & ((1<<axis)))!=0 ? steps[axis] : -steps[axis];
	}

	_TotalSteps++;
	_eventIdx++;

	if (_eventIdx < CacheSize)
	{
	}
	else
	{
		WriteTestResults(_filename);
		InitCache();
	}
}

////////////////////////////////////////////////////////////

void CMsvcStepper::OptimizeMovementQueue(bool force)
{
	if (!DelayOptimization || force || _movements._queue.IsFull())
		__super::OptimizeMovementQueue(force);
}

////////////////////////////////////////////////////////////

void CMsvcStepper::InitTest(const char* filename)
{
	if (_oldCacheSize != CacheSize)
	{
		delete[] _TimerEvents;
		_TimerEvents = NULL;
	}
	if (_TimerEvents==NULL)
	{
		_TimerEvents = new STimerEvent[CacheSize];
		_oldCacheSize = CacheSize;
	}
	_flushcount = 0;
	_filename = filename;

	Init();

	SetDefaultMaxSpeed(5000, 100, 150);
	ContinueMove();

	_TotalSteps = 1;
	_exportIdx = 1;

	for (axis_t x = 0; x < NUM_AXIS_MVC; x++)
	{
		if (x < NUM_AXIS)
		{
			SetJerkSpeed(x, 500);
			SetPosition(x, 0);
		}
		_sumtime[x] = 0;
		_count[x] = 0;
		_sumtime[x] = 0;
		_total[x] = 0;
		_speed[x][0] = 0;
	}
	_totaltime = 0;
	_lasttimer = 0;

	MSCInfo = "";

	SetWaitConditional(false);

	InitCache();
}

////////////////////////////////////////////////////////////

void CMsvcStepper::EndTest(const char* filename)
{
	_filename = filename ? filename : _filename;

	OptimizeMovementQueue(true);
	WaitBusy();

	WriteTestResults(_filename);
}

////////////////////////////////////////////////////////////

void CMsvcStepper::WriteTestResults(const char* filename)
{
	bool append = (_flushcount++) != 0;
	char fname[_MAX_PATH] = { 0 };
	fname[0]=0;

/*
	if(false)
	{
		char tempPath[_MAX_PATH];
		::GetTempPathA(_MAX_PATH, tempPath); 
		strcat(fname,tempPath);
	}
*/
	strcat(fname,filename);

	if (append)
	{
		if (SplitFile)
		{
			char *dot = strrchr(fname,'.');
			char ext[16];
			strcpy(ext,dot);
			*(dot++) = '#';
			_itoa(_flushcount,dot,10);
			strcat(dot,ext);
			append = false;
		}
	}

	FILE*f = fopen(fname, append ? "at" : "wt");

	int timerconstant = 65536 * 32;

	for (int i = 0; i < _eventIdx; i++)
	{
		int outtotaltime = int (_totaltime / 1000);
		int timer = _TimerEvents[i].TimerValues;
		if (timer == 0)
		{
			timer = _lasttimer;
		}
		else
		{
			int stepidx;
			for (stepidx = 1; i + stepidx < _eventIdx; stepidx++)
			{
				if (_TimerEvents[i + stepidx].TimerValues)
					break;
			}
			_lasttimer = timer / stepidx;
			timer = _lasttimer;
		}

		for (int x = 0; x < NUM_AXIS; x++)
		{
			int outspeed = 10 * (timerconstant / timer)*_TimerEvents[i].Axis[x].Multiplier;
			_total[x] += _TimerEvents[i].Axis[x].MoveAxis;
			_sumtime[x] += outspeed;
			_count[x] += 1;
			if (_TimerEvents[i].Axis[x].MoveAxis != 0)
			{
				//				speed[j] = sumtime[j] / count[j];
				//				sprintf(speed[j],"%i",outspeed / count[j]);
				int speed = (int)(__int64(outspeed) * __int64(_TimerEvents[i].Axis[x].Distance) / __int64(_TimerEvents[i].Steps));
				if (UseSpeedSign && _TimerEvents[i].Axis[x].MoveAxis < 0)  speed = -speed;
				sprintf(_speed[x], "%i", speed);
				_count[x] = 0;
				_sumtime[x] = 0;
			}
			else
			{
				_speed[x][0] = 0;
			}
		}

		int sysspeed = 10 * (timerconstant / timer)*_TimerEvents[i].Count;

		fprintf(f, "%i;%i;%i;%i;%s;%s;%s;%s;%s;%i;%i;%i;%i;%i;%i;%i;%i;%i;%i;%s\n",
			_exportIdx++,
			_TimerEvents[i].TimerValues,
			outtotaltime,
			sysspeed,
			_speed[0],
			_speed[1],
			_speed[2],
			_speed[3],
			_speed[4],
			_TimerEvents[i].Axis[0].MoveAxis,
			_TimerEvents[i].Axis[1].MoveAxis,
			_TimerEvents[i].Axis[2].MoveAxis,
			_TimerEvents[i].Axis[3].MoveAxis,
			_TimerEvents[i].Axis[4].MoveAxis,
			_total[0],
			_total[1],
			_total[2],
			_total[3],
			_total[4],
			_TimerEvents[i].MSCInfo);
		_totaltime += timer;
	}
	fclose(f);
}
