#include "Header.h"
#include "RWObfuscator.hpp"




LRESULT WINAPI WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	// Interface part
	LPCWSTR WndClassName = L"MainWindow";

	WNDCLASSEX WndClass;
	WndClass.cbSize = sizeof(WNDCLASSEX);
	WndClass.style = CS_HREDRAW;
	WndClass.cbWndExtra = 0;
	WndClass.cbClsExtra = 0;
	WndClass.hInstance = hInstance;
	WndClass.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APPLICATION));
	WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	WndClass.hbrBackground = CreateSolidBrush(RGB(245, 245, 245));
	WndClass.lpszMenuName = NULL;
	WndClass.hIconSm = LoadIcon(WndClass.hInstance, MAKEINTRESOURCE(IDI_APPLICATION));
	WndClass.lpfnWndProc = WndProc;
	WndClass.lpszClassName = WndClassName;
	RegisterClassEx(&WndClass);

	MainFont = CreateFontW(17, 7, 0, 0, 400, 0, 0, 0, DEFAULT_CHARSET, OUT_TT_ONLY_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH, L"Helvetica");
	LabelsFont = CreateFontW(16, 6, 0, 0, 400, 0, 0, 0, DEFAULT_CHARSET, OUT_TT_ONLY_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH, L"Helvetica");

	MainWindow = CreateWindowExW(WS_EX_ACCEPTFILES, WndClassName, L"GUI-Obfuscator", WS_OVERLAPPED | WS_SYSMENU | WS_MINIMIZEBOX, CW_USEDEFAULT, CW_USEDEFAULT, 725, 245, NULL, NULL, hInstance, NULL);

	OriginIMGEdit = CreateWindowExW(WS_EX_CLIENTEDGE, L"edit", L"", WS_CHILD | WS_VISIBLE | ES_LEFT | ES_AUTOHSCROLL | ES_READONLY, 150, 40, 525, 26, MainWindow, (HMENU)IMG_ORIGIN_EDIT, hInstance, NULL);
	OpenFolder = CreateWindowExW(WS_EX_CLIENTEDGE, L"button", L"Open file ", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON, 10, 39, 125, 30, MainWindow, (HMENU)Folder_Button, hInstance, NULL);

	EncryptedIMGEdit = CreateWindowExW(WS_EX_CLIENTEDGE, L"edit", L"", WS_CHILD | WS_VISIBLE | ES_LEFT | ES_AUTOHSCROLL, 10, 100, 665, 26, MainWindow, (HMENU)IMG_ENCRYPTED_EDIT, hInstance, NULL);
	
	EncryptKeyEdit1 = CreateWindowExW(WS_EX_CLIENTEDGE, L"edit", L"6", WS_CHILD | WS_VISIBLE | ES_LEFT | ES_AUTOHSCROLL, 10, 155, 100, 26, MainWindow, (HMENU)IMG_ENCRYPTED_EDIT, hInstance, NULL);
	EncryptKeyEdit2 = CreateWindowExW(WS_EX_CLIENTEDGE, L"edit", L"23", WS_CHILD | WS_VISIBLE | ES_LEFT | ES_AUTOHSCROLL, 135, 155, 100, 26, MainWindow, (HMENU)IMG_ENCRYPTED_EDIT, hInstance, NULL);
	
	EncryptButton = CreateWindowExW(WS_EX_CLIENTEDGE, L"button", L"Run Compile", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON, 500, 155, 200, 30, MainWindow, (HMENU)ENCRYPT_BUTTON, hInstance, NULL);
	
	const COLORREF backgroundColor = RGB(200, 200, 200);
	SetClassLongPtr(MainWindow, GCLP_HBRBACKGROUND, reinterpret_cast<LONG_PTR>(CreateSolidBrush(backgroundColor)));

	ShowWindow(MainWindow, nShowCmd);
	UpdateWindow(MainWindow);
	MSG Message;
	while (GetMessage(&Message, NULL, 0, 0))
	{
		TranslateMessage(&Message);
		DispatchMessage(&Message);
	}
	// Interface part

	return 0;
}


LRESULT WINAPI WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    HDC Writer;
    PAINTSTRUCT Pstr;

    switch (message)
    {
    case WM_DESTROY:
        PostQuitMessage(0); // Завершаем обработку сообщений
        break;
    case WM_CLOSE:
        DestroyWindow(hWnd); // Закрываем окно, что приведет к WM_DESTROY
        break;
    case WM_DROPFILES:
    {
        DWORD Size = DragQueryFileW((HDROP)wParam, 0, NULL, 0);
        LPWSTR DroppedFileName = (LPWSTR)malloc(Size * 2 + 2);
        DragQueryFileW((HDROP)wParam, 0, DroppedFileName, Size + 1);
        SetWindowTextW(OriginIMGEdit, DroppedFileName);
        free(DroppedFileName);

        return 0;
    }
    break;
    case WM_PAINT:
        SendMessageW(OriginIMGEdit, WM_SETFONT, (WPARAM)MainFont, TRUE);
        SendMessageW(EncryptedIMGEdit, WM_SETFONT, (WPARAM)MainFont, TRUE);
        SendMessageW(EncryptButton, WM_SETFONT, (WPARAM)MainFont, TRUE);
        SendMessageW(EncryptButton, WM_SETFONT, (WPARAM)MainFont, TRUE);
        SendMessageW(EncryptKeyEdit1, WM_SETFONT, (WPARAM)MainFont, TRUE);
        SendMessageW(EncryptKeyEdit2, WM_SETFONT, (WPARAM)MainFont, TRUE);
        SendMessageW(OpenFolder, WM_SETFONT, (WPARAM)MainFont, TRUE);

        Writer = BeginPaint(hWnd, &Pstr);
        SelectObject(Writer, LabelsFont);
        SetBkMode(Writer, TRANSPARENT);
        TextOutW(Writer, 15, 15, L"Source File", 12);
        TextOutW(Writer, 10, 80, L"Save path", 15);
        TextOutW(Writer, 10, 135, L"        Key                      Variant", 41);
        EndPaint(hWnd, &Pstr);

        return 0;
        break;
    case WM_COMMAND:
        if (runFunction(hWnd, wParam))
        {
             startMission(lPARTS_NUM, lVARIANT_KEY);
             return 0;
        }

        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }

    return 0;
}

