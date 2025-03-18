#include "ble_provider.h"
#include "pwm.h"

#define MAX_DATA_LEN 256
#define BLE_SAMPLE_SIZE 375
#define FEATURES 4

char stored_data[MAX_DATA_LEN] = "No Data";
uint16_t rate_handle;
uint16_t conn_handle = BLE_HS_CONN_HANDLE_NONE;
bool notify_enabled = false;
uint8_t compressed_data[BLE_SAMPLE_SIZE];
char *TAG = "BLE-Server";
uint8_t ble_addr_type;
static const char* gestures[] = {"Circle", "Cross", "Pad", "Fixed"};

// Array of pointers to other service definitions
// UUID - Universal Unique Identifier
const struct ble_gatt_svc_def gatt_svcs[] = {
    {.type = BLE_GATT_SVC_TYPE_PRIMARY,
     .uuid = BLE_UUID128_DECLARE(0x2c9fdd5d, 0x990c, 0x4801, 0x80c6, 0xac7eff021a8f), // Define UUID for device type
     .characteristics = (struct ble_gatt_chr_def[]){
         {.uuid = BLE_UUID128_DECLARE(0xba851a41, 0x1fd6, 0x4501, 0x8ad0, 0xf715adf43e4e), // Rate Measurement characteristic
          .flags = BLE_GATT_CHR_F_NOTIFY,
          .access_cb = rate_notify, // Callback for notifications
          .val_handle = &rate_handle},
         {.uuid = BLE_UUID128_DECLARE(0xe24d9433, 0x9255, 0x4880, 0xb56e, 0xc09529224418), // Define UUID for reading
          .flags = BLE_GATT_CHR_F_READ,
          .access_cb = device_read},
         {.uuid = BLE_UUID128_DECLARE(0xca561598, 0xf8ba, 0x41a4, 0xbcc1, 0x17c269355adc), // Define UUID for writing
          .flags = BLE_GATT_CHR_F_WRITE,
          .access_cb = device_write},
         {0}}},
    {0}};

void ble_app_advertise(void)
{
    struct ble_hs_adv_fields fields;
    const char *device_name;
    memset(&fields, 0, sizeof(fields));
    device_name = ble_svc_gap_device_name();
    fields.name = (uint8_t *)device_name;
    fields.name_len = strlen(device_name);
    fields.name_is_complete = 1;
    ble_gap_adv_set_fields(&fields);

    struct ble_gap_adv_params adv_params;
    memset(&adv_params, 0, sizeof(adv_params));
    adv_params.conn_mode = BLE_GAP_CONN_MODE_UND;
    adv_params.disc_mode = BLE_GAP_DISC_MODE_GEN;
    ble_gap_adv_start(ble_addr_type, NULL, BLE_HS_FOREVER, &adv_params, ble_gap_event, NULL);
}

int rate_notify(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    os_mbuf_append(ctxt->om, compressed_data, BLE_SAMPLE_SIZE);
    return 0;
}

int device_write(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    int len = ctxt->om->om_len;
    if (len >= MAX_DATA_LEN)
    {
        len = MAX_DATA_LEN - 1;
    }
    memcpy(stored_data, ctxt->om->om_data, len);
    stored_data[len] = '\0';
    printf("Data from the client: %.*s\n", ctxt->om->om_len, ctxt->om->om_data);

    int duty = 0;
    memcpy(&duty, ctxt->om->om_data, len);
    printf("The value of duty is: %d\n", duty);
    pwm_set_duty(duty);

    return 0;
}

int device_read(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    os_mbuf_append(ctxt->om, stored_data, strlen(stored_data));
    return 0;
}

void ble_app_on_sync(void)
{
    ble_hs_id_infer_auto(0, &ble_addr_type);
    ble_app_advertise();
}

void host_task(void *param)
{
    nimble_port_run();
}

int ble_gap_event(struct ble_gap_event *event, void *arg)
{
    switch (event->type)
    {
    case BLE_GAP_EVENT_CONNECT:
        ESP_LOGI(TAG, "BLE GAP EVENT CONNECT %s", event->connect.status == 0 ? "OK!" : "FAILED!");
        if (event->connect.status != 0)
        {
            ble_app_advertise();
            pwm_start();
        }
        break;
    case BLE_GAP_EVENT_SUBSCRIBE:
        ESP_LOGI(TAG, "Subscribe event");
        break;
    case BLE_GAP_EVENT_DISCONNECT:
        ESP_LOGI(TAG, "Device disconnected, restarting advertising...");
        ble_app_advertise();
        pwm_stop();
        break;
    case BLE_GAP_EVENT_ADV_COMPLETE:
        ESP_LOGI("GAP", "BLE GAP EVENT");
        ble_app_advertise();
        break;
    default:
        break;
    }
    return 0;
}

void ble_init()
{
    nvs_flash_init();                          // 1 - Initialize NVS flash using
    esp_nimble_hci_and_controller_init();      // 2 - Initialize ESP controller
    nimble_port_init();                        // 3 - Initialize the host stack
    ble_svc_gap_device_name_set("BLE-Server"); // 4 - Initialize NimBLE configuration - server name
    ble_svc_gap_init();                        // 4 - Initialize NimBLE configuration - gap service
    ble_svc_gatt_init();                       // 4 - Initialize NimBLE configuration - gatt service
    ble_gatts_count_cfg(gatt_svcs);            // 4 - Initialize NimBLE configuration - config gatt services
    ble_gatts_add_svcs(gatt_svcs);             // 4 - Initialize NimBLE configuration - queues gatt services.
    ble_hs_cfg.sync_cb = ble_app_on_sync;      // 5 - Initialize application

    nimble_port_freertos_init(host_task);
    pwm_init();
}

void ble_loop(void *param)
{
    float *data = NULL;
    QueueHandle_t sensorQueue = (QueueHandle_t *)param;

    while (1)
    {
        if (xQueueReceive(sensorQueue, &data, portMAX_DELAY) == pdPASS)
        {
            // for (int i = 0; i < BLE_SAMPLE_SIZE/3; i++) {
            //     ESP_LOGI(TAG, "%.4f, %.4f, %.4f", data[3 * i], data[3 * i + 1], data[3 * i + 2]);
            // }

            for (int i = 0; i < BLE_SAMPLE_SIZE; i++)
            {
                //Quantization of the data to 8 bits
                compressed_data[i] = (int8_t)((data[i] + 2) * 255.0 / 4 - 128.0);
            }
            
            float prediction[FEATURES];
            memcpy(prediction, &data[BLE_SAMPLE_SIZE], FEATURES * sizeof(float));

            int predicted_class = 0;
            float max_confidence = prediction[0];

            // Looking for a class with maximum confidence
            for (int i = 1; i < FEATURES; i++) {
                if (prediction[i] > max_confidence) {
                    max_confidence = prediction[i];
                    predicted_class = i;
                }
            }

            snprintf(stored_data, MAX_DATA_LEN, "%s, %.4f", gestures[predicted_class], max_confidence);

            ESP_LOGI(TAG, "prediction data:");
            for (int i = 0; i < FEATURES; i++) {
                ESP_LOGI(TAG, "[%d]: %.4f", i, prediction[i]);
            }
        

            ble_gatts_chr_updated(rate_handle); // Trigger BLE sending
        }
        // vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}