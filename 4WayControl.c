#include <Windows.h>
#include <CommCtrl.h>
#include <tchar.h>
#include "resource.h"

/* ##############################################################
relationship between 'control layout' and 'contron id'
channel  1 :  1 --  39
         tab       +   1 -  6
		 edit      +  11 - 16
		 spin      +  21 - 26 
         button    +  31 - 36
channel  2 : 41 --  79
channel  3 : 81 --  119
channel  4 : 121 -- 159

for example : id = 96 --> 9/3 = 3    3+1= channel 4
                          90%30 = 0  tab control
                          96%30 = 6  CSO
############################################################## */

#pragma comment(lib, "ComCtl32.lib")

BOOL CALLBACK DlgProc (HWND, UINT, WPARAM, LPARAM);

void InitControls(HWND);
void SetSliderValue(HWND, int);
void SetEditValue(HWND, int);
void SetSpinValue(HWND, int);
void SetValue(UINT, int);
int IsDigipotControl(int);
int GetSliderValue(HWND);
int GetSpinValue(HWND);

void onDigipotSliderMove(HWND, int, int);
void onDigipotSliderStop(HWND, int, int);
void onDigipotSpinStop(HWND, int);
void onDigipotSpinMove(HWND, int);
void onDigipotEditLoseFocus(HWND, int);
void onDigipotEditCRPress(HWND, int);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR szCmdLine, int iCmdShow)
{
	HWND hDlg;
    MSG msg;
	HACCEL hAccel;
    BOOL ret;

    InitCommonControls();
    hDlg = CreateDialogParam(hInstance, MAKEINTRESOURCE(IDD_DIALOG_CE4FT), 0, DlgProc, 0);
    ShowWindow(hDlg, iCmdShow);

    while (GetMessage (&msg, NULL, 0, 0))  {
	    if (hDlg == NULL || !IsDialogMessage (hDlg, &msg))  {
		   if (!TranslateAccelerator (hDlg, hAccel, &msg))   {
              TranslateMessage (&msg) ;
              DispatchMessage (&msg) ;
		   }
		}
	}
  return msg.wParam;
}


BOOL CALLBACK DlgProc  (HWND hDlg, UINT uMsg,
                        WPARAM wParam, LPARAM lParam)
{
	HWND hwnd;
	int id;
	int value;
	
	switch (uMsg) {
    case WM_INITDIALOG:
		 InitControls(hDlg);

		 return TRUE;
    
    case WM_VSCROLL:
		 id = GetDlgCtrlID((HWND)lParam);

		 //which control of digipot trigger the event
		 switch (IsDigipotControl(id)) {
		 //Slider 
		 case 0:
		      switch (LOWORD (wParam))
			  {
			  case TB_LINEDOWN:
			  case TB_LINEUP:
			  case TB_PAGEDOWN:
			  case TB_PAGEUP:
				   value = SendMessage((HWND)lParam, TBM_GETPOS, 0, 0);
				   onDigipotSliderMove(hDlg, id, value);
				   break;

		      case TB_THUMBTRACK:
				   onDigipotSliderMove(hDlg, id, HIWORD(wParam));
			       break;

       	      case TB_ENDTRACK:
				   value = SendMessage((HWND)lParam, TBM_GETPOS, 0, 0);
				   onDigipotSliderStop(hDlg, id, value);
				   break;

			  case TB_THUMBPOSITION: 
				   onDigipotSliderStop(hDlg, id, HIWORD(wParam));
				   break;
			  } 
			  break;
         //edit
		 case 1:
	          break;

         //spin
         case 2:
			  
		      switch (LOWORD (wParam))
			  {
			  case SB_THUMBPOSITION:    
				   onDigipotSpinMove(hDlg, id);
			       break;	
			  
			  case SB_ENDSCROLL: 
				   onDigipotSpinStop(hDlg, id);
				   break;
			  }              
			  
			  break;

         //button
		 case 3:
              break;

		 default:
              break;
		 }


		 return TRUE;

    case WM_KEYDOWN:
		 return TRUE;



    case WM_COMMAND:
		//detect CR press on edit
		if (wParam  ==  IDOK) {
			hwnd = GetFocus();
		    id = GetDlgCtrlID(hwnd);
		    //Digipot edit control
			if (IsDigipotControl(id) == 1) {
				onDigipotEditCRPress(hDlg, id);
				return TRUE;
			}
		}
        //edit control lose focus
		if (HIWORD(wParam) == EN_KILLFOCUS) {
		   id = LOWORD(wParam);
		   if (IsDigipotControl(id) == 1) {
			  onDigipotEditLoseFocus(hDlg, id);
			  return TRUE;
		   }
		}
		return FALSE;

    case WM_CLOSE:
         DestroyWindow(hDlg);
		 return TRUE;

	case WM_DESTROY:
         PostQuitMessage(0);
         return TRUE;
    }
    
	return FALSE;
	
}


