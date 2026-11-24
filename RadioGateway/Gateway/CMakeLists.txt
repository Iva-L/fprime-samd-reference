register_fprime_library(
    AUTOCODER_INPUTS "${CMAKE_CURRENT_LIST_DIR}/Gateway.fpp"
    SOURCES "${CMAKE_CURRENT_LIST_DIR}/Gateway.cpp"
    DEPENDS RadioGateway_RadioTransport
)

register_fprime_ut(
    AUTOCODER_INPUTS "${CMAKE_CURRENT_LIST_DIR}/Gateway.fpp"
    SOURCES "${CMAKE_CURRENT_LIST_DIR}/test/ut/GatewayTest.cpp"
    DEPENDS RadioGateway_RadioTransport
    UT_AUTO_HELPERS
)
