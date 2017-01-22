#pragma once

// Flash memory handling (for AVR only)
#ifdef USE_SIMULATOR
	// This is needed for non-AVR platforms.
	#define PROGMEM
	#define pgm_read_byte(Addr)	(*(Addr))
#else
	#include <avr/pgmspace.h>
#endif

// Asserts (disabled on AVR)
#ifdef USE_SIMULATOR
	#include <cassert>
#else
	// Disable asserts.
	#define assert(Condition) ((void)0)
#endif

// Static asserts
// If you want static asserts, define USE_STATIC_ASSERT to 1.
#ifndef USE_STATIC_ASSERT
	#define USE_STATIC_ASSERT 0
#endif

#if USE_STATIC_ASSERT
	#define STATIC_ASSERT(Condition, Message) static_assert(Condition, Message)
#else
	#define STATIC_ASSERT(Condition, Message)
#endif
