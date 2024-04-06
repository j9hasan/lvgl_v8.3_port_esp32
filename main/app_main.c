
#include "lvgl.h"
#include "lvgl_helpers.h"
#include "ui.h"
#include "sd_fat.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_timer.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if LV_USE_DEMO_WIDGETS
#include "lv_demos.h"
#endif

// Defines
#define LV_TICK_PERIOD_MS 10
#define MONITOR_HEAP 1
#define GUI_TASK_STACK_SIZE 5 * 1024
#define GUI_TASK_PRIORITY 3
#define GUI_TASK_CORE 1

// global vars
SemaphoreHandle_t xGuiSemaphore;

// Static Prototypes
static void lv_tick_task(void *arg);
static void ui_task(void *pvParameter);
static void heapCalc(void *pvParameter);
void my_log_cb(const char *buf);

void app_main()
{

#if MONITOR_HEAP
  xTaskCreatePinnedToCore(heapCalc, "printFreeHeap", 2 * 1024, NULL, 2, NULL, 1);
#endif

  initSD();

  xTaskCreatePinnedToCore(ui_task, "gui", GUI_TASK_STACK_SIZE, NULL, GUI_TASK_PRIORITY, NULL, GUI_TASK_CORE);

  // register logging
  lv_log_register_print_cb(my_log_cb);
}

static void ui_task(void *pvParameter)
{
  // (void)pvParameter;
  xGuiSemaphore = xSemaphoreCreateMutex();

  // initialize lvgl
  lv_init();

  // Initialize SPI or I2C bus used by the drivers
  lvgl_driver_init();

  // Initialize the working display buffers.
  static lv_color_t buf1[DISP_BUF_SIZE];
  static lv_color_t buf2[DISP_BUF_SIZE];

  // register diaplay driver
  static lv_disp_draw_buf_t disp_buf;
  uint32_t size_in_px = DISP_BUF_SIZE;

  lv_disp_draw_buf_init(&disp_buf, buf1, buf2, size_in_px);
  static lv_disp_drv_t disp_drv;
  lv_disp_drv_init(&disp_drv);
  disp_drv.draw_buf = &disp_buf;
  disp_drv.flush_cb = disp_driver_flush;
  disp_drv.hor_res = LV_HOR_RES_MAX;
  disp_drv.ver_res = LV_VER_RES_MAX;
  // disp_drv.rotated = 1;
  lv_disp_drv_register(&disp_drv);

  // register touch driver
  lv_indev_drv_t indev_drv;
  lv_indev_drv_init(&indev_drv);
  indev_drv.read_cb = touch_driver_read;
  indev_drv.type = LV_INDEV_TYPE_POINTER;
  lv_indev_drv_register(&indev_drv);

  // Create and start a periodic timer interrupt to call lv_tick_inc
  const esp_timer_create_args_t periodic_timer_args = {
      .callback = &lv_tick_task, .name = "periodic_gui"};
  esp_timer_handle_t periodic_timer;
  ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &periodic_timer));
  ESP_ERROR_CHECK(esp_timer_start_periodic(periodic_timer, LV_TICK_PERIOD_MS * 1000));

  // initialize ui and set ui initializes bit to safely access ui related stuff
  // ui_init();

  lv_demo_widgets();

  while (1)
  {
    vTaskDelay(pdMS_TO_TICKS(10));
    // take this semaphore to call lvgl related function on success
    if (pdTRUE == xSemaphoreTake(xGuiSemaphore, portMAX_DELAY))
    {
      lv_task_handler();
      xSemaphoreGive(xGuiSemaphore);
    }
  }
  vTaskDelete(NULL);
}

// lvgl timer task
static void lv_tick_task(void *arg)
{
  lv_tick_inc(LV_TICK_PERIOD_MS);
}

// print lvgl related log
void my_log_cb(const char *buf)
{
  printf(buf, strlen(buf));
  fflush(stdout);
}

static void heapCalc(void *pvParameter)
{
  while (1)
  {
    printf("Free heap size: %d Kb\n", esp_get_free_heap_size() / 1024);
    fflush(stdout);
    vTaskDelay(5000 / portTICK_PERIOD_MS);
  }
}
