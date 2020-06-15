/* Simple HTTP Server Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include "driver/gpio.h"
#include <esp_wifi.h>
#include <esp_event.h>
#include <esp_log.h>
#include <esp_system.h>
#include <nvs_flash.h>
#include <sys/param.h>
#include "nvslocal.h"
#include "tcpip_adapter.h"
#include "esp_eth.h"
#include "protocol_examples_common.h"
#include "registerUris.h"
#include "esp32-hal-timer.h"
#include "projdefs.h"
#define CONFIG_EXAMPLE_CONNECT_WIFI 1
#define CONFIG_EXAMPLE_CONNECT_IPV6 1
#include <esp_http_server.h>
#include "time.h"
#include "main.h"
void start_request();
struct tm* get_time(void);
static esp_err_t index_handler(httpd_req_t* req);
static esp_err_t ajax_handler(httpd_req_t *req);
static esp_err_t setCleanerTimer_handler(httpd_req_t *req);
static esp_err_t setPumpTimer_handler(httpd_req_t *req);
extern void smtp_send(void);
void parseTimerRequest(httpd_req_t *,nvs_times_t *); 
void Timer_setup();
void onTime();
void check_timer();
void get_pool_timer_times(nvs_timer_t*);
esp_err_t write_timer_to_mvs(nvs_times_t * timer);
esp_err_t nvs_init();
void Toggle_pin(int ,int);
static nvs_timer_t times = {0,0,0,0,0,0,0,0,0,0,0,0,0};
TaskHandle_t timer_handle;
QueueHandle_t timerQueue; 
volatile int interrupts;
int totalInterrupts;
hw_timer_t * intTimer;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;
static const char *TAG = "example";

bool manualPoolOff = false;
bool manualCleanerOff = false;
bool manualPoolOn = false;
bool manualCleanerOn = false;

struct AMessage
{
     int count;
} xMessage;

//SMTPData data;

typedef struct {
    union {
        struct {
            uint32_t reserved0:   10;
            uint32_t alarm_en:     1;             /*When set  alarm is enabled*/
            uint32_t level_int_en: 1;             /*When set  level type interrupt will be generated during alarm*/
            uint32_t edge_int_en:  1;             /*When set  edge type interrupt will be generated during alarm*/
            uint32_t divider:     16;             /*Timer clock (T0/1_clk) pre-scale value.*/
            uint32_t autoreload:   1;             /*When set  timer 0/1 auto-reload at alarming is enabled*/
            uint32_t increase:     1;             /*When set  timer 0/1 time-base counter increment. When cleared timer 0 time-base counter decrement.*/
            uint32_t enable:       1;             /*When set  timer 0/1 time-base counter is enabled*/
        };
        uint32_t val;
    } config;
    uint32_t cnt_low;                             /*Register to store timer 0/1 time-base counter current value lower 32 bits.*/
    uint32_t cnt_high;                            /*Register to store timer 0 time-base counter current value higher 32 bits.*/
    uint32_t update;                              /*Write any value will trigger a timer 0 time-base counter value update (timer 0 current value will be stored in registers above)*/
    uint32_t alarm_low;                           /*Timer 0 time-base counter value lower 32 bits that will trigger the alarm*/
    uint32_t alarm_high;                          /*Timer 0 time-base counter value higher 32 bits that will trigger the alarm*/
    uint32_t load_low;                            /*Lower 32 bits of the value that will load into timer 0 time-base counter*/
    uint32_t load_high;                           /*higher 32 bits of the value that will load into timer 0 time-base counter*/
    uint32_t reload;                              /*Write any value will trigger timer 0 time-base counter reload*/
} hw_timer_reg_t;

typedef struct hw_timer_s {
        hw_timer_reg_t * dev;
        uint8_t num;
        uint8_t group;
        uint8_t timer;
        portMUX_TYPE lock;
} hw_timer_t;

