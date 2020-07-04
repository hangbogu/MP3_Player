#include "display.h"

static struct c146_disp a = {0};

/*struct 146_disp{
  uint8_t displayfunction;
  uint8_t displaycontrol;
  uint8_t displaymode;
  uint8_t backlightval;
};*/

void lcd_delay(uint32_t ms) {
  clock_t start_time = clock();
  while (clock() < start_time + ms)
    ;
}

void lcd_init() {
  a.displayfunction = LCD_4BITMODE | LCD_2LINE | LCD_5x8DOTS;
  a.backlightval = LCD_BACKLIGHT;
  lcd_delay(50000);
  i2c_write(a.backlightval);
  lcd_delay(100000);
  // send_4_bits(0x03 << 4);
  // lcd_delay(5000);
  send_4_bits(0x03 << 4);
  lcd_delay(5000);
  // send_4_bits(0x03 << 4);
  // lcd_delay(2000);
  send_4_bits(0x02 << 4);
  command(LCD_FUNCTIONSET | a.displayfunction);
  a.displaycontrol = LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF;
  display();
  clear();
  a.displaymode = LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT;
  command(LCD_ENTRYMODESET | a.displaymode);
  home();
}
void lcd_send(uint8_t value, uint8_t mode) {
  uint8_t upper = value & 0xF0;
  uint8_t lower = (value << 4) & 0xF0;
  send_4_bits(upper | mode);
  send_4_bits(lower | mode);
}
void send_4_bits(uint8_t value) {
  i2c_write(value);
  pulse_enable(value);
}
void i2c_write(uint8_t data) {
  uint8_t data_backlight = data | a.backlightval;
  i2c__write_single(I2C__2, 0x4E, 0x30, data_backlight);
}
void pulse_enable(uint8_t data) {
  i2c_write(data | En);
  lcd_delay(1000);
  i2c_write(data & ~En);
  lcd_delay(10000000);
}

void write(uint8_t value) { lcd_send(value, Rs); }
void clear() {
  command(LCD_CLEARDISPLAY);
  lcd_delay(20000000);
}
void home() {
  command(LCD_RETURNHOME);
  lcd_delay(20000000);
}
void display() {
  a.displaycontrol |= LCD_DISPLAYON | 0x00;
  command(LCD_DISPLAYCONTROL | a.displaycontrol);
}
void command(uint8_t data) { lcd_send(data, 0); }
void print_str(char *str) {
  int i = 0;
  while (i < 32 && str[i] != '\0') {
    write(str[i]);
    i++;
  }
}
void move_cursor(uint8_t row) {
  uint8_t offset[2] = {0x00, 0x40};
  command(LCD_SETDDRAMADDR | offset[row]);
}