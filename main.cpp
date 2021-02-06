/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"

/* Example code to drive a 16x2 LCD panel via a I2C bridge chip (e.g. PCF8574)

   NOTE: The panel must be capable of being driven at 3.3v NOT 5v. The Pico
   GPIO (and therefor I2C) cannot be used at 5v.

   You will need to use a level shifter on the I2C lines if you want to run the
   board at 5v.

   Connections on Raspberry Pi Pico board, other boards may vary.

   GPIO 4 (pin 6)-> SDA on LCD bridge board
   GPIO 5 (pin 7)-> SCL on LCD bridge board
   3.3v (pin 36) -> VCC on LCD bridge board
   GND (pin 38)  -> GND on LCD bridge board
*/

// Software I2C
#define I2C_SDA 4
#define I2C_SCL 5
class I2C {
private:
	void delay()
	{
		sleep_us(1);
	}

	// initialize
	void init_i2c()
	{
		// init SCL pin
		gpio_init(I2C_SCL);
		i2c_cl_1();
		// init SDA pin
		gpio_init(I2C_SDA);
		i2c_da_1();
	}

	void i2c_cl_0()
	{
		gpio_put(I2C_SCL, 0);
		gpio_set_dir(I2C_SCL, GPIO_OUT);
	}

	void i2c_cl_1()
	{
		gpio_pull_up(I2C_SCL);
		gpio_set_dir(I2C_SCL, GPIO_IN);
	}

	void i2c_da_0()
	{
		gpio_put(I2C_SDA, 0);
		gpio_set_dir(I2C_SDA, GPIO_OUT);
	}

	void i2c_da_1()
	{
		gpio_pull_up(I2C_SDA);
		gpio_set_dir(I2C_SDA, GPIO_IN);
	}

	int i2c_get_da()
	{
		return gpio_get(I2C_SDA) ? 1 : 0;
	}

	// start condition
	void i2c_start()
	{
		i2c_da_0(); // SDA=0
		delay();
		i2c_cl_0(); // SCL=0
		delay();
	}

	// stop condition
	void i2c_stop()
	{
		i2c_cl_1(); // SCL=1
		delay();
		i2c_da_1(); // SDA=1
		delay();
	}

	// repeated start condition
	void i2c_repeat()
	{
		i2c_cl_1(); // SCL=1
		delay();
		i2c_da_0(); // SDA=0
		delay();
		i2c_cl_0(); // SCL=0
		delay();
	}

	// send byte
	bool i2c_write(int c)
	{
		int i;
		bool nack;

		delay();

		// 8ビット送信
		for (i = 0; i < 8; i++) {
			if (c & 0x80) {
				i2c_da_1(); // SCL=1
			} else {
				i2c_da_0(); // SCL=0
			}
			c <<= 1;
			delay();
			i2c_cl_1(); // SCL=1
			delay();
			i2c_cl_0(); // SCL=0
			delay();
		}

		i2c_da_1(); // SDA=1
		delay();

		i2c_cl_1(); // SCL=1
		delay();
		// receive NACK bit
		nack = i2c_get_da();
		i2c_cl_0(); // SCL=0

		return nack;
	}

	int address; // I2C device address

public:
	I2C(int address)
		: address(address)
	{
		init_i2c();
	}

	// write data
	virtual void write(int data)
	{
		i2c_start();
		i2c_write(address << 1);
		i2c_write(data);
		i2c_stop();
	}
};

I2C *wire;

// commands
const int LCD_CLEARDISPLAY = 0x01;
const int LCD_RETURNHOME = 0x02;
const int LCD_ENTRYMODESET = 0x04;
const int LCD_DISPLAYCONTROL = 0x08;
const int LCD_CURSORSHIFT = 0x10;
const int LCD_FUNCTIONSET = 0x20;
const int LCD_SETCGRAMADDR = 0x40;
const int LCD_SETDDRAMADDR = 0x80;