//initialize slider,edit,spin control 
void InitControls(HWND hDlg)
{
	UINT m, n;       // id= m * 10 + n
    HWND hwnd;

	for (m = 0; m < 16; m++) {
		switch (m % 4) {
//slider	  1-6, 41-46,   81-86,   121-126  
		case 0:
			 for (n = 1; n < 7; n++) {
				hwnd = GetDlgItem(hDlg, m * 10 + n);
				SendMessage(hwnd, TBM_SETRANGE, (WPARAM)TRUE, (LPARAM)MAKELONG(0, 1023));
				SetSliderValue(hwnd, 0);
			 }
			 break;
//edit      11-16, 51-56,   91-96,   131-136
		case 1:
			 for (n = 1; n < 7; n++) {
				hwnd = GetDlgItem(hDlg, m * 10 + n);
				SetEditValue(hwnd, 0);
			 }
			 break;

//spin      21-26, 61-66, 101-106,   141-146
		case 2:
			 for (n = 1; n < 7; n++) {
				hwnd = GetDlgItem(hDlg, m * 10 + n);
				SendMessage(hwnd, UDM_SETRANGE, (WPARAM)TRUE, MAKELPARAM(1023, 0));
			 }	
			 break;

		}
	}
	

}

//set tab contron value
void SetSliderValue(HWND hwnd, int iVal)
{
	SendMessage(hwnd, TBM_SETPOS, (WPARAM)TRUE, (LPARAM)(1023-iVal));
}

//get slider control value 
int GetSliderValue(HWND hwnd)
{
	return(1023 - SendMessage(hwnd, TBM_GETPOS, 0, 0));
	
}

void SetEditValue(HWND hwnd, int iVal)
{
	TCHAR szBuffer[10];

	wsprintf(szBuffer, TEXT("%i"), iVal);
	SetWindowText(hwnd, szBuffer); 
}


void SetSpinValue(HWND hwnd, int iVal)
{
	SendMessage(hwnd, UDM_SETPOS, 0, (LPARAM)iVal);
}

int GetSpinValue(HWND hwnd)
{
	return(SendMessage(hwnd, UDM_GETPOS, 0, 0));
}



void SetValue(int FLAG, int iVal)
{
	PostQuitMessage (0) ;
}


//evaluate whether and control(identifier) is a control 
//for digipot or not
//ouput:   0--Slider 1--Edit 2--spin 3--button -1--no
int IsDigipotControl(int nIDDlgItem)
{
    int val;
	val = nIDDlgItem / 10;
	if (val >= 0 && val < 16)
		return (val % 4);
    else
        return  -1;  
}

//when move slider
void onDigipotSliderMove(HWND hDlg, int id, int value)
{
	HWND hwnd;
	//set the edit control text value ,identifier is 
	//slider id + 10
	hwnd = GetDlgItem(hDlg, id + 10);
	value = 1023 - value;
    SetEditValue(hwnd, value);
}

//when release slider button
void onDigipotSliderStop(HWND hDlg, int id, int value)
{
	HWND hwnd;
	hwnd = GetDlgItem(hDlg, IDC_EDIT2);
	value = 1023 - value;
	SetEditValue(hwnd, value);
}


//when click spin control
void onDigipotSpinMove(HWND hDlg, int id)
{
	HWND hwnd;
	int value;
	hwnd = GetDlgItem(hDlg, id);
	value = SendMessage(hwnd, UDM_GETPOS, 0, 0);
	//id-20 is slider
    hwnd = GetDlgItem(hDlg, id - 20);
	SetSliderValue(hwnd, value);
	

}

//when release spin control
void onDigipotSpinStop(HWND hDlg, int id)
{
	HWND hwnd;
	int value;
	hwnd = GetDlgItem(hDlg, id);
	value = SendMessage(hwnd, UDM_GETPOS, 0, 0);
	hwnd = GetDlgItem(hDlg, IDC_EDIT2);
	SetEditValue(hwnd, value);
}


//when CR press on edit control
void onDigipotEditCRPress(HWND hDlg, int id)
{
	HWND hwnd;
	int value;
	TCHAR szBuffer[4];

	hwnd = GetDlgItem(hDlg, id);

	//get edit value and sychronize slider 
	GetWindowText(hwnd, szBuffer, sizeof(szBuffer));
    value = atoi(szBuffer);
	//if input is not a int, get value from spin
	if (!value) {
	   hwnd = GetDlgItem(hDlg, id + 10);
	   value = GetSpinValue(hwnd);
	}
	else if (value > 1023)
		value = 1023;
	hwnd = GetDlgItem(hDlg, id);
	SetEditValue(hwnd, value);

	//set slider control
	hwnd = GetDlgItem(hDlg, id - 10);
    SetSliderValue(hwnd, value);}


//when edit control lose focus
void onDigipotEditLoseFocus(HWND hDlg, int id)
{
	HWND hwnd;
	int value;
    TCHAR szBuffer[4];

	hwnd = GetDlgItem(hDlg, id);

	//get edit value and sychronize slider 
	GetWindowText(hwnd, szBuffer, sizeof(szBuffer));
    value = atoi(szBuffer);
	//if input is not a int, get value from spin
	if (!value) {
	   hwnd = GetDlgItem(hDlg, id + 10);
	   value = GetSpinValue(hwnd);
	}
	else if (value > 1023)
		value = 1023;
	hwnd = GetDlgItem(hDlg, id);
	SetEditValue(hwnd, value);

	//set slider control
	hwnd = GetDlgItem(hDlg, id - 10);
    SetSliderValue(hwnd, value);
}


