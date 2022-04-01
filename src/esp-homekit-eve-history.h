//
//  esp-homekit-eve-history.h
//
//  Created by David B Brown on 23/12/2021.
//  Copyright © 2021 maccoylton. All rights reserved.
//

#ifndef esp_homekit_eve_history_h
#define esp_homekit_eve_history_h

#define EVE_HISTORY_LOG_HEADER_SIZE 10
#define EVE_HISTORY_STATUS_SIZE 37
#define EVE_EPOCH_OFFSET    978307200
#define EVE_HISTORY_MAX_LOG_ENTRIES 10

#define HOMEKIT_CUSTOM_UUID_EVE_HISTORY(value) (value"-079E-48FF-8F27-9C2605A29F52")

#define HOMEKIT_SERVICE_EVE_HISTORY "E863F007-079E-48FF-8F27-9C2605A29F52"

#define HOMEKIT_CHARACTERISTIC_CUSTOM_EVE_HISTORY_S2R1 HOMEKIT_CUSTOM_UUID_EVE_HISTORY("E863F116")
#define HOMEKIT_DECLARE_CHARACTERISTIC_CUSTOM_EVE_HISTORY_S2R1(_value, _size, ...)\
.type = HOMEKIT_CHARACTERISTIC_CUSTOM_EVE_HISTORY_S2R1, \
.description = "S2R1", \
.format = homekit_format_data, \
.permissions = homekit_permissions_paired_read \
| homekit_permissions_paired_write \
| homekit_permissions_notify \
| homekit_permissions_hidden, \
.value = HOMEKIT_DATA_( _value, _size), \
##__VA_ARGS__


#define HOMEKIT_CHARACTERISTIC_CUSTOM_EVE_HISTORY_S2R2 HOMEKIT_CUSTOM_UUID_EVE_HISTORY("E863F117")
#define HOMEKIT_DECLARE_CHARACTERISTIC_CUSTOM_EVE_HISTORY_S2R2(_value, _size, ...)\
.type = HOMEKIT_CHARACTERISTIC_CUSTOM_EVE_HISTORY_S2R2, \
.description = "S2R2", \
.format = homekit_format_data, \
.permissions = homekit_permissions_paired_read \
| homekit_permissions_paired_write \
| homekit_permissions_notify \
| homekit_permissions_hidden, \
.value = HOMEKIT_DATA_( _value, _size), \
##__VA_ARGS__


#define HOMEKIT_CHARACTERISTIC_CUSTOM_EVE_HISTORY_S2W1 HOMEKIT_CUSTOM_UUID_EVE_HISTORY("E863F11C")
#define HOMEKIT_DECLARE_CHARACTERISTIC_CUSTOM_EVE_HISTORY_S2W1(...)\
.type = HOMEKIT_CHARACTERISTIC_CUSTOM_EVE_HISTORY_S2W1, \
.description = "S2W1", \
.format = homekit_format_data, \
.permissions = homekit_permissions_paired_write \
| homekit_permissions_hidden, \
##__VA_ARGS__


#define HOMEKIT_CHARACTERISTIC_CUSTOM_EVE_HISTORY_S2W2 HOMEKIT_CUSTOM_UUID_EVE_HISTORY("E863F121")
#define HOMEKIT_DECLARE_CHARACTERISTIC_CUSTOM_EVE_HISTORY_S2W2(...)\
.type = HOMEKIT_CHARACTERISTIC_CUSTOM_EVE_HISTORY_S2W2, \
.description = "S2W2", \
.format = homekit_format_data, \
.permissions = homekit_permissions_paired_write \
| homekit_permissions_hidden, \
##__VA_ARGS__

typedef enum {
    eve_history_ref_timestamp_log=0x81,
    eve_history_weather_log=0x07,
    eve_history_energy_log=0x1f,
    eve_histroy_energy2_log=0x1e,
    eve_history_room_log=0x0f,
    eve_history_motion_log=0x02,
    eve_history_door_log= 0x01,
    eve_history_thermo_log=0x1f,
    eve_history_aqua_on_log=0x05,
    eve_history_aqa_off=0x07
} eve_history_entry_type_t;


typedef struct {
    uint32_t actual_time;
    uint32_t negative_offset;
    uint32_t reference_time;
    uint8_t signature_size;
    uint64_t signature;
    uint16_t last_memory_position;
    uint16_t history_size;
    uint32_t memory_rolling_address;
    uint32_t unknown;
    uint16_t fixed;
} eve_history_status_t ;

typedef struct {
    uint8_t length;
    uint32_t entry_counter;
    uint32_t seconds_since_ref_time;
    uint8_t entry_type;
} eve_history_log_header_t ;


typedef struct{
    uint16_t unknown;
    uint16_t unknown2;
    uint16_t power_times_10;
    uint16_t unknow3;
    uint16_t unknow4;
} eve_history_power_t;


extern eve_history_status_t eve_history_status;
extern eve_history_log_header_t eve_history_log_header;
extern byte* eve_history_status_buffer;
extern byte* eve_history_log_buffer;
extern eve_history_power_t power_log[EVE_HISTORY_MAX_LOG_ENTRIES];

homekit_value_t getter_EVE_history_status_S2R1_116 (homekit_value_t value);
/* */

homekit_value_t getter_EVE_history_entries_S2R2_117 (homekit_value_t value);
/* */


void setter_EVE_history_request_S2W1_11C (homekit_value_t value);
/*  */

void setter_EVE_set_time_S2W2_121 (homekit_value_t value);
/*  */

void eve_history_send_log(uint32_t starting_from_address);



#endif /* esp_homekit_eve_history_h */

