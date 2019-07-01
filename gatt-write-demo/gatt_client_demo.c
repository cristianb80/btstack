/*
 * Copyright (C) 2019 BlueKitchen GmbH
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holders nor the names of
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 * 4. Any redistribution, use, or modification is done solely for
 *    personal benefit and not for any commercial purpose or for
 *    monetary gain.
 *
 * THIS SOFTWARE IS PROVIDED BY BLUEKITCHEN GMBH AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL MATTHIAS
 * RINGWALD OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
 * THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * Please inquire about commercial licensing options at 
 * contact@bluekitchen-gmbh.com
 *
 */

#define __BTSTACK_FILE__ "gatt_client_demo.c"

/*
 * gatt_client_demo.c
 */

// *****************************************************************************
/* EXAMPLE_START(gatt_client_demo): Connects to 'GATT Server Demo' and execute different write types
 */
// *****************************************************************************

#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "btstack.h"

// prototypes
static void handle_gatt_client_event(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size);

typedef enum {
    TC_OFF,
    TC_IDLE,
    TC_W4_SCAN_RESULT,
    TC_W4_CONNECT,
    TC_W4_SERVICE_RESULT,
    TC_W4_CHARACTERISTIC_A_RESULT,
    TC_W4_CHARACTERISTIC_B_RESULT,
    TC_W4_CHARACTERISTIC_C_RESULT,
    TC_W4_CHARACTERISTIC_D_RESULT,
    TC_W4_PAIRING_COMPLETE,
    TC_W4_SIGNED_WRITE_COMPLETE,
    TC_W4_WRITE_WITHOUT_RESPONSE_CAN_SEND_NOW,
    TC_W4_WRITE_COMPLETE,
    TC_W4_WRITE_LONG_COMPLETE,
    TC_W4_WRITE_RELIABLE_C_COMPLETE,
    TC_W4_WRITE_RELIABLE_D_COMPLETE,
    TC_W4_WRITE_RELIABLE_ALL_COMPLETE,
} gc_state_t;


// addr and type of device with correct name
static bd_addr_t      gatt_server_demo_addr;
static bd_addr_type_t gatt_server_demo_addr_type;

static hci_con_handle_t connection_handle;

// On the GATT Server, RX Characteristic is used for receive data via Write, and TX Characteristic is used to send data via Notifications
static uint8_t gatt_server_demo_service_uuid[16]           = { 0x00, 0x00, 0xFF, 0x10, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB};
static uint8_t gatt_server_demo_characteristic_a_uuid[16] = { 0x00, 0x00, 0xFF, 0x11, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB};
static uint8_t gatt_server_demo_characteristic_b_uuid[16] = { 0x00, 0x00, 0xFF, 0x12, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB};
static uint8_t gatt_server_demo_characteristic_c_uuid[16] = { 0x00, 0x00, 0xFF, 0x13, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB};
static uint8_t gatt_server_demo_characteristic_d_uuid[16] = { 0x00, 0x00, 0xFF, 0x14, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB};

static gatt_client_service_t        gatt_server_demo_service;
static gatt_client_characteristic_t gatt_server_demo_characteristic_a;
static gatt_client_characteristic_t gatt_server_demo_characteristic_b;
static gatt_client_characteristic_t gatt_server_demo_characteristic_c;
static gatt_client_characteristic_t gatt_server_demo_characteristic_d;

static gc_state_t state = TC_OFF;
static btstack_packet_callback_registration_t hci_event_callback_registration;
static btstack_packet_callback_registration_t sm_event_callback_registration;

// returns 1 if name is found in advertisement
static int advertisement_report_contains_name(const char * name, uint8_t * advertisement_report){
    // get advertisement from report event
    const uint8_t * adv_data = gap_event_advertising_report_get_data(advertisement_report);
    uint16_t        adv_len  = gap_event_advertising_report_get_data_length(advertisement_report);
    int             name_len = strlen(name);

    // iterate over advertisement data
    ad_context_t context;
    for (ad_iterator_init(&context, adv_len, adv_data) ; ad_iterator_has_more(&context) ; ad_iterator_next(&context)){
        uint8_t data_type    = ad_iterator_get_data_type(&context);
        uint8_t data_size    = ad_iterator_get_data_len(&context);
        const uint8_t * data = ad_iterator_get_data(&context);
        int i;
        switch (data_type){
            case BLUETOOTH_DATA_TYPE_SHORTENED_LOCAL_NAME:
            case BLUETOOTH_DATA_TYPE_COMPLETE_LOCAL_NAME:
                // compare common prefix
                for (i=0; i<data_size && i<name_len;i++){
                    if (data[i] != name[i]) break;
                }
                // prefix match
                return 1;
            default:
                break;
        }
    }
    return 0;
}

