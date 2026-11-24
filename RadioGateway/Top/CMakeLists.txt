register_fprime_module(
    AUTOCODER_INPUTS
        "${CMAKE_CURRENT_LIST_DIR}/instances.fpp"
        "${CMAKE_CURRENT_LIST_DIR}/topology.fpp"
    SOURCES "${CMAKE_CURRENT_LIST_DIR}/TopTopology.cpp"
    DEPENDS
        Svc_TlmPacketizer_config_TlmPacketizerConfig
)
