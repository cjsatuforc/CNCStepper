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

////////////////////////////////////////////////////////////

#define SPEEDOVERIDESTEPSIZE	5

////////////////////////////////////////////////////////////

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <Arduino.h>
#include <U8glib.h>

#include <CNCLib.h>
#include <CNCLibEx.h>

#include "Beep.h"
#include "RotaryButton.h"

#include "Menu3D.h"
#include "U8GLcd.h"

#include "GCodeParser.h"
#include "GCode3DParser.h"
#include "GCodeBuilder.h"
#include "SDDirReader.h"

////////////////////////////////////////////////////////////
//
// used full graphic controller for Ramps 1.4 or FD
//
////////////////////////////////////////////////////////////

CU8GLcd::CU8GLcd()
{  
}

////////////////////////////////////////////////////////////

void CU8GLcd::Init()
{
	if (_lcd_numaxis > 5)
	{
		_charHeight = 9;
		_charWidth = 6;
		_font = u8g_font_6x12;
	}

	GetU8G().begin();

	super::Init();

	GetMenu().SetMainMenu();
	SetDefaultPage();
}

////////////////////////////////////////////////////////////

void CU8GLcd::SetRotaryPin(pin_t pin1, pin_t pin2, pin_t pinPush, uint8_t onValuePush)
{
	_rotarypushbutton.SetPin(pinPush, onValuePush);

	_rotarybutton.SetPin(pin1, pin2);

#if defined(__AVR_ARCH__) || defined(_MSC_VER)
#else

	CHAL::attachInterruptPin(pin1, CallRotaryButtonTickISR, CHANGE);
	CHAL::attachInterruptPin(pin2, CallRotaryButtonTickISR, CHANGE);

#endif

	_rotarybutton.Tick();
}

////////////////////////////////////////////////////////////

uint8_t CU8GLcd::GetPageCount()
{
	uint8_t count;
	for (count = 0; GetDrawFunction(&_pagedef[count].draw) != NULL; count++)
	{
	}
	return count;
}

////////////////////////////////////////////////////////////

void CU8GLcd::SetDefaultPage()
{
	_currentpage = 1;					// TODO: first (0 based) page is default
	SetRotaryFocusMainPage();
}

////////////////////////////////////////////////////////////

uint8_t CU8GLcd::GetPage()
{
	if (_rotaryFocus == RotaryMainPage)
	{
		uint8_t page = (uint8_t) _rotarybutton.GetPageIdx(GetPageCount());

		if (page != _currentpage)
		{
			Invalidate();
			_currentpage = page;
			GetMenu().SetMainMenu();
		}
	}

	return _currentpage;
}

////////////////////////////////////////////////////////////

void CU8GLcd::SetRotaryFocusMainPage()
{
	_rotarybutton.SetPageIdx((rotarypos_t) _currentpage); _rotarybutton.SetMinMax(0, GetPageCount() - 1, true);
	_rotaryFocus = RotaryMainPage;
}

////////////////////////////////////////////////////////////

void CU8GLcd::CallRotaryButtonTick()
{
	// called within TimerInterrupt() or attachedInterrupt

	switch (_rotarybutton.Tick())
	{
		case CRotaryButton<rotarypos_t, CU8GLcd_ROTARY_ACCURACY>::Nothing:
			break;
		default:
			_rotaryEventTime = millis();
			break;
	}
}

////////////////////////////////////////////////////////////

void CU8GLcd::TimerInterrupt()
{
	super::TimerInterrupt();

#if defined(__AVR_ARCH__) || defined(_MSC_VER)

	CallRotaryButtonTick();

#endif

	_rotarypushbutton.Check();
}

////////////////////////////////////////////////////////////

void CU8GLcd::Poll()
{
	GetPage();		// force invalidate if page changed

	super::Poll();

	if (_rotarypushbutton.IsOn())
	{
		bool screensaver = IsScreenSaver();
		_rotaryEventTime = millis();
		if (!screensaver)
			ButtonPress();
	}
}

////////////////////////////////////////////////////////////

