#pragma once

#ifdef _WIN32
	#include <windows.h>
	#include <psapi.h>

	#define PSAPI_VERSION 1
#else
	#include <elf.h>
	#include <dlfcn.h>
	#include <unistd.h>
	#include <sys/types.h>
	#include <sys/stat.h>
	#include <sys/mman.h>
	#include <link.h>

	#define Align(addr)	((void *)(((long)addr) & ~(sysconf(_SC_PAGESIZE) - 1)))
#endif // _WIN32

struct lib_t
{
	char *base;
	char *handle;
	size_t size;
};

#ifdef _WIN32
	char *lib_find_pattern(lib_t *lib, const char *pattern, int len);
#else
	char *lib_find_symbol(lib_t *lib, const char *symbol);
	bool mem_memcpy(char *addr, const char *patch, int len);
#endif // _WIN32

bool lib_load_info(void *addr, lib_t *lib);

bool mem_patch(char *addr, unsigned char patch, int pos_byte, int len = 1);
