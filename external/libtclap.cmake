IF(DEFINED LIBTCLAP_BUILD_OWN AND NOT "${LIBTCLAP_BUILD_OWN}" MATCHES "YES|NO")
    MESSAGE(FATAL_ERROR "LIBTCLAP_BUILD_OWN should be one of: YES, NO")
ENDIF()

ADD_CUSTOM_TARGET(libtclap)

IF(NOT DEFINED LIBTCLAP_BUILD_OWN OR LIBTCLAP_BUILD_OWN STREQUAL "NO")
    FIND_PATH(
        libtclap_INC_PATH
        NAMES tclap/CmdLine.h
        PATHS "${LIBTCLAP_PREFIX}" "${LIBTCLAP_PREFIX}/usr/include"
        )
ENDIF()

IF((NOT DEFINED LIBTCLAP_BUILD_OWN AND NOT EXISTS "${libtclap_INC_PATH}") OR LIBTCLAP_BUILD_OWN STREQUAL "YES")
    ExternalProject_Add(libtclap_project
        URL https://netcologne.dl.sourceforge.net/project/tclap/tclap-1.2.2.tar.gz
        URL_HASH SHA256=f5013be7fcaafc69ba0ce2d1710f693f61e9c336b6292ae4f57554f59fde5837

        CONFIGURE_COMMAND
            <SOURCE_DIR>/configure --prefix=<INSTALL_DIR>
        BUILD_COMMAND
            true
        INSTALL_COMMAND
            make -C <BINARY_DIR>/include install
        )

    ExternalProject_Get_Property(libtclap_project
        INSTALL_DIR)
    SET(libtclap_INC_PATH "${INSTALL_DIR}/include")

    ADD_DEPENDENCIES(libtclap libtclap_project)
ENDIF()

IF(libtclap_INC_PATH MATCHES NOTFOUND)
    MESSAGE(FATAL_ERROR "libtclap not found, run with -DLIBTCLAP_BUILD_OWN=YES to build it from source")
ENDIF()

MESSAGE("-- Libtclap configuration:")
MESSAGE("-- - include directory: ${libtclap_INC_PATH}")

INCLUDE_DIRECTORIES("${libtclap_INC_PATH}")
