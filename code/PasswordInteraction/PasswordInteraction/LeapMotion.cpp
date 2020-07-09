#include "stdafx.h"
#include "MyListener.h"

#include <string>
#include <iostream>
#include <vector>
#include <array>
#include <Windows.h>
#include <WindowsX.h>
#include <atlstr.h>
#include "PasswordInteraction.h"
#include "Timer.h"
using namespace std;
HWND g_hwndDlg;
bool g_is_calibration,g_is_run;
#include <mmsystem.h>//导入声音头文件
#pragma comment(lib,"WinMM.Lib")//导入声音头文件库


void UpdatePanel(HWND);
void OutputDebugPrintf(const char* strOutputString, ...);
void setCalibrationCornerColor(HWND hwndDlg, int tag);
void StartCalibration(HWND hwndDlg, Timer& timer, float point3d[]);
void ScreenTouch(HWND hwndDlg, float point3d[]);
void SetInfoBox(HWND hwndDlg, LPARAM text);
Timer timer;
float current_mouse_x = 0;
float current_mouse_y = 0;
extern int shake_tolerance_value,g_screen_width, g_screen_height;
void MouseMove(float x, float y);
void MouseLeftDown();
void MouseLeftUp();
void MouseRightDown();
void MouseRightUp();
void KeyDown(int key_number, int scan_number);
void KeyUp(int key_number, int scan_number);
extern float g_centerX;
extern float g_centerY;
extern float g_centerZ;
extern float g_width;
extern float g_height;




// Create a sample listener and controller
  MyListener listener;
  Controller controller;
void PipeLine(HWND hwndDlg, bool is_calibration)
{
	g_is_run = false;
	
    g_hwndDlg = hwndDlg;
	g_is_calibration = is_calibration;
    if (g_is_calibration)
    {
        setCalibrationCornerColor(g_hwndDlg, 1);
    }

	g_is_run = true;
  // Have the sample listener receive events from the controller
  controller.addListener(listener);

  //if (argc > 1 && strcmp(argv[1], "--bg") == 0)
  //  controller.setPolicy(Leap::Controller::POLICY_BACKGROUND_FRAMES);

  controller.setPolicy(Leap::Controller::POLICY_ALLOW_PAUSE_RESUME);
  controller.setPaused(false);
  // Keep this process running until Enter is pressed
  //std::cout << "Press Enter to quit, or enter 'p' to pause or unpause the service..." << std::endl;
   bool paused = false;
   while (g_is_run) {
   
  }
  controller.setPaused(true);
  // Remove the sample listener when done
  controller.removeListener(listener);

}

void StopPipeLine()
{
	g_is_run = false;
	controller.setPaused(true);
	controller.removeListener(listener);
}


void Capture(float x,float y ,float z)
{
	OutputDebugPrintf("x=%f    y=%f    z=%f    g_is_calibration=%d\n",x,y,z,g_is_calibration);

	float point3d[3] = { x , y , z };
    if (g_is_calibration)
    {
        StartCalibration(g_hwndDlg, timer, point3d);
    }
    else
    {
        ScreenTouch(g_hwndDlg, point3d);
    }
}
////////////////////////////////////////////////////////////////////////////////////////////