void CU8GLcd::Command(char* buffer)
{
	super::Command(buffer);

	if (*buffer)
	{
		uint8_t totalcols = TotalCols();
		for (uint8_t commandlenght = 0; *buffer && commandlenght < totalcols; commandlenght++)
		{
			QueueCommandHistory(*(buffer++));
		}
		QueueCommandHistory(0);
	}
}

////////////////////////////////////////////////////////////

void CU8GLcd::QueueCommandHistory(char ch)
{
	if (_commandHis.IsFull())
	{
		// dequeue last command
		do
		{
			_commandHis.Dequeue();
		} 
		while (!_commandHis.IsEmpty() && _commandHis.Head() != 0);
	}
	_commandHis.Enqueue(ch);

}

////////////////////////////////////////////////////////////

unsigned long CU8GLcd::Draw(EDrawType draw)
{
	if (draw==DrawFirst)
	{
		SetDefaultPage();
	}

	DrawFunction fnc = GetDrawFunction(&_pagedef[GetPage()].draw);

	return DrawLoop(fnc);
}

////////////////////////////////////////////////////////////

bool CU8GLcd::IsScreenSaver() const
{
	return (!IsSplash() && !CStepper::GetInstance()->IsBusy() &&
		(millis() - _rotaryEventTime) > CU8GLcd_SCREENSAVERTIMEOUT &&
		(millis() - CStepper::GetInstance()->IdleTime()) > CU8GLcd_SCREENSAVERTIMEOUT);
}

////////////////////////////////////////////////////////////

unsigned long CU8GLcd::DrawLoop()
{
	unsigned long timeout = 1000;

	DrawFunction curretDraw = _curretDraw;

	if (IsScreenSaver())
		curretDraw = &CU8GLcd::DrawLoopScreenSaver;

	if (curretDraw)
	{
		if ((this->*curretDraw)(DrawLoopSetup,0))
		{
			GetU8G().firstPage();
			do
			{
				if (!(this->*curretDraw)(DrawLoopHeader,0))
					break;

				if (!(this->*curretDraw)(DrawLoopDraw,0))
					break;
			} 
			while (GetU8G().nextPage());
		}
		
		(this->*curretDraw)(DrawLoopQueryTimerout,(uintptr_t) &timeout);
	}
	return timeout;
}

////////////////////////////////////////////////////////////

#if defined(__AVR_ARCH__)

CU8GLcd::ButtonFunction CU8GLcd::GetButtonPress(const void* adr)
{
	struct ButtonFunctionWrapper
	{
		ButtonFunction fnc;
	}x;

	memcpy_P(&x, adr, sizeof(ButtonFunctionWrapper));

	return x.fnc;
}

CU8GLcd::DrawFunction CU8GLcd::GetDrawFunction(const void* adr)
{
	struct DrawFunctionWrapper
	{
		DrawFunction fnc;
	}x;

	memcpy_P(&x, adr, sizeof(DrawFunctionWrapper));

	return x.fnc;
}

#endif

////////////////////////////////////////////////////////////

char* CU8GLcd::DrawPos(axis_t axis, mm1000_t pos, char*tmp,  uint8_t precision)
{
	if (CGCodeParserBase::IsInch(axis))
	{
//		return CInch100000::ToString(MulDivI32(pos, 1000, 254), tmp, precision, 4);
		return CInch100000::ToString(MulDivI32(pos, 500, 127), tmp, precision, 4);
	}
	
	return CMm1000::ToString(pos, tmp, precision, 2);
}
								
////////////////////////////////////////////////////////////

void CU8GLcd::ButtonPress()
{
	ButtonFunction fnc = GetButtonPress(&_pagedef[GetPage()].buttonpress);

	if (fnc)
	{
		(*this.*fnc)();
		DrawRequest(DrawForceAll);
	}
}

////////////////////////////////////////////////////////////

unsigned long CU8GLcd::Splash()
{
	DrawLoop(&CU8GLcd::DrawLoopSplash);
	OKBeep();
	return 3000;
}

////////////////////////////////////////////////////////////

bool CU8GLcd::DrawLoopDefault(EnumAsByte(EDrawLoopType) type, uintptr_t /* data */)
{
	switch (type)
	{
		case DrawLoopSetup:
		{
			GetU8G().setFont(_font);
			return true;
		}
/*		=> default is 1000
		case DrawLoopQueryTimerout: 
		{
			*((unsigned long*)data) = 1000;
			return true;
		}
*/
	}
	return true;
}

