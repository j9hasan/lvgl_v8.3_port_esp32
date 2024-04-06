/* SD card and FAT filesystem example.
   This example uses SPI peripheral to communicate with SD card.

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include "esp_system.h"
#include <esp_log.h>
#include "esp_event.h"
#include "driver/gpio.h"
#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"
#include "lvgl.h"
#include <dirent.h>
#include <string.h>
#include "ui.h"

char **listFilesAndFolders(const char *path, int *numFiles);

/* error status */
esp_err_t sd_card_error = -1;

#define MOUNT_POINT "/S"

// Pin assignments can be set in menuconfig, see "SD SPI Example Configuration" menu.
// You can also change the pin assignments here by changing the following 4 lines.
#define PIN_NUM_MISO 19
#define PIN_NUM_MOSI 23
#define PIN_NUM_CLK 18
#define PIN_NUM_CS 5

void initSD()
{
    // Options for mounting the filesystem.
    // If format_if_mount_failed is set to true, SD card will be partitioned and
    // formatted in case when mounting fails.
    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
#ifdef CONFIG_EXAMPLE_FORMAT_IF_MOUNT_FAILED
        .format_if_mount_failed = true,
#else
        .format_if_mount_failed = false,
#endif // EXAMPLE_FORMAT_IF_MOUNT_FAILED
        .max_files = 5,
        .allocation_unit_size = 16 * 1024};
    sdmmc_card_t *card;
    const char mount_point[] = MOUNT_POINT;
    printf("Initializing SD card");

    printf("Using SPI peripheral");

    sdmmc_host_t host = {
        .flags = SDMMC_HOST_FLAG_SPI | SDMMC_HOST_FLAG_DEINIT_ARG,
        .slot = VSPI_HOST,
        .max_freq_khz = SDMMC_FREQ_DEFAULT,
        .io_voltage = 3.3f,
        .init = &sdspi_host_init,
        .set_bus_width = NULL,
        .get_bus_width = NULL,
        .set_bus_ddr_mode = NULL,
        .set_card_clk = &sdspi_host_set_card_clk,
        .set_cclk_always_on = NULL,
        .do_transaction = &sdspi_host_do_transaction,
        .deinit_p = &sdspi_host_remove_device,
        .io_int_enable = &sdspi_host_io_int_enable,
        .io_int_wait = &sdspi_host_io_int_wait,
        .command_timeout_ms = 0,
    };
    spi_bus_config_t bus_cfg = {
        .mosi_io_num = PIN_NUM_MOSI,
        .miso_io_num = PIN_NUM_MISO,
        .sclk_io_num = PIN_NUM_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 4000,
    };
    sd_card_error = spi_bus_initialize(host.slot, &bus_cfg, VSPI_HOST);
    if (sd_card_error != ESP_OK)
    {
        printf("Failed to initialize bus.");
        return;
    }

    // This initializes the slot without card detect (CD) and write protect (WP) signals.
    // Modify slot_config.gpio_cd and slot_config.gpio_wp if your board has these signals.
    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_config.gpio_cs = PIN_NUM_CS;
    slot_config.host_id = host.slot;

    printf("Mounting filesystem");
    sd_card_error = esp_vfs_fat_sdspi_mount(mount_point, &host, &slot_config, &mount_config, &card);

    if (sd_card_error != ESP_OK)
    {
        if (sd_card_error == ESP_FAIL)
        {
            printf("Failed to mount filesystem. "
                   "If you want the card to be formatted, set the CONFIG_EXAMPLE_FORMAT_IF_MOUNT_FAILED menuconfig option.");
            return;
        }
        else
        {
            printf("Failed to initialize the card (%s). "
                   "Make sure SD card lines have pull-up resistors in place.",
                   esp_err_to_name(sd_card_error));
            return;
        }
    }
    printf("Filesystem mounted");

    /* set icon */
    // lv_label_set_text(ui_sd_icon, LV_SYMBOL_SD_CARD);

    // Card has been initialized, print its properties
    sdmmc_card_print_info(stdout, card);
    // deinitialize the bus after all devices are removed
    // spi_bus_free(host.slot);
}

char **listFilesAndFolders(const char *path, int *numFiles)
{
    DIR *dir = opendir(path);
    struct dirent *entry;
    if (dir == NULL)
    {
        printf("Dir is NULL\n");
        fflush(stdout);
        return NULL;
    }

    // Count the number of files in the directory
    *numFiles = 0;
    while ((entry = readdir(dir)) != NULL)
    {
        (*numFiles)++;
    }
    closedir(dir);

    // Open the directory again to read filenames
    dir = opendir(path);
    if (dir == NULL)
    {
        printf("Unable to open directory\n");
        fflush(stdout);
        return NULL;
    }

    // Allocate memory for the array of strings
    char **fileArray = (char **)malloc(*numFiles * sizeof(char *));
    if (fileArray == NULL)
    {
        printf("fileArray: Memory allocation error\n");
        closedir(dir);
        fflush(stdout);
        return NULL;
    }

    // Read filenames and copy them to the array
    int i = 0;
    while ((entry = readdir(dir)) != NULL)
    {
        fileArray[i] = strdup(entry->d_name);
        printf("%s\n", fileArray[i]);
        fflush(stdout);
        i++;
    }

    closedir(dir);
    return fileArray;
}
/* file io example */
// Use POSIX and C standard library functions to work with files.
// First create a file.
// const char *file_hello = MOUNT_POINT "/hello.txt";

// printf( "Opening file %s", file_hello);
// FILE *f = fopen(file_hello, "w");
// if (f == NULL)
// {
//     ESP_LOGE(TAG, "Failed to open file for writing");
//     return;
// }
// fprintf(f, "Hello %s!\n", card->cid.name);
// fclose(f);
// printf( "File written");

// const char *file_foo = MOUNT_POINT "/foo.txt";

// // Check if destination file exists before renaming
// struct stat st;
// if (stat(file_foo, &st) == 0)
// {
//     // Delete it if it exists
//     unlink(file_foo);
// }

// // Rename original file
// printf( "Renaming file %s to %s", file_hello, file_foo);
// if (rename(file_hello, file_foo) != 0)
// {
//     ESP_LOGE(TAG, "Rename failed");
//     return;
// }

// // Open renamed file for reading
// printf( "Reading file %s", file_foo);
// f = fopen(file_foo, "r");
// if (f == NULL)
// {
//     ESP_LOGE(TAG, "Failed to open file for reading");
//     return;
// }

// // Read a line from file
// char line[64];
// fgets(line, sizeof(line), f);
// fclose(f);

// // Strip newline
// char *pos = strchr(line, '\n');
// if (pos)
// {
//     *pos = '\0';
// }
// printf( "Read from file: '%s'", line);

// All done, unmount partition and disable SPI peripheral
// esp_vfs_fat_sdcard_unmount(mount_point, card);
// printf( "Card unmounted");
