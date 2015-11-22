//
// MATLAB 활용, 칼만필터의 이해, Page 78, SimpleKalman.m
//

#include "stdafx.h"
#include <iostream>

using namespace std;

Vector3 SimpleKalman(const Vector3 z);
Vector3 GetVolt();


void main()
{
	cout << "simple kalman" << endl;

	network::cUDPClient udpClient;
	if (!udpClient.Init("127.0.0.1", 8888, 10))
	{
		cout << "udp client accept error!!" << endl;
		return;
	}

		
}


Vector3 GetVolt()
{
	const float w = ((rand() % 2000) - 1000) * 0.001f;
	const float def = 14.f;
	const float v = def + w*4.f;
	return Vector3(v, v, v);
}


Vector3 SimpleKalman(const Vector3 z)
{
	static Matrix44 A, H, Q, R;
	static Matrix44 P;
	static Vector3 x;
	static bool firstRun = false;

	if (!firstRun)
	{
		firstRun = true;

		A.SetIdentity();
		H.SetIdentity();
		Q.SetZeros();
		R.SetScale(Vector3(4, 0, 0));

		x = Vector3(14, 0, 0);
		P.SetScale(Vector3(6, 0, 0));		
	}

	Vector3 xp = x * A;
	Matrix44 Pp = A * P * A.Transpose() + Q;

	Matrix44 K = Pp * H.Transpose() * (H * Pp * H.Transpose() + R).Inverse();

	x = xp + (z - (xp * H)) * K;
	P = Pp - K * H * Pp;
	
	return x;
}
