#include <rg_system.h>
#include "gui.h"
#include "updater.h"

#ifdef RG_ENABLE_NETWORKING

#define RG_FIND_GAMES_PREFIX "http://"
#define RG_FIND_GAMES_API_SUFFIX "/api"

#define RG_FIND_GAMES_DOWNLOAD "http://192.168.4.1"
#define RG_FIND_GAMES_API "http://192.168.4.1/api"

#include <esp_http_client.h>
#include <malloc.h>
#include <string.h>
#include <cJSON.h>

static cJSON *fetch_list_json(const char *url, const char *path)
{
    RG_ASSERT(url, "bad param");

    RG_LOGI("Fetching: '%s'", url);

    rg_http_req_t *req = NULL;
    char *buffer = NULL;
    cJSON *json = NULL;

    rg_http_cfg_t http_cfg;

    if (!(http_cfg.post_data = (char*)malloc(1024)))
        goto cleanup_nothing;

    snprintf(http_cfg.post_data, 1024, "{\"cmd\":\"list\",\"arg1\":\"%s\",\"arg2\":\"\"}", path);
    RG_LOGI("http_cfg.post_data = %s", http_cfg.post_data);

    if (!(req = rg_network_http_open(url, &http_cfg)))
        goto cleanup_post_data;

    size_t buffer_length = RG_MAX(256 * 1024, req->content_length);

    if (!(buffer = calloc(1, buffer_length + 1)))
        goto cleanup_http_open;

    if (rg_network_http_read(req, buffer, buffer_length) < 16)
        goto cleanup_buffer;

    RG_LOGI("Got file listing...");

    json = cJSON_Parse(buffer);

cleanup_buffer:
    free(buffer);
cleanup_http_open:
    rg_network_http_close(req);
cleanup_post_data:
    free(http_cfg.post_data);
cleanup_nothing:
    return json;
}

typedef struct StackNode {
    const char *path;
    struct StackNode *next;
} StackNode;

void push(StackNode **stack, const char *path) {
    StackNode *new_node = (StackNode *)malloc(sizeof(StackNode));
    if (new_node == NULL) {
        RG_LOGE("Failed to allocate memory for stack node");
        return;
    }
    new_node->path = strdup(path);
    new_node->next = *stack;
    *stack = new_node;
}

const char *pop(StackNode **stack) {
    if (*stack == NULL) {
        return NULL;
    }
    StackNode *temp = *stack;
    const char *path = temp->path;
    *stack = (*stack)->next;
    free(temp);
    return path;
}

char* urlencode_without_free(char* originalText)
{
    // allocate memory for the worst possible case (all characters need to be encoded)
    char *encodedText = (char *)malloc(sizeof(char)*strlen(originalText)*3+1);
    if (!encodedText) {
        RG_LOGW("Could not allocate memory for urlencode!");
        goto bailout;
    }

    const char *hex = "0123456789abcdef";

    int pos = 0;
    for (int i = 0; i < strlen(originalText); i++) {
        if (('a' <= originalText[i] && originalText[i] <= 'z')
            || ('A' <= originalText[i] && originalText[i] <= 'Z')
            || ('0' <= originalText[i] && originalText[i] <= '9')) {
                encodedText[pos++] = originalText[i];
            } else {
                encodedText[pos++] = '%';
                encodedText[pos++] = hex[originalText[i] >> 4];
                encodedText[pos++] = hex[originalText[i] & 15];
            }
    }
    encodedText[pos] = '\0';
bailout:
    return encodedText;
}

