#pragma once

#include "FreeRTOS.h"
#include "i2c.h"
#include "task.h"
#include <stdint.h>
#include <stdio.h>
#include <time.h>

#define LCD_CLEARDISPLAY 0x01
#define LCD_RETURNHOME 0x02
#define LCD_ENTRYMODESET 0x04
#define LCD_DISPLAYCONTROL 0x08
#define LCD_CURSORSHIFT 0x10
#define LCD_FUNCTIONSET 0x20
#define LCD_SETDDRAMADDR 0x80

#define LCD_ENTRYLEFT 0x02
#define LCD_ENTRYSHIFTINCREMENT 0x01
#define LCD_ENTRYSHIFTDECREMENT 0x00

#define LCD_DISPLAYON 0x04
#define LCD_CURSOROFF 0x00
#define LCD_BLINKOFF 0x00

#define LCD_4BITMODE 0x00
#define LCD_2LINE 0x08
#define LCD_5x8DOTS 0x00

#define LCD_BACKLIGHT 0x08

#define En 0x04
#define Rw 0x02
#define Rs 0x01

void lcd_delay(uint32_t);

struct c146_disp {
  uint8_t displayfunction;
  uint8_t displaycontrol;
  uint8_t displaymode;
  uint8_t backlightval;
};

void lcd_init();
void lcd_send(uint8_t, uint8_t);
void send_4_bits(uint8_t);
void i2c_write(uint8_t);
void pulse_enable(uint8_t);
void move_cursor(uint8_t);

void write(uint8_t);
void command(uint8_t);
void clear();
void home();
void display();
void print_str(char *);