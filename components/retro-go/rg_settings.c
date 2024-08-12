#include "rg_system.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <cJSON.h>

static cJSON *config_root = NULL;


static cJSON *json_root(const char *name, bool set_dirty)
{
    if (!config_root)
        return NULL;

    if (name == NS_GLOBAL)
        name = "global";
    else if (name == NS_APP)
        name = rg_system_get_app()->configNs;
    else if (name == NS_FILE)
        name = rg_basename(rg_system_get_app()->romPath);
    else if (name == NS_WIFI)
        name = "wifi";
    else if (name == NS_BOOT)
        name = "boot";

    cJSON *branch = cJSON_GetObjectItem(config_root, name);
    if (!branch)
    {
        branch = cJSON_AddObjectToObject(config_root, name);
        cJSON_AddStringToObject(branch, "namespace", name);
        cJSON_AddNumberToObject(branch, "changed", 0);

        void *data; size_t data_len;
        char pathbuf[RG_PATH_MAX];
        snprintf(pathbuf, RG_PATH_MAX, "%s/%s.json", RG_BASE_PATH_CONFIG, name);

        // Write config files with default values to ensure a smooth UX when plugging in an empty micro SD card
        rg_stat_t info = rg_storage_stat(pathbuf);
        RG_LOGI("file '%s' for '%s' exists: '%d' with length '%d'", pathbuf, name, info.exists, info.size);
        if (!info.exists || (info.size < 10)) {
            char* content = "";
            if (strcmp(name, "wifi") == 0) {
                content = "{\n\"Enable\": 0,\n\"ssid\": \"fri3d-badge\",\n\"password\": \"badge2024\",\n\"ssid0\": \"Zie /retro-go/config/wifi.json\",\n\"password0\": \"voorbeeld\",\n\"ssid1\": \"See /retro-go/config/wifi.json\",\n\"password1\": \"example\",\n\"ssid2\": \"retro-go\",\n\"password2\": \"retro-go\",\n\"ssid3\": \"retro-go-channel-1\",\n\"password3\": \"retro-go\",\n\"ssid4\": \"retro-go-channel-2\",\n\"password4\": \"retro-go\",\n\"ssid5\": \"retro-go-channel-3\",\n\"password5\": \"retro-go\",\n\"ssid6\": \"retro-go-channel-4\",\n\"password6\": \"retro-go\",\n\"ssid7\": \"retro-go-channel-5\",\n\"password7\": \"retro-go\",\n\"ssid8\": \"retro-go-channel-6\",\n\"password8\": \"retro-go\",\n\"ssid9\": \"retro-go-channel-7\",\n\"password9\": \"retro-go\",\n\"ssid10\": \"retro-go-channel-8\",\n\"password10\": \"retro-go\",\n\"ssid11\": \"retro-go-channel-9\",\n\"password11\": \"retro-go\",\n\"ssid12\": \"retro-go-channel-10\",\n\"password12\": \"retro-go\",\n\"ssid13\": \"retro-go-channel-11\",\n\"password13\": \"retro-go\",\n\"Slot\": -1\n}";
            } else if (strcmp(name, "gb") == 0) {
                content = "{\n\"Palette\": 35\n}";
            } else if (strcmp(name, "global") == 0) {
                content = "{\n\"FontType\":        5,\n\"Theme\":  null,\n\"DiskActivity\":        1,\n\"Timezone\":       \"UTC-01:00\",\n\"Migration\":  1390,\n\"AudioDriver\":  \"buzzer\",\n\"AudioDevice\":  0\n}";
            } else if (strcmp(name, "launcher") == 0) {
                content = "{\n\"SelectedTab\":     2,\n\"StartScreen\":    1,\n\"ShowPreview\":    1,\n\"ScrollMode\":     0,\n\"ColorTheme\":     1,\n\"StartupMode\":    0,\n\"HiddenTabs\":     \"\",\n\"HideTab.nes\": 0,\n\"HideTab.gb\":       0,\n\"HideTab.gbc\":    0,\n\"HideTab.doom\":   0,\n\"HideTab.favorite\":       0,\n\"HideTab.recent\": 0\n}";
            } else if (strcmp(name, "doom") == 0) {
                content = "{\n\"Gamma\":   1\n}";
            } else {
                RG_LOGW("Unrecognized config name, not writing default.");
            }
            if (strlen(content) > 0 && (rg_storage_write_file(pathbuf, content, strlen(content), 0) ||
                (rg_storage_mkdir(rg_dirname(pathbuf)) && rg_storage_write_file(pathbuf, content, strlen(content), 0)))) {
                RG_LOGW("Failed to write default config file!");
            }
        }

        if (rg_storage_read_file(pathbuf, &data, &data_len, 0))
        {
            cJSON *values = cJSON_Parse((char *)data);
            if (values)
            {
                RG_LOGI("Config file loaded: '%s'", pathbuf);
                cJSON_AddItemToObject(branch, "values", values);
            }
            else
                RG_LOGE("Config file parsing failed: '%s'", pathbuf);
            free(data);
        }
    }

    cJSON *root = cJSON_GetObjectItem(branch, "values");
    if (!cJSON_IsObject(root))
    {
        cJSON_DeleteItemFromObject(branch, "values");
        root = cJSON_AddObjectToObject(branch, "values");
    }

    if (set_dirty)
    {
        cJSON_SetNumberHelper(cJSON_GetObjectItem(branch, "changed"), 1);
    }

    return root;
}

