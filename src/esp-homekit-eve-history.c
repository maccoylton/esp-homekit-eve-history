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

byte* eve_history_status_buffer; /* used as a buffer to send data to Eve, log entries are packed into here */
byte* eve_history_log_send_buffer; /* used to store log entries */


homekit_value_t getter_EVE_history_status_S2R1_116 (homekit_value_t value){
    
    /*
     This read-only characteristic is used by the accessory to signal how many entries are in the log (and other infos). Comparing this characteristics over different type of accessory, it was possible to obtain the following partial decoding. Data is composed by a fixed size portion (12 bytes) with info about time, 1 byte indicating the length of the following variable length portion with accessory "signature" and finally a fixed length portion with info about memory status.
     
     4 bytes: Actual time, in seconds from last time update
     4 bytes: negative offset of reference time
     4 bytes: reference time/last Accessory time update (taken from E863F117-079E-48FF-8F27-9C2605A29F52)
     1 byte: number of 16 bits word of the following "signature" portion
     2-12 bytes: variable length "signature"
     2 bytes: last physical memory position occupied (used by Eve.app to understand how many transfers are needed). If set to an address lower than the last successfully uploaded entry, forces Eve.app to start from the beginning of the memory, asking address 00 in E863F11C. Accessory answers with entry 01. Once the memory is fully written and memory overwriting is necessary this field remains equal to history size.
     2 bytes: history size
     4 bytes: once memory rolling occurred it indicates the address of the oldest entry present in memory (if memory rolling did not occur yet, these bytes are at 0)
     4 bytes:??
     2 bytes:?? always 01ff or 0101

    */
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
    /* E863F117-079E-48FF-8F27-9C2605A29F52
     
     This read-only characteristics is used to send the actual log entry to Eve.app It is an array of logs with each entry having x bytes as determined by Byte 1. The first portion up to byte 10 included is common, the last portion contain the data specific for each accessory.
    
     Byte 1: Length (i.e. 14 for 20 Bytes)
     Bytes 2-5: entry counter
     Bytes 6-9: Seconds since reference time set with type 0x81 entry. In order to account for multiple iOS devices, the actual reference time and offset are apparently always reported also in E863F116
     Byte 10: Entry type
     */
    
    printf("%s:\n", __func__);
    if ( !eve_history_log_send_buffer){
        printf("%s:\n Allocations status buffer", __func__);
        eve_history_log_send_buffer = malloc (EVE_HISTORY_LOG_HEADER_SIZE+(EVE_HISTORY_LOG_ENERGY_SIZE*EVE_HISTORY_MAX_LOG_ENTRIES));
    }
    
    memcpy( eve_history_log_send_buffer, &eve_history_log_header.length, 10);
    memcpy( eve_history_log_send_buffer+1, &eve_history_log_header.entry_counter, 4);
    memcpy( eve_history_log_send_buffer+5, &eve_history_log_header.seconds_since_ref_time, 4);
    memcpy( eve_history_log_send_buffer+9, &eve_history_log_header.entry_type, 1);
    int buffer_offset=10; /* header is 10 bytes, so log entries stat at array position 10*/
    /* Eve Energy (20 bytes in total)
    
    Bytes 11 & 12: ??
    Bytes 13 & 14: ??
    Bytes 15 & 16: Power multiplied by 10. Eve.app will assume that the same power is drawn for all the sampling time (10 minutes), so it will show a point with an energy consumption equal to this value divided by 60. Example. Your appliance is consuming 1000W, which means a total energy of 1kWh if run for 1 hour. The value reported is 1000 x 10. The value shown is 1000 x 10 / 60 = 166Wh, which is correct because this sample covers 10min, i.e. 1/6 of an hour. At the end of the hour, Eve.app will show 6 samples at 166Wh, totalizing 1kWh.
        Bytes 17 & 18: ??
        Bytes 19 & 20: ??
        */
    for (int i=0; i < eve_history_log_header.entry_counter; i++) {
        memset(eve_history_log_send_buffer+buffer_offset, 0, 4);
        memcpy(eve_history_log_send_buffer+buffer_offset+4, &power_log[i].power_times_10,2);
        memset(eve_history_log_send_buffer+buffer_offset+6, 0, 4);
        buffer_offset+=EVE_HISTORY_LOG_ENERGY_SIZE;
    }
    
 //   return (HOMEKIT_DATA(&eve_history_log_header,EVE_HISTORY_LOG_HEADER_SIZE));
 //   return (HOMEKIT_DATA(NULL, 0) );
    return (HOMEKIT_DATA(eve_history_log_send_buffer, EVE_HISTORY_LOG_HEADER_SIZE+(EVE_HISTORY_LOG_ENERGY_SIZE*eve_history_log_header.entry_counter), .is_static=true));
    
}


void setter_EVE_history_request_S2W1_11C (homekit_value_t value){
    /* This write-only characteristic seem to control data flux from accessory to Eve.app. A typical value when reading from a fake Eve Weather accessory is 01140100 000000. Tentative decoding:
     
     byte 1: ??
     byte 2: ??
     byte 3-6: Requested memory entry, based on the last entry that Eve.app downloaded. If set to 0000, asks the accessory the start restart from the beginning of the memory
     byte 7-8: ??
     */

    uint32_t eve_history_requested_memory_entry = 0;
    printf("%s: Got request to send history:" , __func__);
    for (int i=0; i< value.data_size ; i++){
        printf ("%03d %02x ", value.data_value[i], value.data_value[i]);
    }
    
    eve_history_requested_memory_entry = (uint32_t) (value.data_value[2] | (value.data_value[3] << 8) |  (value.data_value[4]<<16) |  (value.data_value[5] << 24));
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




