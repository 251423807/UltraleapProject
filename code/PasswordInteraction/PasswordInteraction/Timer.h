#pragma once
#include <windows.h>

class Timer {
public:

	Timer(void);
	void Tick(HWND hwndDlg);

	int Calibration(int index, float point3d[]);
	bool SamePoint3d(float point3d_last[], float point3d_now[]);
protected:

	LARGE_INTEGER freq, last;
	int fps;

	LARGE_INTEGER calibration_freq, calibration_last;
	float point3d_last[3];
};

