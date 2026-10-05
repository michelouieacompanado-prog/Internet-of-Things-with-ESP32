#include "app_ble.h"
#include <string.h>
#include <inttypes.h>
#include "esp_log.h"
#include "esp_ble_mesh_defs.h"
#include "esp_ble_mesh_common_api.h"
#include "esp_ble_mesh_networking_api.h"
#include "esp_ble_mesh_provisioning_api.h"
#include "esp_ble_mesh_config_model_api.h"
#include "esp_ble_mesh_generic_model_api.h"
#include "esp_ble_mesh_health_model_api.h"

#define TAG "app_ble"
#define CID_ESP 0x02E5

static uint8_t dev_uuid[16];

static esp_ble_mesh_cfg_srv_t config_server = {
    .relay = ESP_BLE_MESH_RELAY_DISABLED,
    .beacon = ESP_BLE_MESH_BEACON_ENABLED,
#if defined(CONFIG_BLE_MESH_FRIEND)
    .frnd = ESP_BLE_MESH_FRIEND_ENABLED,
#else
    .frnd = ESP_BLE_MESH_FRIEND_NOT_SUPPORTED,
#endif
#if defined(CONFIG_BLE_MESH_GATT_PROXY_SERVER)
    .gatt_proxy = ESP_BLE_MESH_GATT_PROXY_ENABLED,
#else
    .gatt_proxy = ESP_BLE_MESH_GATT_PROXY_NOT_SUPPORTED,
#endif
    .default_ttl = 7,
    .net_transmit = ESP_BLE_MESH_TRANSMIT(2, 20),
    .relay_retransmit = ESP_BLE_MESH_TRANSMIT(2, 20),
};

static esp_ble_mesh_health_srv_t health_server = {
};

ESP_BLE_MESH_MODEL_PUB_DEFINE(health_pub_0, 2 + 11, ROLE_NODE);
ESP_BLE_MESH_MODEL_PUB_DEFINE(onoff_pub_0, 2 + 1, ROLE_NODE);

static esp_ble_mesh_gen_onoff_srv_t onoff_server_0 = {
    .rsp_ctrl.get_auto_rsp = ESP_BLE_MESH_SERVER_RSP_BY_APP,
    .rsp_ctrl.set_auto_rsp = ESP_BLE_MESH_SERVER_RSP_BY_APP,
};

static esp_ble_mesh_model_t root_models[] = {
    ESP_BLE_MESH_MODEL_CFG_SRV(&config_server),
    ESP_BLE_MESH_MODEL_GEN_ONOFF_SRV(&onoff_pub_0, &onoff_server_0),
    ESP_BLE_MESH_MODEL_HEALTH_SRV(&health_server, &health_pub_0),
};

static esp_ble_mesh_elem_t elements[] = {
    ESP_BLE_MESH_ELEMENT(0, root_models, ESP_BLE_MESH_MODEL_NONE),
};

static esp_ble_mesh_comp_t composition = {
    .cid = CID_ESP,
    .elements = elements,
    .element_count = ARRAY_SIZE(elements),
};

static esp_ble_mesh_prov_t provision = {
    .uuid = dev_uuid,
    .output_size = 0,
    .output_actions = 0,
};

static uint8_t s_onoff_state = 0;

static void generic_server_cb(esp_ble_mesh_generic_server_cb_event_t event,
                              esp_ble_mesh_generic_server_cb_param_t *param)
{
    ESP_LOGI(TAG, "generic_server_cb: event 0x%02x, opcode 0x%04" PRIx32 ", model id 0x%04x",
             event, param->ctx.recv_op, param->model->model_id);

    switch (event) {
    case ESP_BLE_MESH_GENERIC_SERVER_STATE_CHANGE_EVT:
        ESP_LOGI(TAG, "ESP_BLE_MESH_GENERIC_SERVER_STATE_CHANGE_EVT");
        if (param->ctx.recv_op == ESP_BLE_MESH_MODEL_OP_GEN_ONOFF_SET ||
            param->ctx.recv_op == ESP_BLE_MESH_MODEL_OP_GEN_ONOFF_SET_UNACK) {
            ESP_LOGI(TAG, "OnOff Server state set to: %d", param->value.state_change.onoff_set.onoff);
            s_onoff_state = param->value.state_change.onoff_set.onoff;
        }
        break;
    case ESP_BLE_MESH_GENERIC_SERVER_RECV_GET_MSG_EVT:
        ESP_LOGI(TAG, "ESP_BLE_MESH_GENERIC_SERVER_RECV_GET_MSG_EVT");
        if (param->ctx.recv_op == ESP_BLE_MESH_MODEL_OP_GEN_ONOFF_GET) {
            esp_ble_mesh_server_model_send_msg(param->model, &param->ctx,
                                               ESP_BLE_MESH_MODEL_OP_GEN_ONOFF_STATUS,
                                               sizeof(s_onoff_state), &s_onoff_state);
        }
        break;
    case ESP_BLE_MESH_GENERIC_SERVER_RECV_SET_MSG_EVT:
        ESP_LOGI(TAG, "ESP_BLE_MESH_GENERIC_SERVER_RECV_SET_MSG_EVT");
        if (param->ctx.recv_op == ESP_BLE_MESH_MODEL_OP_GEN_ONOFF_SET ||
            param->ctx.recv_op == ESP_BLE_MESH_MODEL_OP_GEN_ONOFF_SET_UNACK) {
            s_onoff_state = param->value.set.generic.onoff.onoff;
            if (param->ctx.recv_op == ESP_BLE_MESH_MODEL_OP_GEN_ONOFF_SET) {
                esp_ble_mesh_server_model_send_msg(param->model, &param->ctx,
                                                   ESP_BLE_MESH_MODEL_OP_GEN_ONOFF_STATUS,
                                                   sizeof(s_onoff_state), &s_onoff_state);
            }
        }
        break;
    default:
        break;
    }
}

void appble_update_state(bool onoff)
{
    s_onoff_state = onoff ? 1 : 0;
    ESP_LOGI(TAG, "appble_update_state: publishing onoff=%d", s_onoff_state);
    esp_err_t err = esp_ble_mesh_model_publish(onoff_server_0.model,
                                               ESP_BLE_MESH_MODEL_OP_GEN_ONOFF_STATUS,
                                               sizeof(s_onoff_state), &s_onoff_state,
                                               ROLE_NODE);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "esp_ble_mesh_model_publish failed: %s", esp_err_to_name(err));
    }
}

void init_ble(appble_attn_cbs_t cbs)
{
    appble_set_attn_cbs(cbs);
    ESP_ERROR_CHECK(appble_bt_init());
    appble_get_dev_uuid(dev_uuid);

    esp_ble_mesh_register_prov_callback(appble_provisioning_handler);
    esp_ble_mesh_register_config_server_callback(appble_config_handler);
    esp_ble_mesh_register_generic_server_callback(generic_server_cb);
    esp_ble_mesh_register_health_server_callback(appble_health_evt_handler);

    ESP_ERROR_CHECK(esp_ble_mesh_init(&provision, &composition));
    ESP_ERROR_CHECK(esp_ble_mesh_node_prov_enable(ESP_BLE_MESH_PROV_ADV | ESP_BLE_MESH_PROV_GATT));
    ESP_LOGI(TAG, "BLE Mesh light sensor node initialized successfully");
}