////////////////////////////////////////////////////////////

bool CU8GLcd::DrawLoopScreenSaver(EnumAsByte(EDrawLoopType) type, uintptr_t data)
{
	if (type == DrawLoopHeader)	return true;
	if (type == DrawLoopQueryTimerout) { *((unsigned long*)data) = 50; return true; }
	if (type != DrawLoopDraw)	return DrawLoopDefault(type, data);

	DrawString(ToCol(_screensaveX), ToRow(_screensaveY), F("*"));

	const int8_t textsize = 0;

	if (_screensaveTime > millis()) return true;
	_screensaveTime = millis() + 500;

	_screensaveX += _screensaveXDiff;
	if (_screensaveX >= (TotalCols() - textsize) || _screensaveX < 0)
	{
		_screensaveX -= _screensaveXDiff*2;
		_screensaveXDiff = -_screensaveXDiff;
	}

	_screensaveY += _screensaveYDiff;
	if (_screensaveY >= TotalRows() || _screensaveY < 0)
	{
		_screensaveY -= _screensaveYDiff * 2;
		_screensaveYDiff = -_screensaveYDiff;
	}


	return true;
}

////////////////////////////////////////////////////////////

bool CU8GLcd::DrawLoopSplash(EnumAsByte(EDrawLoopType) type, uintptr_t data)
{
	if (type==DrawLoopQueryTimerout)	{ *((unsigned long*)data) = 200000; return true; }
	if (type!=DrawLoopDraw)	return DrawLoopDefault(type,data);

	DrawString(ToCol(TotalCols() / 2 - 1), ToRow(2), F("by"));
	DrawString(ToCol(3), ToRow(3), F("H. Aitenbichler"));
	DrawString(ToCol(5), ToRow(5), F(__DATE__));

	return true;
}

////////////////////////////////////////////////////////////

bool CU8GLcd::DrawLoopDebug(EnumAsByte(EDrawLoopType) type, uintptr_t data)
{
	if (type==DrawLoopHeader)	return true;
	if (type!=DrawLoopDraw)		return DrawLoopDefault(type,data);

	DrawString(ToCol(0), ToRow(0) - HeadLineOffset(), F("Debug"));

	char tmp[16];

	for (uint8_t i = 0; i < _lcd_numaxis; i++)
	{
		SetPosition(ToCol(0), ToRow(i + 1) + PosLineOffset());

		udist_t pos = CStepper::GetInstance()->GetCurrentPosition(i);

		Print(CSDist::ToString(pos, tmp, 6));
		Print(F(" "));
		Print(DrawPos(i,CMotionControlBase::GetInstance()->ToMm1000(i, pos),tmp,6));
		Print(F(" "));

		Print(CStepper::GetInstance()->GetReferenceValue(CStepper::GetInstance()->ToReferenceId(i, true)) ? '1' : '0');
		Print(CStepper::GetInstance()->GetReferenceValue(CStepper::GetInstance()->ToReferenceId(i, false)) ? '1' : '0');

		Print((CStepper::GetInstance()->GetLastDirection()&(1 << i)) ? '+' : '-');
	}

	SetPosition(ToCol(19), ToRow(0 + 1) + PosLineOffset());
	Print(CControl::GetInstance()->IOControl(CControl::Probe) ? '1' : '0');

	SetPosition(ToCol(20), ToRow(0 + 1) + PosLineOffset());
	Print(CControl::GetInstance()->IsHold() ? '1' : '0');

	SetPosition(ToCol(19), ToRow(0 + 3) + PosLineOffset());
	Print(CSDist::ToString(CControl::GetInstance()->GetBufferCount(), tmp, 2));

	SetPosition(ToCol(19), ToRow(0 + 4) + PosLineOffset());
	Print(CSDist::ToString(CStepper::GetInstance()->QueuedMovements(), tmp, 2));

	SetPosition(ToCol(18), ToRow(0 + 5) + PosLineOffset());
	Print(CSDist::ToString(CStepper::SpeedOverrideToP(CStepper::GetInstance()->GetSpeedOverride()), tmp, 3));


	return true;
}

