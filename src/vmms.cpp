/*	Elton Vinh
Project 1
CS149
Frank Butt
*/
/* 
  vmms.cpp - This file contains the code for each of the memory functions as well as initialization of the "shared" memory.
*/
#include "vmms_error.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <process.h>
#include <windows.h>
#include <psapi.h>

#define MAX_PHY_SIZE 8196    // 8K Bytes     ** Hardcode for testing !!
#define MAX_TABLE_SIZE 1024  // 1K entries
#define DEFAULT_BOUNDRY 8    // minimum 8 byte allocation block

// ************************************************************************
// Global Shared variables (shared by multiple instances of the DLL)
// ************************************************************************

/* Global shared memory section */
#pragma data_seg(".SHARED")  // Simulated Physical Mem // Hardcoded for now !!
int byte_boundry = DEFAULT_BOUNDRY;
int mem_size = MAX_PHY_SIZE;            // size of simulated phy mem (in bytes)
char mem_start[DEFAULT_BOUNDRY];	// simulated Phy Memory
char simMem[MAX_PHY_SIZE] = { 0 };
unsigned long int MMT[MAX_TABLE_SIZE][4] = { 0 };
unsigned long int freeList[MAX_TABLE_SIZE][2] = { { ((unsigned long int) (&simMem[0])), MAX_PHY_SIZE } };
int MMT_size = 0;
int freeList_size = 1;
#pragma data_seg()



/* Here are the 5 exported functions for the application programs to use */
__declspec(dllexport) char* vmms_malloc (  int size, int* error_code );
__declspec(dllexport) int vmms_memset ( char* dest_ptr, char c, int size );
__declspec(dllexport) int vmms_memcpy ( char* dest_ptr, char* src_ptr, int size );
__declspec(dllexport) int vmms_print ( char* src_ptr, int size );
__declspec(dllexport) int vmms_free ( char* mem_ptr );

/* Here are several exported functions specifically for mmc.cpp */
__declspec(dllexport) int mmc_initialize (  int boundry_size );
__declspec(dllexport) int mmc_display_memtable ( char* filename );
__declspec(dllexport) int mmc_display_memory ( char* filename );
 
/* Non exported inner functions */
int find_free(int actualSize);
void update_free(int freedex, int actualSize);
int findPtr(char* mem_ptr);
SYSTEMTIME _getSystemTime();
void getProgramName(FILE* fp);
void write_to_VMMSmem(); 

#pragma comment(linker, "/section:SHARED,RWS")

__declspec(dllexport) int mmc_initialize (  int boundry_size )
{
	int rc = VMMS_SUCCESS;
	byte_boundry = boundry_size;

	return rc;
}

__declspec(dllexport) int mmc_display_memtable ( char* filename )
{
	int rc = VMMS_SUCCESS;

	/* Put your source code here */
	FILE* printFile;
	errno_t err;
	char *pos;
	int counter = MMT_size;

	if ((pos = strchr(filename, '\n')) != NULL)
		*pos = '\0';
	
	if (fopen_s(&printFile, filename, "w") != 0) {
		printf("Unaable to open file.");
	}
	else {
		for (int i = 0; i < counter; i++) {
			if (MMT[i][0] == 0 && MMT[i][1] == 0 && MMT[i][2] == 0 && MMT[i][3] == 0)
				counter += 1;
			else
				fprintf(printFile, "%d\t%p\t%d\t%d\n", MMT[i][0], MMT[i][1], MMT[i][2], MMT[i][3]);
		}
		fclose(printFile);
	}

	printf("\nMEMORY MAPPING TABLE\n");
	counter = MMT_size;
	for (int i = 0; i < counter; i++) {
		if (MMT[i][0] == 0 && MMT[i][1] == 0 && MMT[i][2] == 0 && MMT[i][3] == 0)
			counter += 1;
		else
			printf("%d\t%p\t%d\t%d\n", MMT[i][0], MMT[i][1], MMT[i][2], MMT[i][3]);
	}

	return rc;
}

