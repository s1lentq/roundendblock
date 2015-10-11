#pragma once

#include <cstring>

#include <eiface.h>
#include <meta_api.h>

#include "memory.h"

#ifndef _WIN32

#define TRUE	1
#define FALSE	0

#endif // _WIN32

#undef DLLEXPORT

#ifdef _WIN32
	#define DLLEXPORT	__declspec(dllexport)
#else
	#define DLLEXPORT	__attribute__((visibility("default")))
	#define WINAPI		/* */
#endif // _WIN32

#define C_DLLEXPORT extern "C" DLLEXPORT
