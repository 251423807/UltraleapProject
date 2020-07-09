#include "stdafx.h"
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include "Timer.h"
#include <math.h>

Timer::Timer(void) {
	QueryPerformanceFrequency(&freq);
	QueryPerformanceCounter(&last);
	QueryPerformanceFrequency(&calibration_freq);
	QueryPerformanceCounter(&calibration_last);
	fps = 0;
}

void Timer::Tick(HWND hwndDlg) {
	LARGE_INTEGER now;
	QueryPerformanceCounter(&now);
	fps++;
	if (now.QuadPart - last.QuadPart > freq.QuadPart) { // update every second
		last = now;
		fps = 0;
	}
}

bool Timer::SamePoint3d(float point3d_last[], float point3d_now[]) {
	bool is_same = false;

	if (fabs(point3d_last[0] - point3d_now[0]) <15&&fabs(float(point3d_last[1] - point3d_now[1])) < 15 &&fabs(float(point3d_last[2] - point3d_now[2])) < 15)
	{
		is_same = true;
	}
	return is_same;
}
int Timer::Calibration(int index, float point3d[]) {
	int return_code = 0;//没检测
	LARGE_INTEGER calibration_now;
	QueryPerformanceCounter(&calibration_now);
	float point3d_now[3];
	if (calibration_now.QuadPart - calibration_last.QuadPart > calibration_freq.QuadPart) { // update every second
		calibration_last = calibration_now;
		//一秒钟
		if (index == 0)
		{
			point3d_last[0] = point3d[0];
			point3d_last[1] = point3d[1];
			point3d_last[2] = point3d[2];
			return_code = 1;//第一个肯定是同一个地点
		}
		else
		{
			point3d_now[0] = point3d[0];
			point3d_now[1] = point3d[1];
			point3d_now[2] = point3d[2];
			if (SamePoint3d(point3d_last, point3d_now))
			{
				return_code = 1;//二个是同一个地点
			}
			else
			{
				return_code = -1;//二个不是同一个地点
			}
		}
	}
	return return_code;
}