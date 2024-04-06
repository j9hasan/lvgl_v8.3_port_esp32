#include <stdio.h>
#include <esp_log.h>
#include "lvgl.h"
#include "ui.h"
#include "sd_fat.h"

void ui_init()
{
	// lv_obj_t *label = lv_label_create(lv_scr_act());
	// lv_obj_center(label);
	// lv_label_set_text(label, "HELLO");

	// LV_IMG_DECLARE(img_cogwheel_argb);
	lv_obj_t *img1 = lv_img_create(lv_scr_act());
	lv_img_set_src(img1, "S:/img/astrocfraw.bin");
	lv_obj_align(img1, LV_ALIGN_CENTER, 0, -20);
	// lv_obj_set_size(img1, 200, 200);

	// lv_obj_t *img2 = lv_img_create(lv_scr_act());
	// lv_img_set_src(img2, LV_SYMBOL_OK "Accept");
	// lv_obj_align_to(img2, img1, LV_ALIGN_OUT_BOTTOM_MID, 0, 20);
}