__declspec(dllexport) int mmc_display_memory ( char* filename )
{
	int rc = VMMS_SUCCESS;

	/* Put your source code here */
	FILE* printFile;
	errno_t err;

	char *pos;
	if ((pos = strchr(filename, '\n')) != NULL)
		*pos = '\0';
	if (fopen_s(&printFile, filename, "w") != 0) {
		printf("Unable to open file.");
	}
	else {
		for (int i = 0; i < MAX_PHY_SIZE; i = i + 8) {
			fprintf(printFile, "\n%p	x%-2x x%-2x x%-2x x%-2x x%-2x x%-2x x%-2x x%-2x", &simMem[i], (unsigned char)simMem[i], (unsigned char)simMem[i + 1], (unsigned char)simMem[i + 2], (unsigned char)simMem[i + 3]
				, (unsigned char)simMem[i + 4], (unsigned char)simMem[i + 5], (unsigned char)simMem[i + 6], (unsigned char)simMem[i + 7]);
			fprintf(printFile, "%c%c%c%c%c%c%c%c", simMem[i], simMem[i + 1], simMem[i + 2], simMem[i + 3]
				, simMem[i + 4], simMem[i + 5], simMem[i + 6], simMem[i + 7]);
		}
		fclose(printFile);
	}

	for (int i = 0; i < MAX_PHY_SIZE; i = i+8) {
		printf("\n%p	x%-2x x%-2x x%-2x x%-2x x%-2x x%-2x x%-2x x%-2x", &simMem[i], (unsigned char)simMem[i], (unsigned char)simMem[i + 1], (unsigned char)simMem[i + 2], (unsigned char)simMem[i + 3]
														,(unsigned char)simMem[i + 4], (unsigned char)simMem[i + 5], (unsigned char)simMem[i + 6], (unsigned char)simMem[i + 7]);
		printf("%c%c%c%c%c%c%c%c", simMem[i], simMem[i + 1], simMem[i + 2], simMem[i + 3]
			,simMem[i + 4], simMem[i + 5], simMem[i + 6], simMem[i + 7]);
	}
	printf("\n");
	return rc;
}

__declspec(dllexport) char* vmms_malloc(int size, int* error_code)
{
	/* Put your source code here */
	char *malloc_loc = NULL;
	int actualSize = 0;
	int freedex = 0;

	if (size % byte_boundry == 0)
		actualSize = (size / byte_boundry)*byte_boundry;
	else
		actualSize = ((size + (byte_boundry - 1)) / byte_boundry) * byte_boundry;  // round up to nearest byte_boundary
	freedex = find_free(actualSize);

	if (freedex != -1) {
		/* Update MMT */
		for (int i = 0; i < MAX_PHY_SIZE; i++) {
			if (MMT[i][0] == 0 && MMT[i][1] == 0 && MMT[i][2] == 0 && MMT[i][3] == 0) {
				MMT_size += 1;
				MMT[i][0] = _getpid();			  // PID
				MMT[i][1] = freeList[freedex][0];  // ADDRESS
				update_free(freedex, actualSize);			  // 
				MMT[i][2] = size;				  // REQ_SIZE
				MMT[i][3] = actualSize;			  // ACTUAL_SIZE

				malloc_loc = (char*)MMT[i][1];
				*error_code = VMMS_SUCCESS;
				write_to_VMMSmem();
				break;
			}
		}
	}
	else {
		*error_code = OUT_OF_MEM;
	}


	FILE* logFile;
	errno_t err;
	SYSTEMTIME st = _getSystemTime();

	if ((err = fopen_s(&logFile, "VMMS.log", "ab")) != 0) {
		fprintf(stdout, "cannot open file '%s'\n",
			"VMMS.log");
	}
	else {
		fprintf(logFile, "%d%d%d%d%d%d %d", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, _getpid());
		getProgramName(logFile);
		fprintf(logFile, "%s %p %d %d\x0D\x0A", "vmms_malloc", malloc_loc, size, *error_code );
		fclose(logFile);
	}


	return malloc_loc;
}

