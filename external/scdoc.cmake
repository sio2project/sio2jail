IF(DEFINED SCDOC_BUILD_OWN AND NOT "${SCDOC_BUILD_OWN}" MATCHES "YES|NO")
    MESSAGE(FATAL_ERROR "SCDOC_BUILD_OWN should be one of: YES, NO")
ENDIF()

ADD_CUSTOM_TARGET(scdoc)

IF(NOT DEFINED SCDOC_BUILD_OWN OR SCDOC_BUILD_OWN STREQUAL "NO")
    FIND_FILE(
        scdoc_BINARY_PATH
        NAMES scdoc
        PATHS "${SCDOC_PREFIX}" "${SCDOC_PREFIX}/usr/bin" "${SCDOC_PREFIX}/bin"
        )
ENDIF()

IF((NOT DEFINED SCDOC_BUILD_OWN AND NOT EXISTS "${scdoc_BINARY_PATH}") OR SCDOC_BUILD_OWN STREQUAL "YES")
    ExternalProject_Add(scdoc_project
        URL https://git.sr.ht/%7Esircmpwn/scdoc/archive/1.5.2.tar.gz
        URL_HASH SHA256=86591de3741bea5443e7fbc11ff9dc22da90621105b06be524422efd5dec3a29

        CONFIGURE_COMMAND
            true
        BUILD_IN_SOURCE true
        BUILD_COMMAND
            make PREFIX=<INSTALL_DIR>
        INSTALL_COMMAND
            make PREFIX=<INSTALL_DIR> install
        )

    ExternalProject_Get_Property(scdoc_project
        INSTALL_DIR)
    SET(scdoc_BINARY_PATH ${INSTALL_DIR}/bin/scdoc)

    ADD_DEPENDENCIES(scdoc scdoc_project)
ENDIF()

IF(scdoc_BINARY_PATH MATCHES NOTFOUND)
    MESSAGE(FATAL_ERROR "scdoc binary not found, run with -DSCDOC_BUILD_OWN=YES to build one from source")
ENDIF()

MESSAGE("-- Using scdoc from: ${scdoc_BINARY_PATH}")