/* An HTTP GET handler */
static esp_err_t hello_get_handler(httpd_req_t *req)
{
    char*  buf;
    volatile size_t buf_len; 
    //test =  strstr(req->uri,"gettime");
    //test =  strstr(req->uri,"gettime1");
    /* Get header value string length and allocate memory for length + 1,
     * extra byte for null termination */

    buf_len = httpd_req_get_hdr_value_len(req, "Host") + 1;
    if (buf_len > 1) {
        buf = malloc(buf_len);
        /* Copy null terminated value string into buffer */
        if (httpd_req_get_hdr_value_str(req, "Host", buf, buf_len) == ESP_OK) {
            ESP_LOGI(TAG, "Found header => Host: %s", buf);
        }
        free(buf);
    }

    buf_len = httpd_req_get_hdr_value_len(req, "Test-Header-2") + 1;
    if (buf_len > 1) {
        buf = malloc(buf_len);
        if (httpd_req_get_hdr_value_str(req, "Test-Header-2", buf, buf_len) == ESP_OK) {
            ESP_LOGI(TAG, "Found header => Test-Header-2: %s", buf);
        }
        free(buf);
    }

    buf_len = httpd_req_get_hdr_value_len(req, "Test-Header-1") + 1;
    if (buf_len > 1) {
        buf = malloc(buf_len);
        if (httpd_req_get_hdr_value_str(req, "Test-Header-1", buf, buf_len) == ESP_OK) {
            ESP_LOGI(TAG, "Found header => Test-Header-1: %s", buf);
        }
        free(buf);
    }

    /* Read URL query string length and allocate memory for length + 1,
     * extra byte for null termination */
    buf_len = httpd_req_get_url_query_len(req) + 1;
    if (buf_len > 1) {
        buf = malloc(buf_len);
        if (httpd_req_get_url_query_str(req, buf, buf_len) == ESP_OK) {
            ESP_LOGI(TAG, "Found URL query => %s", buf);
            char param[32];
            /* Get value of expected key from query string */
            if (httpd_query_key_value(buf, "query1", param, sizeof(param)) == ESP_OK) {
                ESP_LOGI(TAG, "Found URL query parameter => query1=%s", param);
            }
            if (httpd_query_key_value(buf, "query3", param, sizeof(param)) == ESP_OK) {
                ESP_LOGI(TAG, "Found URL query parameter => query3=%s", param);
            }
            if (httpd_query_key_value(buf, "query2", param, sizeof(param)) == ESP_OK) {
                ESP_LOGI(TAG, "Found URL query parameter => query2=%s", param);
            }
        }
        free(buf);
    }

    /* Set some custom headers */
    httpd_resp_set_hdr(req, "Custom-Header-1", "Custom-Value-1");
    httpd_resp_set_hdr(req, "Custom-Header-2", "Custom-Value-2");

    struct tm* time;
    time = get_time();
    char resp_buf[64];
    sprintf(resp_buf, "day = %d hour = %d minute = %d second = %d year = %d",time->tm_mday,time->tm_hour,time->tm_min,time->tm_sec,time->tm_year);


    /* Send response with custom headers and body set as the
     * string passed in user context*/
    const char* resp_str = (const char*) resp_buf;
    httpd_resp_send(req, resp_str, strlen(resp_str));

    /* After sending the HTTP response the old HTTP request
     * headers are lost. Check if HTTP request headers can be read now. */
    //if (httpd_req_get_hdr_value_len(req, "Host") == 0) {
     //   ESP_LOGI(TAG, "Request headers lost");
   // }
    return ESP_OK;
}

static esp_err_t index_handler(httpd_req_t* req) 
{
    bool timerAskedFor =  strstr(req->uri,"Timers");
    
    httpd_resp_set_type(req, "text/html");
	httpd_resp_set_hdr(req, "Content-Encoding", "gzip");

    if(!timerAskedFor)
        {
	        extern const unsigned char pool1_html_gz_start[] asm("_binary_pool1_html_gz_start");
	        extern const unsigned char pool1_html_gz_end[]   asm("_binary_pool1_html_gz_end");
	        size_t pool1_html_gz_len = pool1_html_gz_end - pool1_html_gz_start;
            return httpd_resp_send(req, (const char*)pool1_html_gz_start, pool1_html_gz_len);
        }
    else
    {
            extern const unsigned char pool2_html_gz_start[] asm("_binary_pool2_html_gz_start");
	        extern const unsigned char pool2_html_gz_end[]   asm("_binary_pool2_html_gz_end");
	        size_t pool2_html_gz_len = pool2_html_gz_end - pool2_html_gz_start;
            return httpd_resp_send(req, (const char*)pool2_html_gz_start, pool2_html_gz_len);
    }
        
	//httpd_resp_set_type(req, "text/html");
	//httpd_resp_set_hdr(req, "Content-Encoding", "gzip");
	
	//return httpd_resp_send(req, (const char*)pool1_html_gz_start, pool1_html_gz_len);
}



