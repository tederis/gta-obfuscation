/*******************************************************
* Copyright (C) 2016 TEDERIs <xcplay@gmail.com>
*
* This file is part of RWDeobfuscator.
*
* RWObfuscator can not be copied and/or distributed without the express
* permission of TEDERIs.
*******************************************************/

#include <windows.h>
#include <math.h>
#include <stdlib.h> 
#include "helper.h"

void HookInit();

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		HookInit();
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

#define PARTS_NUM 6 // Количество частей, на которое разобран файл
#define VARIANT_KEY 23 // Ключ для дешифровки варианта перестановки в теле файла
#define TEMP_BUFFER_SIZE 8388608 // Макимальный размер зашифрованного файла
#define XR_SWAP(x,y) temp = *x; *x = *y; *y = temp
unsigned *permLookup;
char* tempBuffer;

long double fact(int N)
{
	if (N < 0)
		return 0;
	if (N == 0)
		return 1;
	else
		return N * fact(N - 1);
}

void permute(unsigned *a, int l, int r)
{
	int i;
	if (l == r)
	{
		memcpy(permLookup, a, sizeof(unsigned)*PARTS_NUM);
		permLookup += PARTS_NUM;
	}
	else
	{
		char temp;
		for (i = l; i <= r; i++)
		{
			XR_SWAP((a + l), (a + i));
			permute(a, l + 1, r);
			XR_SWAP((a + l), (a + i));
		}
	}
}

#define HOOKPOS_CStreaming_ConvertBufferToObject			0x40C6B0
#define HOOKSIZE_CStreaming_ConvertBufferToObject			7

char* xrFilePointer;
DWORD xrIndex = 0;
void ConvertBufferToObjectHandler()
{
	DWORD oldProt;
	VirtualProtect((LPVOID)xrFilePointer, 0x1000, PAGE_EXECUTE_READWRITE, &oldProt);

	// Проверим, зашифрован файл или нет
	if (*((unsigned*)xrFilePointer) == 0x18D5E82)
	{
		// Читаем размер и номер перестановки
		xrFilePointer += 4;
		unsigned fileSize = *((unsigned*)xrFilePointer);
		xrFilePointer += 4;
		unsigned variant = *((unsigned*)xrFilePointer);
		variant /= VARIANT_KEY; // Дешифруем номер перестановки
		xrFilePointer += 4;

		unsigned partSize = floor(fileSize / PARTS_NUM);
		unsigned totalPartsSize = partSize * PARTS_NUM;
		unsigned tailSize = fileSize - totalPartsSize;

		// Копируем во временный буфер
		memcpy(tempBuffer, xrFilePointer, fileSize);

		// Строим исходный файл
		unsigned* permPtr = permLookup + variant*PARTS_NUM;
		for (unsigned i = 0; i < PARTS_NUM; i++)
		{
			// Переходим к чанку в исходном файле
			unsigned chunkIndex = permPtr[i];
			memcpy(xrFilePointer + chunkIndex*partSize, tempBuffer + i*partSize, partSize);
		}
	}
	VirtualProtect((LPVOID)xrFilePointer, 0x1000, oldProt, &oldProt);
}

DWORD RETURN_CStreaming_ConvertBufferToObject = 0x40C6B7;

void _declspec(naked) HOOK_CStreaming_ConvertBufferToObject()
{
	_asm
	{
		sub esp, 32
		mov ecx, [esp + 36]

		mov xrFilePointer, ecx
		mov xrIndex, esi

		pushad
	}

	ConvertBufferToObjectHandler();

	_asm
	{
		// Restore registers
		popad

		mov ecx, xrFilePointer

		jmp RETURN_CStreaming_ConvertBufferToObject
	}
}

void HookInit()
{
	// Строим перестановки
	unsigned totalVariants = fact(PARTS_NUM);
	unsigned perm[PARTS_NUM];
	for (unsigned i = 0; i < PARTS_NUM; i++)
		perm[i] = i;
	unsigned* tempPtr = permLookup = new unsigned[totalVariants*PARTS_NUM];
	permute(perm, 0, PARTS_NUM - 1);
	permLookup = tempPtr; // Восстановим указатель

	// Выделяем буфер для восстановления файла
	tempBuffer = (char*)malloc(TEMP_BUFFER_SIZE);

	DWORD oldProt;
	VirtualProtect((LPVOID)HOOKPOS_CStreaming_ConvertBufferToObject, 0x1000, PAGE_EXECUTE_READWRITE, &oldProt);
	HookInstall(HOOKPOS_CStreaming_ConvertBufferToObject, (DWORD)HOOK_CStreaming_ConvertBufferToObject, HOOKSIZE_CStreaming_ConvertBufferToObject);
	VirtualProtect((LPVOID)HOOKPOS_CStreaming_ConvertBufferToObject, 0x1000, oldProt, &oldProt);
}