// PasswordInteraction.cpp : 定义应用程序的入口点。
//

#include "stdafx.h"
#include "PasswordInteraction.h"
#include <Windows.h>
#include <Math.h>
#include <WindowsX.h>
#include "atlbase.h"
#include "atlstr.h"

#define MAX_LOADSTRING 100





HINSTANCE hInst;								
TCHAR szTitle[MAX_LOADSTRING];					
TCHAR szWindowClass[MAX_LOADSTRING];			


ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);



void OutputDebugPrintf(const char* strOutputString, ...)
{
	char strBuffer[4096] = { 0 };
	va_list vlArgs;
	va_start(vlArgs, strOutputString);
	_vsnprintf_s(strBuffer, sizeof(strBuffer) - 1, strOutputString, vlArgs);
	//vsprintf(strBuffer,strOutputString,vlArgs);
	va_end(vlArgs);
	OutputDebugString(CA2W(strBuffer));
}

using namespace std;

HINSTANCE g_hInst;
HWND hwnd_main;
int shake_tolerance_value = 15;
bool readUI();
bool writeUI();

int initMainDialog(HINSTANCE hInstance);
INT_PTR CALLBACK MainDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
static DWORD WINAPI ThreadProc(LPVOID arg);
/* Threading control */
volatile bool g_running = false;
volatile bool g_stop = true;

HANDLE m_thread = NULL;
HANDLE m_calibration_thread = NULL;


int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPTSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	g_hInst = hInstance;
    
	initMainDialog(hInstance);
    return 0;
}


//////////////////////////////////////////////////////////////////////////////////
static DWORD WINAPI ThreadCalibrationProc(LPVOID arg);

float g_centerX = 0.0;
float g_centerY = 0.0;
float g_centerZ = 0.0;
float g_width = 0.0;
float g_height = 0.0;
int g_screen_width = 0, g_screen_height = 0;
bool isZero(float a);
bool readCalibration();
bool initScreen();
bool ckeckCalibration(HWND hwndDlg);
int initCalibrationForm();
INT_PTR CALLBACK DialogCalibrationProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM);
void setCalibrationCornerColor(HWND hwndDlg, int tag);
void RedoCalibrationLayout(HWND hwndDlg);
//////////////////////////////////////////////////////////////////////////////////
int initMainDialog(HINSTANCE hInstance)
{
	HWND hWnd = CreateDialogW(hInstance, MAKEINTRESOURCE(IDD_MAIN), 0, MainDialogProc);
	if (!hWnd) {
		MessageBoxW(0, L"窗体初始化失败", L"easpeed interaction", MB_ICONEXCLAMATION | MB_OK);
		return 1;
	}
	hwnd_main = hWnd;
	HICON hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_PASSWORDINTERACTION));
	if (NULL == hIcon)
	{
		MessageBoxW(0, L"图标获取失败", L"easpeed interaction", MB_ICONEXCLAMATION | MB_OK);
		return 1;
	}
	::SendMessage(hWnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
	::SendMessage(hWnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);

	//显示窗口
	ShowWindow(hWnd, SW_SHOWNORMAL);

	if (!initScreen())
	{
		MessageBoxW(0, L"屏幕尺寸获取失败", L"easpeed interaction", MB_ICONEXCLAMATION | MB_OK);
		return 1;
	}

	UpdateWindow(hWnd);
	MSG msg;
	// 消息循环  
	for (int sts; (sts = GetMessageW(&msg, NULL, 0, 0));) {
		if (sts == -1) return sts;
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	g_stop = true;
	while (g_running) Sleep(5);

	return (int)msg.wParam;
}
// “MAIN”框的消息处理程序。
INT_PTR CALLBACK MainDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	HWND slider;
	HWND slider_text;
	wchar_t line[12];
	switch (message)
	{
	case WM_INITDIALOG:

		EnableWindow(GetDlgItem(hDlg, ID_MAIN_START), true);
		EnableWindow(GetDlgItem(hDlg, ID_MAIN_STOP), false);
		//音效默认已选择
		CheckDlgButton(hDlg, IDC_CHECK2, BST_CHECKED);
		
		readUI();
		return (INT_PTR)TRUE;
	case WM_HSCROLL:
		
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_CHECK1:
			return TRUE;

		case IDC_CHECK2:
			return TRUE;

		case IDCANCEL:
			g_stop = true;
			if (g_running) {
				PostMessage(hDlg, WM_COMMAND, IDCANCEL, 0);
			}
			else {
				DestroyWindow(hDlg);
				PostQuitMessage(0);
			}
			return TRUE;

		case ID_MAIN_START:

			if (ckeckCalibration(hDlg))
			{
				return FALSE;
			}

			EnableWindow(GetDlgItem(hDlg, ID_MAIN_START), false);
			EnableWindow(GetDlgItem(hDlg, ID_MAIN_STOP), true);

			g_stop = false;
			g_running = true;
			m_thread = CreateThread(0, 0, ThreadProc, hDlg, 0, 0);
			Sleep(0);
			//标定按钮不能使用
			EnableWindow(GetDlgItem(hDlg, IDC_CHECK1), false);

			return TRUE;
			
		case ID_MAIN_STOP:
			//close
			void StopPipeLine();
			StopPipeLine();
			//end 

			g_stop = true;
			if (g_running) {
				PostMessage(hDlg, WM_COMMAND, ID_MAIN_STOP, 0);
			}
			else
			{
				EnableWindow(GetDlgItem(hDlg, ID_MAIN_START), true);
				EnableWindow(GetDlgItem(hDlg, ID_MAIN_STOP), false);
			}
			//标定按钮能使用
			EnableWindow(GetDlgItem(hDlg, IDC_CHECK1), true);
			return TRUE;
	
		}
		break;
	
	case WM_CLOSE:
		//close
		void StopPipeLine();
		StopPipeLine();
		//end 

		writeUI();
		DestroyWindow(hDlg);
		PostQuitMessage(0);
		return (INT_PTR)TRUE;
	}
	return (INT_PTR)FALSE;
}