/* An HTTP POST handler */
static esp_err_t echo_post_handler(httpd_req_t *req)
{
    char buf[100];
    int ret, remaining = req->content_len;

    while (remaining > 0) {
        /* Read the data for the request */
        if ((ret = httpd_req_recv(req, buf,
                        MIN(remaining, sizeof(buf)))) <= 0) {
            if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
                /* Retry receiving if timeout occurred */
                continue;
            }
            return ESP_FAIL;
        }

        /* Send back the same data */
        httpd_resp_send_chunk(req, buf, ret);
        remaining -= ret;

        /* Log data received */
        ESP_LOGI(TAG, "=========== RECEIVED DATA ==========");
        ESP_LOGI(TAG, "%.*s", ret, buf);
        ESP_LOGI(TAG, "====================================");
    }

    // End response
    httpd_resp_send_chunk(req, NULL, 0);
    return ESP_OK;
}



/* This handler allows the custom error handling functionality to be
 * tested from client side. For that, when a PUT request 0 is sent to
 * URI /ctrl, the /hello and /echo URIs are unregistered and following
 * custom error handler http_404_error_handler() is registered.
 * Afterwards, when /hello or /echo is requested, this custom error
 * handler is invoked which, after sending an error message to client,
 * either closes the underlying socket (when requested URI is /echo)
 * or keeps it open (when requested URI is /hello). This allows the
 * client to infer if the custom error handler is functioning as expected
 * by observing the socket state.
 */
esp_err_t http_404_error_handler(httpd_req_t *req, httpd_err_code_t err)
{
    if (strcmp("/hello", req->uri) == 0) {
        httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "/hello URI is not available");
        /* Return ESP_OK to keep underlying socket open */
        return ESP_OK;
    } else if (strcmp("/echo", req->uri) == 0) {
        httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "/echo URI is not available");
        /* Return ESP_FAIL to close underlying socket */
        return ESP_FAIL;
    }
    /* For any other URI send 404 and close socket */
    httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "Some 404 error message");
    return ESP_FAIL;
}

/* An HTTP PUT handler. This demonstrates realtime
 * registration and deregistration of URI handlers
 */
static esp_err_t ctrl_put_handler(httpd_req_t *req)
{
    char buf;
    int ret;

    if ((ret = httpd_req_recv(req, &buf, 1)) <= 0) {
        if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
            httpd_resp_send_408(req);
        }
        return ESP_FAIL;
    }

    if (buf == '0') {
        /* URI handlers can be unregistered using the uri string */
        ESP_LOGI(TAG, "Unregistering /hello and /echo URIs");
        httpd_unregister_uri(req->handle, "/pool");
        httpd_unregister_uri(req->handle, "/pump");
        httpd_unregister_uri(req->handle, "/pump");
        httpd_unregister_uri(req->handle, "/light");
        httpd_unregister_uri(req->handle, "/cleaner");
        httpd_unregister_uri(req->handle, "/timer");
        httpd_unregister_uri(req->handle, "/poolOnOff");
        httpd_unregister_uri(req->handle, "/cleanerOnOff");
        httpd_unregister_uri(req->handle, "/lightOnOff");
        httpd_unregister_uri(req->handle, "/getPumpStatus");
        httpd_unregister_uri(req->handle, "/getCleanerStatus");
        httpd_unregister_uri(req->handle, "/getLightStatus");
        httpd_unregister_uri(req->handle, "/setCleanerTimer");
        httpd_unregister_uri(req->handle, "/setPumpTimer");
        httpd_unregister_uri(req->handle, "/setCleanerTimer");
        httpd_unregister_uri(req->handle, "/getPumpOnTime");
        httpd_unregister_uri(req->handle, "/getPumpOffTime");
        httpd_unregister_uri(req->handle, "/getCleanerOnTime");
        httpd_unregister_uri(req->handle, "/getCleanerOffTime");
        /* Register the custom error handler */
        httpd_register_err_handler(req->handle, HTTPD_404_NOT_FOUND, http_404_error_handler);
    }
    else {
        ESP_LOGI(TAG, "Registering /hello and /echo URIs");
        httpd_register_uri_handler(req->handle, &pool);
        httpd_register_uri_handler(req->handle, &pump);
        httpd_register_uri_handler(req->handle, &light);
        httpd_register_uri_handler(req->handle, &cleaner);
        httpd_register_uri_handler(req->handle, &timer);
        httpd_register_uri_handler(req->handle, &poolOnOff);
        httpd_register_uri_handler(req->handle, &cleanerOnOff);
        httpd_register_uri_handler(req->handle, &lightOnOff);
        httpd_register_uri_handler(req->handle, &getPumpStatus);
        httpd_register_uri_handler(req->handle, &getCleanerStatus);
        httpd_register_uri_handler(req->handle, &getLightStatus);
        httpd_register_uri_handler(req->handle, &setPumpTimer);
        httpd_register_uri_handler(req->handle, &setCleanerTimer);
        httpd_register_uri_handler(req->handle, &getPumpOnTime);
        httpd_register_uri_handler(req->handle, &getPumpOffTime);
        httpd_register_uri_handler(req->handle, &getCleanerOnTime);
        httpd_register_uri_handler(req->handle, &getCleanerOffTime);
        /* Unregister custom error handler */
        httpd_register_err_handler(req->handle, HTTPD_404_NOT_FOUND, NULL);
    }

    /* Respond with empty body */
    httpd_resp_send(req, NULL, 0);
    return ESP_OK;
}