__declspec(dllexport) int vmms_memset ( char* dest_ptr, char c, int size )
{
	int rc = VMMS_SUCCESS;
	int index = -1;
	/* Put your source code here */
	for (int i = 0; i < MMT_size; i++) {
		if (MMT[i][1] == (unsigned long int)dest_ptr) {
			index = i;
		}
	}

	if (index != -1) {
		if (MMT[index][3] >= size) {
			memset(dest_ptr, c, size);
			write_to_VMMSmem();
		}
		else
			rc = MEM_TOO_SMALL;
	}
	else
		rc = INVALID_DEST_ADDR;


	FILE* logFile;
	errno_t err;
	SYSTEMTIME st = _getSystemTime();

	if ((err = fopen_s(&logFile, "VMMS.log", "ab")) != 0) {
		fprintf(stdout, "cannot open file '%s'\n",
			"VMMS.log");
	}
	else {
		fprintf(logFile, "%d%d%d%d%d%d %d", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond,_getpid());
		getProgramName(logFile);
		fprintf(logFile, "%s %d %p %d %d\x0D\x0A", "vmms_memset", rc, dest_ptr, c, size);
		fclose(logFile);
	}


	return rc;
}

__declspec(dllexport) int vmms_memcpy ( char* dest_ptr, char* src_ptr, int size )
{
	int rc = VMMS_SUCCESS;
	int dest_ptr_index = -1;
	int src_ptr_index = -1;
	dest_ptr_index = findPtr(dest_ptr);
	src_ptr_index = findPtr(src_ptr);
	
	/* Put your source code here */
	if (dest_ptr_index != -1 && src_ptr_index != -1) {
		if (MMT[src_ptr_index][3] >= size) {
			if (MMT[dest_ptr_index][3] >= size) {
				// cpy
				for (int i = 0; i < size; i++)
					memset(dest_ptr+i, *(src_ptr+i), 1);
				write_to_VMMSmem();
			}
			else
				rc = MEM_TOO_SMALL;
		}
		else
			rc = INVALID_CPY_ADDR;
	}
	else
		rc = INVALID_CPY_ADDR;

	FILE* logFile;
	errno_t err;
	SYSTEMTIME st = _getSystemTime();

	if ((err = fopen_s(&logFile, "VMMS.log", "ab")) != 0) {
		fprintf(stdout, "cannot open file '%s'\n",
			"VMMS.log");
	}
	else {
		fprintf(logFile, "%d%d%d%d%d%d %d", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, _getpid());
		getProgramName(logFile);
		fprintf(logFile, "%s %d %p %p %d\x0D\x0A", "vmms_memcpy", rc, dest_ptr, src_ptr, size);
		fclose(logFile);
	}


	return rc;
}

__declspec(dllexport) int vmms_print(char* src_ptr, int size)
{
	int rc = VMMS_SUCCESS;
	/* Put your source code here */
	if (size != 0) {
		for (int i = 0; i < size; i++)
			printf("%c", *(src_ptr + i));
	}
	else if (size == 0) {
		int i = 0;
		while (*(src_ptr + i) != '\0') {
			printf("%c", *src_ptr);
			i++;
		}
	}
	printf("\n");
	return rc;
}

