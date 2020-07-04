#pragma once

#include <stdint.h>
#include <stdlib.h>

#include "board_io.h"
#include "gpio.h"
#include "lpc40xx.h"
#include "ssp2.h"

// VS1053 R/W Opcode
#define VS1053_SCI_READ 0x03
#define VS1053_SCI_WRITE 0x02

// VS1053 Registors
#define VS1053_REG_MODE 0x00
#define VS1053_REG_STATUS 0x01
#define VS1053_REG_BASS 0x02
#define VS1053_REG_CLOCKF 0x03
#define VS1053_REG_VOLUME 0x0B
//#define VS1053_REG_DECODETIME 0x04
//#define VS1053_REG_AUDATA 0x05
//#define VS1053_REG_WRAM 0x06
//#define VS1053_REG_WRAMADDR 0x07
//#define VS1053_REG_HDAT0 0x08
//#define VS1053_REG_HDAT1 0x09

static gpio_s DREQ, CS, SDCS, XDCS, RESET;

void mp3_initialization() {

 // Initialize the spi.
 uint32_t max_clock_khz = 24;
 ssp2__initialize(max_clock_khz);

 gpio__construct_with_function(1, 0, 4); // SCK
 gpio__construct_with_function(1, 1, 4); // MOSI
 gpio__construct_with_function(1, 4, 4); // MISO

 DREQ = gpio__construct_as_input(2, 0);   // DREQ
 CS = gpio__construct_as_output(2, 1);    // CS
 SDCS = gpio__construct_as_output(2, 2);  // SDCS
 XDCS = gpio__construct_as_output(2, 4);  // XDCS
 RESET = gpio__construct_as_output(2, 5); // RESET

 // Initialize the pin out put.
 gpio__set(CS);
 gpio__set(SDCS);
 gpio__set(XDCS);

 // Hardware Reset.
 gpio__set(RESET);
 gpio__reset(RESET);
 gpio__set(RESET);

 // Basic Setup. (Refer to VS1053 DATASHEET.)
 decoder_w_reg(VS1053_REG_VOLUME, 0x3838); // Page 47.
 decoder_w_reg(VS1053_REG_CLOCKF, 0xE000); // Page 42.
 decoder_w_reg(VS1053_REG_BASS, 0x0806);   // Page 41.
}

void send_data(uint8_t data[], int size) {
 int counter = 0;
 int buffer_size = 0;
 gpio__reset(XDCS);
 while (counter < size) {
   if (gpio__get(DREQ)) {
     for (int i = 0; i < 32; i++) {
       ssp2__exchange_byte(data[counter++]);
     }
     // printf("%d %d\n", counter, data[counter]);
   }
 }
 gpio__set(XDCS);
}

// Write to a register on the VS1053.
void decoder_w_reg(uint8_t address, uint16_t data) {
 uint8_t buffer[2] = {0};
 buffer[0] = data;
 buffer[1] = data >> 8;
 gpio__reset(CS);
 ssp2__exchange_byte(VS1053_SCI_WRITE);
 ssp2__exchange_byte(address);
 ssp2__exchange_byte(buffer[1]);
 ssp2__exchange_byte(buffer[0]);
 gpio__set(CS);
 while (!gpio__get(DREQ))
   ;
}

// Read from a register on the VS1053
uint16_t decoder_r_reg(uint8_t address) {
 uint16_t buffer = 0;
 while (!gpio__get(DREQ))
   ;
 gpio__reset(CS);
 ssp2__exchange_byte(VS1053_SCI_READ);
 ssp2__exchange_byte(address);
 while (!gpio__get(DREQ))
   ;
 ssp2__exchange_byte(0xAA);
 buffer = ssp2__exchange_byte(0xAA);
 buffer = buffer << 8;
 // printf("MSB %d", buffer);
 while (!gpio__get(DREQ))
   ;
 buffer |= ssp2__exchange_byte(0xAA);
 while (!gpio__get(DREQ))
   ;
 gpio__set(CS);

 return buffer;
}