static httpd_handle_t start_webserver(void)
{
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    // Start the httpd server
    ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);
    if (httpd_start(&server, &config) == ESP_OK) {
        // Set URI handlers
        ESP_LOGI(TAG, "Registering URI handlers");
        httpd_register_uri_handler(server, &pool);
        httpd_register_uri_handler(server, &pump);
        httpd_register_uri_handler(server, &light);
        httpd_register_uri_handler(server, &cleaner);
        httpd_register_uri_handler(server, &timer);
        httpd_register_uri_handler(server, &poolOnOff);
        httpd_register_uri_handler(server, &cleanerOnOff);
        httpd_register_uri_handler(server, &lightOnOff);
        httpd_register_uri_handler(server, &getPumpStatus);
        httpd_register_uri_handler(server, &getCleanerStatus);
        httpd_register_uri_handler(server, &getLightStatus);
        httpd_register_uri_handler(server, &setCleanerTimer);
        httpd_register_uri_handler(server, &setPumpTimer);
        httpd_register_uri_handler(server, &getPumpOnTime);
        httpd_register_uri_handler(server, &getPumpOffTime);
        httpd_register_uri_handler(server, &getCleanerOnTime);
        httpd_register_uri_handler(server, &getCleanerOffTime);
        return server;
    }

    ESP_LOGI(TAG, "Error starting server!");
    return NULL;
}

static void stop_webserver(httpd_handle_t server)
{
    // Stop the httpd server
    httpd_stop(server);
}

static void disconnect_handler(void* arg, esp_event_base_t event_base, 
                               int32_t event_id, void* event_data)
{
    httpd_handle_t* server = (httpd_handle_t*) arg;
    if (*server) {
        ESP_LOGI(TAG, "Stopping webserver");
        stop_webserver(*server);
        *server = NULL;
    }
}

static void connect_handler(void* arg, esp_event_base_t event_base, 
                            int32_t event_id, void* event_data)
{
    httpd_handle_t* server = (httpd_handle_t*) arg;
    if (*server == NULL) {
        ESP_LOGI(TAG, "Starting webserver");
        *server = start_webserver();
    }
}



void app_main()
{
    static httpd_handle_t server = NULL;
    //ESP_ERROR_CHECK(nvs_flash_init());
    esp_err_t err = nvs_flash_init();
    if(err != ESP_OK)
      printf("flash Init failed with err = %d\n",err);
    tcpip_adapter_init();
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    /* This helper function configures Wi-Fi or Ethernet, as selected in menuconfig.
     * Read "Establishing Wi-Fi or Ethernet Connection" section in
     * examples/protocols/README.md for more information about this function.
     */
    ESP_ERROR_CHECK(example_connect());
    gpio_pad_select_gpio(PUMP_PIN);
    gpio_pad_select_gpio(POOL_CLEANER_PIN);
    gpio_pad_select_gpio(POOL_LIGHT_PIN);

    /* Set the GPIO as a push/pull output */
    gpio_set_direction(PUMP_PIN, GPIO_MODE_INPUT_OUTPUT);
    gpio_set_direction(POOL_CLEANER_PIN, GPIO_MODE_INPUT_OUTPUT);
    gpio_set_direction(POOL_LIGHT_PIN, GPIO_MODE_INPUT_OUTPUT);

    //turn off all motors and lights
    Toggle_pin(PUMP_PIN,0); 
    Toggle_pin(POOL_CLEANER_PIN,0);
    Toggle_pin(POOL_LIGHT_PIN,0);

    /* Register event handlers to stop the server when Wi-Fi or Ethernet is disconnected,
     * and re-start it upon connection.
     */
#ifdef CONFIG_EXAMPLE_CONNECT_WIFI
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &connect_handler, &server));
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &disconnect_handler, &server));
#endif // CONFIG_EXAMPLE_CONNECT_WIFI
#ifdef CONFIG_EXAMPLE_CONNECT_ETHERNET
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_ETH_GOT_IP, &connect_handler, &server));
    ESP_ERROR_CHECK(esp_event_handler_register(ETH_EVENT, ETHERNET_EVENT_DISCONNECTED, &disconnect_handler, &server));
