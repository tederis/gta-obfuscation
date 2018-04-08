/*******************************************************
* Copyright (C) 2016 TEDERIs <xcplay@gmail.com>
*
* This file is part of RWObfuscator.
*
* RWObfuscator can not be copied and/or distributed without the express
* permission of TEDERIs.
*******************************************************/

#include <stdio.h>
#include <tchar.h>

#include <fstream>
#include <iostream>
#include <ctime>

#include "helper.h"

using namespace std;

#define PARTS_NUM 6 // Количество частей, на которое разобран файл
#define VARIANT_KEY 23 // Ключ для дешифровки варианта перестановки в теле файла
#define XR_SWAP(x,y) temp = *x; *x = *y; *y = temp

long double fact(int N)
{
	if (N < 0)
		return 0;
	if (N == 0)
		return 1;
	else
		return N * fact(N - 1);
}

void permute(unsigned *a, unsigned** lookup, int l, int r)
{
	int i;
	if (l == r)
	{
		memcpy(*lookup, a, sizeof(unsigned)*PARTS_NUM);
		(*lookup) += PARTS_NUM;
	}
	else
	{
		char temp;
		for (i = l; i <= r; i++)
		{
			XR_SWAP((a + l), (a + i));
			permute(a, lookup, l + 1, r);
			XR_SWAP((a + l), (a + i));
		}
	}
}

int main(int argc, char *argv[])
{
	if (argc < 3)
	{
		cerr << "usage: RWObfuscator.exe in_file out_file";
		return 1;
	}

	/*
		Читаем исходный файл
	*/
	ifstream in(argv[1], ios::binary | ios::ate);
	if (in.fail()) {
		cerr << "cannot open " << argv[1] << endl;
		return 1;
	}

	cout << "start parsing " << argv[1] << endl;

	fpos_t fileSize = in.tellg().seekpos();
	in.seekg(0, ios_base::beg);
	
	unsigned partSize = floor(fileSize / PARTS_NUM);
	unsigned totalPartsSize = partSize * PARTS_NUM;
	unsigned tailSize = fileSize - totalPartsSize;
	
	// Вычисляем случайную комбинацию
	unsigned totalVariants = fact(PARTS_NUM);
	srand((unsigned)time(NULL) + fileSize);
	unsigned variant = rand() % totalVariants;

	// Строим массив всех вариаций
	unsigned perm[PARTS_NUM];
	for (unsigned i = 0; i < PARTS_NUM; i++)
	{
		perm[i] = i;
	}
	unsigned* permLookup = new unsigned[totalVariants*PARTS_NUM];
	unsigned* tempPtr = permLookup;

	permute(perm, &tempPtr, 0, PARTS_NUM-1);

	/* 
		Записываем зашифрованный файл
	*/
	ofstream out(argv[2], ios::binary);
	if (out.fail()) {
		cerr << "cannot open " << argv[1] << endl;
		return 1;
	}

	writeUInt32(0x18D5E82, out); // Идентификатор, указывающий что файл зашифрован
	writeUInt32(fileSize, out); // Размер файла
	writeUInt32(variant * VARIANT_KEY, out); // Закодированная вариация перестановки

	char* buf = (char*)malloc(partSize);
	unsigned* permPtr = permLookup + variant*PARTS_NUM;
	for (unsigned i = 0; i < PARTS_NUM; i++)
	{
		// Переходим к чанку в исходном файле
		unsigned chunkIndex = permPtr[i];
		in.seekg(chunkIndex * partSize, ios_base::beg);

		in.read(buf, partSize);
		out.write(buf, partSize);
	}

	// Записываем хвост
	in.seekg(totalPartsSize, ios_base::beg);
	in.read(buf, tailSize);
	out.write(buf, tailSize);

	delete buf;
	delete[] permLookup;

	out.close();
	in.close();

	cout << "end parsing variant " << variant << endl;

    return 0;
}