void find_games(const char *initial_path, const char* ip) {
    StackNode *stack = NULL;
    push(&stack, initial_path);

    char list_api_url[50]; // http://192.168.4.1/api
    snprintf(list_api_url, sizeof(list_api_url), "%s%s%s", RG_FIND_GAMES_PREFIX, ip, RG_FIND_GAMES_API_SUFFIX);

    while (stack != NULL) {
        const char *path = pop(&stack);
        if (path == NULL) {
            goto continue_without_cleanup;
        }

        RG_LOGI("find_games checking path: %s", path);

        cJSON *files_json = fetch_list_json(list_api_url, path);
        if (!files_json) {
            rg_gui_alert("File listing failed", "Make sure you are connected to a retro-go device's hotspot.");            
            goto continue_cleanup_path;
        }

        RG_LOGI("Creating folder: '%s'", path);
        rg_storage_mkdir(path);

        /*
        char *string = cJSON_Print(files_json);
        printf("printf files_json is: %s\n", string); // use printf because RG_LOGI() truncates the output
        free(string); // Free the printed JSON string
        */

        const cJSON *files = cJSON_GetObjectItemCaseSensitive(files_json, "files");
        const cJSON *file;
        cJSON_ArrayForEach(file, files) {
            char full_path[RG_PATH_MAX*3];

            cJSON *name = cJSON_GetObjectItemCaseSensitive(file, "name");
            cJSON *size = cJSON_GetObjectItemCaseSensitive(file, "size");
            cJSON *isdir = cJSON_GetObjectItemCaseSensitive(file, "is_dir");

            if (!(cJSON_IsString(name) && (name->valuestring != NULL)) || !cJSON_IsNumber(size) || !cJSON_IsBool(isdir))
            {
                RG_LOGW("Invalid JSON received, skipping...");
                goto continue_cleanup_files_json;
            }

            snprintf(full_path, sizeof(full_path), "%s/%s", path, name->valuestring);
            RG_LOGI("Found entry with full_path: %s", full_path);

            if (strlen(full_path) > 128) {
                // Dirty workaround, I know, but what else can you do after spending a whole day trying to determine the cause of a crash!
                // It usually only happens inside a huge "collections" folder and for long filenames:
                // /sd/roms/gbc/collection/Hacks/Pokemon Blue Version - Colorization (Pokemon Blue Version modification) (1.2) 012345678901234567890123456789.gb
                RG_LOGW("Skipping paths longer than 128 bytes because they seem to cause undetermined crashes...");
            } else {
                printf("%s,", name->valuestring);
                printf("%d,", size->valueint);
                if (cJSON_IsTrue(isdir)) {
                    printf("true\n");
                    push(&stack, full_path);
                } else {
                    printf("false\n");
                    rg_stat_t info = rg_storage_stat(full_path);
                    if (info.exists && (info.size == size->valueint)) { // also check that it's not a directory? info.is_dir?
                        RG_LOGI("Skipping download because file already exists and is correct size!");
                    } else {
                        RG_LOGI("File to download has size: %d", (size->valueint));
                        int64_t freespace = rg_storage_get_free_space(full_path);
                        RG_LOGI("Free space: %lu\n", (unsigned long int)freespace);
                        if (freespace < 2*(size->valueint)) {
                            RG_LOGW("Storage too full, skipping!");
                        } else {
                            char full_url[RG_PATH_MAX*3]; // needs more than RG_PATH_MAX because of the http://......./ prefix
                            char* full_path_without_first_slash = full_path + 1; // remove first character (/), otherwise the HTTP download fails
                            char* encoded_url = urlencode_without_free(full_path_without_first_slash);
                            if (encoded_url) {
                                snprintf(full_url, sizeof(full_url), "%s%s/%s", RG_FIND_GAMES_PREFIX, ip, encoded_url);
                                download_file(full_url, full_path);
                                free(encoded_url);
                            }
                        } // free space check
                    } // file exists check
                } // is file or directory check
            } // strlen(full_path) check
        } // cJSON_ArrayForEach

continue_cleanup_files_json:
        cJSON_Delete(files_json);
continue_cleanup_path:
        free((void *)path); // Free the duplicated path
continue_without_cleanup:
    }

    RG_LOGI("find_games returning");
}

#endif
