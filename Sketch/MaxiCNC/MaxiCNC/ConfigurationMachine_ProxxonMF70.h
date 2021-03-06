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

#pragma once

////////////////////////////////////////////////////////

#define CMyStepper CStepperRamps14

////////////////////////////////////////////////////////

#define USBBAUDRATE 115200

////////////////////////////////////////////////////////

#define MYUSE_LCD

////////////////////////////////////////////////////////

#define MYNUM_AXIS	4

////////////////////////////////////////////////////////

#define X_MAXSIZE 130000				// in mm1000_t
#define Y_MAXSIZE 45000 
#define Z_MAXSIZE 81000 
#define A_MAXSIZE 360000 
#define B_MAXSIZE 360000 
#define C_MAXSIZE 360000 

////////////////////////////////////////////////////////

#define STEPPERDIRECTION 0
//#define STEPPERDIRECTION (1<<X_AXIS) + (1<<Y_AXIS)		// set bit to invert direction of each axis

#define STEPSPERROTATION  200
#define MICROSTEPPING     16
#define SCREWLEAD         1.0

////////////////////////////////////////////////////////

#define CNC_MAXSPEED ((steprate_t)28000)        // steps/sec => 8.75 rot /sec
#define CNC_ACC  350                            // 0.257 => time to full speed
#define CNC_DEC  400                            // 0.1975 => time to break
#define CNC_JERKSPEED 1000

////////////////////////////////////////////////////////
// NoReference, ReferenceToMin, ReferenceToMax

#define X_USEREFERENCE	EReverenceType::ReferenceToMin
#define Y_USEREFERENCE	EReverenceType::ReferenceToMin
#define Z_USEREFERENCE	EReverenceType::ReferenceToMax
#define A_USEREFERENCE	EReverenceType::NoReference
#define B_USEREFERENCE	EReverenceType::NoReference
#define C_USEREFERENCE	EReverenceType::NoReference

#define REFMOVE_1_AXIS  Z_AXIS
#define REFMOVE_2_AXIS  Y_AXIS
#define REFMOVE_3_AXIS  X_AXIS
#define REFMOVE_4_AXIS  255
#define REFMOVE_5_AXIS  255
#define REFMOVE_6_AXIS  255

#define X_REFERENCEHITVALUE_MIN LOW
#define Y_REFERENCEHITVALUE_MIN LOW
#define Z_REFERENCEHITVALUE_MIN LOW
#define A_REFERENCEHITVALUE_MIN LOW
#define B_REFERENCEHITVALUE_MIN LOW
#define C_REFERENCEHITVALUE_MIN LOW

#define X_REFERENCEHITVALUE_MAX LOW
#define Y_REFERENCEHITVALUE_MAX LOW
#define Z_REFERENCEHITVALUE_MAX LOW
#define A_REFERENCEHITVALUE_MAX LOW
#define B_REFERENCEHITVALUE_MAX LOW
#define C_REFERENCEHITVALUE_MAX LOW

#define MOVEAWAYFROMREF_MM1000 125

#undef SPINDLE_ANALOGSPEED
#define SPINDLE_MAXSPEED	255			// analog 255
#define SPINDEL_FADETIMEDELAY  0	// 8ms * 255 => 2040ms from 0 to max, 4080 from -max to +max

////////////////////////////////////////////////////////

#include "ConfigurationStepper_Ramps14.h"

////////////////////////////////////////////////////////

#define GO_DEFAULT_STEPRATE		  ((steprate_t) CConfigEeprom::GetConfigU32(offsetof(CConfigEeprom::SCNCEeprom, maxsteprate)))	// steps/sec
#define G1_DEFAULT_MAXSTEPRATE	((steprate_t) CConfigEeprom::GetConfigU32(offsetof(CConfigEeprom::SCNCEeprom, maxsteprate)))	// steps/sec
#define G1_DEFAULT_FEEDPRATE	  100000	// in mm1000 / min

#define STEPRATERATE_REFMOVE		(CNC_MAXSPEED/3)
//#define FEEDRATE_REFMOVE_PHASE2		200000

////////////////////////////////////////////////////////

#define CONTROLLERFAN_ONTIME	  10000			// switch off controllerfan if idle for 10 Sec
#define CONTROLLERFAN_FAN_PIN	  CAT(BOARDNAME,_FET2D9_PIN)
#define CONTROLLERFAN_ANALOGSPEED

////////////////////////////////////////////////////////

#define COOLANT_PIN	CAT(BOARDNAME,_AUX2_8)	// Ramps1.4 D42

#define COOLANT_PIN_ON  LOW
#define COOLANT_PIN_OFF HIGH

////////////////////////////////////////////////////////

#define SPINDLE_ENABLE_PIN	CAT(BOARDNAME,_AUX2_6)	// Ramps1.4 D40

#define SPINDLE_DIGITAL_ON  LOW
#define SPINDLE_DIGITAL_OFF HIGH

////////////////////////////////////////////////////////

#define PROBE_PIN	CAT(BOARDNAME,_AUX2_7)	// Ramps 1.4 D44 
#define PROBE2_PIN	CAT(BOARDNAME,_AUX2_5)	// Ramps 1.4 A10 

#define PROBE_PIN_ON  LOW
#define PROBE_PIN_OFF HIGH

#define HOLDRESUME_PIN		CAT(BOARDNAME, _LCD_KILL_PIN)
#define HOLDRESUME_PIN_ON	CAT(BOARDNAME, _LCD_KILL_PIN_ON)

////////////////////////////////////////////////////////

//#define LCD_MENU_MOVE100

#define LCD_GROW 64
#define LCD_GCOL 128

#define ROTARY_ENC           CAT(BOARDNAME,_LCD_ROTARY_ENC)
#define ROTARY_ENC_ON		 CAT(BOARDNAME,_LCD_ROTARY_ENC_ON)

#define ROTARY_EN1           CAT(BOARDNAME,_LCD_ROTARY_EN1)
#define ROTARY_EN2           CAT(BOARDNAME,_LCD_ROTARY_EN2)
//#define ROTARY_INVERTDIRECTION

#define SD_ENABLE_PIN		 CAT(BOARDNAME,_SDSS_PIN)

////////////////////////////////////////////////////////

#include <MessageCNCLibEx.h>

#if defined(__SAM3X8E__)
#define MESSAGE_MYCONTROL_Starting			F("Proxxon MF 70(HA) Ramps 1.4 due is starting ... (" __DATE__ ", " __TIME__ ")")
#define MESSAGE_LCD_HEADLINE						F("Proxxon MF70 Ramps14D")
#else
#define MESSAGE_MYCONTROL_Starting			F("Proxxon MF 70(HA) Ramps 1.4 is starting ... (" __DATE__ ", " __TIME__ ")")
#define MESSAGE_LCD_HEADLINE						F("Proxxon MF70 Ramps14M")
#endif

