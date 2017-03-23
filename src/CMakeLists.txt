# Compilo solo quelli che mi interessano
set(CODICE
    ad_parser.c
    btstack_linked_list.c
    btstack_memory.c
    btstack_memory_pool.c
    btstack_run_loop.c
    btstack_util.c
    hci.c
    hci_cmd.c
    #hci_dump.c : implementation specific
    hci_transport_h4.c
    l2cap.c
    l2cap_signaling.c
    classic/bnep.c
    classic/btstack_cvsd_plc.c
    #classic/btstack_link_key_db_memory.c : implementation specific
    classic/rfcomm.c
    classic/sdp_server.c
    classic/sdp_util.c
)

include_directories("${PROJECT_SOURCE_DIR}/per_cmake")
include_directories("${PROJECT_SOURCE_DIR}/src")

add_library(src OBJECT ${CODICE})
