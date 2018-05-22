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

#define PARTS_NUM 6 // A number of parts for assets decomposing
#define VARIANT_KEY 23 // The key for permutation value
#define TEMP_BUFFER_SIZE 8388608 // The max size of obfuscated file
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

	// Check the obfuscation flag
	if (*((unsigned*)xrFilePointer) == 0x18D5E82)
	{
		// Read size and permutation number
		xrFilePointer += 4;
		unsigned fileSize = *((unsigned*)xrFilePointer);
		xrFilePointer += 4;
		unsigned variant = *((unsigned*)xrFilePointer);
		variant /= VARIANT_KEY; // Unpack the permutation value
		xrFilePointer += 4;

		unsigned partSize = floor(fileSize / PARTS_NUM);
		unsigned totalPartsSize = partSize * PARTS_NUM;
		unsigned tailSize = fileSize - totalPartsSize;

		// Copy to temporary buffer
		memcpy(tempBuffer, xrFilePointer, fileSize);

		// Build the default asset
		unsigned* permPtr = permLookup + variant*PARTS_NUM;
		for (unsigned i = 0; i < PARTS_NUM; i++)
		{
			// Move to default chunk
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
	// Build the look table for permutations
	unsigned totalVariants = fact(PARTS_NUM);
	unsigned perm[PARTS_NUM];
	for (unsigned i = 0; i < PARTS_NUM; i++)
		perm[i] = i;
	unsigned* tempPtr = permLookup = new unsigned[totalVariants*PARTS_NUM];
	permute(perm, 0, PARTS_NUM - 1);
	permLookup = tempPtr; // Restore the pointer

	// Allocate a place for our buffer
	tempBuffer = (char*)malloc(TEMP_BUFFER_SIZE);

	DWORD oldProt;
	VirtualProtect((LPVOID)HOOKPOS_CStreaming_ConvertBufferToObject, 0x1000, PAGE_EXECUTE_READWRITE, &oldProt);
	HookInstall(HOOKPOS_CStreaming_ConvertBufferToObject, (DWORD)HOOK_CStreaming_ConvertBufferToObject, HOOKSIZE_CStreaming_ConvertBufferToObject);
	VirtualProtect((LPVOID)HOOKPOS_CStreaming_ConvertBufferToObject, 0x1000, oldProt, &oldProt);
}