////////////////////////////////////////////////////////////

bool CU8GLcd::DrawLoopPosAbs(EnumAsByte(EDrawLoopType) type, uintptr_t data)
{
	if (type==DrawLoopHeader)	return true;
	if (type!=DrawLoopDraw)		return DrawLoopDefault(type,data);

	SetPosition(ToCol(0), ToRow(0) - HeadLineOffset()); Print(F("Absolut  Current"));
	char tmp[16];

	for (uint8_t i = 0; i < _lcd_numaxis; i++)
	{
		mm1000_t psall = CGCodeParser::GetAllPreset(i);

		SetPosition(ToCol(0), ToRow(i + 1) + PosLineOffset());
		Print(CGCodeBuilder::AxisToChar(i));

		Print(DrawPos(i,CMotionControlBase::GetInstance()->GetPosition(i),tmp,7));
		Print(F(" "));
		Print(DrawPos(i,CMotionControlBase::GetInstance()->GetPosition(i) - psall,tmp,7));
	}

	return true;
}

////////////////////////////////////////////////////////////

bool CU8GLcd::DrawLoopPos(EnumAsByte(EDrawLoopType) type, uintptr_t data)
{
	if (type == DrawLoopHeader)	return true;
	if (type != DrawLoopDraw)	return DrawLoopDefault(type, data);

	SetPosition(ToCol(0), ToRow(0) - HeadLineOffset()); Print(F("Absolut# Current"));
	char tmp[16];

	mm1000_t dest[NUM_AXIS];
	udist_t src[NUM_AXIS];
	CStepper::GetInstance()->GetCurrentPositions(src);

	CMotionControlBase::GetInstance()->GetPosition(src, dest);

	for (uint8_t i = 0; i < _lcd_numaxis; i++)
	{
		mm1000_t psall = CGCodeParser::GetAllPreset(i);

		SetPosition(ToCol(0), ToRow(i + 1) + PosLineOffset());
		Print(CGCodeBuilder::AxisToChar(i));

		Print(DrawPos(i,dest[i],tmp,7));
		Print(F(" "));
		Print(DrawPos(i,dest[i] - psall,tmp,7));
	}

	return true;
}

////////////////////////////////////////////////////////////

bool CU8GLcd::DrawLoopRotate2D(EnumAsByte(EDrawLoopType) type, uintptr_t data)
{
	if (type == DrawLoopHeader)	return true;
	if (type==DrawLoopQueryTimerout)	{ *((unsigned long*)data) = 200000; return true; }
	if (type != DrawLoopDraw)	return DrawLoopDefault(type, data);

	SetPosition(ToCol(0), ToRow(0) - HeadLineOffset()); Print(F("Rotate 2D"));
	char tmp[16];

	for (uint8_t i = 0; i < NUM_AXISXYZ; i++)
	{
		SetPosition(ToCol(0), ToRow(i + 1) + PosLineOffset());
		Print(CGCodeBuilder::AxisToChar(i));

		mm1000_t ofs = CMotionControl::GetInstance()->GetOffset2D(i);
		Print(DrawPos(i,ofs,tmp,7));

		if (CMotionControl::GetInstance()->IsEnabled2D(i))
		{
			mm1000_t rad = CMm1000::FromRAD(CMotionControl::GetInstance()->GetAngle2D(i));

			Print(F(" "));
			Print(CMm1000::ToString(rad,tmp,7,2));
		}
	}

	return true;
}

////////////////////////////////////////////////////////////

