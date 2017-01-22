#pragma once

#include <avr/io.h>

#define F_CPU 8000000UL

#include <util/delay.h>

// LEDs
#define LED_STRIP_PORT		PORTA
#define LED_STRIP_DDR		DDRA
#define LED_STRIP_PIN		PORTA3