#endif // CONFIG_EXAMPLE_CONNECT_ETHERNET
    
    //create task to handle timer interrupts
    /* Start the server for the first time */
    server = start_webserver();
    //start_request();
    struct tm* time;
    time = get_time();
    printf("day = %d hour = %d minute = %d second = %d year = %d\n",time->tm_mday,time->tm_hour,time->tm_min,time->tm_sec,time->tm_year);
    times.pumpTimerOnHour = 0;
    get_pool_timer_times(&times);
    Timer_setup();

    xTaskCreate(check_timer,"Check_timer",2000,NULL,1,timer_handle);   /* Task handle to keep track of created task */
    timerQueue = xQueueCreate( 10, sizeof( int ) );
    printf("started task\n");
    
    //smtp_send();
}

/* An HTTP GET handler */
static esp_err_t ajax_handler(httpd_req_t *req)
{
    //char*  buf;
    //volatile size_t buf_len; 
    bool poolOnOff =  strstr(req->uri,"pumpOnOff");
    bool cleanerOnOff =  strstr(req->uri,"cleanerOnOff");
    bool lightOnOff =  strstr(req->uri,"lightOnOff");
    bool getPumpStatus =  strstr(req->uri,"getPumpStatus");
    bool getCleanerStatus =  strstr(req->uri,"getCleanerStatus");
    bool getLightStatus =  strstr(req->uri,"getLightStatus");
    bool getpumpOnTime =  strstr(req->uri,"getPumpOnTime");
    bool getCleanerOnTime =  strstr(req->uri,"getCleanerOnTime");
    bool getpumpOffTime =  strstr(req->uri,"getPumpOffTime");
    bool getCleanerOffTime =  strstr(req->uri,"getCleanerOffTime");
    if(poolOnOff)
    {
       if(gpio_get_level(PUMP_PIN))
       {
            if(gpio_get_level(POOL_CLEANER_PIN) == 0)
            {
                Toggle_pin(PUMP_PIN,0);
                httpd_resp_send(req, "Off", 3);
                manualPoolOff = true;
                manualPoolOn = false;
            }
            else
            {
                printf("can't turn off pump because cleaner is on\n");
                httpd_resp_send(req, "On", 3);
            }   
       }
       else
       {
            Toggle_pin(PUMP_PIN,1);
            httpd_resp_send(req, "On", 2);
            manualPoolOn = true;
            manualPoolOff = false;
       }       
    }
    if(cleanerOnOff)
    {
       if(gpio_get_level(POOL_CLEANER_PIN))
       {
            Toggle_pin(POOL_CLEANER_PIN,0);
            httpd_resp_send(req, "Off", 3);
            manualCleanerOff = true;
            manualCleanerOn = false;
       }
       else
       {
            if(gpio_get_level(PUMP_PIN) == 1) //you can't turn cleaner on if pump isn't on
            {
                Toggle_pin(POOL_CLEANER_PIN,1); 
                httpd_resp_send(req, "On", 2);
                manualCleanerOff = false;
                manualCleanerOn = true;
            }
            else
            {
                printf("can't turn on cleaner because pump is not on\n");
                 httpd_resp_send(req, "Off", 3);
            }   
       }       
    }
    if(lightOnOff)
    {
       int onOff = gpio_get_level(POOL_LIGHT_PIN);
       if(onOff)
       {
            Toggle_pin(POOL_LIGHT_PIN,0);
            httpd_resp_send(req, "Off", 3);
       }
       else
       {
            Toggle_pin(POOL_LIGHT_PIN,1);
            httpd_resp_send(req, "On", 2);
       }       
    }

    if(getPumpStatus)
    {
       int onOff = gpio_get_level(PUMP_PIN);
       if(onOff)
       {
            httpd_resp_send(req, "On", 2);
       }
       else
       {
            httpd_resp_send(req, "Off", 3);
       }       
    }
    if(getCleanerStatus)
    {
       int onOff = gpio_get_level(POOL_CLEANER_PIN);
       if(onOff)
       {
            httpd_resp_send(req, "On", 2);
       }
       else
       {
            httpd_resp_send(req, "Off", 3);
       }       
    }
    if(getLightStatus)
    {
       int onOff = gpio_get_level(POOL_LIGHT_PIN);
       if(onOff)
       {
            httpd_resp_send(req, "On", 2);
       }
       else
       {
            httpd_resp_send(req, "Off", 3);
       }       
    }

    if(getpumpOnTime)
    {  
        char onTime[50];
        memset(onTime,0,50);
        strcat(onTime,"Tue Sep 01 2015 " );
        char buffer[3];
        memset(buffer,0,3);
        itoa(times.pumpTimerOnHour,buffer,10);
        strcat(onTime,buffer);
        strcat(onTime,":");
        memset(buffer,0,3);
        itoa(times.pumpTimerOnMinute,buffer,10);
        strcat(onTime,buffer);
        strcat(onTime,":00 ");
        if(times.pumpTimerOnAmPm == 0)
           strcat(onTime,"AM");
        else
           strcat(onTime,"PM"); 
        printf("sent pump on time with time = %s\n",onTime);
        httpd_resp_send(req, onTime, strlen(onTime));     
    }
     if(getpumpOffTime)
    {  
       char onTime[50];
        memset(onTime,0,50);
        strcat(onTime,"Tue Sep 01 2015 " );
        char buffer[3];
        memset(buffer,0,3);
        itoa(times.pumpTimerOffHour,buffer,10);
        strcat(onTime,buffer);
        strcat(onTime,":");
        memset(buffer,0,3);
        itoa(times.pumpTimerOffMinute,buffer,10);
        strcat(onTime,buffer);
        strcat(onTime,":00 ");
        if(times.pumpTimerOffAmPm == 0)
           strcat(onTime,"AM");
        else
           strcat(onTime,"PM"); 
        printf("sent pump off time with time = %s\n",onTime);
        httpd_resp_send(req, onTime, strlen(onTime));       
    }

    if(getCleanerOnTime)
    {  
        char onTime[50];
        memset(onTime,0,50);
        strcat(onTime,"Tue Sep 01 2015 " );
        char buffer[3];
        memset(buffer,0,3);
        itoa(times.cleanerTimerOnHour,buffer,10);
        strcat(onTime,buffer);
        strcat(onTime,":");
        memset(buffer,0,3);
        itoa(times.cleanerTimerOnMinute,buffer,10);
        strcat(onTime,buffer);
        strcat(onTime,":00 ");
        if(times.cleanerTimerOnAmPm == 0)
           strcat(onTime,"AM");
        else
           strcat(onTime,"PM"); 
        printf("sent cleaner on time with time = %s\n",onTime);
        httpd_resp_send(req, onTime, strlen(onTime));   
    }

    if(getCleanerOffTime)
    {  
        char onTime[50];
        memset(onTime,0,50);
        strcat(onTime,"Tue Sep 01 2015 " );
        char buffer[3];
        memset(buffer,0,3);
        itoa(times.cleanerTimerOffHour,buffer,10);
        strcat(onTime,buffer);
        strcat(onTime,":");
        memset(buffer,0,3);
        itoa(times.cleanerTimerOffMinute,buffer,10);
        strcat(onTime,buffer);
        strcat(onTime,":00 ");
        if(times.cleanerTimerOffAmPm == 0)
           strcat(onTime,"AM");
        else
           strcat(onTime,"PM"); 
        printf("sent cleaner off time with time = %s\n",onTime);
        httpd_resp_send(req, onTime, strlen(onTime)); 
    }
    //httpd_resp_send(req, "On ", 3);
    return ESP_OK;
}
esp_err_t err;
static esp_err_t setCleanerTimer_handler(httpd_req_t *req)
{
    nvs_times_t cleanerTimes;
    cleanerTimes.cleaner = true;
    cleanerTimes.pump = false;
    parseTimerRequest(req,&cleanerTimes);
    err =  write_timer_to_mvs(&cleanerTimes);
    if(err != ESP_OK)
    {
       printf("write_timer_to_mvs for cleaner failed with err = %d\n",err);
       httpd_resp_send(req, "Pool cleaner Timer change failed!!!",36);
       return err;
    }
    get_pool_timer_times(&times);
   memcpy(req->uri,"/Timers",8);
   index_handler(req); 
   return ESP_OK;
}

