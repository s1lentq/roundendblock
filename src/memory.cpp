#include "precompiled.h"

#ifdef _MSC_VER
#pragma comment(lib, "psapi.lib")
#endif // _MSC_VER

#ifdef _WIN32
bool lib_load_info(void *addr, lib_t *lib)
{
	MEMORY_BASIC_INFORMATION mem;
	VirtualQuery(addr, &mem, sizeof(mem));

	IMAGE_DOS_HEADER *dos = (IMAGE_DOS_HEADER*)mem.AllocationBase;
	IMAGE_NT_HEADERS *pe = (IMAGE_NT_HEADERS*)((dword)dos + (dword)dos->e_lfanew);

	if(pe->Signature != IMAGE_NT_SIGNATURE)
		return false;

	lib->base = (char *)mem.AllocationBase;
	lib->size = (size_t)pe->OptionalHeader.SizeOfImage;
	lib->handle = lib->base;

	return true;
}

static inline bool mem_compare_c(const char *addr,const char *pattern,const char *pattern_end)
{
	const char *c;
	for(c = pattern; c < pattern_end; ++c, ++addr)
	{
		if(*c == *addr || *c == '\x2A')
			continue;

		return false;
	}

	return true;
}

static char *mem_find_pattern(char *pos,int range,const char *pattern,int len)
{
	char *end;
	const char *pattern_end;
	
	pattern_end = pattern + len;

	for(end = pos + range - len; pos < end; ++pos)
	{
		if(mem_compare_c(pos,pattern,pattern_end))
			return pos;
	}

	return NULL;
}

char *lib_find_pattern(lib_t *lib, const char *pattern, int len)
{
	return mem_find_pattern(lib->base, lib->size, pattern, len);
}

#else // _WIN32

char *lib_find_symbol(lib_t *lib, const char *symbol)
{
	return (char *)dlsym(lib->handle, symbol);
}

static ElfW(Addr) dlsize(void *base)
{
	int i;
	ElfW(Ehdr) *ehdr;
	ElfW(Phdr) *phdr;
	ElfW(Addr) end;

	ehdr = (ElfW(Ehdr)*)base;
	phdr = (ElfW(Phdr)*)((ElfW(Addr))ehdr + ehdr->e_phoff);

	for(i = 0; i < ehdr->e_phnum; ++i)
	{
		if(phdr[i].p_type == PT_LOAD)
			end = phdr[i].p_vaddr + phdr[i].p_memsz;
	}

	return end;
}

bool lib_load_info(void *addr, lib_t *lib)
{
	Dl_info info;
	if((!dladdr(addr, &info) && !info.dli_fbase) || !info.dli_fname)
		return false;

	lib->base = (char *)info.dli_fbase;
	lib->size = (size_t)dlsize(info.dli_fbase);
	lib->handle = (char *)dlopen(info.dli_fname, RTLD_NOW);

	return true;
}

bool mem_memcpy(char *addr, const char *patch, int len)
{
	size_t size = sysconf(_SC_PAGESIZE);
	void *alignedAddress = Align(addr);

	if(Align(addr + len - 1) != alignedAddress)
		size *= 2;

	if(!mprotect(alignedAddress, size, (PROT_READ | PROT_WRITE | PROT_EXEC)))
	{
		memcpy(addr, patch, len);
		return !mprotect(alignedAddress, size, (PROT_READ | PROT_EXEC));
	}

	return false;
}

#endif // _WIN32

bool mem_patch(char *addr, unsigned char patch, int pos_byte, int len)
{
#ifdef _WIN32
	static HANDLE process = 0;

	DWORD OldProtection = 0;
	DWORD NewProtection = PAGE_EXECUTE_READWRITE;

	if(!process)
		process = GetCurrentProcess();

	FlushInstructionCache(process, addr, len);
	if(VirtualProtect(addr, len, NewProtection, &OldProtection))
	{
		*(addr + pos_byte) = patch;
		return VirtualProtect(addr, len, OldProtection, &NewProtection) != FALSE;
	}
#else
	size_t size = sysconf(_SC_PAGESIZE);
	void *alignedAddress = Align(addr);

	if(Align(addr) != alignedAddress)
		size *= 2;

	if(!mprotect(alignedAddress, size, (PROT_READ | PROT_WRITE | PROT_EXEC)))
	{
		*(addr + pos_byte) = patch;
		return !mprotect(alignedAddress, size, (PROT_READ | PROT_EXEC));
	}
#endif // _WIN32

	return false;
}