static void report_characteristic(const char * name, gatt_client_characteristic_t * characteristic){
    printf("Found Characteristic %s with value handle 0x%04x\n", name, characteristic->value_handle);    
}

#ifdef ENABLE_LE_SIGNED_WRITE
static void demo_signed_write(void){
    printf("Signed write to Characteristic B - handle 0x%04x\n", gatt_server_demo_characteristic_b.value_handle);
    state = TC_W4_SIGNED_WRITE_COMPLETE;
    const char * message = "Signed Write";
    gatt_client_signed_write_without_response(handle_gatt_client_event, connection_handle, gatt_server_demo_characteristic_b.value_handle, strlen(message), (uint8_t *) message );
}
#endif

static void demo_write(void){
    printf("Write to Characteristic C - handle 0x%04x\n", gatt_server_demo_characteristic_c.value_handle);
    state = TC_W4_WRITE_COMPLETE;
    const char * message = "Normal Write";
    gatt_client_write_value_of_characteristic(handle_gatt_client_event, connection_handle, gatt_server_demo_characteristic_c.value_handle, strlen(message), (uint8_t *) message );
}

static uint8_t long_write_data[300];

static void demo_write_long(void){
    printf("Long Write to Characteristic C - handle 0x%04x\n", gatt_server_demo_characteristic_c.value_handle);
    state = TC_W4_WRITE_LONG_COMPLETE;
    unsigned int i;
    for (i=0;i<sizeof(long_write_data);i++){
        long_write_data[i] = '0' + (i % 10);
    }
    const char * message = "Long Write ";
    memcpy(long_write_data, message, strlen(message));
    gatt_client_write_value_of_characteristic(handle_gatt_client_event, connection_handle, gatt_server_demo_characteristic_c.value_handle, sizeof(long_write_data), long_write_data );
}

static void demo_write_reliable_c(void){
    printf("Prepare Write to Characteristic C - handle 0x%04x\n", gatt_server_demo_characteristic_c.value_handle);
    state = TC_W4_WRITE_RELIABLE_C_COMPLETE;
    const char * message = "Reliable Write to C";
    gatt_client_prepare_write(handle_gatt_client_event, connection_handle, gatt_server_demo_characteristic_c.value_handle, 0, strlen(message), (uint8_t *) message );
}

static void demo_write_reliable_d(void){
    printf("Prepare Write to Characteristic D - handle 0x%04x\n", gatt_server_demo_characteristic_d.value_handle);
    state = TC_W4_WRITE_RELIABLE_D_COMPLETE;
    const char * message = "Reliable Write to D";
    gatt_client_prepare_write(handle_gatt_client_event, connection_handle, gatt_server_demo_characteristic_d.value_handle, 0, strlen(message), (uint8_t *) message );
}

static void demo_write_reliable_execute(void){
    printf("Execute Prepared Write to Characteristics C and D\n");
    state = TC_W4_WRITE_RELIABLE_ALL_COMPLETE;
    gatt_client_execute_write(handle_gatt_client_event, connection_handle);
}

static void demo_write_without_response_prepare(void){
    state = TC_W4_WRITE_WITHOUT_RESPONSE_CAN_SEND_NOW;
    // request write without response
    printf("Request to send Write Without Response\n");
    gatt_client_request_can_write_without_response_event(handle_gatt_client_event, connection_handle);

}

static void demo_write_without_response_execute(void){
    printf("Write without response to Characteristic A - handle 0x%04x\n", gatt_server_demo_characteristic_a.value_handle);
    const char * message = "Write Without Response";
    gatt_client_write_value_of_characteristic_without_response(connection_handle, gatt_server_demo_characteristic_a.value_handle, strlen(message), (uint8_t *) message );

    // now... as write without response is without response, there's nothing to wait for, go to next step
    demo_write();
}

