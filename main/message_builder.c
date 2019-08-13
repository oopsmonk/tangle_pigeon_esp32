#include <sys/time.h>
#include <time.h>
#include "cJSON.h"
#include "esp_log.h"
#include "esp_sleep.h"

// iota cclient library
#include "cclient/api/core/core_api.h"
#include "cclient/api/extended/extended_api.h"

static const char *TAG = "message-builder";

static iota_client_service_t g_cclient;

static char const *amazon_ca1_pem =
    "-----BEGIN CERTIFICATE-----\r\n"
    "MIIDQTCCAimgAwIBAgITBmyfz5m/jAo54vB4ikPmljZbyjANBgkqhkiG9w0BAQsF\r\n"
    "ADA5MQswCQYDVQQGEwJVUzEPMA0GA1UEChMGQW1hem9uMRkwFwYDVQQDExBBbWF6\r\n"
    "b24gUm9vdCBDQSAxMB4XDTE1MDUyNjAwMDAwMFoXDTM4MDExNzAwMDAwMFowOTEL\r\n"
    "MAkGA1UEBhMCVVMxDzANBgNVBAoTBkFtYXpvbjEZMBcGA1UEAxMQQW1hem9uIFJv\r\n"
    "b3QgQ0EgMTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALJ4gHHKeNXj\r\n"
    "ca9HgFB0fW7Y14h29Jlo91ghYPl0hAEvrAIthtOgQ3pOsqTQNroBvo3bSMgHFzZM\r\n"
    "9O6II8c+6zf1tRn4SWiw3te5djgdYZ6k/oI2peVKVuRF4fn9tBb6dNqcmzU5L/qw\r\n"
    "IFAGbHrQgLKm+a/sRxmPUDgH3KKHOVj4utWp+UhnMJbulHheb4mjUcAwhmahRWa6\r\n"
    "VOujw5H5SNz/0egwLX0tdHA114gk957EWW67c4cX8jJGKLhD+rcdqsq08p8kDi1L\r\n"
    "93FcXmn/6pUCyziKrlA4b9v7LWIbxcceVOF34GfID5yHI9Y/QCB/IIDEgEw+OyQm\r\n"
    "jgSubJrIqg0CAwEAAaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMC\r\n"
    "AYYwHQYDVR0OBBYEFIQYzIU07LwMlJQuCFmcx7IQTgoIMA0GCSqGSIb3DQEBCwUA\r\n"
    "A4IBAQCY8jdaQZChGsV2USggNiMOruYou6r4lK5IpDB/G/wkjUu0yKGX9rbxenDI\r\n"
    "U5PMCCjjmCXPI6T53iHTfIUJrU6adTrCC2qJeHZERxhlbI1Bjjt/msv0tadQ1wUs\r\n"
    "N+gDS63pYaACbvXy8MWy7Vu33PqUXHeeE6V/Uq2V8viTO96LXFvKWlJbYK8U90vv\r\n"
    "o/ufQJVtMVT8QtPHRh8jrdkPSHCa2XV4cdFyQzR1bldZwgJcJmApzyMZFo6IQ6XU\r\n"
    "5MsI+yMRQ+hDKXJioaldXgjUkK642M4UwtBV8ob2xJNDd2ZhwLnoQdeXeGADbkpy\r\n"
    "rqXRfboQnoZsG4q5WTP468SQvvG5\r\n"
    "-----END CERTIFICATE-----\r\n";

static void init_iota_client() {
  g_cclient.http.path = "/";
  g_cclient.http.content_type = "application/json";
  g_cclient.http.accept = "application/json";
  g_cclient.http.host = CONFIG_IRI_NODE_URI;
  g_cclient.http.port = CONFIG_IRI_NODE_PORT;
  g_cclient.http.api_version = 1;
#ifdef CONFIG_ENABLE_HTTPS
  g_cclient.http.ca_pem = amazon_ca1_pem;
#else
  g_cclient.http.ca_pem = NULL;
#endif
  g_cclient.serializer_type = SR_JSON;
  // logger_helper_init(LOGGER_DEBUG);
  iota_client_core_init(&g_cclient);
  iota_client_extended_init();
}

