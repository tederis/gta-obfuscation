#pragma once

#include <Windows.h> // HandlerRoutine
#include <iostream>
#include <shlwapi.h>
#include <string>
#pragma comment(lib, "wininet.lib")
#pragma comment(lib, "shlwapi.lib")

using namespace std;

//#define PARTS_NUM 600 // A number of parts for assets decomposing
//#define VARIANT_KEY 2300 // The key for permutation value

#include "BinaryIOHelpers.h"
//#include "RWObfuscator.hpp"


#define IMG_ORIGIN_EDIT    100 // .img
#define IMG_ENCRYPTED_EDIT 101
#define ENCRYPT_BUTTON     102
#define Folder_Button      103
#define DECRYPT_BUTTON     105 // decrypt button

// Interface
HWND MainWindow, OpenFolder, OriginIMGEdit, EncryptedIMGEdit, EncryptButton, EncryptKeyEdit1, EncryptKeyEdit2;
HFONT MainFont, LabelsFont;
DWORD g_Key1, g_Key2;
int lPARTS_NUM, lVARIANT_KEY;
// Interface

bool    select_dir = false;
wchar_t Local_File_Dir;
wchar_t szFileName[MAX_PATH] = { 0 }; // Глобальная переменная для буфера имени файла

void ShowErrorMessage(const wchar_t* message)
{
	MessageBoxW(MainWindow, message, L"Ошибка", MB_ICONERROR);
}

bool IsNumericString(const wchar_t* str)
{
	for (size_t i = 0; str[i] != L'\0'; i++)
	{
		if (!iswdigit(str[i]))
		{
			return false;
		}
	}
	return true;
}

bool CheckKeyValidity(DWORD key, const wchar_t* keyName)
{
	wchar_t keyText1[MAX_PATH];
	wchar_t keyText2[MAX_PATH];
	GetWindowTextW(EncryptKeyEdit1, keyText1, MAX_PATH);
	GetWindowTextW(EncryptKeyEdit2, keyText2, MAX_PATH);
	wstring errorMessage = L"[" + wstring(keyName) + L"]";
	errorMessage += L" it can't be empty!";

	if (key == 0)
	{
		ShowErrorMessage(errorMessage.c_str());
		return false;
	}

	if (!IsNumericString(keyText1) && keyName == L"Key" || !IsNumericString(keyText2) && keyName == L"Variant")
	{
		errorMessage = L"[" + wstring(keyName) + L"]";
		errorMessage += L" contains invalid characters when creating a password!\n Use only numbers!";
		ShowErrorMessage(errorMessage.c_str());
		return false;
	}

	if (key > 0xFFFFFFFF) // Проверка на превышение максимального значения DWORD
	{
		errorMessage = L"[" + wstring(keyName) + L"]";
		errorMessage += L" exceeds the maximum DWORD value!";
		ShowErrorMessage(errorMessage.c_str());
		return false;
	}

	if (wcscmp(keyText1, L"0") == 0 && keyName == L"Key" || wcscmp(keyText2, L"0") == 0 && keyName == L"Variant")
	{
		errorMessage = L"[" + wstring(keyName) + L"]";
		errorMessage += L" contains 0 - this is not allowed when creating a password!";
		ShowErrorMessage(errorMessage.c_str());
		return false;
	}
	return true;
}


void OpenFolderSCRIPT(HWND hWnd)
{
	OPENFILENAME ofn;       // Структура для настроек диалогового окна
	// Задаем настройки диалогового окна
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = NULL;
	ofn.lpstrFilter = L"Other (*.*)\0*.*\0IMG files (*.img)\0*.img\0IDE files (*.ide)\0*.ide\0IPL files (*.ipl)\0*.ipl\0BAT files (*.bat)\0*.bat\0";
	ofn.lpstrFile = szFileName;
	ofn.nMaxFile = MAX_PATH;
	ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
	ofn.lpstrDefExt = L"assembled";
	if (GetOpenFileName(&ofn))
	{
		wchar_t message[MAX_PATH + 50];
		swprintf_s(message, L"Do you want to select this file \"%s\"?", PathFindFileNameW(szFileName));
		if (MessageBox(NULL, message, L"Confirmation", MB_YESNO) == IDYES)
		{
			wchar_t* fileName = PathFindFileNameW(szFileName);
			SetWindowTextW(OriginIMGEdit, szFileName);
			PathRenameExtensionW(fileName, L".assembled");
			SetWindowTextW(OpenFolder, fileName);
			SetWindowTextW(EncryptedIMGEdit, szFileName);
			Local_File_Dir = (wchar_t)szFileName;
			select_dir = true;
		}
		else
		{
			SendMessage(hWnd, WM_COMMAND, Folder_Button, NULL);
		}
	}
}

bool CheckValidArgumentSCRIPT()
{
	DWORD OriginalIMGNameLength = GetWindowTextLengthW(OriginIMGEdit);
	DWORD EncryptedIMGNameLength = GetWindowTextLengthW(EncryptedIMGEdit);
	wchar_t EncryptedPath[MAX_PATH];
	GetWindowTextW(EncryptedIMGEdit, EncryptedPath, MAX_PATH);
	g_Key1 = GetWindowTextLengthW(EncryptKeyEdit1);
	g_Key2 = GetWindowTextLengthW(EncryptKeyEdit2);

	if (select_dir == false)
	{
		MessageBoxW(MainWindow, L"Specify the path to the file!", L"Error", MB_ICONERROR);
		return false;
	}
	if (wcslen(EncryptedPath) == 0)
	{
		MessageBoxW(MainWindow, L"The path cannot be empty!", L"Error", MB_ICONERROR);
		return false;
	}
	else
	{
		if (PathIsRelativeW(EncryptedPath))
		{
			MessageBoxW(MainWindow, L"The path must be absolute!", L"Error", MB_ICONERROR);
			return false;
		}
	}

	bool finalResult;
	finalResult = CheckKeyValidity(g_Key1, L"Key");
	finalResult = CheckKeyValidity(g_Key2, L"Variant");
	
	return finalResult;

}


bool RunBuildFile()
{
	if (CheckValidArgumentSCRIPT())
	{
		wchar_t buffer1[50];
		wchar_t buffer2[50];
		GetWindowTextW(EncryptKeyEdit1, buffer1, sizeof(buffer1));
		GetWindowTextW(EncryptKeyEdit2, buffer2, sizeof(buffer2));

		int keyText1 = 0;
		int keyText2 = 0;

		try {
			keyText1 = std::stoi(buffer1);
			keyText2 = std::stoi(buffer2);
		}
		catch (const std::exception& e) {
			MessageBoxW(NULL, L"Ошибка при преобразовании строки в число", L"Error", MB_ICONERROR);
			return false;
		}
		lPARTS_NUM = keyText1;
		lVARIANT_KEY = keyText2;

		//testValue();
		Beep(300, 74); // если все в порядке
		return true;
	}
	return false;
}

bool runFunction(HWND hWnd,WPARAM wParam)
{
	if (wParam == Folder_Button)
	{
		OpenFolderSCRIPT(hWnd);

	}
	else if (wParam == ENCRYPT_BUTTON)
	{
		return RunBuildFile();
	}
	return false;
}