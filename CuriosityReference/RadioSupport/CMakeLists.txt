register_fprime_module(
    AUTOCODER_INPUTS
        "${CMAKE_CURRENT_LIST_DIR}/RadioSupport.fpp"
    SOURCES
        "${CMAKE_CURRENT_LIST_DIR}/RadioSupport.cpp"
    DEPENDS
        fprime-sensors_Rfm9X_Components_Rfm9XManager_Rfm9XManagerCfg
)

# Reuse the radio manager's generated test harness to exercise the real
# manager and support component together against the register-level model.
set(RADIO_MANAGER_TEST_DIR "${CMAKE_BINARY_DIR}/fprime-sensors/Rfm9X/Components/Rfm9XManager")
set(RADIO_MANAGER_TEST_BASES
    "${RADIO_MANAGER_TEST_DIR}/Rfm9XManagerTesterBase.cpp"
    "${RADIO_MANAGER_TEST_DIR}/Rfm9XManagerGTestBase.cpp"
)
set_source_files_properties(${RADIO_MANAGER_TEST_BASES} PROPERTIES GENERATED TRUE)

register_fprime_ut(
    AUTOCODER_INPUTS
        "${CMAKE_CURRENT_LIST_DIR}/RadioSupport.fpp"
    SOURCES
        "${CMAKE_CURRENT_LIST_DIR}/test/ut/RadioSupportTestMain.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/test/ut/RadioSupportTester.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/test/ut/RadioEchoTest.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/../../lib/fprime-sensors/fprime-sensors/Rfm9X/Components/Rfm9XSim/Rfm9XSimModel.cpp"
    DEPENDS
        STest
        fprime-sensors_Rfm9X_Components_Rfm9XManager
    UT_AUTO_HELPERS
)

if(BUILD_TESTING)
    target_sources(CuriosityReference_RadioSupport_ut_exe PRIVATE ${RADIO_MANAGER_TEST_BASES})
    add_dependencies(CuriosityReference_RadioSupport_ut_exe fprime-sensors_Rfm9X_Components_Rfm9XManager_ut_exe)
    target_include_directories(CuriosityReference_RadioSupport_ut_exe PRIVATE "${RADIO_MANAGER_TEST_DIR}")
endif()
