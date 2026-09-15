#include "web_server.h"
#include <string.h>
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_http_server.h"
#include "button.h"

static const char *WEB = "WEB_SERVER";

#define WIFI_SSID "ESP32_ButtonLed"
#define WIFI_PASS "12345678"

static const char HTML_PAGE[] =
"<!DOCTYPE html><html>"
"<head><meta name='viewport' content='width=device-width, initial-scale=1'>"
"<style>"
"body { font-family: Arial; text-align: center; margin:0; padding: 20px; background: #121212; color: white; }"
"h1 { color: #00ADB5; }"
".btn { display: block; width: 80%; max-width: 300px; margin: 15px auto; padding: 15px; font-size: 18px; "
"       border: none; border-radius: 8px; cursor: pointer; font-weight: bold; text-decoration: none; color: white; }"
".btn-mode { background-color: #393E46; }"
".btn-mode:active { background-color: #00ADB5; }"
".btn-hang { background-color: #E23E57; }"
".status-box { margin: 20px auto; padding: 15px; max-width: 300px; background: #222831; border-radius: 8px; }"
"</style></head>"
"<body>"
"  <h1>Button LED Server</h1>"
"  <div class='status-box'>"
"    <p>Button controlled by Web</p>"
"  </div>"
"  <a href='/change_mode' class='btn btn-mode'>CHANGE MODE </a>"
"  <a href='/trigger_hang' class='btn btn-hang'>TEST WATCHDOG CRASH</a>"
"</body></html>";

static esp_err_t root_get_handler(httpd_req_t *req){
    httpd_resp_set_type(req, "text/html");
    httpd_resp_send(req, HTML_PAGE, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

static esp_err_t change_mode_handler(httpd_req_t *req) {
    ESP_LOGI(WEB, "Web button clicked: CHANGE_MODE");
    button_event_t evt = BTN_EVT_CLICK;
    xQueueSend(button_queue, &evt, 0);
    
    httpd_resp_set_status(req, "303 See Other");
    httpd_resp_set_hdr(req, "Location", "/");
    httpd_resp_send(req, NULL, 0);
    return ESP_OK;
}

static esp_err_t trigger_hang_handler(httpd_req_t *req){
    ESP_LOGW(WEB, "Web button clicked: TRIGGER_HANG");

    button_event_t evt = BTN_EVT_TRIGGER_HANG;
    xQueueSend(button_queue, &evt, 0);

    const char *resp =  
    "<!DOCTYPE html><html><head>"
    "<meta name='viewport' content='width=device-width, initial-scale=1'>"    
    "<meta http-equiv='refresh' content='6;url=/'>"
    "<style>body{font-family:Arial;background:#121212;color:white;text-align:center;padding:50px;}</style>"       
    "</head><body>"     
    "<h2 style='color:#E23E57;'>⚠️ WATCHDOG DEADLOCK TRIGGERED!</h2>"
    "<p>ESP32 is frozen and will automatically reset after 3 seconds....</p>" 
    "<p> Automatically reconnecting after... <b><span id='timer'>6</span></b> seconds...</p>"     
    "<script>" 
    "  let sec = 6;"    
    "  setInterval(() => {"      
    "    sec--; if(sec >= 0) document.getElementById('timer').innerText = sec;"        
    "    if(sec === 0) location.href = '/';"       
    "  }, 1000);"       
    "</script>"
    "</body></html>";

    httpd_resp_send(req, resp, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

static httpd_handle_t start_webserver(void){
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    httpd_handle_t server = NULL;

    if (httpd_start(&server, &config) == ESP_OK) {
        httpd_uri_t uri_root = { .uri = "/", .method = HTTP_GET, .handler = root_get_handler };
        httpd_register_uri_handler(server, &uri_root);

        httpd_uri_t uri_change = { .uri = "/change_mode", .method = HTTP_GET, .handler = change_mode_handler };
        httpd_register_uri_handler(server, &uri_change);

        httpd_uri_t uri_hang = { .uri = "/trigger_hang", .method = HTTP_GET, .handler = trigger_hang_handler };
        httpd_register_uri_handler(server, &uri_hang);

        ESP_LOGI(WEB, "Web server started on port %d", config.server_port);    
    }
    return server;
}

static void wifi_init_softap(void) {
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_ap();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    wifi_config_t wifi_config = {
        .ap = {
            .ssid = WIFI_SSID,
            .ssid_len = strlen(WIFI_SSID),
            .channel = 1,
            .password = WIFI_PASS,
            .max_connection = 4,
            .authmode = WIFI_AUTH_WPA2_PSK,
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(WEB, "Wi-Fi SoftAP ready! SSID: %s | PASS: %s", WIFI_SSID, WIFI_PASS);
    ESP_LOGI(WEB, "Web UI Access URL: http://192.168.4.1");
}

esp_err_t WebServer_Init(void){
    esp_err_t ret = nvs_flash_init();
    if(ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND){
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    wifi_init_softap();
    start_webserver();
    return ESP_OK;
}