static void handle_gatt_client_event(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size){
    UNUSED(packet_type);
    UNUSED(channel);
    UNUSED(size);

    // printf("State 0x%02x, event 0x%02x\n", state, hci_event_packet_get_type(packet));

    uint16_t mtu;
    switch(state){
        case TC_W4_SERVICE_RESULT:
            switch(hci_event_packet_get_type(packet)){
                case GATT_EVENT_SERVICE_QUERY_RESULT:
                    // store service (we expect only one)
                    gatt_event_service_query_result_get_service(packet, &gatt_server_demo_service);
                    break;
                case GATT_EVENT_QUERY_COMPLETE:
                    if (packet[4] != 0){
                        printf("SERVICE_QUERY_RESULT - Error status %x.\n", packet[4]);
                        break;  
                    } 
                    // Report MTU after first GATT queries were executed
                    gatt_client_get_mtu(connection_handle, &mtu);
                    printf("ATT MTU = %u\n", mtu);

                    // service query complete, look for characteristic
                    state = TC_W4_CHARACTERISTIC_A_RESULT;
                    printf("Search for GATT Server Demo characteristic A.\n");
                    gatt_client_discover_characteristics_for_service_by_uuid128(handle_gatt_client_event, connection_handle, &gatt_server_demo_service, gatt_server_demo_characteristic_a_uuid);
                    break;
                default:
                    break;
            }
            break;
            
        case TC_W4_CHARACTERISTIC_A_RESULT:
            switch(hci_event_packet_get_type(packet)){
                case GATT_EVENT_CHARACTERISTIC_QUERY_RESULT:
                    gatt_event_characteristic_query_result_get_characteristic(packet, &gatt_server_demo_characteristic_a);
                    report_characteristic("A", &gatt_server_demo_characteristic_a);
                    break;
                case GATT_EVENT_QUERY_COMPLETE:
                    if (packet[4] != 0){
                        printf("CHARACTERISTIC_QUERY_RESULT - Error status %x.\n", packet[4]);
                        break;  
                    } 
                    // A characteristic A found, look for characteristic B
                    state = TC_W4_CHARACTERISTIC_B_RESULT;
                    printf("Search for GATT Server Demo characteristic B.\n");
                    gatt_client_discover_characteristics_for_service_by_uuid128(handle_gatt_client_event, connection_handle, &gatt_server_demo_service, gatt_server_demo_characteristic_b_uuid);
                    break;
                default:
                    break;
            }
            break;

        case TC_W4_CHARACTERISTIC_B_RESULT:
            switch(hci_event_packet_get_type(packet)){
                case GATT_EVENT_CHARACTERISTIC_QUERY_RESULT:
                    gatt_event_characteristic_query_result_get_characteristic(packet, &gatt_server_demo_characteristic_b);
                    report_characteristic("B", &gatt_server_demo_characteristic_b);
                    break;
                case GATT_EVENT_QUERY_COMPLETE:
                    if (packet[4] != 0){
                        printf("CHARACTERISTIC_QUERY_RESULT - Error status %x.\n", packet[4]);
                        break;  
                    } 
                    // A characteristic A found, look for characteristic B
                    state = TC_W4_CHARACTERISTIC_C_RESULT;
                    printf("Search for GATT Server Demo characteristic C.\n");
                    gatt_client_discover_characteristics_for_service_by_uuid128(handle_gatt_client_event, connection_handle, &gatt_server_demo_service, gatt_server_demo_characteristic_c_uuid);
                    break;
                default:
                    break;
            }
            break;

        case TC_W4_CHARACTERISTIC_C_RESULT:
            switch(hci_event_packet_get_type(packet)){
                case GATT_EVENT_CHARACTERISTIC_QUERY_RESULT:
                    gatt_event_characteristic_query_result_get_characteristic(packet, &gatt_server_demo_characteristic_c);
                    report_characteristic("C", &gatt_server_demo_characteristic_c);
                    break;
                case GATT_EVENT_QUERY_COMPLETE:
                    if (packet[4] != 0){
                        printf("CHARACTERISTIC_QUERY_RESULT - Error status %x.\n", packet[4]);
                        break;  
                    } 
                    // A characteristic A found, look for characteristic B
                    state = TC_W4_CHARACTERISTIC_D_RESULT;
                    printf("Search for GATT Server Demo characteristic D.\n");
                    gatt_client_discover_characteristics_for_service_by_uuid128(handle_gatt_client_event, connection_handle, &gatt_server_demo_service, gatt_server_demo_characteristic_d_uuid);
                    break;
                default:
                    break;
            }
            break;

        case TC_W4_CHARACTERISTIC_D_RESULT:
            switch(hci_event_packet_get_type(packet)){
                case GATT_EVENT_CHARACTERISTIC_QUERY_RESULT:
                    gatt_event_characteristic_query_result_get_characteristic(packet, &gatt_server_demo_characteristic_d);
                    report_characteristic("D", &gatt_server_demo_characteristic_d);
                    break;
                case GATT_EVENT_QUERY_COMPLETE:
                    if (packet[4] != 0){
                        printf("CHARACTERISTIC_QUERY_RESULT - Error status %x.\n", packet[4]);
                        break;  
                    } 
                    printf("All Characteristics found!\n");
                    demo_write_without_response_prepare();
                    break;
                default:
                    break;
            }
            break;

        case TC_W4_WRITE_WITHOUT_RESPONSE_CAN_SEND_NOW:
            if (hci_event_packet_get_type(packet) != GATT_EVENT_CAN_WRITE_WITHOUT_RESPONSE) break;
            demo_write_without_response_execute();
            break;
        case TC_W4_WRITE_COMPLETE:
            switch(hci_event_packet_get_type(packet)){
                case GATT_EVENT_QUERY_COMPLETE:
                    switch (gatt_event_query_complete_get_status(packet)){
                        case 0:
                            printf("Write successful.\n");
                            // next:
                            demo_write_long();
                            break;
                        default:
                            printf("Write failed - Error status %x.\n", gatt_event_query_complete_get_status(packet));
                            break;
                    }
                    break;
            }
            break;
        case TC_W4_WRITE_LONG_COMPLETE:
            switch(hci_event_packet_get_type(packet)){
                case GATT_EVENT_QUERY_COMPLETE:
                    switch (gatt_event_query_complete_get_status(packet)){
                        case 0:
                            printf("Write Long successful.\n");
                            // next:
                            demo_write_reliable_c();
                            break;
                        default:
                            printf("Write Long failed - Error status %x.\n", gatt_event_query_complete_get_status(packet));
                            break;
                    }
                    break;
            }
            break;
        case TC_W4_WRITE_RELIABLE_C_COMPLETE:
            switch(hci_event_packet_get_type(packet)){
                case GATT_EVENT_QUERY_COMPLETE:
                    switch (gatt_event_query_complete_get_status(packet)){
                        case 0:
                            printf("Reliable Write of Characteristic C successful.\n");
                            // next:
                            demo_write_reliable_d();
                            break;
                        default:
                            printf("Reliable Write of Characteristic C failed - Error status %x.\n", gatt_event_query_complete_get_status(packet));
                            break;
                    }
                    break;
            }
            break;
        case TC_W4_WRITE_RELIABLE_D_COMPLETE:
            switch(hci_event_packet_get_type(packet)){
                case GATT_EVENT_QUERY_COMPLETE:
                    switch (gatt_event_query_complete_get_status(packet)){
                        case 0:
                            printf("Reliable Write of Characteristic D successful.\n");
                            // next:
                            demo_write_reliable_execute();
                            break;
                        default:
                            printf("Reliable Write of Characteristic D failed - Error status %x.\n", gatt_event_query_complete_get_status(packet));
                            break;
                    }
                    break;
            }
            break;
        case TC_W4_WRITE_RELIABLE_ALL_COMPLETE:
            switch(hci_event_packet_get_type(packet)){
                case GATT_EVENT_QUERY_COMPLETE:
                    switch (gatt_event_query_complete_get_status(packet)){
                        case 0:
                            printf("Reliable Write execution successful.\n");
#ifdef ENABLE_LE_SIGNED_WRITE
                            demo_signed_write();
#else
                            printf("ENABLE_LE_SIGNED_WRITE not defined in btstack_config.h\n");
#endif
                            break;
                        default:
                            printf("Reliable Write execution failed - Error status %x.\n", gatt_event_query_complete_get_status(packet));
                            break;
                    }
                    break;
            }
            break;
        case TC_W4_SIGNED_WRITE_COMPLETE:
            switch(hci_event_packet_get_type(packet)){
                case GATT_EVENT_QUERY_COMPLETE:
                    switch (gatt_event_query_complete_get_status(packet)){
                        case 0:
                            printf("Signed write sent.\n");
                            printf("\nEnd of demo\n");
                            break;
                        case ATT_ERROR_BONDING_INFORMATION_MISSING:
                            printf("Signed write failed - no pairing information available. Trigger pairing...\n");
                            state = TC_W4_PAIRING_COMPLETE;
                            sm_request_pairing(connection_handle);
                            break;
                        default:
                            printf("Signed write failed - Error status %x.\n", gatt_event_query_complete_get_status(packet));
                            break;
                    }
                    break;
                default:
                    break;
            }
            break;
        default:
            printf("Unhandled state 0x%02x, event 0x%02x\n", state, hci_event_packet_get_type(packet));
            break;
    }
    
}

