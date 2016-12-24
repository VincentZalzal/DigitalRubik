#pragma once

// Flash memory handling.
#ifndef USE_AVR
	// This code is needed for non-AVR platforms.
	#define PROGMEM
	#define pgm_read_byte(Addr)	(*(Addr))
#endif

// Asserts
#ifdef USE_AVR
	#define assert(Condition) ((void)0)
#else
	#include <cassert>
#endif

// Static asserts
#ifndef USE_STATIC_ASSERT
	#define USE_STATIC_ASSERT 1
#endif

#if USE_STATIC_ASSERT
	#define STATIC_ASSERT(Condition, Message) static_assert(Condition, Message)
#else
	#define STATIC_ASSERT(Condition, Message)
#endif
