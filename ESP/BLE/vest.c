#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_bt.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_bt_main.h"
#include "esp_gap_ble_api.h"
#include "esp_gatts_api.h"
esp_ble_adv_data_t adv_data;
uint16_t srvc_handle;
uint16_t char_handle;

esp_attr_value_t attri_value;
uint8_t curr_val = 'b';

char bda[18];
void set_adv_data()
{
    adv_data.set_scan_rsp = false;
    adv_data.include_name = false;
    adv_data.include_txpower = false;
    adv_data.min_interval = 0x50;
    adv_data.max_interval = 0x50;
    adv_data.appearance = 0x00;
    adv_data.manufacturer_len = 0x00;
    adv_data.p_manufacturer_data = NULL;
    adv_data.flag = (ESP_BLE_ADV_FLAG_GEN_DISC | ESP_BLE_ADV_FLAG_BREDR_NOT_SPT);
    adv_data.service_data_len = 0;
    adv_data.p_service_data = NULL;
    esp_ble_gap_config_adv_data(&adv_data);
}


void my_gap_cb(esp_gap_ble_cb_event_t event,esp_ble_gap_cb_param_t *param)
{
    switch(event)
    {
        case ESP_GAP_BLE_ADV_DATA_SET_COMPLETE_EVT:
            printf("advertising data set\n");
            esp_ble_adv_params_t adv_params;
            adv_params.adv_int_min = 0x0020;
            adv_params.adv_int_max = 0x0020;
            adv_params.adv_type = ADV_TYPE_IND;
            adv_params.own_addr_type = BLE_ADDR_TYPE_PUBLIC;
            adv_params.peer_addr_type = BLE_ADDR_TYPE_PUBLIC;
            adv_params.channel_map = ADV_CHNL_ALL;
            adv_params.adv_filter_policy = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY;
            esp_ble_gap_start_advertising(&adv_params);
            break;
        default:
            printf("default case\n");
            break;
    }
}

char  * bda2str(esp_bd_addr_t bda_in)
{
    sprintf(bda,"%02x:%02x:%02x:%02x:%02x:%02x",bda_in[0],bda_in[1],bda_in[2],bda_in[3],bda_in[4],bda_in[5]);
    return bda;
}

void server_callback(esp_gap_ble_cb_event_t event,esp_gatt_if_t gatts_if,esp_ble_gatts_cb_param_t * param)
{
    switch(event)
    {
        case ESP_GATTS_REG_EVT:
            printf("registered application\n");
            esp_gatt_srvc_id_t service_1 = {
                .is_primary = true,
                .id.inst_id = 0,
                .id.uuid.len = ESP_UUID_LEN_16,
                .id.uuid.uuid.uuid16 = 0x1234,
            };
            esp_ble_gatts_create_service(gatts_if,&service_1,4);
            break;

        case ESP_GATTS_CREATE_EVT:
            printf("service created\n");
            if(param->create.status == ESP_GATT_OK)
            {
                srvc_handle = param->create.service_handle;
            }   
            esp_bt_uuid_t my_char = {
                .len = ESP_UUID_LEN_16,
                .uuid.uuid16 = 0x5678,
            };
            esp_gatt_char_prop_t char_prop = ESP_GATT_CHAR_PROP_BIT_READ | ESP_GATT_CHAR_PROP_BIT_WRITE;
            attri_value.attr_max_len = 10;
            attri_value.attr_len = 1;
            attri_value.attr_value = &curr_val;
            esp_ble_gatts_add_char(srvc_handle,&my_char,ESP_GATT_PERM_WRITE | ESP_GATT_PERM_READ,char_prop,&attri_value,NULL);
            esp_ble_gatts_start_service(srvc_handle);
            break;
        case ESP_GATTS_ADD_CHAR_EVT:
            printf("character added\n");
            char_handle = param->add_char.attr_handle;
            break;
        case ESP_GATTS_CONNECT_EVT:
            printf("client has connected\n");
            printf("client bda : %s\n",bda2str(param->connect.remote_bda));
            break;

        case  ESP_GATTS_WRITE_EVT:
            printf("client has written\n");
            uint16_t length;
            const uint8_t val;
            printf("the device that has been written to : %s\n",bda2str(param->write.bda));
            printf("the value that is written : %d\n",*(param->write.value));
            break;
        default:
            printf("some shit is happening\n");
            break;
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
    const char * bname = "vest1";
    esp_ble_gap_set_device_name(bname);
    esp_ble_gap_register_callback(my_gap_cb);
    set_adv_data();
    esp_ble_gatts_register_callback(server_callback);
    esp_ble_gatts_app_register(0x55);
}
