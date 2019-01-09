#ifndef COMUNI_H_
#define COMUNI_H_

/*
 * Inclusions shared by all btport's files
 */

// Commonly included
// ==========================================

#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
// if you print PRIu32 etc
#include <inttypes.h>

// Useful macros
// ==========================================

#define MIN(a, b)			((a) < (b) ? (a) : (b))
#define MAX(a, b)			((a) > (b) ? (a) : (b))

#define UNUSED(x)          	(void)(sizeof(x))
#define NOT(x)             	(~(unsigned int) (x))
#define ABS(x)				(x < 0 ? -(x) : x)

#define SIZE_OF(a)         sizeof(a) / sizeof(a[0])

// Debug
// ==========================================

#ifdef NDEBUG
	// assert() is empty in release ...
#   define ASSERT(a)
	// ... but I want to keep the expression
#   define CHECK_IT(a)      (void)(a)

#   define BPOINT
#else
#   define ASSERT(a)        assert(a)
#   define CHECK_IT(a)      assert(a)

#   define BPOINT			bsp_breakpoint()
#endif

// Print
// ==========================================

#ifdef NDEBUG
#	define PRINTF(f, ...)
#	define PUTS(a)
#else
#	define PRINTF(f, ...)		bsp_printf(f, ##__VA_ARGS__)
#	define PUTS(a)				bsp_puts(a)
#endif

// Every file can enable its DBG_xxx
#ifdef STAMPA_DBG
#	ifndef NDEBUG
#		define DBG_ABIL					1
#	endif
#	define DBG_FUN						PUTS(__FUNCTION__)
#	define DBG_ERR						PRINTF("ERR %s %d\n", __FILE__, __LINE__)
#	define DBG_PRINTF(f, ...)			PRINTF(f, ##__VA_ARGS__)
#	define DBG_PUTS(a)					PUTS(a)
#else
#	define DBG_FUN
#	define DBG_ERR
#	define DBG_PRINTF(f, ...)
#	define DBG_PUTS(a)
#endif

// Shared values
// ==========================================

#define NUM_TIMER		4
#define NUM_APC			4

#endif

