#include <stdshit.h>
#include <win32hlp.h>
#include "resource.h"
#include <math.h>
#define BUTTON_BORDER 40

const char progName[] = "Image Measure";

HDC scrnShotDC;
HBITMAP scrnShotBM;
POINT pointLeft;
POINT pointRight;
double pointCal = 1.0;
bool pointDrag;
double pointLen;
POINT scrnSize;
int calTarget = IDC_READING;

void SetLayeredMode(HWND hwnd, BOOL enable)
{
	DWORD exStyle = GetWindowLong(hwnd, GWL_EXSTYLE);
	if(enable) { SetWindowLong(hwnd, GWL_EXSTYLE, exStyle | WS_EX_LAYERED);
		SetLayeredWindowAttributes(hwnd, RGB(255,0,255), 255, LWA_COLORKEY);
	} else { SetWindowLong(hwnd, GWL_EXSTYLE, exStyle & ~WS_EX_LAYERED); }
}

void mainDlgInit(HWND hwnd)
{
	SetLayeredMode(hwnd, TRUE);
		

	
}

void mainDlgPaint(HWND hwnd)
{
	PAINTSTRUCT ps;
	HDC hdc = BeginPaint(hwnd, &ps);
	BitBlt(hdc, 0, BUTTON_BORDER, 16383, 
		16383, scrnShotDC, 0, 0, SRCCOPY);
		
	// draw the line
	if(pointLeft.x || pointLeft.y) {
		SelectObject(hdc, GetStockObject(DC_PEN));
		SetDCPenColor(hdc, RGB(255,0,0));
		MoveToEx(hdc, pointLeft.x, pointLeft.y, 0);
		LineTo(hdc, pointRight.x, pointRight.y);
		
		// get the unit vector
		int dx = (pointLeft.x - pointRight.x);
		int dy = (pointLeft.y - pointRight.y);
		dx = lrint((dx * 5) / pointLen);
		dy = lrint((dy * 5) / pointLen);
		
		// draw first line bar
		MoveToEx(hdc, pointLeft.x+dy, pointLeft.y-dx, 0);
		LineTo(hdc, pointLeft.x, pointLeft.y);
		MoveToEx(hdc, pointLeft.x-dy, pointLeft.y+dx, 0);
		LineTo(hdc, pointLeft.x, pointLeft.y);
		
		// draw second line bar
		MoveToEx(hdc, pointRight.x+dy, pointRight.y-dx, 0);
		LineTo(hdc, pointRight.x, pointRight.y);
		MoveToEx(hdc, pointRight.x-dy, pointRight.y+dx, 0);
		LineTo(hdc, pointRight.x, pointRight.y);		
	}
	
	EndPaint(hwnd, &ps);
}

void mainDlgMouse(HWND hwnd, 
	UINT uMsg, LPARAM lParam)
{
	POINT point = {GET_X_LPARAM(lParam),
		GET_Y_LPARAM(lParam) };
	max_ref(point.y, BUTTON_BORDER);
		
	if(uMsg == WM_LBUTTONDOWN) { 
		pointLeft = point; pointDrag = true; }
	ei(uMsg == WM_LBUTTONUP) { pointDrag = false; }
	ei(uMsg != WM_MOUSEMOVE || !pointDrag) return;
	pointRight = point;
	
	InvalidateRect(hwnd, 0, FALSE);
	
	// calculate the vector length
	int dx = (pointLeft.x - pointRight.x);
	int dy = (pointLeft.y - pointRight.y);
	pointLen = sqrt(dx*dx + dy*dy);
	
	// set the reading text
	double len = pointLen * pointCal;
	char buff[32]; sprintf(buff, "%.2f in", len);
	SetDlgItemTextA(hwnd, IDC_READING, buff);
	sprintf(buff, "%.2f cm", len*2.54);
	SetDlgItemTextA(hwnd, IDC_READING2, buff);
}

void mainDlgCalib(HWND hwnd)
{
	char buff[32];
	GetDlgItemTextA(hwnd, calTarget, buff, 32);
	double readVal = atof(buff);
	if(calTarget != IDC_READING) 
		readVal /= 2.54;
	pointCal = readVal / pointLen;
}

