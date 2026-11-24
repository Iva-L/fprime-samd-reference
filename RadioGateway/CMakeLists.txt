add_fprime_subdirectory("${CMAKE_CURRENT_LIST_DIR}/Gateway")

restrict_platforms(Samd21)
add_fprime_subdirectory("${CMAKE_CURRENT_LIST_DIR}/Top")

set(CMAKE_CXX_LINK_LIBRARY_USING_WHOLE_ARCHIVE
    "-Wl,--whole-archive" "<LINK_ITEM>" "-Wl,--no-whole-archive")
set(CMAKE_CXX_LINK_LIBRARY_USING_WHOLE_ARCHIVE_SUPPORTED TRUE)

register_fprime_deployment(
    SOURCES "${CMAKE_CURRENT_LIST_DIR}/Main.cpp"
    DEPENDS RadioGateway_Top
    CHOOSES_IMPLEMENTATIONS Os_CountingSemaphore_Stub
)

target_link_libraries(RadioGateway PRIVATE
    fprime_samd_mcu
    -Wl,--whole-archive
    fprime-samd_Drv_RtcDriver
    fprime-samd_Drv_DmaDriver
    fprime-samd_Drv_GpioDriver
    fprime-samd_Drv_SpiDriver
    fprime-samd_Drv_Types
    -Wl,--no-whole-archive
)
set_property(TARGET RadioGateway PROPERTY LINK_LIBRARY_OVERRIDE_RadioGateway_Top WHOLE_ARCHIVE)
finalize_samd21_executable(RadioGateway)
