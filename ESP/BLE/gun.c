#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_bt.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_bt_main.h"
#include "esp_gap_ble_api.h"
#include "esp_gattc_api.h"
#include <string.h>

char bda[18];
char des_bda[] = "2c:bc:bb:06:61:86";
esp_gatt_if_t gatt_client;
esp_gattc_service_elem_t res;
esp_gattc_char_elem_t res_char;
uint16_t n_char = 1;
void set_scan_params()
{
    esp_ble_scan_params_t scan_params;
    scan_params.scan_type = BLE_SCAN_TYPE_ACTIVE;
    scan_params.own_addr_type = BLE_ADDR_TYPE_PUBLIC;
    scan_params.scan_filter_policy = BLE_SCAN_FILTER_ALLOW_ALL;
    scan_params.scan_interval = 0x1000;
    scan_params.scan_window = 0x1000;
    scan_params.scan_duplicate = BLE_SCAN_DUPLICATE_ENABLE;
    esp_ble_gap_set_scan_params(&scan_params);
}

char  * bda2str(esp_bd_addr_t bda_in)
{
    sprintf(bda,"%02x:%02x:%02x:%02x:%02x:%02x",bda_in[0],bda_in[1],bda_in[2],bda_in[3],bda_in[4],bda_in[5]);
    return bda;
}

void my_gap_cb(esp_gap_ble_cb_event_t event,esp_ble_gap_cb_param_t *param)
{
    switch(event)
    {
        case ESP_GAP_BLE_SCAN_PARAM_SET_COMPLETE_EVT:
            printf("successfully set scan params\n");
            esp_ble_gap_start_scanning(10);
            break;
        case ESP_GAP_BLE_SCAN_RESULT_EVT:
            if(param->scan_rst.search_evt==ESP_GAP_SEARCH_INQ_RES_EVT)
            {
                if(strcmp(bda2str(param->scan_rst.bda),des_bda)==0)
                {
                    esp_ble_gap_stop_scanning();
                    esp_ble_gattc_open(gatt_client,param->scan_rst.bda,param->scan_rst.ble_addr_type,true);
                }
                printf("%s\n",bda2str(param->scan_rst.bda));
                if(param->scan_rst.ble_evt_type == ESP_BLE_EVT_CONN_ADV)
                {
                    printf("unidirected connectable advertising\n");
                }
                else
                {
                    printf("this isn't the expected result\n");
                }
            }
            break;
        case ESP_GAP_BLE_SCAN_STOP_COMPLETE_EVT:
            printf("scan has stopped\n");
            break;
        default:
            printf("default case\n");
            break;
    }
}

void client_cb(esp_gattc_cb_event_t event,esp_gatt_if_t gattc_if,esp_ble_gattc_cb_param_t * param)
{
    switch(event)
    {
        case ESP_GATTC_REG_EVT:
            printf("registered\n");
            gatt_client = gattc_if;
            break;
        case ESP_GATTC_OPEN_EVT:
            printf("connection has been set up and is open\n");
            esp_bt_uuid_t srvc_uuid;
            srvc_uuid.len = ESP_UUID_LEN_16;
            srvc_uuid.uuid.uuid16 = 0x1234;
            if(esp_ble_gattc_search_service(gattc_if,param->open.conn_id,&srvc_uuid)==ESP_OK)
            {
                printf("successfully searched for the service\n");
            }
            break;
        case ESP_GATTC_SEARCH_RES_EVT:
            printf("search result from service search\n");
            printf("service uuid : %li\n",param->search_res.srvc_id.uuid.uuid.uuid32);
            uint16_t offlen = 0;
            esp_bt_uuid_t srvic_uuid;
            srvic_uuid.len = ESP_UUID_LEN_16;
            srvic_uuid.uuid.uuid16 = 0x1234;
            esp_gatt_status_t rc;
            rc = esp_ble_gattc_get_service(gattc_if,param->search_res.conn_id,&srvic_uuid,&res,&n_char,offlen);
            if(rc == ESP_OK)
            {
                printf("all good\n");
            }
            rc = esp_ble_gattc_get_all_char(gattc_if,param->search_res.conn_id,param->search_res.start_handle,param->search_res.end_handle,&res_char,&n_char,offlen);
            if(rc == ESP_OK)
            {
                printf("char also good.. char uuid : %i\n",res_char.uuid.uuid.uuid16);
            }
            uint8_t val = 'yashwanth';
            esp_err_t rc1;
            rc1 = esp_ble_gattc_write_char(gattc_if,param->search_res.conn_id,res_char.char_handle,1,&val,ESP_GATT_WRITE_TYPE_NO_RSP,ESP_GATT_AUTH_REQ_NONE);
            if(rc1 == ESP_OK)
            {
                printf("successfully written\n");
            }
            break;
        case ESP_GATTC_READ_CHAR_EVT:
            printf("here is my read function\n");
            break;
        case ESP_GATTC_WRITE_CHAR_EVT:
            printf("here is my write function\n");
            break;
        default:
            printf("default case (from client)\n");
    }
}

void app_main(void)
{
    nvs_flash_init();
    esp_bt_controller_config_t my_bt_controller = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    esp_bt_controller_init(&my_bt_controller);
    esp_bt_controller_enable(ESP_BT_MODE_BLE);
    esp_bluedroid_init();
    esp_bluedroid_enable();
    const char * bname = "gun1";
    esp_ble_gap_set_device_name(bname);
    esp_ble_gap_register_callback(my_gap_cb);
    set_scan_params();
    esp_ble_gattc_register_callback(client_cb);
    esp_ble_gattc_app_register(0x00);
}
