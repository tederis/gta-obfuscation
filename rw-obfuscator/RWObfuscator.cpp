#include <stdio.h>
#include <tchar.h>

#include <fstream>
#include <iostream>
#include <ctime>

#include "helper.h"

using namespace std;

#define PARTS_NUM 6 // A number of parts for assets decomposing
#define VARIANT_KEY 23 // The key for permutation value
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
		Read a source file
	*/
	ifstream in(argv[1], ios::binary | ios::ate);
	if (in.fail()) {
		cerr << "cannot open " << argv[1] << endl;
		return 1;
	}

	cout << "start parsing " << argv[1] << endl;

	fpos_t fileSize = in.tellg();
	in.seekg(0, ios_base::beg);
	
	unsigned partSize = floor(fileSize / PARTS_NUM);
	unsigned totalPartsSize = partSize * PARTS_NUM;
	unsigned tailSize = fileSize - totalPartsSize;
	
	// Calculate a random permutation key
	unsigned totalVariants = fact(PARTS_NUM);
	srand((unsigned)time(NULL) + fileSize);
	unsigned variant = rand() % totalVariants;

	// Build an auxiliary array
	unsigned perm[PARTS_NUM];
	for (unsigned i = 0; i < PARTS_NUM; i++)
	{
		perm[i] = i;
	}
	unsigned* permLookup = new unsigned[totalVariants*PARTS_NUM];
	unsigned* tempPtr = permLookup;

	permute(perm, &tempPtr, 0, PARTS_NUM-1);

	/* 
		Build
	*/
	ofstream out(argv[2], ios::binary);
	if (out.fail()) {
		cerr << "cannot open " << argv[1] << endl;
		return 1;
	}

	writeUInt32(0x18D5E82, out); // Our flag for the deobfuscator
	writeUInt32(fileSize, out); // File size
	writeUInt32(variant * VARIANT_KEY, out); // Permutation key

	char* buf = (char*)malloc(partSize);
	unsigned* permPtr = permLookup + variant*PARTS_NUM;
	for (unsigned i = 0; i < PARTS_NUM; i++)
	{
		// Move to a chunk in an default asset
		unsigned chunkIndex = permPtr[i];
		in.seekg(chunkIndex * partSize, ios_base::beg);

		in.read(buf, partSize);
		out.write(buf, partSize);
	}

	// Write a tail
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