//  Start scan for device with "GATT SERVERr" in advertisement
static void gatt_client_demo_start(void){
    printf("Start scanning!\n");
    state = TC_W4_SCAN_RESULT;
    gap_set_scan_parameters(0,0x0030, 0x0030);
    gap_start_scan();
}

static void hci_event_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size){
    UNUSED(channel);
    UNUSED(size);

    if (packet_type != HCI_EVENT_PACKET) return;
    
    uint16_t conn_interval;
    uint8_t event = hci_event_packet_get_type(packet);
    switch (event) {
        case BTSTACK_EVENT_STATE:
            // BTstack activated, get started
            if (btstack_event_state_get_state(packet) == HCI_STATE_WORKING) {
                gatt_client_demo_start();
            } else {
                state = TC_OFF;
            }
            break;
        case GAP_EVENT_ADVERTISING_REPORT:
            if (state != TC_W4_SCAN_RESULT) return;
            // check name in advertisement
            if (!advertisement_report_contains_name("GATT SERVER", packet)) return;
            // store address and type
            gap_event_advertising_report_get_address(packet, gatt_server_demo_addr);
            gatt_server_demo_addr_type = gap_event_advertising_report_get_address_type(packet);
            // stop scanning, and connect to the device
            state = TC_W4_CONNECT;
            gap_stop_scan();
            printf("Stop scan. Connect to device with addr %s.\n", bd_addr_to_str(gatt_server_demo_addr));
            gap_connect(gatt_server_demo_addr,gatt_server_demo_addr_type);
            break;
        case HCI_EVENT_LE_META:
            // wait for connection complete
            if (hci_event_le_meta_get_subevent_code(packet) !=  HCI_SUBEVENT_LE_CONNECTION_COMPLETE) break;
            if (state != TC_W4_CONNECT) return;
            connection_handle = hci_subevent_le_connection_complete_get_connection_handle(packet);
            // print connection parameters (without using float operations)
            conn_interval = hci_subevent_le_connection_complete_get_conn_interval(packet);
            printf("Connection Interval: %u.%02u ms\n", conn_interval * 125 / 100, 25 * (conn_interval & 3));
            printf("Connection Latency: %u\n", hci_subevent_le_connection_complete_get_conn_latency(packet));  
            // initialize gatt client context with handle, and add it to the list of active clients
            // query primary services
            printf("Search for GATT Server Demo service.\n");
            state = TC_W4_SERVICE_RESULT;
            gatt_client_discover_primary_services_by_uuid128(handle_gatt_client_event, connection_handle, gatt_server_demo_service_uuid);
            break;
        case HCI_EVENT_DISCONNECTION_COMPLETE:
            printf("Disconnected %s\n", bd_addr_to_str(gatt_server_demo_addr));
            if (state == TC_OFF) break;
            gatt_client_demo_start();
            break;
        case SM_EVENT_JUST_WORKS_REQUEST:
            printf("Pairing: Just Works requeste, accepting\n");
            sm_just_works_confirm(sm_event_just_works_request_get_handle(packet));
            break;
        case SM_EVENT_PAIRING_COMPLETE:
            switch (sm_event_pairing_complete_get_status(packet)){
                case ERROR_CODE_SUCCESS:
                    printf("Pairing complete, success\n");
                    if (state == TC_W4_PAIRING_COMPLETE){
                        printf("Retry signed write..\n");
                        demo_signed_write();
                    }
                    break;
                case ERROR_CODE_CONNECTION_TIMEOUT:
                    printf("Pairing failed, timeout\n");
                    break;
                case ERROR_CODE_REMOTE_USER_TERMINATED_CONNECTION:
                    printf("Pairing faileed, disconnected\n");
                    break;
                case ERROR_CODE_AUTHENTICATION_FAILURE:
                    printf("Pairing failed, reason = %u\n", sm_event_pairing_complete_get_reason(packet));
                    break;
                default:
                    break;
            }
            break;
        default:
            break;
    }
}

int btstack_main(int argc, const char * argv[]);
int btstack_main(int argc, const char * argv[]){
    (void)argc;
    (void)argv;

    l2cap_init();

    // setup SM: No Input/Output, require bonding = storing of pairing information
    sm_init();
    sm_set_io_capabilities(IO_CAPABILITY_NO_INPUT_NO_OUTPUT);
    sm_set_authentication_requirements(SM_AUTHREQ_BONDING);

    // sm_init needed before gatt_client_init
    gatt_client_init();

    hci_event_callback_registration.callback = &hci_event_handler;
    hci_add_event_handler(&hci_event_callback_registration);

    // register for SM events
    sm_event_callback_registration.callback = &hci_event_handler;
    sm_add_event_handler(&sm_event_callback_registration);

    // turn on!
    hci_power_control(HCI_POWER_ON);

    return 0;
}
/* EXAMPLE_END */
