IF(DEFINED LIBCAP_BUILD_OWN AND NOT "${LIBCAP_BUILD_OWN}" MATCHES "YES|NO")
    MESSAGE(FATAL_ERROR "LIBCAP_BUILD_OWN should be one of: YES, NO")
ENDIF()

ADD_CUSTOM_TARGET(libcap)

IF(NOT DEFINED LIBCAP_BUILD_OWN OR LIBCAP_BUILD_OWN STREQUAL "NO")
    FIND_PATH(
        libcap_INC_PATH
        NAMES sys/capability.h
        PATHS "${LIBCAP_PREFIX}" "${LIBCAP_PREFIX}/usr/include"
        )
ENDIF()

IF((NOT DEFINED LIBCAP_BUILD_OWN AND NOT EXISTS "${libcap_INC_PATH}") OR LIBCAP_BUILD_OWN STREQUAL "YES")
    ExternalProject_Add(libcap_project
        URL https://mirrors.edge.kernel.org/pub/linux/libs/security/linux-privs/libcap2/libcap-2.25.tar.xz
        URL_HASH SHA256=693c8ac51e983ee678205571ef272439d83afe62dd8e424ea14ad9790bc35162

        PATCH_COMMAND
            patch -p1 -i ${CMAKE_CURRENT_LIST_DIR}/libcap.patch
        CONFIGURE_COMMAND
            true
        BUILD_IN_SOURCE true
        BUILD_COMMAND
            make FAKEROOT=<INSTALL_DIR>
        INSTALL_COMMAND
            make FAKEROOT=<INSTALL_DIR> RAISE_SETFCAP=no install
        )

    ExternalProject_Get_Property(libcap_project
        INSTALL_DIR)
    SET(libcap_INC_PATH "${INSTALL_DIR}/usr/include")

    ADD_DEPENDENCIES(libcap libcap_project)
ENDIF()

IF(libcap_INC_PATH MATCHES NOTFOUND)
    MESSAGE(FATAL_ERROR "libcap not found, run with -DLIBCAP_BUILD_OWN=YES to build it from source")
ENDIF()

MESSAGE("-- Libcap configuration:")
MESSAGE("-- - include directory: ${libcap_INC_PATH}")

INCLUDE_DIRECTORIES("${libcap_INC_PATH}")