static esp_err_t setPumpTimer_handler(httpd_req_t *req)
{
    nvs_times_t pumpTimes;
    pumpTimes.cleaner = false;
    pumpTimes.pump = true;
    parseTimerRequest(req,&pumpTimes);
    err =  write_timer_to_mvs(&pumpTimes);
    if(err != ESP_OK)
    {
       printf("write_timer_to_mvs for pump failed with err = %d\n",err);
        httpd_resp_send(req, "Pool Pump Timer change failed!!!",33);
        return err;
    }
    get_pool_timer_times(&times);
    memcpy(req->uri,"/Timers",8);
    index_handler(req);
    return ESP_OK;
}
 
void parseTimerRequest(httpd_req_t *req,nvs_times_t *times) 
{
    char * startHour = strstr(req->uri,"On=");
    startHour += 3;
    char * colon = strstr(startHour,"%");
    *colon = 0x00; 
    char * startMinute = colon +3;
    colon = strstr(startMinute,"%");
    *colon = 0x00;
    colon++;
    colon = strstr(colon,"+");
    char * startAmPm = colon +1;
    colon = strstr(startAmPm,"&");
    *colon = 0x00;
    colon++;
    char * stopHour = strstr(colon,"Off=");
    stopHour += 4;
    colon = strstr(stopHour,"%");
    *colon = 0x00; 
    char * stopMinute = colon +3;
    colon = strstr(stopMinute,"%");
    *colon = 0x00;
    colon++;
    colon = strstr(colon,"+");
    char * stopAmPm = colon +1;

    if(strstr(startAmPm,"AM"))
        times->timerOnAmPm = 0;
    else
        times->timerOnAmPm = 1;
    times->timerOnHour = atoi(startHour);
    times->timerOnMinute = atoi(startMinute); 
    
    if(strstr(stopAmPm,"AM"))
        times->timerOffAmPm = 0;
    else
        times->timerOffAmPm = 1;
    times->timerOffHour = atoi(stopHour);
    times->timerOffMinute = atoi(stopMinute); 
    

    printf("startHour = %d\n",times->timerOnHour);
    printf("startMinute = %d\n",times->timerOnMinute);
    printf("startAmPm = %d\n",times->timerOnAmPm);
    printf("stopHour = %d\n",times->timerOffHour);
    printf("stopMinute = %d\n",times->timerOffMinute);
    printf("stopAmPm = %d\n",times->timerOffAmPm);
}

