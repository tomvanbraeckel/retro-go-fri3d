#include <rg_system.h>
#include "gui.h"
#include "updater.h"

#ifdef RG_ENABLE_NETWORKING

#define RG_FIND_GAMES_PREFIX "http://"
#define RG_FIND_GAMES_API_SUFFIX "/api"

#define RG_FIND_GAMES_DOWNLOAD "http://192.168.4.1"
//#define RG_FIND_GAMES_API "http://192.168.1.16/api"
#define RG_FIND_GAMES_API "http://192.168.4.1/api"

#include <esp_http_client.h>
#include <malloc.h>
#include <string.h>
#include <cJSON.h>

#define NAMELENGTH 64

char status_msg[255];

/*
static bool download_file(const char *url, const char *filename)
{
    RG_ASSERT(url && filename, "bad param");

    rg_http_req_t *req = NULL;
    FILE *fp = NULL;
    void *buffer = NULL;
    int received = 0;
    int written = 0;
    int len;

    RG_LOGI("Downloading: '%s' to '%s'", url, filename);
    if (!(req = rg_network_http_open(url, NULL)))
    {
        rg_gui_alert("Download failed!", "Connection failed!");
        return false;
    }

    if (!(buffer = malloc(16 * 1024)))
    {
        rg_network_http_close(req);
        rg_gui_alert("Download failed!", "Out of memory!");
        return false;
    }

    if (!(fp = fopen(filename, "wb")))
    {
        rg_network_http_close(req);
        free(buffer);
        rg_gui_alert("Download failed!", "File open failed!");
        return false;
    }

    snprintf(buffer, RECEIVEBUFFER, "File: %s", filename);
    rg_gui_draw_dialog(buffer, NULL, 0);
    int content_length = req->content_length;

    while ((len = rg_network_http_read(req, buffer, 16 * 1024)) > 0)
    {
        received += len;
        written += fwrite(buffer, 1, len, fp);
        if (received != written)
            break; // No point in continuing
    }

    rg_network_http_close(req);
    free(buffer);
    fclose(fp);

    if (received != written || (received != content_length && content_length != -1))
    {
        rg_storage_delete(filename);
        rg_gui_alert("Download failed!", "Read/write error!");
        return false;
    }

    return true;
}
*/

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

    char list_api_url[50];
    snprintf(list_api_url, sizeof(list_api_url), "%s%s%s", RG_FIND_GAMES_PREFIX, ip, RG_FIND_GAMES_API_SUFFIX);

    while (stack != NULL) {
        const char *path = pop(&stack);
        if (path == NULL) {
            continue;
        }

        RG_LOGI("find_games_show_dialog with path: %s", path);

        cJSON *files_json = fetch_list_json(list_api_url, path);
        if (!files_json) {
            rg_gui_alert("File listing failed", "Make sure you are connected to a retro-go device's hotspot.");
            free((void *)path); // Free the duplicated path
            continue;
        }

        RG_LOGI("Creating folder: '%s'", path);
        rg_storage_mkdir(path);

        //char *string = cJSON_Print(files_json);
        //RG_LOGI("okay, files_json is: %s\n", string);
        //free(string); // Free the printed JSON string

        const cJSON *files = cJSON_GetObjectItemCaseSensitive(files_json, "files");
        const cJSON *file;
        cJSON_ArrayForEach(file, files) {
            char full_path[RG_PATH_MAX];

            cJSON *name = cJSON_GetObjectItemCaseSensitive(file, "name");
            cJSON *size = cJSON_GetObjectItemCaseSensitive(file, "size");
            cJSON *isdir = cJSON_GetObjectItemCaseSensitive(file, "is_dir");

            snprintf(full_path, sizeof(full_path), "%s/%s", path, name->valuestring);
            RG_LOGI("Found entry with full_path: %s", full_path);

            if (cJSON_IsString(name) && (name->valuestring != NULL))
                printf("%s,", name->valuestring);
            if (cJSON_IsNumber(size))
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
                    char full_url[RG_PATH_MAX];
                    char* full_path_without_first_slash = full_path + 1; // remove first character (/), otherwise the HTTP download fails
                    char* encoded_url = urlencode_without_free(full_path_without_first_slash);
                    if (encoded_url) {
                        snprintf(full_url, sizeof(full_url), "%s%s/%s", RG_FIND_GAMES_PREFIX, ip, encoded_url);
                        download_file(full_url, full_path);
                        free(encoded_url);
                    }
                }
            }
        }

        cJSON_Delete(files_json);
        free((void *)path); // Free the duplicated path
    }

    // is this necessary? gui_redraw();
}

#endif
