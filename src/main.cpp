#include "precompiled.h"

extern mutil_funcs_t *gpMetaUtilFuncs;

int OnMetaAttach(void)
{
	lib_t g_dllGameMod;

	char *addr;
	char patch[] = { '\x8B', '\x01', '\xC3' };

	lib_load_info((void *)gpGamedllFuncs->dllapi_table->pfnSpawn, &g_dllGameMod);

#ifdef _WIN32
	const char pattern1[] = "\x83\x2A\x2A\x53\x55\x8B\x2A\x56\x33\x2A\x57\x8A";
	const char pattern2[] = "\xE8\x2A\x2A\x2A\x2A\x8A\x2A\x2A\x83\x2A\x2A\x3C";

	addr = lib_find_pattern(&g_dllGameMod, pattern1, sizeof(pattern1) - 1);
#else
	addr = lib_find_symbol(&g_dllGameMod, "CheckWinConditions__18CHalfLifeMultiplay");

	if (!addr)
	{
		addr = lib_find_symbol(&g_dllGameMod, "_ZN18CHalfLifeMultiplay18CheckWinConditionsEv");
	}
#endif // _WIN32

	if (!addr)
	{
		gpMetaUtilFuncs->pfnLogError(PLID, "can't find \"CheckWinConditions\"");
		return 0;
	}

	if (!mem_patch(addr, '\xC3', 0))
	{
		return 0;
	}

#ifdef _WIN32
	addr = lib_find_pattern(&g_dllGameMod, pattern2, sizeof(pattern2) - 1);
#else
	addr = lib_find_symbol(&g_dllGameMod, "HasRoundTimeExpired__18CHalfLifeMultiplay");

	if(!addr)
	{
		addr = lib_find_symbol(&g_dllGameMod, "_ZN18CHalfLifeMultiplay19HasRoundTimeExpiredEv");
	}
#endif // _WIN32

	if(!addr)
	{
		gpMetaUtilFuncs->pfnLogError(PLID, "can't find \"HasRoundTimeExpired\"");
		return 0;
	}

#ifdef _WIN32
	addr += 11;

	if (!mem_patch(addr, '\x02', 1))
	{
		return 0;
	}
#else
	if (*addr == '\x53')
	{
		if (!mem_patch(addr, '\xC3', 0))
		{
			return 0;
		}
	}
	else
	{
		if (!mem_memcpy(addr, patch, sizeof(patch)))
		{
			return 0;
		}
	}
#endif // _WIN32

	return 1;
}