void setCalibrationCornerColor(HWND hwndDlg, int tag);
void playSound(HWND hwndDlg, LPCWSTR sound);
void StartCalibration(HWND hwndDlg, Timer& timer, float point3d[]);
int calibration_index = -1;
float g_left_top[3] = { 0,0,0 };
float g_left_bottom[3] = { 0,0,0 };
float g_right_bottom[3] = { 0,0,0 };
float g_right_top[3] = { 0,0,0 };
float point3d[3] = { 0,0,0 };
bool writeCalibration();
void StartCalibration(HWND hwndDlg, Timer& timer, float point3d[])
{
    if (calibration_index == -1)
    {
        calibration_index++;
        if (g_left_top[0] == 0 && g_left_top[1] == 0 && g_left_top[2] == 0)
        {
            setCalibrationCornerColor(hwndDlg, 1);
        }
        else if (g_left_bottom[0] == 0 && g_left_bottom[1] == 0 && g_left_bottom[2] == 0)
        {
            setCalibrationCornerColor(hwndDlg, 2);
        }
        else if (g_right_bottom[0] == 0 && g_right_bottom[1] == 0 && g_right_bottom[2] == 0)
        {
            setCalibrationCornerColor(hwndDlg, 3);
        }
        else if (g_right_top[0] == 0 && g_right_top[1] == 0 && g_right_top[2] == 0)
        {
            setCalibrationCornerColor(hwndDlg, 4);
        }
        else
        {
            setCalibrationCornerColor(hwndDlg, 5);
            g_centerX = (g_left_top[0] + g_left_bottom[0] + g_right_bottom[0] + g_right_top[0]) / 4.0;
            g_centerY = (g_left_top[1] + g_left_bottom[1] + g_right_bottom[1] + g_right_top[1]) / 4.0;
            g_centerZ = (g_left_top[2] + g_left_bottom[2] + g_right_bottom[2] + g_right_top[2]) / 4.0;
            g_width = fabs((g_left_top[0] + g_left_bottom[0]) / 2.0 - (g_right_bottom[0] + g_right_top[0]) / 2.0);
            g_height = fabs((g_left_top[1] + g_right_top[1]) / 2.0 - (g_left_bottom[1] + g_right_bottom[1]) / 2.0);
            writeCalibration();
        }
    }
    //OutputDebugPrintf("{x=%f,y=%f},value=%f\n", point3d[0], point3d[1], point3d[2]);
    int tag = timer.Calibration(calibration_index, point3d);
    if (tag == 1)
    {
        calibration_index++;
        if (calibration_index >= 3)
        {
            if (g_left_top[0] == 0 && g_left_top[1] == 0 && g_left_top[2] == 0)
            {
                PlaySound(L"./calibration_successful.wav", NULL, SND_FILENAME | SND_ASYNC);
                g_left_top[0] = point3d[0];
                g_left_top[1] = point3d[1];
                g_left_top[2] = point3d[2];
                calibration_index = -1;
            }
            else if (g_left_bottom[0] == 0 && g_left_bottom[1] == 0 && g_left_bottom[2] == 0)
            {
                PlaySound(L"./calibration_successful.wav", NULL, SND_FILENAME | SND_ASYNC);
                g_left_bottom[0] = point3d[0];
                g_left_bottom[1] = point3d[1];
                g_left_bottom[2] = point3d[2];
                calibration_index = -1;
            }
            else if (g_right_bottom[0] == 0 && g_right_bottom[1] == 0 && g_right_bottom[2] == 0)
            {
                PlaySound(L"./calibration_successful.wav", NULL, SND_FILENAME | SND_ASYNC);
                g_right_bottom[0] = point3d[0];
                g_right_bottom[1] = point3d[1];
                g_right_bottom[2] = point3d[2];
                calibration_index = -1;
            }
            else if (g_right_top[0] == 0 && g_right_top[1] == 0 && g_right_top[2] == 0)
            {
                PlaySound(L"./calibration_successful.wav", NULL, SND_FILENAME | SND_ASYNC);
                g_right_top[0] = point3d[0];
                g_right_top[1] = point3d[1];
                g_right_top[2] = point3d[2];
                calibration_index = -1;
            }
        }
    }
    else if (tag == -1)
    {
        //返回-1表示不是同一个点，重新开始。返回是0那就继续等待
        calibration_index = 0;
    }
}
bool writeCalibration()
{
    //下面是写数据，将标定的center写入到calibration.txt文件中  
    FILE* fpWrite = fopen("./calibration.txt", "w");
    if (fpWrite == NULL)
    {
        return false;
    }
    if (g_centerX != 0.0 && g_centerY != 0.0 && g_centerZ != 0.0 && g_width != 0.0 && g_height != 0.0)
    {
        fprintf(fpWrite, "%f ", g_centerX);
        fprintf(fpWrite, "%f ", g_centerY);
        fprintf(fpWrite, "%f ", g_centerZ);
        fprintf(fpWrite, "%f ", g_width);
        fprintf(fpWrite, "%f ", g_height);
        fclose(fpWrite);
        return true;
    }
    fclose(fpWrite);

    return false;
}
void playSound(HWND hwndDlg, LPCWSTR sound)
{
    if (Button_GetState(GetDlgItem(hwndDlg, IDC_CHECK2)) & BST_CHECKED)
    {
        PlaySound(sound, NULL, SND_FILENAME | SND_ASYNC);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////


int screen_touch_index = -1;
int screen_touch = -1;
void KeyDown(int key_number, int scan_number)
{
    keybd_event(key_number, scan_number, 0, 0);
}

void KeyUp(int key_number, int scan_number)
{
    keybd_event(key_number, scan_number, KEYEVENTF_KEYUP, 0);
}

void moveCursorPos(float x, float y)
{
    //防抖动
    if (fabs(current_mouse_x - x) > shake_tolerance_value || fabs(current_mouse_y - y) > shake_tolerance_value)
    {
        SetCursorPos(x, y);
        current_mouse_x = x;
        current_mouse_y = y;
    }
}

void MouseMove(float x, float y)
{
   
    if (fabs(current_mouse_x - x) > shake_tolerance_value || fabs(current_mouse_y - y) > shake_tolerance_value)
    {
        INPUT input = { 0 };
        input.type = INPUT_MOUSE;
        input.mi.dwFlags = MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE;
        input.mi.dx = x * (65535.0f / g_screen_width);
        input.mi.dy = y * (65535.0f / g_screen_height);
        SendInput(1, &input, sizeof(INPUT));

        current_mouse_x = x;
        current_mouse_y = y;
    }
}

void MouseLeftDown()
{
    INPUT input = { 0 };
    input.type = INPUT_MOUSE;
    input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
    SendInput(1, &input, sizeof(INPUT));

}

void MouseLeftUp()
{
    INPUT input = { 0 };
    input.type = INPUT_MOUSE;
    input.mi.dwFlags = MOUSEEVENTF_LEFTUP;
    SendInput(1, &input, sizeof(INPUT));
}

void MouseRightDown()
{
    INPUT input = { 0 };
    input.type = INPUT_MOUSE;
    input.mi.dwFlags = MOUSEEVENTF_RIGHTDOWN;
    SendInput(1, &input, sizeof(INPUT));
}
void MouseRightUp()
{
    INPUT input = { 0 };
    input.type = INPUT_MOUSE;
    input.mi.dwFlags = MOUSEEVENTF_RIGHTUP;
    SendInput(1, &input, sizeof(INPUT));
}

void ScreenTouch(HWND hwndDlg, float point3d[])
{
    //TODO
    float  minZPoint3d[3];
    minZPoint3d[2] = 20000;
    if (point3d[2] < minZPoint3d[2])
    {
        minZPoint3d[0] = point3d[0];
        minZPoint3d[1] = point3d[1];
        minZPoint3d[2] = point3d[2];
    }
    if (minZPoint3d[2] > 19999)
    {
        return;
    }
    float curentPoint3d[3] = { minZPoint3d[0],minZPoint3d[1],minZPoint3d[2] };

   
    if (screen_touch_index != -1)
    {
        screen_touch_index++;
        if (screen_touch_index >= 60)
        {
            screen_touch_index = -1;
        }
    }
    if (curentPoint3d[2] <= g_centerZ)
    {
        float dx = curentPoint3d[0] - g_centerX;
        float dy = g_centerY - curentPoint3d[1];
        float ddx = dx / (g_width / 2.0f);
        float ddy = dy / (g_height / 2.0f);
        //MouseMove(g_screen_width / 2.0f - ddx * (g_screen_width / 2.0f), g_screen_height / 2.0f + ddy * (g_screen_height / 2.0f));
        moveCursorPos(g_screen_width / 2.0f + ddx * (g_screen_width / 2.0f), g_screen_height / 2.0f + ddy * (g_screen_height / 2.0f));
        
        if (screen_touch == -1)
        {
            screen_touch = 1;
            MouseLeftDown();
            if (screen_touch_index == -1)
            {
                playSound(hwndDlg, L"./screen_touch.wav");
                screen_touch_index = 0;
            }
        }
    }
    else
    {
        float dx = curentPoint3d[0] - g_centerX;
        float dy = curentPoint3d[1] - g_centerY;
        float ddx = dx / (g_width / 2.0f);
        float ddy = dy / (g_height / 2.0f);
        //MouseMove(g_screen_width / 2.0f - ddx * (g_screen_width / 2.0f), g_screen_height / 2.0f + ddy * (g_screen_height / 2.0f));
		moveCursorPos(g_screen_width / 2.0f + ddx * (g_screen_width / 2.0f), g_screen_height / 2.0f - ddy * (g_screen_height / 2.0f));
       
        if (screen_touch == 1)
        {
            screen_touch = -1;
            MouseLeftUp();
        }
    }
}


/////////////////////////////////////////////////////////////////////////////////////////////