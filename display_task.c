#include "ff.h"

static char playlist[64][32];
static uint8_t counter = 0;

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

  for (uint8_t i = 0; i < counter; i++) {

    vTaskDelay(1);

    // printf("%s \n", playlist[i]); // debug

    print_str(playlist[i]);

    vTaskDelay(5000);

    clear();

    vTaskDelay(1);

    home();

  }

  while (1) {

    vTaskDelay(1000); // todo replace with queue for new things to display

  }

}