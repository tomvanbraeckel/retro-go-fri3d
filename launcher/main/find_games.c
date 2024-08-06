#include <rg_system.h>
#include "gui.h"

#ifdef RG_ENABLE_NETWORKING

#define RG_FIND_GAMES_DOWNLOAD "http://192.168.4.1"
//#define RG_FIND_GAMES_API "http://192.168.1.16/api"
#define RG_FIND_GAMES_API "http://192.168.4.1/api"

#include <esp_http_client.h>
#include <malloc.h>
#include <string.h>
#include <cJSON.h>

#define NAMELENGTH 64

char status_msg[255];

typedef struct
{
    char name[NAMELENGTH];
    char url[256];
} asset_t;

typedef struct
{
    char name[NAMELENGTH];
    char date[32];
    asset_t *assets;
    size_t assets_count;
} release_t;

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
    rg_gui_draw_dialog("Connecting...", NULL, 0);

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

    rg_gui_draw_dialog("Receiving file...", NULL, 0);
    int content_length = req->content_length;

    while ((len = rg_network_http_read(req, buffer, 16 * 1024)) > 0)
    {
        received += len;
        written += fwrite(buffer, 1, len, fp);
        if (content_length>0)
            sprintf(buffer, "Received %d / %d of %s", received, content_length, filename);
        else
            sprintf(buffer, "Received %d bytes of %s", received, filename);
        rg_gui_draw_dialog(buffer, NULL, 0);
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

static cJSON *fetch_list_json(const char *url, const char *path)
{
    RG_ASSERT(url, "bad param");

    RG_LOGI("Fetching: '%s'", url);
    //rg_gui_draw_hourglass();

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

    //RG_LOGI("Got file listing: '%s'", buffer);
    RG_LOGI("Got file listing...");

    json = cJSON_Parse(buffer);

cleanup_buffer:
    free(buffer); // what if buffer was not initialized because rg_network_http_open() failed?
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

char* urlencode(char* originalText)
{
    // allocate memory for the worst possible case (all characters need to be encoded)
    char *encodedText = (char *)malloc(sizeof(char)*strlen(originalText)*3+1);

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
    return encodedText;
}


void find_games_show_dialog(const char *initial_path) {
    StackNode *stack = NULL;
    push(&stack, initial_path);

    while (stack != NULL) {
        const char *path = pop(&stack);
        if (path == NULL) {
            continue;
        }

        RG_LOGI("find_games_show_dialog with path: %s", path);

        cJSON *files_json = fetch_list_json(RG_FIND_GAMES_API, path);
        if (!files_json) {
            rg_gui_alert("File listing failed", "Make sure you are connected to a retro-go device's hotspot.");
            free((void *)path); // Free the duplicated path
            continue;
        }

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
                char full_url[RG_PATH_MAX];
                char* full_path_without_first_slash = full_path + 1; // remove first character (/)
                snprintf(full_url, sizeof(full_url), "%s/%s", RG_FIND_GAMES_DOWNLOAD, urlencode(full_path_without_first_slash));
                download_file(full_url, full_path);
            }
        }

        cJSON_Delete(files_json);
        free((void *)path); // Free the duplicated path
    }

    gui_redraw();
}

#endif

    /*
    if (releases_count > 0)
    {
        rg_gui_option_t options[releases_count + 1];
        rg_gui_option_t *opt = options;

        for (int i = 0; i < releases_count; ++i)
            *opt++ = (rg_gui_option_t){(intptr_t)&releases[i], releases[i].name, NULL, 1, &view_release_cb};
        *opt++ = (rg_gui_option_t)RG_DIALOG_END;

        rg_gui_dialog("Available Releases", options, 0);
    }
    else
    {
        rg_gui_alert("Available Releases", "Received empty list!");
    }

    for (int i = 0; i < releases_count; ++i)
        free(releases[i].assets);
    free(releases);
    */
