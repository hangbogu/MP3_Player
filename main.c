#include "FreeRTOS.h"
#include "board_io.h"
#include "cli_handlers.h"
#include "common_macros.h"
#include "delay.h"
#include "display.h"
#include "ff.h"
#include "gpio.h"
#include "gpio_isr.h"
#include "mp3_decoder.h"
#include "queue.h"
#include "semphr.h"
#include "sj2_cli.h"
#include "string.h"
#include "task.h"
#include "uart.h"
#include "uart_printf.h"

#include "ff.h"

static char playlist[64][32];
static uint8_t counter = 0;
static void display_task(void *params);

static void blink_task(void *params);
static void uart_task(void *params);
typedef char songname[32];
xQueueHandle Q_songname = NULL;
xQueueHandle Q_songname_buttom = NULL;
xQueueHandle Q_songdata;

static gpio_s ex_sw, ex_led;

bool play = true;
uint8_t left, right;
bool paused;

const char *get_filename_ext(const char *filename) {

  const char *dot = strrchr(filename, '.');

  if (!dot || dot == filename)

    return "";

  return dot + 1;
}

static void display_task(void *p) {

  DIR directory;

  FILINFO fno;

  lcd_init();

  vTaskDelay(5);

  FRESULT o_result = f_opendir(&directory, "");

  vTaskDelay(1000);

  if (FR_OK == o_result) {

    FRESULT f_result = f_readdir(&directory, &fno);

    while ('\0' != fno.fname[0]) {

      f_result = f_readdir(&directory, &fno);

      if (FR_OK == f_result)

        if (!strcmp(get_filename_ext(fno.fname), "mp3")) {

          // vTaskDelay(1);

          if (fno.fname[0] != '.' && fno.fname[1] != '_') {

            strcpy(playlist[counter], fno.fname);

            counter++;
          }

          // print_str(fno.fname);
        }
    }
  }

  FRESULT c_result = f_closedir(&directory);

  // for (uint8_t i = 0; i < counter; i++) {

  int counter_display = 0;

  vTaskDelay(1);

  // printf("%s \n", playlist[i]); // debug

  print_str(playlist[0]);

  //}

  while (1) {

    vTaskDelay(1000); // todo replace with queue for new things to display
    if (!play) {
      clear();
      vTaskDelay(1);
      home();
      vTaskDelay(1);
      print_str(playlist[counter_display]);
      xQueueSend(Q_songname_buttom, &playlist[counter_display], portMAX_DELAY);
      vTaskDelay(100);
      play = true;
      paused = false;
    }
  }
}

// void ex_isr(void) {
//   ex_led = gpio__construct_as_output(2, 4);
//   gpio__toggle(ex_led);
//   uart_printf__polled(UART__0, "INT from Port 2 Pin 1. (Extenal SW)\n");
// }

void play_pause_isr(void) {
  play = !play;
  // uart_printf__polled(UART__0, "play_pause_isr\n");
}

void vol_up_isr(void) {
  ex_led = gpio__construct_as_output(2, 4);
  gpio__toggle(ex_led);
  uart_printf__polled(UART__0, "INT from Port 2 Pin 1. (Extenal SW)\n");
}

void vol_down_isr(void) {
  ex_led = gpio__construct_as_output(2, 4);
  gpio__toggle(ex_led);
  uart_printf__polled(UART__0, "INT from Port 2 Pin 1. (Extenal SW)\n");
}

void next_isr(void) {
  ex_led = gpio__construct_as_output(2, 4);
  gpio__toggle(ex_led);
  uart_printf__polled(UART__0, "INT from Port 2 Pin 1. (Extenal SW)\n");
}

void prev_isr(void) {
  ex_led = gpio__construct_as_output(2, 4);
  gpio__toggle(ex_led);
  uart_printf__polled(UART__0, "INT from Port 2 Pin 1. (Extenal SW)\n");
}