void mainDlgShot(HWND hwnd)
{
	RECT rc; GetClientRect(hwnd,&rc);	
	HDC hdc = GetWindowDC(NULL);
	
	// create the bitmap
	scrnShotDC = CreateCompatibleDC(hdc);
	scrnSize = {rc.right, rc.bottom};
	scrnShotBM = CreateCompatibleBitmap(hdc, rc.right, rc.bottom);
	SelectObject(scrnShotDC, scrnShotBM);
	
	// 
	POINT pnt = {0, BUTTON_BORDER}; 
	ClientToScreen(hwnd, &pnt);
	BitBlt(scrnShotDC, 0, 0, rc.right,
		rc.bottom, hdc, pnt.x, pnt.y, SRCCOPY);
	
	ReleaseDC(hwnd, hdc);
	EnableDlgItem(hwnd, IDC_SCRNSHOT, FALSE);
	EnableDlgItem(hwnd, IDC_RESET, TRUE);	
	SetLayeredMode(hwnd, FALSE);
	InvalidateRect(hwnd, 0, TRUE);
}

void mainDlgReset(HWND hwnd)
{
	// reset screenshot state
	if(!scrnShotDC) return;
	DeleteObject(scrnShotDC); scrnShotDC = 0;
	DeleteObject(scrnShotBM); scrnShotBM = 0;
	EnableDlgItem(hwnd, IDC_SCRNSHOT, TRUE);
	EnableDlgItem(hwnd, IDC_RESET, FALSE);
	SetLayeredMode(hwnd, TRUE);
	
	// reset calibration state
	pointLeft = {};
	pointCal = 1.0;
}

void mainDlgErase(HWND hwnd, HDC hdc)
{
	SetDCBrushColor(hdc, RGB(255,0,255));
	RECT rc; GetClientRect(hwnd,&rc);	
	LONG oldBot = rc.bottom; rc.bottom = BUTTON_BORDER;
	FillRect(hdc, &rc, GetSysColorBrush(COLOR_3DFACE)); 
	rc.bottom = oldBot; rc.top = BUTTON_BORDER;
	FillRect(hdc, &rc, (HBRUSH)GetStockObject(DC_BRUSH)); 
}

void mainDlgSize(HWND hwnd)
{
	RECT rc; GetClientRect(hwnd, &rc);
	if((rc.right > scrnSize.x)
	||(rc.bottom > scrnSize.y))
		mainDlgReset(hwnd);
}

INT_PTR CALLBACK mainDlgProc(
	HWND hwnd, UINT uMsg,
	WPARAM wParam, LPARAM lParam)
{
	DLGMSG_SWITCH(
	  ON_MESSAGE(WM_SIZE, mainDlgSize(hwnd));
		ON_MESSAGE(WM_CLOSE, EndDialog(hwnd, 0))
		ON_MESSAGE(WM_INITDIALOG, mainDlgInit(hwnd))
		ON_MESSAGE(WM_PAINT, mainDlgPaint(hwnd))
		
		
		OM_MOUSEMSG(mainDlgMouse(hwnd, uMsg, lParam))
		
		
	CASE_COMMAND(
		ON_CONTROL(EN_SETFOCUS, IDC_READING, calTarget = IDC_READING)
		ON_CONTROL(EN_SETFOCUS, IDC_READING2, calTarget = IDC_READING2)
	
		ON_COMMAND(IDC_RESET, mainDlgReset(hwnd))
	  ON_COMMAND(IDC_SCRNSHOT, mainDlgShot(hwnd))
		ON_COMMAND(IDC_CALIBRATE, mainDlgCalib(hwnd))
		
	,)
	
	ON_MESSAGE(WM_ERASEBKGND, mainDlgErase(
		hwnd, (HDC)wParam); return TRUE;)
	,)
	
	return FALSE;
}

int main()
{
	DialogBox(NULL, MAKEINTRESOURCE(
		IDD_DIALOG1), NULL, mainDlgProc);
}