
#include "esp_http_server.h"
static esp_err_t index_handler(httpd_req_t* req);
static esp_err_t hello_get_handler(httpd_req_t* req);
static esp_err_t echo_post_handler(httpd_req_t *req);
static esp_err_t ctrl_put_handler(httpd_req_t *req);
static esp_err_t ajax_handler(httpd_req_t *req);
static esp_err_t setPumpTimer_handler(httpd_req_t *req);
static esp_err_t setCleanerTimer_handler(httpd_req_t *req);
//---------------------------------------------------------Gets ---------------------------------------------------------------

//Turn on/off Pool Pump
static httpd_uri_t pump = {
		.uri = "/Pump",
		.method = HTTP_GET,
		.handler = index_handler,
		.user_ctx = NULL
	};
static httpd_uri_t poolOnOff = {
		.uri = "/pumpOnOff",
		.method = HTTP_GET,
		.handler = ajax_handler,
		.user_ctx = (int*) 16
	};
static httpd_uri_t cleanerOnOff = {
		.uri = "/cleanerOnOff",
		.method = HTTP_GET,
		.handler = ajax_handler,
		.user_ctx = (int*) 16
	};
static httpd_uri_t lightOnOff = {
		.uri = "/lightOnOff",
		.method = HTTP_GET,
		.handler = ajax_handler,
		.user_ctx = (int*) 16
	};

static httpd_uri_t getPumpStatus = {
		.uri = "/getPumpStatus",
		.method = HTTP_GET,
		.handler = ajax_handler,
		.user_ctx = (int*) 16
	};
static httpd_uri_t getCleanerStatus = {
		.uri = "/getCleanerStatus",
		.method = HTTP_GET,
		.handler = ajax_handler,
		.user_ctx = (int*) 16
	};
static httpd_uri_t getLightStatus = {
		.uri = "/getLightStatus",
		.method = HTTP_GET,
		.handler = ajax_handler,
		.user_ctx = (int*) 16
	};
static httpd_uri_t setPumpTimer = {
		.uri = "/setPumpTimer",
		.method = HTTP_GET,
		.handler = setPumpTimer_handler,
		.user_ctx = (int*) 16
	};

static httpd_uri_t setCleanerTimer = {
		.uri = "/setCleanerTimer",
		.method = HTTP_GET,
		.handler = setCleanerTimer_handler,
		.user_ctx = (int*) 16
	};

	

//Turn on/off Pool Light
static httpd_uri_t light = {
		.uri = "/Light",
		.method = HTTP_GET,
		.handler = index_handler,
		.user_ctx = NULL
	};

//Turn on/off Pool Cleaner
static httpd_uri_t cleaner = {
		.uri = "/Cleaner",
		.method = HTTP_GET,
		.handler = index_handler,
		.user_ctx = NULL
	};

//Bring up web page with buttons
static httpd_uri_t pool = {
		.uri = "/",
		.method = HTTP_GET,
		.handler = index_handler,
		.user_ctx = NULL
	};

//Bring up web page to set timers for pump, cleaner and lights
static httpd_uri_t timer = {
		.uri = "/Timers",
		.method = HTTP_GET,
		.handler = index_handler,
		.user_ctx = NULL
	};

//Ajax to display timer setting for pump
static httpd_uri_t getPumpOnTime = {
		.uri = "/getPumpOnTime",
		.method = HTTP_GET,
		.handler = ajax_handler,
		.user_ctx = NULL
	};

static httpd_uri_t getPumpOffTime = {
		.uri = "/getPumpOffTime",
		.method = HTTP_GET,
		.handler = ajax_handler,
		.user_ctx = NULL
	};

//Ajax to display timer setting for pump
static httpd_uri_t getCleanerOnTime = {
		.uri = "/getCleanerOnTime",
		.method = HTTP_GET,
		.handler = ajax_handler,
		.user_ctx = NULL
	};

	static httpd_uri_t getCleanerOffTime = {
		.uri = "/getCleanerOffTime",
		.method = HTTP_GET,
		.handler = ajax_handler,
		.user_ctx = NULL
	};
//---------------------------------------------------------Post ---------------------------------------------------------------

static const httpd_uri_t echo = {
    .uri       = "/echo",
    .method    = HTTP_POST,
    .handler   = ajax_handler,
    .user_ctx  = NULL
};


//---------------------------------------------------------Put ---------------------------------------------------------------
static const httpd_uri_t ctrl = {
    .uri       = "/ctrl",
    .method    = HTTP_PUT,
    .handler   = ctrl_put_handler,
    .user_ctx  = NULL
};