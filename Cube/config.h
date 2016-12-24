#pragma once

// Flash memory handling (for AVR only)
#ifndef USE_AVR
	// This is needed for non-AVR platforms.
	#define PROGMEM
	#define pgm_read_byte(Addr)	(*(Addr))
#endif

// Asserts (disabled on AVR)
#ifdef USE_AVR
	// Disable asserts.
	#define assert(Condition) ((void)0)
#else
	#include <cassert>
#endif

// Static asserts
// If you don't want static asserts, define USE_STATIC_ASSERT to 0.
#ifndef USE_STATIC_ASSERT
	#define USE_STATIC_ASSERT 1
#endif

#if USE_STATIC_ASSERT
	#define STATIC_ASSERT(Condition, Message) static_assert(Condition, Message)
#else
	#define STATIC_ASSERT(Condition, Message)
#endif
