#include <rg_system.h>
#include <string.h>

#ifdef ESP_PLATFORM
// For storing previous app partition
#include "esp_flash_partitions.h"
#include "bootloader_common.h"
#include "esp_err.h"
#include "esp_partition.h"
#include "esp_ota_ops.h"
#include <esp_spi_flash.h>
#endif

static const char *SETTING_MICROPYTHON_OTA_SLOT = "MicroPython_latest_OTA_partition_subtype";

// Read otadata partition and fill array from two otadata structures.
// Also return pointer to otadata info partition.
static const esp_partition_t *read_otadata(esp_ota_select_entry_t *two_otadata)
{
    const esp_partition_t *otadata_partition = esp_partition_find_first(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_DATA_OTA, NULL);

    if (otadata_partition == NULL) {
        printf("not found otadata");
        return NULL;
    }

    //esp_partition_mmap_handle_t ota_data_map;
    spi_flash_mmap_handle_t ota_data_map;
    const void *result = NULL;
    //esp_err_t err = esp_partition_mmap(otadata_partition, 0, otadata_partition->size, ESP_PARTITION_MMAP_DATA, &result, &ota_data_map);
    esp_err_t err = esp_partition_mmap(otadata_partition, 0, otadata_partition->size, SPI_FLASH_MMAP_DATA, &result, &ota_data_map);
    if (err != ESP_OK) {
        printf("mmap otadata filed. Err=0x%8x", err);
        return NULL;
    } else {
        memcpy(&two_otadata[0], result, sizeof(esp_ota_select_entry_t));
        memcpy(&two_otadata[1], result + SPI_FLASH_SEC_SIZE, sizeof(esp_ota_select_entry_t));
        //esp_partition_munmap(ota_data_map);
        spi_flash_munmap(ota_data_map);
    }
    return otadata_partition;
}


static uint8_t get_ota_partition_count(void)
{
    uint16_t ota_app_count = 0;
    while (esp_partition_find_first(ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_APP_OTA_MIN + ota_app_count, NULL) != NULL) {
            assert(ota_app_count < 16 && "must erase the partition before writing to it");
            ota_app_count++;
    }
    return ota_app_count;
}

// Inspired by esp_ota_erase_last_boot_app_partition:
esp_err_t esp_ota_store_last_boot_app_partition(void)
{
    esp_ota_select_entry_t otadata[2];
    const esp_partition_t* ota_data_partition = read_otadata(otadata);
    if (ota_data_partition == NULL) {
        return ESP_FAIL;
    }

    int active_otadata = bootloader_common_get_active_otadata(otadata);
    int ota_app_count = get_ota_partition_count();
    if (active_otadata == -1 || ota_app_count == 0) {
        return ESP_FAIL;
    }

    int inactive_otadata = (~active_otadata)&1;
    if (otadata[inactive_otadata].ota_seq == UINT32_MAX || otadata[inactive_otadata].crc != bootloader_common_ota_select_crc(&otadata[inactive_otadata])) {
        return ESP_FAIL;
    }

    int ota_slot = (otadata[inactive_otadata].ota_seq - 1) % ota_app_count; // Actual OTA partition selection
    ota_slot += ESP_PARTITION_SUBTYPE_APP_OTA_MIN;
    RG_LOGW("Found last_boot_app_partition ota_%d app...", ota_slot);

    const esp_partition_t* last_boot_app_partition_from_otadata = esp_partition_find_first(ESP_PARTITION_TYPE_APP, ota_slot, NULL);
    if (last_boot_app_partition_from_otadata == NULL) {
        RG_LOGW("Not storing ota_slot %d because no partition was found with that subtype.", ota_slot);
        return ESP_FAIL;
    }

    const esp_partition_t* running_partition = esp_ota_get_running_partition();
    if (running_partition == NULL || last_boot_app_partition_from_otadata == running_partition) {
        RG_LOGW("Not storing ota_slot %d because it equals the current running partition, not previous one.", ota_slot);
        return ESP_FAIL;
    }

    // Only store it if 0 or 1
    if (ota_slot != ESP_PARTITION_SUBTYPE_APP_OTA_0 && ota_slot != ESP_PARTITION_SUBTYPE_APP_OTA_1) {
        RG_LOGW("Not storing ota_slot %d because it's not OTA_0 or OTA_1!", ota_slot);
        return ESP_FAIL;
    }

    // store ota_slot in a file for later retrieval
    rg_settings_set_number(NS_GLOBAL, SETTING_MICROPYTHON_OTA_SLOT, ota_slot);
    rg_settings_commit();

    // Mark this partition as valid so it doesn't rollback
    esp_ota_mark_app_valid_cancel_rollback();
    RG_LOGW("Marked retro-go partition as valid so it doesn't rollback.");

    return ESP_OK;
}

// Boots into MicroPython, using OTA 0 or 1 partition number that was saved when first booting from MicroPython into retro-go
void rg_system_boot_micropython() {
#ifdef ESP_PLATFORM
    RG_LOGI("Switching to MicroPython");
    // Choose the OTA partition that was saved by the launcher previously when we booted from MicroPython
    int ota_slot = rg_settings_get_number(NS_GLOBAL, SETTING_MICROPYTHON_OTA_SLOT, ESP_PARTITION_SUBTYPE_APP_OTA_0);
    RG_LOGW("got ota_slot from NS_GLOBAL: %d", ota_slot);

    if (ota_slot != ESP_PARTITION_SUBTYPE_APP_OTA_0 && ota_slot != ESP_PARTITION_SUBTYPE_APP_OTA_1) {
        RG_LOGW("Not using ota_slot %d because it's not OTA_0 or OTA_1. Defaulting to OTA_0.", ota_slot);
        ota_slot = ESP_PARTITION_SUBTYPE_APP_OTA_0;
    }

    const esp_partition_t * next_update_partition2 = esp_partition_find_first(ESP_PARTITION_TYPE_APP, ota_slot, NULL);
    //RG_LOGI("Setting boot partition of type: %d, subtype: %d, address: %lu, size: %lu, label: %s", next_update_partition2->type, next_update_partition2->subtype, (uint32_t)next_update_partition2->address, (uint32_t)next_update_partition2->size, next_update_partition2->label);
    esp_err_t err = esp_ota_set_boot_partition(next_update_partition2);
    if (err != ESP_OK)
    {
        RG_LOGE("esp_ota_set_boot_partition returned 0x%02X!\n", err);
        RG_PANIC("Unable to set boot app!");
    }
    RG_LOGI("Booting into MicroPython...");
    //rg_task_delay(1000*15);
    rg_system_restart();
#else
    RG_LOGW("Switching to MicroPython is only supported on the ESP!");
#endif
}

