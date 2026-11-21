####
# config-samd-reference/CMakeLists.txt
####

# Native unit-test builds link the POSIX OSAL, whose Os handles are larger than the
# SAMD21-tuned FW_*_HANDLE_MAX_SIZE maxima in PlatformCfg.fpp. Fall back to the F Prime
# defaults under BUILD_TESTING and apply the trimmed handle sizes only to flight builds.
if(BUILD_TESTING)
    set(PLATFORM_CFG_OVERRIDE "")
    set(ASYNC_SPI_CFG_OVERRIDE "")
    # The upstream radio tests exercise the library's default queue depth and
    # reset timing; the smaller deployment settings are for the SAMD21 image.
    if(SAMD_RADIO_TEST_DEPLOYMENT_CONFIG)
        set(RADIO_CFG_OVERRIDE "${CMAKE_CURRENT_LIST_DIR}/Rfm9XManagerCfg.fpp")
    else()
        set(RADIO_CFG_OVERRIDE "")
    endif()
else()
    set(PLATFORM_CFG_OVERRIDE "${CMAKE_CURRENT_LIST_DIR}/PlatformCfg.fpp")
    set(ASYNC_SPI_CFG_OVERRIDE "${CMAKE_CURRENT_LIST_DIR}/AsyncSpiCfg.fpp")
    set(RADIO_CFG_OVERRIDE "${CMAKE_CURRENT_LIST_DIR}/Rfm9XManagerCfg.fpp")
endif()

# Same treatment, different reason: this override SHADOWS
# lib/fprime-samd/default/samd-config/FramerConfig.hpp rather than extending it, and this
# repository is the harness fprime-samd's own CI builds in. A constant omitted here is
# therefore undeclared for the library's unit tests too -- which is how adding
# MAX_TRACKED_APIDS upstream broke them. Under BUILD_TESTING the library's tests see the
# library's defaults, which is what they are written against. Flight builds keep the
# trimmed FRAMER_TX_BUFFER_SIZE, and must still declare everything the cross `all` target
# compiles, TmFramer included.
if(BUILD_TESTING)
    set(SAMD_FRAMER_CFG_OVERRIDE "")
else()
    set(SAMD_FRAMER_CFG_OVERRIDE "${CMAKE_CURRENT_LIST_DIR}/FramerConfig.hpp")
endif()

register_fprime_config(
        config-samd-reference
    CONFIGURATION_OVERRIDES
        # --- overrides lib/fprime/default/config ---
        "${CMAKE_CURRENT_LIST_DIR}/AcConstants.fpp"
        ${ASYNC_SPI_CFG_OVERRIDE}
        "${CMAKE_CURRENT_LIST_DIR}/ComCfg.fpp"
        "${CMAKE_CURRENT_LIST_DIR}/FpConfig.fpp"
        "${CMAKE_CURRENT_LIST_DIR}/FpConstants.fpp"
        ${PLATFORM_CFG_OVERRIDE}
        "${CMAKE_CURRENT_LIST_DIR}/FpConfig.h"
        "${CMAKE_CURRENT_LIST_DIR}/BufferManagerComponentImplCfg.hpp"
        "${CMAKE_CURRENT_LIST_DIR}/CommandDispatcherImplCfg.hpp"
        "${CMAKE_CURRENT_LIST_DIR}/PassiveRateGroupCfg.hpp"
        "${CMAKE_CURRENT_LIST_DIR}/StaticMemoryConfig.hpp"
        ${RADIO_CFG_OVERRIDE}
        # --- overrides lib/fprime-samd/default/samd-config ---
        ${SAMD_FRAMER_CFG_OVERRIDE}
    INTERFACE
)