__declspec(dllexport) int vmms_free ( char* mem_ptr )
{
	int rc = VMMS_SUCCESS;
	unsigned long int addrToFree = 0;
	int found = -1;
	/* Put your source code here */
	addrToFree = (long) mem_ptr;
	for (int i = 0; i < MAX_TABLE_SIZE; i++) {
		if (MMT[i][1] == addrToFree) {
			found = i;
			break;
		}
	}

	if (found != -1) {
		vmms_memset(mem_ptr, (unsigned char)0xff, MMT[found][3]);  // "erase" physical memory taken up by pointer with 0xFF

		/* add newly free memory to freeList */

		for (int i = 0; i < MAX_TABLE_SIZE; i++) {
			if (freeList[i][0] == 0 && freeList[i][1] == 0) {
				freeList[i][0] = (long)mem_ptr;
				freeList[i][1] = MMT[found][3];
				freeList_size += 1;
				break;
			}
		}


		MMT[found][0] = 0;
		MMT[found][1] = 0;
		MMT[found][2] = 0;
		MMT[found][3] = 0;

		MMT_size -= 1;
		write_to_VMMSmem();
	}
	else
		rc = INVALID_MEM_ADDR;
	
	FILE* logFile;
	errno_t err;
	SYSTEMTIME st = _getSystemTime();

	if ((err = fopen_s(&logFile, "VMMS.log", "ab")) != 0) {
		fprintf(stdout, "cannot open file '%s'\n",
			"VMMS.log");
	}
	else {
		fprintf(logFile, "%d%d%d%d%d%d %d", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, _getpid());
		getProgramName(logFile);
		fprintf(logFile, "%s %d %p\x0D\x0A", "vmms_free", rc, mem_ptr);
		fclose(logFile);
	}


	return rc;
}

int find_free(int actualSize) {
	unsigned long freeAddress = 0;
	int bestFit = 0;
	int index = 0;

	/* Search Free List */
	if (freeList_size != 0 && actualSize < freeList[index][1]) {
		for (; index < freeList_size; index++) {
			if (freeList[index][1] == actualSize) {
				bestFit = index;
				break;
			}
			else if (freeList[index][1] > actualSize) {
				if (freeList[index][1] < freeList[bestFit][1])
					bestFit = index;
			}
		}
	}
	else
		bestFit = -1;

	return bestFit;
}

void update_free(int freedex, int actualSize) {
	if (freeList[freedex][1] == actualSize) {
		freeList[freedex][0] = 0;
		freeList[freedex][1] = 0;
		freeList_size -= 1;
	}
	else {
		freeList[freedex][0] = (unsigned long int)((char*) freeList[freedex][0] + actualSize);
		freeList[freedex][1] -= actualSize;
	}
}

int findPtr(char* mem_ptr) {
	int found = -1;
	for (int i = 0; i < MMT_size; i++) {
		if (MMT[i][1] == (unsigned long int)mem_ptr) {
			found = i;
			break;
		}
	}
	return found;
}

SYSTEMTIME _getSystemTime() {
	SYSTEMTIME st;
	GetSystemTime(&st);
	return st;
}

void getProgramName(FILE* fp) {
	TCHAR szProcessName[MAX_PATH] = TEXT("<unknown>");

	HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION |
		PROCESS_VM_READ,
		FALSE, _getpid());
	
	if (NULL != hProcess)
	{
		HMODULE hMod;
		DWORD cbNeeded;

		if (EnumProcessModules(hProcess, &hMod, sizeof(hMod),
			&cbNeeded))
		{
			GetModuleBaseName(hProcess, hMod, szProcessName,
				sizeof(szProcessName) / sizeof(TCHAR));
		}
	}

	fprintf(fp, " %s ", szProcessName);
}

void write_to_VMMSmem() {
	FILE* memFile;
	errno_t err;

	/* open new vmms.mem */
	if ((err = fopen_s(&memFile, "VMMS.mem", "wb")) != 0) {
		fprintf(stdout, "cannot open file '%s'\n",
			"VMMS.mem");
	}
	else {
		fwrite(simMem, sizeof(char), MAX_PHY_SIZE, memFile);
		fprintf(memFile, "\x0D\x0A");
		for (int i = 0; i<MMT_size; i++)
		{
			fwrite(MMT[i], sizeof(unsigned long int), 4, memFile);
		}
		fprintf(memFile, "\x0D\x0A");
		for (int i = 0; i<freeList_size; i++)
		{
			fwrite(freeList[i], sizeof(unsigned long int), 2, memFile);
		}
		fprintf(memFile, "\x0D\x0A");
		fclose(memFile);
	}

}