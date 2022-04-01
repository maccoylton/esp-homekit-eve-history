//
//  esp-homekit-eve-history.h
//
//  Created by David B Brown on 23/12/2021.
//  Copyright © 2021 maccoylton. All rights reserved.
//

#include <stdio.h>
#include <espressif/esp_wifi.h>
#include <espressif/esp_sta.h>
#include <espressif/esp_common.h>
#include <esp/uart.h>
#include <esp8266.h>
#include <FreeRTOS.h>
#include <task.h>
#include <time.h>
#include <string.h>

#include <homekit/homekit.h>
#include <homekit/characteristics.h>


#include <esp-homekit-eve-history.h>

#include <stdio.h>

#define REV(n) ((n << 24) | (((n>>16)<<24)>>16) | (((n<<16)>>24)<<16) | (n>>24))

eve_history_status_t eve_history_status;
eve_history_log_header_t eve_history_log_header;

byte* eve_history_status_buffer;
byte* eve_history_log_buffer;


homekit_value_t getter_EVE_history_status_S2R1_116 (homekit_value_t value){
    
    printf("%s:\n", __func__);
    if ( !eve_history_status_buffer){
        printf("%s:\n Allocations status buffer", __func__);
        eve_history_status_buffer = malloc (37);
    }
    
    memcpy(eve_history_status_buffer, &eve_history_status.actual_time, 4);
    memcpy(eve_history_status_buffer+4, &eve_history_status.negative_offset, 4);
    memcpy(eve_history_status_buffer+8, &eve_history_status.reference_time, 4);
    memcpy(eve_history_status_buffer+12, &eve_history_status.signature_size, 1);
    memcpy(eve_history_status_buffer+13, &eve_history_status.signature, 10);
    memcpy(eve_history_status_buffer+23, &eve_history_status.last_memory_position, 2);
    memcpy(eve_history_status_buffer+25, &eve_history_status.history_size, 2);
    memcpy(eve_history_status_buffer+27, &eve_history_status.memory_rolling_address, 4);
    memcpy(eve_history_status_buffer+31, &eve_history_status.unknown, 4);
    memcpy(eve_history_status_buffer+35, &eve_history_status.fixed, 2);
    
    return (HOMEKIT_DATA(eve_history_status_buffer, 37, .is_static=true) );
}



homekit_value_t getter_EVE_history_entries_S2R2_117 (homekit_value_t value){
/* */
    printf("%s:\n", __func__);
    if ( !eve_history_log_buffer){
        printf("%s:\n Allocations status buffer", __func__);
        eve_history_log_buffer = malloc (10+10*EVE_HISTORY_MAX_LOG_ENTRIES);
    }
    
    memcpy( eve_history_log_buffer, &eve_history_log_header.length, 1);
    memcpy( eve_history_log_buffer+1, &eve_history_log_header.entry_counter, 4);
    memcpy( eve_history_log_buffer+5, &eve_history_log_header.seconds_since_ref_time, 4);
    memcpy( eve_history_log_buffer+9, &eve_history_log_header.entry_type, 1);
    int memory_offset=0;
    for (int i=0; i < eve_history_log_header.entry_counter; i++) {
        memset(eve_history_log_buffer+9+i+1+memory_offset, 0, 4);
        memcpy(eve_history_log_buffer+9+i+5+memory_offset, &power_log[i].power_times_10,2);
        memset(eve_history_log_buffer+9+i+7+memory_offset, 0, 4);
        memory_offset+=10; 
    }
    
 //   return (HOMEKIT_DATA(&eve_history_log_header,EVE_HISTORY_LOG_HEADER_SIZE));
 //   return (HOMEKIT_DATA(NULL, 0) );
    return (HOMEKIT_DATA(eve_history_log_buffer, 10+(10*eve_history_log_header.entry_counter), .is_static=true));
    
}


void setter_EVE_history_request_S2W1_11C (homekit_value_t value){
/*  */

    uint32_t eve_history_requested_memory_entry = 0;
    printf("%s: Got request to send history:" , __func__);
    for (int i=0; i< value.data_size ; i++){
        printf ("%03d %02x ", value.data_value[i], value.data_value[i]);
    }
    
    eve_history_requested_memory_entry = (uint32_t) (value.data_value[0] | (value.data_value[1] << 8) |  (value.data_value[2]<<16) |  (value.data_value[3] << 24));
    printf (" entries from address: %d\n ", eve_history_requested_memory_entry );
    
}


void setter_EVE_set_time_S2W2_121 (homekit_value_t value){
/*  */
    time_t now;
    time_t eve_epoch_raw = EVE_EPOCH_OFFSET;
    time_t eve_offset_raw;
    time_t eve_history_time;
    
    
    printf("%s: Got timestamp from Eve\n", __func__);
    for (int i=0; i< value.data_size ; i++){
        printf ("%03d %02x ", value.data_value[i], value.data_value[i] );
    }
    printf ( "Offset: %u\n", ( value.data_value[0] | (value.data_value[1] << 8) |  (value.data_value[2]<<16) |  (value.data_value[3] << 24)));
   
    eve_offset_raw  = (time_t) ( value.data_value[0] | (value.data_value[1] << 8) |  (value.data_value[2]<<16) |  (value.data_value[3] << 24));
    eve_history_time = eve_epoch_raw + eve_offset_raw;
    printf ( "Time now: %s, Eve Time: %s, Offset:%u\n", ctime(&now), ctime(&eve_history_time), (uint32_t) eve_offset_raw);
}