static DWORD WINAPI ThreadProc(LPVOID arg) {
	void PipeLine(HWND hwndDlg, bool is_calibration);
	PipeLine((HWND)arg,false);
	PostMessage((HWND)arg, WM_COMMAND, ID_MAIN_STOP, 0);
	g_running = false;
	CloseHandle(m_thread);
	return 0;
}



bool initScreen()
{
	g_screen_width = ::GetSystemMetrics(SM_CXSCREEN);
	g_screen_height = ::GetSystemMetrics(SM_CYSCREEN);
	if (g_screen_width != 0 && g_screen_height != 0)
	{
		return true;
	}
	return false;
}
static RECT GetResizeRect(RECT rc, BITMAP bm) { /* Keep the aspect ratio */
	RECT rc1;
	float sx = (float)rc.right / (float)bm.bmWidth;
	float sy = (float)rc.bottom / (float)bm.bmHeight;
	float sxy = sx < sy ? sx : sy;
	rc1.right = (int)(bm.bmWidth*sxy);
	rc1.left = (rc.right - rc1.right) / 2 + rc.left;
	rc1.bottom = (int)(bm.bmHeight*sxy);
	rc1.top = (rc.bottom - rc1.bottom) / 2 + rc.top;
	return rc1;
}


bool readUI()
{
	
	FILE *fpRead = fopen("./ui.txt", "r+");
	if (fpRead == NULL)
	{
		shake_tolerance_value = 15;
		return false;
	}
	fscanf(fpRead, "%d ", &shake_tolerance_value);

	if (shake_tolerance_value >= 0 && shake_tolerance_value <= 100 )
	{
		return true;
	}
	shake_tolerance_value = 20;
	return false;
}

