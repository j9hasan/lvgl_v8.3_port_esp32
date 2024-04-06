
#ifndef _UI_H_
#define _UI_H_
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"
extern SemaphoreHandle_t xGuiSemaphore;
void ui_init();

#endif
