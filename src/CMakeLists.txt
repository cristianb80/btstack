set(CODICE
    ad_parser.c
    btstack_linked_list.c
    btstack_memory.c
    btstack_memory_pool.c
    #btstack_ring_buffer.c : non usato
    btstack_run_loop.c
    btstack_slip.c
    btstack_util.c
    hci.c
    hci_cmd.c
    #hci_dump.c : implementation specific
    hci_transport_h4.c
    #hci_transport_h5.c : non usato
    l2cap.c
    l2cap_signaling.c
    classic/bnep.c
    classic/btstack_cvsd_plc.c
    classic/btstack_link_key_db_memory.c
    #classic/btstack_sbc_bludroid.c ???
    classic/btstack_sbc_plc.c
    classic/hfp.c
    classic/hfp_ag.c
    classic/hfp_gsm_model.c
    classic/hfp_hf.c
    classic/hfp_msbc.c
    classic/hsp_ag.c 
    classic/hsp_hs.c 
    classic/pan.c
    classic/rfcomm.c
    classic/sdp_client.c
    classic/sdp_client_rfcomm.c
    classic/sdp_server.c
    classic/sdp_util.c
    classic/spp_server.c
    ble/ancs_client.c
    ble/att_db.c
    ble/att_db_util.c
    ble/att_dispatch.c
    ble/att_server.c
    ble/gatt_client.c
    ble/le_device_db_memory.c
    ble/sm.c
    ble/sm_mbedtls_allocator.c
)

include_directories("${PROJECT_SOURCE_DIR}/per_cmake")
include_directories("${PROJECT_SOURCE_DIR}/src")

add_library(src OBJECT ${CODICE})
