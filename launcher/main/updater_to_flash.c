#include <rg_system.h>
#include <esp_flash.h>

#ifdef RG_ENABLE_NETWORKING
#include <malloc.h>
#include <string.h>
#include <cJSON.h>

bool download_file_to_flash(const char *url, const char *filename)
{
    RG_ASSERT(url && filename, "bad param");

    rg_http_req_t *req = NULL;
    void *buffer = NULL;
    int received = 0;
    int written = 0;
    int len;

    RG_LOGI("Downloading: '%s' to internal flash at offset 0, thus overwriting everything. Don't interrupt this process!", url);
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

    rg_gui_draw_dialog("Receiving file...", NULL, 0);
    int content_length = req->content_length;

    while ((len = rg_network_http_read(req, buffer, 16 * 1024)) > 0)
    {
	esp_err_t err = esp_flash_erase_region(NULL, received, len); // start and len must be multiple of chip->drv->sector_size field (typically 4096 bytes)
	if (err)
            RG_LOGW("esp_flash_erase_region(NULL,%d,%d) returned %d", received, len, err);
        err = esp_flash_write(NULL, buffer, received, len);
	if (err)
            RG_LOGW("esp_flash_write(NULL,%d,%d) returned %d", received, len, err);

        received += len;
        written += len;
        RG_LOGI("Update download received total of %d bytes ", received);
        sprintf(buffer, "Received %d / %d", received, content_length);
        rg_gui_draw_dialog(buffer, NULL, 0);
        if (received != written)
            break; // No point in continuing
    }

    rg_network_http_close(req);
    free(buffer);

    if (received != written || (received != content_length && content_length != -1))
    {
        rg_gui_alert("Download failed!", "Read/write error!");
        return false;
    }

    return true;
}

#endif
