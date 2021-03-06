//
// btstack_config.h for WICED port
//

#ifndef __BTSTACK_CONFIG
#define __BTSTACK_CONFIG

#include "utili.h"


static inline void bts_vprintf(const char * fmt, va_list arg)
{
	UNUSED(fmt) ;
	UNUSED(arg) ;
}
static inline void bts_printf(const char * fmt, ...)
{
	UNUSED(fmt) ;
}

#define BTSTACK_PRINTF		bts_printf
#define BTSTACK_VPRINTF		bts_vprintf


// MZ: non voglio che usi la malloc (HAVE_MALLOC)
#define MAX_ATT_DB_SIZE 	200

// Port related features
#define HAVE_EMBEDDED_TIME_MS
#define WICED_BT_UART_MANUAL_CTS_RTS

// BTstack features that can be enabled
#define ENABLE_BLE
#define ENABLE_CLASSIC
#define ENABLE_LE_PERIPHERAL
// #define ENABLE_LE_SECURE_CONNECTIONS

#define ENABLE_LOG_DEBUG
#define ENABLE_LOG_INFO
#define ENABLE_LOG_ERROR

#define MAX_NR_LE_DEVICE_DB_ENTRIES		1

// BTstack configuration. buffers, sizes, ...
#define HCI_ACL_PAYLOAD_SIZE 52
#define MAX_SPP_CONNECTIONS 1
#define MAX_NR_GATT_CLIENTS 0
#define MAX_NR_GATT_SUBCLIENTS 0
#define MAX_NR_HCI_CONNECTIONS MAX_SPP_CONNECTIONS
#define MAX_NR_L2CAP_SERVICES  2
#define MAX_NR_L2CAP_CHANNELS  (1+MAX_SPP_CONNECTIONS)
#define MAX_NR_RFCOMM_MULTIPLEXERS MAX_SPP_CONNECTIONS
#define MAX_NR_RFCOMM_SERVICES 1
#define MAX_NR_RFCOMM_CHANNELS MAX_SPP_CONNECTIONS
#define MAX_NR_BTSTACK_LINK_KEY_DB_MEMORY_ENTRIES 2
#define MAX_NR_BNEP_SERVICES 0
#define MAX_NR_BNEP_CHANNELS 0
#define MAX_NR_HFP_CONNECTIONS 0
#define MAX_NR_WHITELIST_ENTRIES 1
#define MAX_NR_SM_LOOKUP_ENTRIES 3
#define MAX_NR_SERVICE_RECORD_ITEMS 1

#endif
