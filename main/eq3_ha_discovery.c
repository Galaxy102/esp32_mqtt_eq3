#include "eq3_ha_discovery.h"
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <esp_netif.h>
#include <esp_log.h>

#if (__STDC_VERSION__ >= 199901L)
#include <stdint.h>
#endif

#define HA_TAG "HA_DISCOVERY"

cJSON* generate_ha_therm_payload (char mac[6]) {
    char rawmacstr[7];
    char macstr[18];

    snprintf (rawmacstr, sizeof(rawmacstr), "%02X%02X%02X", mac[3], mac[4], mac[5]);
    snprintf (macstr, sizeof(macstr), "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

    ESP_LOGI (HA_TAG, "rawmacstr: %s", rawmacstr);

    cJSON* root = cJSON_CreateObject ();
    // "name": "eq3_{{rawmacstr}}_thermostat"
    char buffer[40];
    snprintf (buffer, sizeof (buffer), "eq3_%s_thermostat", rawmacstr);
    cJSON_AddStringToObject (root, "name", buffer);
    // "unique_id": "eq3_YYYYYY_thermostat"
    cJSON_AddStringToObject (root, "unique_id", buffer);
    // "device": {
    cJSON* device;
    cJSON_AddItemToObject (root, "device", device = cJSON_CreateObject ());
    // "name": "Equiva EQ-3 BT YYYYYY"
    snprintf (buffer, sizeof (buffer), "eq3_Equiva EQ-3 BT %s", rawmacstr);
    cJSON_AddStringToObject (device, "name", buffer);
    // "configuration_url": "http://123.123.123.123"
    esp_netif_t* netif = NULL;
    netif = esp_netif_get_handle_from_ifkey ("WIFI_STA_DEF");
    esp_netif_ip_info_t ipInfo;
    esp_netif_get_ip_info (netif, &ipInfo);
    uint8_t* ip_address_bytes = (uint8_t*)&(ipInfo.ip.addr);
    snprintf (buffer, sizeof (buffer), "http://%u.%u.%u.%u", ip_address_bytes[0], ip_address_bytes[1], ip_address_bytes[2], ip_address_bytes[3]);
    cJSON_AddStringToObject (device, "configuration_url", buffer);
    // "manufacturer": "Equiva"
    cJSON_AddStringToObject (device, "manufacturer", "Equiva");
    // "model": "EQ-3 BT"
    cJSON_AddStringToObject (device, "model", "EQ-3 BT");
    // "identifiers": ["XX:XX:XX:YY:YY:YY"]
    cJSON* identifiers = cJSON_AddArrayToObject (device, "identifiers");
    //cJSON* identifier = cJSON_CreateString (macstr);
    cJSON_AddItemToArray (identifiers, cJSON_CreateString (macstr));
    // "modes": ["off", "heat", "auto"]
    cJSON* modes = cJSON_AddArrayToObject (root, "modes");
    cJSON_AddItemToArray (modes, cJSON_CreateString ("off"));
    cJSON_AddItemToArray (modes, cJSON_CreateString ("heat"));
    cJSON_AddItemToArray (modes, cJSON_CreateString ("auto"));
    // "max_temp": 29.5
    cJSON_AddNumberToObject (root, "max_temp", 29.5);
    // "min_temp": 5
    cJSON_AddNumberToObject (root, "min_temp", 5);
    // "precision": 0.5
    cJSON_AddNumberToObject (root, "precision", 0.5);
    // "temp_step": 0.5
    cJSON_AddNumberToObject (root, "temp_step", 0.5);
    // "temperature_unit": "C"
    cJSON_AddStringToObject (root, "temperature_unit", "C");
    // "mode_command_topic": "eq3_radin/trv/XX:XX:XX:YY:YY:YY/mode"
    snprintf (buffer, sizeof (buffer), "eq3_radin/trv/%s/mode", macstr);
    cJSON_AddStringToObject (root, "mode_command_topic", buffer);
    // "json_attributes_topic": "eq3_radout/status/XX:XX:XX:YY:YY:YY"
    snprintf (buffer, sizeof (buffer), "eq3_radout/status/%s", macstr);
    cJSON_AddStringToObject (root, "json_attributes_topic", buffer);
    //"temperature_command_topic": "eq3_radin/trv/XX:XX:XX:YY:YY:YY/settemp"
    snprintf (buffer, sizeof (buffer), "eq3_radin/trv/%s/settemp", macstr);
    cJSON_AddStringToObject (root, "temperature_command_topic", buffer);
    //"temperature_state_topic" : "eq3_radout/status/XX:XX:XX:YY:YY:YY"
    snprintf (buffer, sizeof (buffer), "eq3_radout/status/%s", macstr);
    cJSON_AddStringToObject (root, "temperature_state_topic", buffer);
    //"current_temperature_topic" : "eq3_radout/status/XX:XX:XX:YY:YY:YY",
    cJSON_AddStringToObject (root, "current_temperature_topic", buffer);
    //"mode_state_topic": "eq3_radout/status/XX:XX:XX:YY:YY:YY"
    cJSON_AddStringToObject (root, "mode_state_topic", buffer);
    //"mode_state_template" : "{{ value_json.mode_ha }}"
    cJSON_AddStringToObject (root, "mode_state_template", "{{ value_json.mode_ha }}");
    //"temperature_state_template" : "{{ value_json.temp }}"
    snprintf (buffer, sizeof (buffer), "{{ value_json.temp }}");
    cJSON_AddStringToObject (root, "temperature_state_template", buffer);
    // "current_temperature_template" : "{{ value_json.temp }}"
    cJSON_AddStringToObject (root, "current_temperature_template", buffer);

    char* ha_therm_payload = cJSON_Print (root);
    ESP_LOGI (HA_TAG, "HA Climate autodiscovery\n%s", ha_therm_payload);

    return root;
}

cJSON* generate_ha_valve_payload (char mac[6]) {
    char rawmacstr[7];
    char macstr[18];

    snprintf (rawmacstr, sizeof (rawmacstr), "%02X%02X%02X", mac[3], mac[4], mac[5]);
    snprintf (macstr, sizeof (macstr), "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

    ESP_LOGI (HA_TAG, "rawmacstr: %s", rawmacstr);

    cJSON* root = cJSON_CreateObject ();
    // "name": "eq3_{{rawmacstr}}_valve"
    char buffer[40];
    snprintf (buffer, sizeof (buffer), "eq3_%s_valve", rawmacstr);
    cJSON_AddStringToObject (root, "name", buffer);
    // "unique_id": "eq3_YYYYYY_valve"
    cJSON_AddStringToObject (root, "unique_id", buffer);
    // "device": {
    cJSON* device;
    cJSON_AddItemToObject (root, "device", device = cJSON_CreateObject ());
    // "name": "Equiva EQ-3 BT YYYYYY"
    snprintf (buffer, sizeof (buffer), "eq3_Equiva EQ-3 BT %s", rawmacstr);
    cJSON_AddStringToObject (device, "name", buffer);
    // "configuration_url": "http://123.123.123.123"
    esp_netif_t* netif = NULL;
    netif = esp_netif_get_handle_from_ifkey ("WIFI_STA_DEF");
    esp_netif_ip_info_t ipInfo;
    esp_netif_get_ip_info (netif, &ipInfo);
    uint8_t* ip_address_bytes = (uint8_t*)&(ipInfo.ip.addr);
    snprintf (buffer, sizeof (buffer), "http://%u.%u.%u.%u", ip_address_bytes[0], ip_address_bytes[1], ip_address_bytes[2], ip_address_bytes[3]);
    cJSON_AddStringToObject (device, "configuration_url", buffer);
    // "manufacturer": "Equiva"
    cJSON_AddStringToObject (device, "manufacturer", "Equiva");
    // "model": "EQ-3 BT"
    cJSON_AddStringToObject (device, "model", "EQ-3 BT");
    // "identifiers": ["XX:XX:XX:YY:YY:YY"]
    cJSON* identifiers = cJSON_AddArrayToObject (device, "identifiers");
    cJSON_AddItemToArray (identifiers, cJSON_CreateString (macstr));
    // "device_class": "power"
    cJSON_AddStringToObject (root, "device_class", "power");
    // "unit_of_measurement": "%"
    cJSON_AddStringToObject (root, "unit_of_measurement", "%");
    // "state_topic": "eq3_radout/status/XX:XX:XX:YY:YY:YY",
    snprintf (buffer, sizeof (buffer), "eq3_radout/status/%s", macstr);
    cJSON_AddStringToObject (root, "state_topic", buffer);
    // "value_template": "{{ value_json.valve }}"
    cJSON_AddStringToObject (root, "value_template", "{{ value_json.valve }}");

    char* ha_valve_payload = cJSON_Print (root);
    ESP_LOGI (HA_TAG, "HA Valve autodiscovery\n%s", ha_valve_payload);

    return root;
}

cJSON* generate_ha_battery_payload (char mac[6]) {
    char rawmacstr[7];
    char macstr[18];

    snprintf (rawmacstr, sizeof (rawmacstr), "%02X%02X%02X", mac[3], mac[4], mac[5]);
    snprintf (macstr, sizeof (macstr), "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

    ESP_LOGI (HA_TAG, "rawmacstr: %s", rawmacstr);

    cJSON* root = cJSON_CreateObject ();
    // "name": "eq3_{{rawmacstr}}_battery"
    char buffer[40];
    snprintf (buffer, sizeof (buffer), "eq3_%s_battery", rawmacstr);
    cJSON_AddStringToObject (root, "name", buffer);
    // "unique_id": "eq3_YYYYYY_battery"
    cJSON_AddStringToObject (root, "unique_id", buffer);
    // "device": {
    cJSON* device;
    cJSON_AddItemToObject (root, "device", device = cJSON_CreateObject ());
    // "name": "Equiva EQ-3 BT YYYYYY"
    snprintf (buffer, sizeof (buffer), "eq3_Equiva EQ-3 BT %s", rawmacstr);
    cJSON_AddStringToObject (device, "name", buffer);
    // "configuration_url": "http://123.123.123.123"
    esp_netif_t* netif = NULL;
    netif = esp_netif_get_handle_from_ifkey ("WIFI_STA_DEF");
    esp_netif_ip_info_t ipInfo;
    esp_netif_get_ip_info (netif, &ipInfo);
    uint8_t* ip_address_bytes = (uint8_t*)&(ipInfo.ip.addr);
    snprintf (buffer, sizeof (buffer), "http://%u.%u.%u.%u", ip_address_bytes[0], ip_address_bytes[1], ip_address_bytes[2], ip_address_bytes[3]);
    cJSON_AddStringToObject (device, "configuration_url", buffer);
    // "manufacturer": "Equiva"
    cJSON_AddStringToObject (device, "manufacturer", "Equiva");
    // "model": "EQ-3 BT"
    cJSON_AddStringToObject (device, "model", "EQ-3 BT");
    // "identifiers": ["XX:XX:XX:YY:YY:YY"]
    cJSON* identifiers = cJSON_AddArrayToObject (device, "identifiers");
    cJSON_AddItemToArray (identifiers, cJSON_CreateString (macstr));
    // "device_class": "power"
    cJSON_AddStringToObject (root, "device_class", "battery");
    // "state_topic": "eq3_radout/status/XX:XX:XX:YY:YY:YY",
    snprintf (buffer, sizeof (buffer), "eq3_radout/status/%s", macstr);
    cJSON_AddStringToObject (root, "state_topic", buffer);
    // "value_template": "{{ value_json.battery }}"
    cJSON_AddStringToObject (root, "value_template", "{{ value_json.battery }}");
    // "payload_on": "GOOD"
    cJSON_AddStringToObject (root, "payload_off", "GOOD");
    // "payload_off": "LOW"
    cJSON_AddStringToObject (root, "payload_on", "LOW");

    char* ha_valve_payload = cJSON_Print (root);
    ESP_LOGI (HA_TAG, "HA Battery autodiscovery\n%s", ha_valve_payload);

    return root;
}