bool writeUI()
{
  
	FILE *fpWrite = fopen("./ui.txt", "w");
	if (fpWrite == NULL)
	{
		return false;
	}
	if (shake_tolerance_value >= 0 && shake_tolerance_value <= 100 )
	{
		fprintf(fpWrite, "%d ", shake_tolerance_value);
		fclose(fpWrite);
		return true;
	}
	fclose(fpWrite);
	return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
	标定界面
*/
////////////////////////////////////////////////////////////////////////////////////////////////////////////

static DWORD WINAPI ThreadCalibrationProc(LPVOID arg) {
	void PipeLine(HWND hwndDlg,bool is_calibration);
	PipeLine((HWND)arg,true);
	CloseHandle(m_calibration_thread);
	return 0;
}

bool ckeckCalibration(HWND hwndDlg)
{
	bool calibration = Button_GetState(GetDlgItem(hwndDlg, IDC_CHECK1))&BST_CHECKED;
	if (calibration)
	{
		initCalibrationForm();
		return true;
	}
	else
	{
		if (!readCalibration())
		{
			MessageBoxW(0, L"The screen has not been calibrated, please select the calibration screen first and then click the Start button.", L"password interaction", MB_ICONEXCLAMATION | MB_OK);
			return true;
		}
	}
	return false;
}

bool readCalibration()
{
	//下面是读数据，将读到的数据存到数组a[10]中，并且打印到控制台上  
	FILE *fpRead = fopen("./calibration.txt", "r+");
	if (fpRead == NULL)
	{
		return false;
	}

	fscanf(fpRead, "%f ", &g_centerX);
	fscanf(fpRead, "%f ", &g_centerY);
	fscanf(fpRead, "%f ", &g_centerZ);
	fscanf(fpRead, "%f ", &g_width);
	fscanf(fpRead, "%f ", &g_height);

	if (!isZero(g_centerX) && !isZero(g_centerY) && !isZero(g_centerZ) && !isZero(g_width) && !isZero(g_height))
	{
		//printf("读取标定中心(%f,%f,%f)\n", g_centerX, g_centerY, g_centerZ);
		return true;
	}
	return false;
}
bool isZero(float a)
{
	if (fabs(a - 0.0) < 1e-6)
		return true;
	else
		return false;
}
//最大化后调整空间位置
void RedoCalibrationLayout(HWND hwndDlg) {
	
	int nWidth = GetSystemMetrics(SM_CXSCREEN);
	int nHeight = GetSystemMetrics(SM_CYSCREEN);

	HWND hLeftTop = GetDlgItem(hwndDlg, IDC_LEFT_TOP);
	HWND hLeftBotton = GetDlgItem(hwndDlg, IDC_LEFT_BOTTOM);
	HWND hRightBotton = GetDlgItem(hwndDlg, IDC_RIGHT_BOTTOM);
	HWND hRightTop = GetDlgItem(hwndDlg, IDC_RIGHT_TOP);
	HWND hExitButton = GetDlgItem(hwndDlg, IDC_CALIBRATION_EXIT);
	//HWND hPanel = GetDlgItem(hwndDlg, IDC_MAIN_PANEL);

	MoveWindow(hLeftTop, 0, 0, 30, 30, TRUE);
	MoveWindow(hLeftBotton, 0, nHeight - 35, 30, 30, TRUE);
	MoveWindow(hRightBotton, nWidth - 35, nHeight - 35, 35, 35, TRUE);
	MoveWindow(hRightTop, nWidth - 35, 0, 30, 30, TRUE);
	MoveWindow(hExitButton, nWidth / 2.0f - 50, nHeight - 130, 120, 35, TRUE);
	//MoveWindow(hPanel, nWidth / 2.0f - nWidth / 8, 0, nWidth / 4, nWidth / 4 / 4 * 3, TRUE);
}

//设置标定时候4个角的颜色
void setCalibrationCornerColor(HWND hwndDlg, int tag) {
	HWND hLeftTop = GetDlgItem(hwndDlg, IDC_LEFT_TOP);
	HWND hLeftBotton = GetDlgItem(hwndDlg, IDC_LEFT_BOTTOM);
	HWND hRightBotton = GetDlgItem(hwndDlg, IDC_RIGHT_BOTTOM);
	HWND hRightTop = GetDlgItem(hwndDlg, IDC_RIGHT_TOP);
	if (tag == 1)
	{
		ShowWindow(hLeftTop, SW_SHOW);
		ShowWindow(hLeftBotton, SW_HIDE);
		ShowWindow(hRightBotton, SW_HIDE);
		ShowWindow(hRightTop, SW_HIDE);
	}
	else if (tag == 2)
	{
		ShowWindow(hLeftTop, SW_HIDE);
		ShowWindow(hLeftBotton, SW_SHOW);
		ShowWindow(hRightBotton, SW_HIDE);
		ShowWindow(hRightTop, SW_HIDE);
	}
	else if (tag == 3)
	{
		ShowWindow(hLeftTop, SW_HIDE);
		ShowWindow(hLeftBotton, SW_HIDE);
		ShowWindow(hRightBotton, SW_SHOW);
		ShowWindow(hRightTop, SW_HIDE);
	}
	else if (tag == 4)
	{
		ShowWindow(hLeftTop, SW_HIDE);
		ShowWindow(hLeftBotton, SW_HIDE);
		ShowWindow(hRightBotton, SW_HIDE);
		ShowWindow(hRightTop, SW_SHOW);
	}
	else
	{
		ShowWindow(hLeftTop, SW_HIDE);
		ShowWindow(hLeftBotton, SW_HIDE);
		ShowWindow(hRightBotton, SW_HIDE);
		ShowWindow(hRightTop, SW_HIDE);
	}
}

INT_PTR CALLBACK DialogCalibrationProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM) {
	switch (message) {
	case WM_INITDIALOG:
		//主窗口的start不能使用
		EnableWindow(GetDlgItem(hwnd_main, ID_MAIN_START), false);
		g_stop = false;
		m_calibration_thread = CreateThread(0, 0, ThreadCalibrationProc, hwndDlg, 0, 0);
		Sleep(0);
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDCANCEL:
			//主窗口的start能使用
			EnableWindow(GetDlgItem(hwnd_main, ID_MAIN_START), true);
			//主窗口的标定设置为未选中
			CheckDlgButton(hwnd_main, IDC_CHECK1, BST_UNCHECKED);
			g_stop = true;
			writeUI();
			DestroyWindow(hwndDlg);
			PostQuitMessage(0);
			return TRUE;

		case IDC_CALIBRATION_EXIT:
			//close
			void StopPipeLine();
			StopPipeLine();
			//end 

			//主窗口的start能使用
			EnableWindow(GetDlgItem(hwnd_main, ID_MAIN_START), true);
			//主窗口的标定设置为未选中
			CheckDlgButton(hwnd_main, IDC_CHECK1, BST_UNCHECKED);
			g_stop = true;
			writeUI();
			DestroyWindow(hwndDlg);
			PostQuitMessage(0);
			return TRUE;
		}
		break;
	case WM_SIZE:
		RedoCalibrationLayout(hwndDlg);
		return TRUE;
	}
	return FALSE;
}