bool CU8GLcd::DrawLoopRotate3D(EnumAsByte(EDrawLoopType) type, uintptr_t data)
{
	if (type == DrawLoopHeader)	return true;
	if (type==DrawLoopQueryTimerout)	{ *((unsigned long*)data) = 200000; return true; }
	if (type != DrawLoopDraw)		return DrawLoopDefault(type, data);

	SetPosition(ToCol(0), ToRow(0) - HeadLineOffset()); Print(F("Rotate 3D"));

	if (CMotionControl::GetInstance()->IsRotate())
	{
		char tmp[16];
	
		for (uint8_t i = 0; i < NUM_AXISXYZ; i++)
		{
			mm1000_t ofs  = CMotionControl::GetInstance()->GetOffset(i);
			mm1000_t vect = CMotionControl::GetInstance()->GetVector(i);

			SetPosition(ToCol(0), ToRow(i + 1) + PosLineOffset());
			Print(CGCodeBuilder::AxisToChar(i));

			Print(DrawPos(i,ofs,tmp,7));
			Print(F(" "));
			Print(DrawPos(i,vect,tmp,7));
		}

		SetPosition(ToCol(0), ToRow(NUM_AXISXYZ + 1) + PosLineOffset());
		Print(F("R"));
		Print(CMm1000::ToString(CMm1000::FromRAD(CMotionControl::GetInstance()->GetAngle()), tmp, 7, 2));
	}
	else
	{
		SetPosition(ToCol(6), ToRow(2 + 1) + PosLineOffset());
		Print(F("no rotation"));
	}

	return true;
}

////////////////////////////////////////////////////////////

inline uint8_t ToPageIdx(uint8_t idx)
{
	return idx / SPEEDOVERIDESTEPSIZE;
}

bool CU8GLcd::DrawLoopSpeedOverride(EnumAsByte(EDrawLoopType) type, uintptr_t data)
{
	if (type == DrawLoopHeader)			return true;
	if (type==DrawLoopQueryTimerout && _rotaryFocus == RotarySlider)	{ *((unsigned long*)data) = 200; return true; }
	if (type != DrawLoopDraw)			return DrawLoopDefault(type, data);

	SetPosition(ToCol(0), ToRow(0) - HeadLineOffset()); Print(F("Speed Override"));
	char tmp[16];

	SetPosition(ToCol(8), ToRow(2 + 1) + PosLineOffset());

	if (_rotaryFocus == RotarySlider)
	{
		uint8_t speedInP = _rotarybutton.GetPageIdx(ToPageIdx(100)+1) * SPEEDOVERIDESTEPSIZE;
		CStepper::GetInstance()->SetSpeedOverride(CStepper::PToSpeedOverride(speedInP));
		Print('>');
	}

	Print(CSDist::ToString(CStepper::SpeedOverrideToP(CStepper::GetInstance()->GetSpeedOverride()), tmp, 3));

	if (_rotaryFocus == RotarySlider)
		Print('<');

	return true;
}

////////////////////////////////////////////////////////////

void CU8GLcd::ButtonPressSpeedOverride()
{
	if (_rotaryFocus == RotarySlider)	
	{
		SetRotaryFocusMainPage();
		OKBeep();
	}
	else 
	{
		_rotarybutton.SetMinMax(1, ToPageIdx(100), false);
		_rotarybutton.SetPageIdx((rotarypos_t)ToPageIdx(CStepper::SpeedOverrideToP(CStepper::GetInstance()->GetSpeedOverride())));
		_rotaryFocus = RotarySlider;
		OKBeep();
	}
}

////////////////////////////////////////////////////////////

bool CU8GLcd::DrawLoopPreset(EnumAsByte(EDrawLoopType) type, uintptr_t data)
{
	if (type==DrawLoopHeader)			return true;
	if (type==DrawLoopQueryTimerout)	{ *((unsigned long*)data) = 200000; return true; }
	if (type!=DrawLoopDraw)				return DrawLoopDefault(type,data);

	mm1000_t ps;

	const __FlashStringHelper* zeroShiftName[] PROGMEM = { F("G53"), F("G54"), F("G55"), F("G56"), F("G57"), F("G58"), F("G59") };

	SetPosition(ToCol(0), ToRow(0) - HeadLineOffset());  Print(F("Preset: ")); Print(zeroShiftName[CGCodeParser::GetZeroPresetIdx()]); Print(F(" G92 Height"));

	char tmp[16];

	for (uint8_t i = 0; i < _lcd_numaxis; i++)
	{
		SetPosition(ToCol(0), ToRow(i + 1) + PosLineOffset());
		Print(CGCodeBuilder::AxisToChar(i));
		ps = CGCodeParser::GetG54PosPreset(i);
		Print(DrawPos(i,ps,tmp,7));

		ps = CGCodeParser::GetG92PosPreset(i);
		Print(DrawPos(i,ps,tmp,7));

		ps = CGCodeParser::GetToolHeightPosPreset(i);
		Print(DrawPos(i,ps, tmp, 6));
	}
	return true;
}

