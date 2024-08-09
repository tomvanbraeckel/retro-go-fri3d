#include <rg_system.h>

#ifdef ESP_PLATFORM
#include <nvs_flash.h>
#include <esp_ota_ops.h>
#endif

void rg_mark_app_valid_cancel_rollback() {
    RG_LOGI("Marking this partition as valid so it doesn't rollback...");
    if (esp_ota_mark_app_valid_cancel_rollback()) {
        RG_LOGI("Marked as valid so it doesn't rollback.");
    } else {
        RG_LOGW("Failed to mark as valid, so it might rollback!");
    }
}

int32_t read_nvs_boot_partition() {
    nvs_handle_t my_handle;
    esp_err_t ret;
    int32_t boot_id = -1;

    // Open NVS partition 'fri3d.sys'
    ret = nvs_open("fri3d.sys", NVS_READONLY, &my_handle);
    if (ret != ESP_OK) {
        printf("Failed to open NVS partition: %s\n", esp_err_to_name(ret));
        goto cleanup_nothing;
    }

    // Get the value of 'boot_partition' from NVS
    ret = nvs_get_i32(my_handle, "boot_partition", &boot_id);
    if (ret != ESP_OK) {
        printf("Failed to read 'boot_partition': %s\n", esp_err_to_name(ret));
        goto close_nvs;
    }

    // Use the retrieved value of 'boot_partition'
    printf("Boot partition ID: %d\n", (int)boot_id);

close_nvs:
    nvs_close(my_handle);
cleanup_nothing:
    return boot_id;
}

// Boots into the Fri3d App on OTA 0/1, determined by the partition number that was saved by the Fri3d App
void rg_boot_fri3d_app() {
#ifdef ESP_PLATFORM
    RG_LOGI("Booting Fri3d App");
    char *label = "ota_0";

    // Choose the OTA partition that was saved by the main OTA0/1 Fri3d App previously
    int32_t ota_slot = read_nvs_boot_partition();
    if (ota_slot == 1)
        label = "ota_1";
    else
        RG_LOGW("read_nvs_boot_partition returned %d instead of 0 or 1, defaulting to 0...", (int)ota_slot);

    const esp_partition_t * next_update_partition = esp_partition_find_first(ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_ANY, label);
    if (!next_update_partition)
        RG_LOGE("Could not find partition with label '%s', can't set_boot_partition to Fri3d App!", label);

    RG_LOGI("Setting boot partition of type: %d, subtype: %d, address: %lu, size: %lu, label: %s", next_update_partition->type, next_update_partition->subtype, (uint32_t)next_update_partition->address, (uint32_t)next_update_partition->size, next_update_partition->label);
    esp_err_t err = esp_ota_set_boot_partition(next_update_partition);
    if (err != ESP_OK)
        RG_LOGE("esp_ota_set_boot_partition returned error %d, booting Fri3d App might not work!", err);

    RG_LOGI("Restarting...");
    rg_system_restart();
#else
    RG_LOGW("Booting Fri3d App is only supported on the ESP!");
#endif
}