int initCalibrationForm()
{
	HWND hWnd = CreateDialogW(g_hInst, MAKEINTRESOURCE(IDD_CALIBRATION), 0, DialogCalibrationProc);
	if (!hWnd) {
		MessageBoxW(0, L"Failed to create a calibration", L"password interaction", MB_ICONEXCLAMATION | MB_OK);
		return 1;
	}

	HICON hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_PASSWORDINTERACTION));
	if (NULL == hIcon)
	{
		MessageBoxW(0, L"Failed to create icon", L"password interaction", MB_ICONEXCLAMATION | MB_OK);
		return 1;
	}
	::SendMessage(hWnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
	::SendMessage(hWnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);

	//显示窗口
	ShowWindow(hWnd, SW_SHOWNORMAL);

	//最大化
	long Style = ::GetWindowLong(hWnd, GWL_STYLE)& ~WS_CAPTION;
	SetWindowLong(hWnd, GWL_STYLE, Style);
	int x = GetSystemMetrics(SM_CXSCREEN);
	int y = GetSystemMetrics(SM_CYSCREEN);
	UINT flag = 0;
	SetWindowPos(hWnd, NULL, 0, 0, x, y, NULL);
	UpdateWindow(hWnd);
	MSG msg;
	// 消息循环  
	for (int sts; (sts = GetMessageW(&msg, NULL, 0, 0));) {
		if (sts == -1) return sts;
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return 0;
}