// Reader tasks receives song-name over Q_songname to start reading it
void mp3_reader_task(void *p) {
  songname name;
  // char bytes_512[512];
  UINT byte_read = 0;

  while (1) {
    vTaskDelay(1000);
    // const char *name = "file.txt";
    // xQueueReceive(Q_songname, &name[0], portMAX_DELAY);
    xQueueReceive(Q_songname_buttom, &name[0], portMAX_DELAY);
    printf("Received song to play: %s\n", name);

    FIL file;
    FRESULT o_result = f_open(&file, name, FA_READ);
    // open_file();
    if (FR_OK == o_result) {
      uint32_t size;
      size = f_size(&file);
      printf("Size %d\n", size);
      do {
        char read_buffer[512] = {0};
        // read_from_file(bytes_512);
        FRESULT r_result;
        if (size > 511) {
          r_result = f_read(&file, read_buffer, 512, &byte_read);
        } else {
          r_result = f_read(&file, read_buffer, size, &byte_read);
        }
        // printf("Read %d bytes\n", byte_read);

        if (FR_OK == r_result) {
          // printf("Send %s to the song data queue.\n", bytes_512);
          // printf("R: %X\n", read_buffer[0]);
          xQueueSend(Q_songdata, &read_buffer[0], portMAX_DELAY);
          // send_data(read_buffer, 512);
          // printf("Successfully read the file, %s\n", name);
          // printf("Read %s\n", bytes_512);
          size = size - byte_read;
        } else {
          printf("Failed to read the file, %s\n", name);
        }
      } while (size > 0);
    } else {
      printf("Failed to open the file, %s\n", name);
    }

    f_close(&file);
    vTaskDelay(500);
  }
}

void spi_send_to_mp3_decoder();
bool paused = false;
// Player task receives song data over Q_songdata to send it to the MP3 decoder
void mp3_player_task(void *p) {
  char play_buffer[512];

  while (1) {
    xQueueReceive(Q_songdata, &play_buffer[0], portMAX_DELAY);
    // printf("P: %X\n", play_buffer[0]);
    // printf("Received from queue.\n");
    // for (int i = 0; i < sizeof(bytes_512); i++) {
    // while (!mp3_decoder_needs_data()) {
    // vTaskDelay(1);
    //}

    // spi_send_to_mp3_decoder(bytes_512[i]);
    send_data(play_buffer, 512);

    while (!play) {

      // right = right - 10;
      // left = left - 10;
      // taskENTER_CRITICAL();
      // volume_control(left, right);
      // taskEXIT_CRITICAL();
      // uart_printf__polled(UART__0, "Volume: %d\n", left);
      // // printf("Volume: %d", volume);
      // play = !play;

      if (paused == false) {
        clear();
        vTaskDelay(5);
        home();
        vTaskDelay(5);
        print_str("Paused ! ! !");
        paused = true;
      }
      vTaskDelay(100);
    }

    // printf("%s\n", bytes_512);
    //}
  }
}

int main(void) {
  Q_songname_buttom = xQueueCreate(1, sizeof(songname));
  Q_songdata = xQueueCreate(1, 512);

  // volume = 1;
  left = 56;
  right = 56;
  paused = false;

  NVIC_EnableIRQ(GPIO_IRQn);
  gpio__attach_interrupt(2, 6, GPIO_INTR__RISING_EDGE, play_pause_isr);

  printf("Before mp3.\n");
  mp3_initialization();
  printf("After mp3.\n");

  xTaskCreate(display_task, "display", 512, NULL, 3, NULL);
  xTaskCreate(mp3_player_task, "player", 512, NULL, 2, NULL);
  xTaskCreate(mp3_reader_task, "reader", 512, NULL, 1, NULL);

  sj2_cli__init();

  puts("Starting RTOS");
  vTaskStartScheduler(); // This function never returns unless RTOS scheduler runs out of memory and fails

  return 0;
}