static void client_send_message(char const *message) {
  retcode_t ret_code = RC_OK;
  uint8_t dummy_security = 2;  // security is ignored when sending zero-value transaction.

  bundle_transactions_t *bundle = NULL;
  bundle_transactions_new(&bundle);
  transfer_array_t *transfers = transfer_array_new();

  /* transfer setup */
  transfer_t tf = {};

  // receiver
  if (flex_trits_from_trytes(tf.address, NUM_TRITS_ADDRESS, (tryte_t const *)CONFIG_MSG_RECEIVER, NUM_TRYTES_ADDRESS,
                             NUM_TRYTES_ADDRESS) == 0) {
    ESP_LOGE(TAG, "address flex_trits convertion failed");
    goto done;
  }

  // tag
  if (flex_trits_from_trytes(tf.tag, NUM_TRITS_TAG, (tryte_t const *)"TANGLEPIGEON999999999999999", NUM_TRYTES_TAG,
                             NUM_TRYTES_TAG) == 0) {
    ESP_LOGE(TAG, "tag flex_trits convertion failed");
    goto done;
  }

  // value
  tf.value = 0;
  transfer_message_set_string(&tf, message);
  transfer_array_add(transfers, &tf);

  ret_code = iota_client_send_transfer(&g_cclient, NULL, dummy_security, CONFIG_IOTA_DEPTH, CONFIG_IOTA_MWM, false,
                                       transfers, NULL, NULL, NULL, bundle);

  ESP_LOGI(TAG, "transaction sent: %s", error_2_string(ret_code));

done:
  bundle_transactions_free(&bundle);
  transfer_message_free(&tf);
  transfer_array_free(transfers);
}

int send_message(esp_sleep_source_t trigger) {
  // prepare message
  cJSON *json_root = cJSON_CreateObject();
  if (json_root == NULL) {
    ESP_LOGE(TAG, "OOM on create json object");
    return -1;
  }

  // version element
  cJSON_AddItemToObject(json_root, "TanglePigeon", cJSON_CreateString("v0.0.1"));

  // Trigger element
  switch (trigger) {
    case ESP_SLEEP_WAKEUP_EXT1:
      cJSON_AddItemToObject(json_root, "Trigger", cJSON_CreateString("GPIO_EXT1"));
      break;
    case ESP_SLEEP_WAKEUP_EXT0:
      cJSON_AddItemToObject(json_root, "Trigger", cJSON_CreateString("GPIO_EXT0"));
      break;
    case ESP_SLEEP_WAKEUP_TIMER:
      cJSON_AddItemToObject(json_root, "Trigger", cJSON_CreateString("TIMER"));
      break;
    case ESP_SLEEP_WAKEUP_UNDEFINED:
      cJSON_AddItemToObject(json_root, "Trigger", cJSON_CreateString("POWER_ON"));
      break;
    default:
      cJSON_AddItemToObject(json_root, "Trigger", cJSON_CreateString("UNKNOWN"));
  }

  // LocalTime element
  char strftime_buf[32];
  time_t now = 0;
  struct tm timeinfo = {0};
  time(&now);
  localtime_r(&now, &timeinfo);
  strftime(strftime_buf, sizeof(strftime_buf), "%Y-%m-%d %X", &timeinfo);
  cJSON_AddItemToObject(json_root, "LocalTime", cJSON_CreateString(strftime_buf));

  cJSON_AddItemToObject(json_root, "TimeZone", cJSON_CreateString(CONFIG_SNTP_TZ));
  // data element
  cJSON_AddItemToObject(json_root, "Data", cJSON_CreateString("Hello IOTA"));

  // send message
  char const *json_text = cJSON_PrintUnformatted(json_root);
  init_iota_client();
  client_send_message(json_text);

  // clean up
  cJSON_free((void *)json_text);
  cJSON_Delete(json_root);

  return 0;
}