void Toggle_pin(int gpio,int offOn)
{
    gpio_set_level(gpio, offOn);
    
}
void check_timer()
{ 
    bool pump_timer_spanned_midnight = false;
    bool cleaner_timer_spanned_midnight = false;
    bool pump_on = false;
    struct AMessage
    {
        int count;
    } zMessage;
    int * queueItem;
    struct tm* time;
    while(1)
    {
    xQueueReceive(timerQueue, &(zMessage), portMAX_DELAY);  

    //printf("in queue count = %d \n",zMessage.count);
    time = get_time();
    int pump_on_minutes,pump_off_minutes,cleaner_on_minutes,cleaner_off_minutes;
    
    int total_mins_today = (time->tm_hour * 60) + time->tm_min; //get the number of elapsed minutes in the current day
    //get elapsed minutes for pump on
    if(times.pumpTimerOnAmPm == 1)
       pump_on_minutes = (times.pumpTimerOnHour+12) * 60;
    else
       pump_on_minutes = times.pumpTimerOnHour * 60;
    pump_on_minutes += times.pumpTimerOnMinute;

    //get elapsed minutes for pump off
    if(times.pumpTimerOffAmPm == 1)
       pump_off_minutes = (times.pumpTimerOffHour+12) * 60;
    else
       pump_off_minutes = times.pumpTimerOffHour * 60;
    pump_off_minutes += times.pumpTimerOffMinute;

    if(pump_off_minutes < pump_on_minutes)
        pump_timer_spanned_midnight = true;
      
    //get elapsed minutes for cleaner on
    if(times.cleanerTimerOnAmPm == 1)
       cleaner_on_minutes = (times.cleanerTimerOnHour+12) * 60;
    else
       cleaner_on_minutes = times.cleanerTimerOnHour * 60;
    cleaner_on_minutes += times.cleanerTimerOnMinute;

    //get elapsed minutes for cleaner off
    if(times.cleanerTimerOffAmPm == 1)
       cleaner_off_minutes = (times.cleanerTimerOffHour+12) * 60;
    else
       cleaner_off_minutes = times.cleanerTimerOffHour * 60;
    cleaner_off_minutes += times.cleanerTimerOffMinute;

    if(cleaner_off_minutes < cleaner_on_minutes)
        cleaner_timer_spanned_midnight = true;

    if(pump_timer_spanned_midnight == false)
    {
        if(total_mins_today >= pump_on_minutes && total_mins_today <= pump_off_minutes)
           pump_on = true;
    } 
    else
    {
        if(total_mins_today >= pump_off_minutes && total_mins_today <= pump_on_minutes)
           pump_on = true;
    }
       


    
    if(total_mins_today >= pump_on_minutes && total_mins_today <= pump_off_minutes)
        {
            if(gpio_get_level(PUMP_PIN) == 0 && manualPoolOff == false )
            {
                Toggle_pin(PUMP_PIN,1);
                printf("turning pump on at ");
            printf("day = %d hour = %d minute = %d second = %d year = %d\n",time->tm_mday,time->tm_hour,time->tm_min,time->tm_sec,time->tm_year);
            }
        }
    else
        {
            if(gpio_get_level(PUMP_PIN) == 1 && manualPoolOn == false)
            {
                Toggle_pin(POOL_CLEANER_PIN,0); //if shutting the pump off, you must shut off the cleaner
                Toggle_pin(PUMP_PIN,0);
                printf("turning pump and cleaner off at ");
                printf("day = %d hour = %d minute = %d second = %d year = %d\n",time->tm_mday,time->tm_hour,time->tm_min,time->tm_sec,time->tm_year);
            }
    }
    

    if(total_mins_today >= cleaner_on_minutes && total_mins_today <= cleaner_off_minutes)
    {
        if(gpio_get_level(POOL_CLEANER_PIN) == 0 && gpio_get_level(PUMP_PIN) == 1 && manualCleanerOff == false)
        {
            Toggle_pin(PUMP_PIN,1); //turn on the pump just to be safe
            Toggle_pin(POOL_CLEANER_PIN,1);
            printf("turning cleaner on at ");
            printf("day = %d hour = %d minute = %d second = %d year = %d\n",time->tm_mday,time->tm_hour,time->tm_min,time->tm_sec,time->tm_year);
        }
    }
    else
    {
        if(gpio_get_level(POOL_CLEANER_PIN) == 1 && manualCleanerOn == false)
        {
            Toggle_pin(POOL_CLEANER_PIN,0);
            printf("turning cleaner off at ");
            printf("day = %d hour = %d minute = %d second = %d year = %d\n",time->tm_mday,time->tm_hour,time->tm_min,time->tm_sec,time->tm_year);
        }
    }
        
    if(total_mins_today > 1380) //if it's after 11PM turn the pool light off
    {
        Toggle_pin(POOL_LIGHT_PIN,0);
        manualPoolOff = false; //reset for next day
        manualPoolOn = false; //reset for next day
        manualCleanerOff = false; //reset for next day
        manualCleanerOn = false; //reset for next day
        printf("turning cleaner on at ");
        printf("day = %d hour = %d minute = %d second = %d year = %d\n",time->tm_mday,time->tm_hour,time->tm_min,time->tm_sec,time->tm_year);
    }
    }
}
void Timer_setup() {

	// Configure Prescaler to 80, as our timer runs @ 80Mhz
	// Giving an output of 80,000,000 / 80 = 1,000,000 ticks / second
	intTimer = timerBegin(0, 80, true);                
	timerAttachInterrupt(intTimer, &onTime, true);    
	// Fire Interrupt every 1m ticks, so 1s
	timerAlarmWrite(intTimer, 10000000, true);			
	timerAlarmEnable(intTimer);
}
void IRAM_ATTR onTime() {
	portENTER_CRITICAL_ISR(&timerMux);
    BaseType_t xHigherPriorityTaskWoken = 0;
    //xMessage.count++;
	xQueueSendFromISR(timerQueue, &xMessage, xHigherPriorityTaskWoken);
	portEXIT_CRITICAL_ISR(&timerMux);
}