////////////////////////////////////////////////////////////

void CU8GLcd::ButtonPressStartSDPage()
{
	_currentpage = GetPageCount() - 1;	// TODO: last is default menu
	GetMenu().SetSDMenu();
	SetRotaryFocusMenuPage();
	OKBeep();
}

////////////////////////////////////////////////////////////

bool CU8GLcd::DrawLoopStartSD(EnumAsByte(EDrawLoopType) type, uintptr_t data)
{
	if (type!=DrawLoopDraw)				return DrawLoopDefault(type,data);
	if (type==DrawLoopQueryTimerout)	{ *((unsigned long*)data) = CGCode3DParser::GetExecutingFile() ? 1000 : 5000; return true; }

	if (CGCode3DParser::GetExecutingFileName()[0])
	{
		char tmp[16];

		SetPosition(ToCol(0), ToRow(3) + PosLineOffset()); Print(F("File: ")); Print(CGCode3DParser::GetExecutingFileName());
		SetPosition(ToCol(0), ToRow(4) + PosLineOffset()); Print(F("At:   ")); Print(CSDist::ToString(CGCode3DParser::GetExecutingFilePosition(), tmp, 8));
		SetPosition(ToCol(0), ToRow(5) + PosLineOffset()); Print(F("Line: ")); Print(CSDist::ToString(CGCode3DParser::GetExecutingFileLine(), tmp, 8));
	}
	else
	{
		DrawString(ToCol(3), ToRow(2), F("No SD card found"));
		DrawString(ToCol(2), ToRow(3), F("or no file selected"));
	}

	return true;
}

////////////////////////////////////////////////////////////

bool CU8GLcd::DrawLoopError(EnumAsByte(EDrawLoopType) type, uintptr_t data)
{
	if (type!=DrawLoopDraw)		return DrawLoopDefault(type,data);
	if (type==DrawLoopQueryTimerout)	{ *((unsigned long*)data) = 5000; return true; }

	uint8_t errors = 0;

	if (CStepper::GetInstance()->GetFatalError())
		DrawString(ToCol(0), ToRow(++errors + 1), CStepper::GetInstance()->GetFatalError());

	if (GetDiagnostic())
		DrawString(ToCol(0), ToRow(++errors + 1), GetDiagnostic());

	if (CControl::GetInstance()->IsKilled())
		DrawString(ToCol(0), ToRow(++errors + 1), F("emergency stop"));

	if (CControl::GetInstance()->IsHold())
		DrawString(ToCol(0), ToRow(++errors + 1), F("hold"));

	if (errors == 0)
		DrawString(ToCol(0), ToRow(2), F("no errors"));

	return true;
}

////////////////////////////////////////////////////////////

bool CU8GLcd::DrawLoopCommandHis(EnumAsByte(EDrawLoopType) type,uintptr_t data)
{
	if (type==DrawLoopQueryTimerout)	{ *((unsigned long*)data) = 5000; return true; }
	if (type!=DrawLoopDraw)		return DrawLoopDefault(type,data);

	uint8_t totalCols = TotalCols();
	uint8_t totalRows = TotalRows();
	//	char tmp[totalCols + 1];
	char tmp[40 + 1];
	uint8_t commandpos = _commandHis.T2HInit();	// idx of \0 of last command

	for (uint8_t i = 0; i < totalRows - 1; i++)
	{
		SetPosition(ToCol(0), ToRow(totalRows - i - 1) + PosLineOffset());

		uint8_t idx = totalCols;
		tmp[idx] = 0;

		if (_commandHis.T2HTest(commandpos))
		{
			for (commandpos = _commandHis.T2HInc(commandpos); _commandHis.T2HTest(commandpos) && _commandHis.Buffer[commandpos] != 0;commandpos = _commandHis.T2HInc(commandpos))
			{
				tmp[--idx] = _commandHis.Buffer[commandpos];
			}
			Print(&tmp[idx]);
		}
	}

	return true;
}

////////////////////////////////////////////////////////////
