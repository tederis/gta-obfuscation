#pragma once

typedef int BOOL;
typedef unsigned long DWORD;
typedef unsigned char BYTE;

#define MAX_JUMPCODE_SIZE 50

void HookInstall(DWORD dwInstallAddress, DWORD dwHookHandler, int iJmpCodeSize)
{
	BYTE JumpBytes[MAX_JUMPCODE_SIZE];
	memset(JumpBytes, 0x90, MAX_JUMPCODE_SIZE);
	JumpBytes[0] = 0xE9;
	*((DWORD*)(&JumpBytes[1])) = dwHookHandler - (dwInstallAddress + 5);
	
	memcpy((PVOID)dwInstallAddress, JumpBytes, iJmpCodeSize);
}
