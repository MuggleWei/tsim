#ifndef TSIM_MACRO_H_
#define TSIM_MACRO_H_

#include "muggle/c/base/macro.h"
#include "tsim/config.h"

EXTERN_C_BEGIN

// dll export
#if MUGGLE_PLATFORM_WINDOWS && defined(TSIM_USE_DLL)
	#ifdef TSIM_EXPORTS
		#define TSIM_EXPORT __declspec(dllexport)
	#else
		#define TSIM_EXPORT __declspec(dllimport)
	#endif
#else
	#define TSIM_EXPORT
#endif

EXTERN_C_END

#endif // !TSIM_MACRO_H_