// flags for display entry mode
const int LCD_ENTRYSHIFTINCREMENT = 0x01;
const int LCD_ENTRYLEFT = 0x02;

// flags for display and cursor control
const int LCD_BLINKON = 0x01;
const int LCD_CURSORON = 0x02;
const int LCD_DISPLAYON = 0x04;

// flags for display and cursor shift
const int LCD_MOVERIGHT = 0x04;
const int LCD_DISPLAYMOVE = 0x08;

// flags for function set
const int LCD_5x10DOTS = 0x04;
const int LCD_2LINE = 0x08;
const int LCD_8BITMODE = 0x10;

// flag for backlight control
const int LCD_BACKLIGHT = 0x08;

const int LCD_ENABLE_BIT = 0x04;

#define I2C_PORT i2c0
// By default these LCD display drivers are on bus address 0x27
static int addr = 0x27;

// Modes for lcd_send_byte
#define LCD_CHARACTER  1
#define LCD_COMMAND    0

#define MAX_LINES      2
#define MAX_CHARS      16

void lcd_send_byte(uint8_t val, int mode)
{
	uint8_t hi = mode | (val & 0xf0) | LCD_BACKLIGHT;
	uint8_t lo = mode | ((val << 4) & 0xf0) | LCD_BACKLIGHT;
#define DELAY_US 500 // 100..600
	sleep_us(DELAY_US);
	wire->write(hi);
	sleep_us(DELAY_US);
	wire->write(hi | LCD_ENABLE_BIT);
	sleep_us(DELAY_US);
	wire->write(hi);
	sleep_us(DELAY_US);
	wire->write(lo);
	sleep_us(DELAY_US);
	wire->write(lo | LCD_ENABLE_BIT);
	sleep_us(DELAY_US);
	wire->write(lo);
	sleep_us(DELAY_US);
}

void lcd_clear(void)
{
	lcd_send_byte(LCD_CLEARDISPLAY, LCD_COMMAND);
	sleep_ms(1);
}

// go to location on LCD
void lcd_set_cursor(int line, int position)
{
	int val = (line == 0) ? 0x80 + position : 0xC0 + position;
	lcd_send_byte(val, LCD_COMMAND);
}

static void inline lcd_char(char val)
{
	lcd_send_byte(val, LCD_CHARACTER);
}

void lcd_string(const char *s)
{
	while (*s) {
		lcd_char(*s++);
	}
}

void lcd_init()
{
	wire = new I2C(addr);
	sleep_ms(1);

	lcd_send_byte(0x03, LCD_COMMAND);
	lcd_send_byte(0x03, LCD_COMMAND);
	lcd_send_byte(0x03, LCD_COMMAND);
	lcd_send_byte(0x02, LCD_COMMAND);

	lcd_send_byte(LCD_ENTRYMODESET | LCD_ENTRYLEFT, LCD_COMMAND);
	lcd_send_byte(LCD_FUNCTIONSET | LCD_2LINE, LCD_COMMAND);
	lcd_send_byte(LCD_DISPLAYCONTROL | LCD_DISPLAYON, LCD_COMMAND);

	lcd_clear();
}

int main()
{
	lcd_init();

	gpio_init(25);
	gpio_set_dir(25, GPIO_OUT);

	static char const *message[] = {
		"RP2040 by", "Raspberry Pi",
		"A brand new", "microcontroller",
		"Twin core M0", "Full C SDK",
		"More power in", "your product",
		"More beans", "than Heinz!"
	};

	while (1) {
		for (int m = 0; m < sizeof(message) / sizeof(message[0]); m += MAX_LINES) {
			for (int line = 0; line < MAX_LINES; line++) {
				lcd_set_cursor(line, (MAX_CHARS / 2) - strlen(message[m + line]) / 2);
				lcd_string(message[m + line]);
			}
			sleep_ms(2000);
			lcd_clear();
		}
	}

	return 0;
}
