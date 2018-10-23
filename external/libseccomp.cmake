IF(DEFINED LIBSECCOMP_BUILD_OWN AND NOT "${LIBSECCOMP_BUILD_OWN}" MATCHES "YES|NO")
    MESSAGE(FATAL_ERROR "LIBSECCOMP_BUILD_OWN should be one of: YES, NO")
ENDIF()

ADD_LIBRARY(seccomp STATIC IMPORTED)

IF(NOT DEFINED LIBSECCOMP_BUILD_OWN OR LIBSECCOMP_BUILD_OWN STREQUAL "NO")
    FIND_FILE(
        libseccomp_LIB_PATH
        NAMES libseccomp.a
        PATHS "${LIBSECCOMP_PREFIX}" "${LIBSECCOMP_PREFIX}/lib" "${LIBSECCOMP_PREFIX}/usr/lib"
        )
    FIND_PATH(
        libseccomp_INC_PATH
        NAMES seccomp.h
        PATHS "${LIBSECCOMP_PREFIX}" "${LIBSECCOMP_PREFIX}/usr/include"
        )
    EXECUTE_PROCESS(
        COMMAND
        bash -c "
            exe=`mktemp`
            echo -e '#include<stdio.h>\n#include<seccomp.h>\nint main(){printf(\"%d.%d\",SCMP_VER_MAJOR,SCMP_VER_MINOR);}' \
                | gcc -I ${libseccomp_INC_PATH} -xc /dev/stdin -o $exe && $exe
            rc=$?
            rm -f $exe
            exit $rc"
        OUTPUT_VARIABLE libseccomp_VERSION
        RESULT_VARIABLE libseccomp_VERSION_RC
        )
    IF(NOT libseccomp_VERSION_RC EQUAL 0 OR libseccomp_VERSION VERSION_LESS 2.3)
        SET(libseccomp_LIB_PATH "libseccomp_LIB_PATH-NOTFOUND")
        SET(libseccomp_INC_PATH "libseccomp_INC_PATH-NOTFOUND")
    ENDIF()
ENDIF()

IF((NOT DEFINED LIBSECCOMP_BUILD_OWN AND (NOT EXISTS "${libseccomp_LIB_PATH}" OR NOT EXISTS "${libseccomp_LIB_PATH}"))
        OR LIBSECCOMP_BUILD_OWN STREQUAL "YES")
    ExternalProject_Add(seccomp_project
        URL https://github.com/seccomp/libseccomp/releases/download/v2.3.3/libseccomp-2.3.3.tar.gz
        URL_HASH SHA256=7fc28f4294cc72e61c529bedf97e705c3acf9c479a8f1a3028d4cd2ca9f3b155

        CONFIGURE_COMMAND
            <SOURCE_DIR>/configure
            --prefix=<INSTALL_DIR>
            --enable-static
            --enable-shared
        BUILD_COMMAND
            make
        INSTALL_COMMAND
            make install
        )

    ExternalProject_Get_Property(seccomp_project
        INSTALL_DIR)
    SET(libseccomp_LIB_PATH "${INSTALL_DIR}/lib/libseccomp.a")
    SET(libseccomp_INC_PATH "${INSTALL_DIR}/include")

    ADD_DEPENDENCIES(seccomp seccomp_project)
ENDIF()

IF(libseccomp_LIB_PATH MATCHES NOTFOUND OR libseccomp_INC_PATH MATCHES NOTFOUND)
    MESSAGE(FATAL_ERROR "libseccomp not found, run with -DLIBSECCOMP_BUILD_OWN=YES to build it from source")
ENDIF()

MESSAGE("-- Libseccomp configuration:")
MESSAGE("-- - library: ${libseccomp_LIB_PATH}")
MESSAGE("-- - include directory: ${libseccomp_INC_PATH}")

SET_PROPERTY(TARGET seccomp
    PROPERTY IMPORTED_LOCATION
        "${libseccomp_LIB_PATH}"
    )
INCLUDE_DIRECTORIES("${libseccomp_INC_PATH}")
