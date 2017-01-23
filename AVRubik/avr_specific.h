#pragma once

#define F_CPU 8000000UL

#include <avr/io.h>
#include <util/delay.h>

// LEDs
#define LED_STRIP_PORT		PORTA
#define LED_STRIP_DDR		DDRA
#define LED_STRIP_PIN		PORTA3

// Rings (shift registers).
#define SH_REG_DDR		DDRB
#define SH_REG_PORT		PORTB
#define SH_REG_SER_IN		PORTB0
#define SH_REG_SRCK		PORTB1
#define SH_REG_RCK		PORTB2
