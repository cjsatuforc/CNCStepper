////////////////////////////////////////////////////////
/*
  This file is part of CNCLib - A library for stepper motors.

  Copyright (c) 2013-2015 Herbert Aitenbichler

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

#include "stdafx.h"
#include <math.h>

#include "..\MsvcStepper\MsvcStepper.h"
#include "TestTools.h"

#include "..\..\..\sketch\libraries\CNCLib\src\Matrix4x4.h"
#include "DenavitHartenberg.h"

////////////////////////////////////////////////////////////////////////////////////////

bool CompareMatrix(CMatrix4x4<float>& m, float in1[][4], float out1[][4], unsigned char size)
{
	for (unsigned char i = 0; i < size; i++)
	{
		float v2[4];
		m.Mul(in1[i], v2);

		for (unsigned char n = 0; n < 4; n++)
		{
			if (!CMatrix4x4<float>::IsEqual(out1[i][n], v2[n], 0.00001))
			{
				return false;
			}
		}
	}
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////

int _tmain(int /* argc */, _TCHAR* /* argv */ [])
{
	float t[4][4] = { { 1.0, 2.0, 3.0, 4.0 }, { 5.0, 6.0, 7.0, 8.0 }, { 9.0, 10.0, 11.0, 12.0 }, { 13.0, 14.0, 15.0, 16.0 } };

	//////////////////////////////////////////
	//Test Compare
	{
		CMatrix4x4<float> T1(t);
		CMatrix4x4<float> T2(t);

		if (T1 == T2)
		{
			if (T1 != T2)
			{
				printf("Error Compare\n");
			}
		}
		else
		{
			printf("Error Compare\n");
		}

		T1.Set(3, 3, 1.2345);

		if (T1 == T2)
		{
			printf("Error Compare\n");
		}
		else
		{
		}
	}

	//////////////////////////////////////////
	//Test Mul
	{
		CMatrix4x4<float> T1(t);
		CMatrix4x4<float> T2(t);

		CMatrix4x4<float> T3 = T1*T2;

		float r[4][4] = { { 90.0, 100.0, 110.0, 120.0 }, { 202.0, 228.0, 254.0, 280.0 }, { 314.0, 356.0, 398.0, 440.0 }, { 426.0, 484.0, 542.0, 600.0 } };

		CMatrix4x4<float> T4(r);

		if (T3 != T4)
		{
			printf("Error Mul\n");
		}
	}

	//////////////////////////////////////////
	{
		CMatrix4x4<float> T1; T1.InitDenavitHartenberg1Rot(M_PI/4);
		CMatrix4x4<float> T2; T2.InitDenavitHartenberg1Rot(-M_PI/4);

		CMatrix4x4<float> T3 = T1*T2;
		CMatrix4x4<float> T4 = T4.InitDenavitHartenbergNOP();


		if (!T3.IsEqual(T4,0.0000001))
		{
			printf("Error InitDenavitHartenberg1Rot\n");
		}
	}

	//////////////////////////////////////////
	{
		const float angle = M_PI / 5;
		CMatrix4x4<float> T1; T1.InitDenavitHartenberg1Rot(angle);

		float in1[][4] = { 
				{ 1, 0, 0, 1 },
				{ 0, 1, 0, 1 }, 
				{ 0, 0, 1, 1 },
				{ 1, 1, 0, 1 }
		};
		float out1[][4] = {
				{ cos(angle), sin(angle), 0, 1 },
				{ -sin(angle), cos(angle), 0, 1 },
				{ 0, 0, 1, 1 },
				{ cos(angle) - sin(angle), cos(angle) + sin(angle), 0, 1 }
		};

		if (!CompareMatrix(T1, in1, out1, sizeof(in1) / sizeof(float[4])))
		{
			printf("Error InitDenavitHartenberg1Rot#2\n");
		}
	}

	//////////////////////////////////////////

	{
		CMatrix4x4<float> T1; T1.InitDenavitHartenberg2Trans(M_PI / 4);
		CMatrix4x4<float> T2; T2.InitDenavitHartenberg2Trans(-M_PI / 4);

		CMatrix4x4<float> T3 = T1*T2;
		CMatrix4x4<float> T4 = T4.InitDenavitHartenbergNOP();


		if (!T3.IsEqual(T4, 0.0000001))
		{
			printf("Error InitDenavitHartenberg2Trans\n");
		}
	}

	//////////////////////////////////////////
	{
		const float d = M_PI / 5;
		CMatrix4x4<float> T1; T1.InitDenavitHartenberg2Trans(d);

		float in1[][4] = {
			{ 1, 0, 0, 1 },
			{ 0, 1, 0, 1 },
			{ 0, 0, 1, 1 },
			{ 1, 1, 0, 1 }
		};
		float out1[][4] = {
			{ 1, 0, d, 1 },
			{ 0, 1, d, 1 },
			{ 0, 0, 1+d, 1 },
			{ 1, 1, 0+d, 1 }
		};

		if (!CompareMatrix(T1, in1, out1, sizeof(in1) / sizeof(float[4])))
		{
			printf("Error InitDenavitHartenberg2Trans#2\n");
		}
	}

	//////////////////////////////////////////

	{
		CMatrix4x4<float> T1; T1.InitDenavitHartenberg3Trans(M_PI / 4);
		CMatrix4x4<float> T2; T2.InitDenavitHartenberg3Trans(-M_PI / 4);

		CMatrix4x4<float> T3 = T1*T2;
		CMatrix4x4<float> T4 = T4.InitDenavitHartenbergNOP();


		if (!T3.IsEqual(T4, 0.0000001))
		{
			printf("Error InitDenavitHartenberg3Trans\n");
		}
	}

	//////////////////////////////////////////
	{
		const float a = M_PI / 5;
		CMatrix4x4<float> T1; T1.InitDenavitHartenberg3Trans(a);

		float in1[][4] = {
			{ 1, 0, 0, 1 },
			{ 0, 1, 0, 1 },
			{ 0, 0, 1, 1 },
			{ 1, 1, 0, 1 }
		};
		float out1[][4] = {
			{ 1+a, 0, 0, 1 },
			{ 0+a, 1, 0, 1 },
			{ 0+a, 0, 1, 1 },
			{ 1+a, 1, 0, 1 }
		};

		if (!CompareMatrix(T1, in1, out1, sizeof(in1) / sizeof(float[4])))
		{
			printf("Error InitDenavitHartenberg3Trans#2\n");
		}
	}

	//////////////////////////////////////////

	{
		CMatrix4x4<float> T1; T1.InitDenavitHartenberg4Rot(M_PI / 4);
		CMatrix4x4<float> T2; T2.InitDenavitHartenberg4Rot(-M_PI / 4);

		CMatrix4x4<float> T3 = T1*T2;
		CMatrix4x4<float> T4 = T4.InitDenavitHartenbergNOP();


		if (!T3.IsEqual(T4, 0.0000001))
		{
			printf("Error InitDenavitHartenberg4Rot\n");
		}
	}

	//////////////////////////////////////////
	{
		const float angle = M_PI / 5;
		CMatrix4x4<float> T1; T1.InitDenavitHartenberg4Rot(angle);

		float in1[][4] = {
			{ 1, 0, 0, 1 },
			{ 0, 1, 0, 1 },
			{ 0, 0, 1, 1 },
			{ 1, 1, 1, 1 }
		};
		float out1[][4] = {
			{ 1, 0, 0, 1 },
			{ 0, cos(angle), sin(angle), 1 },
			{ 0, -sin(angle), cos(angle), 1 },
			{ 1, cos(angle) - sin(angle), cos(angle) + sin(angle), 1 }
		};

		if (!CompareMatrix(T1, in1, out1, sizeof(in1) / sizeof(float[4])))
		{
			printf("Error InitDenavitHartenberg4Rot#2\n");
		}
	}

	//////////////////////////////////////////

	{
		float alpha = M_PI / 5;
		float theta = M_PI / 6;
		float a = 1.123456;
		float d = 4.321;

		CMatrix4x4<float> T1; T1.InitDenavitHartenberg(alpha,theta,a,d);
		CMatrix4x4<float> T2; T2.InitDenavitHartenbergInverse(alpha, theta, a, d);

		CMatrix4x4<float> T3 = T1*T2;
		CMatrix4x4<float> T4 = T4.InitDenavitHartenbergNOP();


		if (!T3.IsEqual(T4, 0.00001))
		{
			printf("Error InitDenavitHartenberg\n");
		}
	}

	//////////////////////////////////////////

	{
		float alpha = M_PI / 5;
		float theta = M_PI / 6;
		float a = 1.123456;
		float d = 4.321;

		CMatrix4x4<float> T10; T10.InitDenavitHartenberg(alpha, theta, a, d);

		CMatrix4x4<float> T1; T1.InitDenavitHartenberg1Rot(theta);
		CMatrix4x4<float> T2; T2.InitDenavitHartenberg2Trans(d);
		CMatrix4x4<float> T3; T3.InitDenavitHartenberg3Trans(a);
		CMatrix4x4<float> T4; T4.InitDenavitHartenberg4Rot(alpha);

		CMatrix4x4<float> T5 = T1*T2*T3*T4;

		if (!T5.IsEqual(T10, 0.00001))
		{
			printf("Error InitDenavitHartenberg sequenze\n");
		}
	}

	//////////////////////////////////////////
	{
		CDenavitHartenberg dh;

		float out[3];

		dh.ToPosition(out);

	}
}