static void update_value(const char *section, const char *key, cJSON *new_value)
{
    cJSON *obj = cJSON_GetObjectItem(json_root(section, 0), key);
    if (obj == NULL)
        cJSON_AddItemToObject(json_root(section, 1), key, new_value);
    else if (!cJSON_Compare(obj, new_value, true))
        cJSON_ReplaceItemInObject(json_root(section, 1), key, new_value);
    else
        cJSON_Delete(new_value);
}

void rg_settings_init(void)
{
    config_root = cJSON_CreateObject();
    json_root(NS_GLOBAL, 0);
    json_root(NS_BOOT, 0);
}

void rg_settings_commit(void)
{
    if (!config_root)
        return;

    for (cJSON *branch = config_root->child; branch; branch = branch->next)
    {
        char *name = cJSON_GetStringValue(cJSON_GetObjectItem(branch, "namespace"));
        int changed = cJSON_GetNumberValue(cJSON_GetObjectItem(branch, "changed"));

        if (!changed)
            continue;

        char *buffer = cJSON_Print(cJSON_GetObjectItem(branch, "values"));
        if (!buffer)
            continue;

        size_t buffer_len = strlen(buffer) + 1;

        char pathbuf[RG_PATH_MAX];
        snprintf(pathbuf, RG_PATH_MAX, "%s/%s.json", RG_BASE_PATH_CONFIG, name);
        if (rg_storage_write_file(pathbuf, buffer, buffer_len, 0) ||
            (rg_storage_mkdir(rg_dirname(pathbuf)) && rg_storage_write_file(pathbuf, buffer, buffer_len, 0)))
        {
            cJSON_SetNumberHelper(cJSON_GetObjectItem(branch, "changed"), 0);
        }

        cJSON_free(buffer);
    }

    rg_storage_commit();
}

void rg_settings_reset(void)
{
    RG_LOGI("Clearing settings...\n");
    rg_storage_delete(RG_BASE_PATH_CONFIG);
    rg_storage_mkdir(RG_BASE_PATH_CONFIG);
    if (config_root)
    {
        cJSON_Delete(config_root);
        config_root = cJSON_CreateObject();
    }
}

double rg_settings_get_number(const char *section, const char *key, double default_value)
{
    cJSON *obj = cJSON_GetObjectItem(json_root(section, 0), key);
    return obj ? obj->valuedouble : default_value;
}

void rg_settings_set_number(const char *section, const char *key, double value)
{
    update_value(section, key, cJSON_CreateNumber(value));
}

char *rg_settings_get_string(const char *section, const char *key, const char *default_value)
{
    cJSON *obj = cJSON_GetObjectItem(json_root(section, 0), key);
    if (cJSON_IsString(obj))
        return strdup(obj->valuestring);
    return default_value ? strdup(default_value) : NULL;
}

void rg_settings_set_string(const char *section, const char *key, const char *value)
{
    update_value(section, key, value ? cJSON_CreateString(value) : cJSON_CreateNull());
}

void rg_settings_delete(const char *section, const char *key)
{
    cJSON_DeleteItemFromObject(json_root(section, 1), key);
}
