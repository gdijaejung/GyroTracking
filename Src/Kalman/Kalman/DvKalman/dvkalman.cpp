//
// MATLAB 활용, 칼만필터의 이해, Page 92, DvKalman.m
//
#include "stdafx.h"
#include <iostream>
#include <windows.h>

using namespace std;

Vector3 DvKalman(const Vector3 z);
Vector3 GetPos();


void main()
{

}


Vector3 GetPos()
{
	static Vector3 Posp;
	static Vector3 Velp;

	static bool first = true;
	if (first)
	{
		first = false;
		Posp = Vector3(0, 0, 0);
		Velp = Vector3(80, 0, 0);
	}

	const float dt = 0.1f;

	float w = ((rand() % 2000) - 1000) * 0.001f;
	float v = ((rand() % 2000) - 1000) * 0.001f;
	w *= 10.f;
	v *= 10.f;

	Vector3 z = Posp + Velp * dt + Vector3(v, 0, 0);

	Posp = z - Vector3(v, 0, 0);
	Velp = Vector3(80, 0, 0) + Vector3(w, 0, 0);

	return z;
}


Vector3 DvKalman(const Vector3 z)
{
	static Matrix44 A, H, Q, R;
	static Matrix44 P;
	static Vector3 x;
	static bool firstRun = false;

	if (!firstRun)
	{
		firstRun = true;

		const float dt = 0.1f;

		A.SetIdentity();
		A._12 = dt;

		H.SetZeros();
		H._11 = 1;

		Q.SetIdentity();
		Q._22 = 3;
		
		R.SetScale(Vector3(10, 1, 1));

		x = Vector3(0, 20, 0);
		P.SetScale(Vector3(5, 5, 5));
	}

	Vector3 xp = x * A;
	Matrix44 Pp = (A * P * A.Transpose()) + Q;
	Matrix44 K = Pp * H.Transpose() * (H * Pp * H.Transpose() + R).Inverse();

	x = xp + (z - (xp * H)) * K;
	P = Pp - (K * H * Pp);

	return x;
}
