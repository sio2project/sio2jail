IF(DEFINED LIBSECCOMP_BUILD_OWN AND NOT "${LIBSECCOMP_BUILD_OWN}" MATCHES "YES|NO")
    MESSAGE(FATAL_ERROR "LIBSECCOMP_BUILD_OWN should be one of: YES, NO")
ENDIF()

ADD_LIBRARY(seccomp STATIC IMPORTED)

IF(NOT DEFINED LIBSECCOMP_BUILD_OWN OR LIBSECCOMP_BUILD_OWN STREQUAL "NO")
    IF(LINK STREQUAL "STATIC")
        SET(libseccomp_LIB_FILE_NAME "libseccomp.a")
    ELSE()
        SET(libseccomp_LIB_FILE_NAME "libseccomp.so")
    ENDIF()
    FIND_FILE(
        libseccomp_LIB_PATH
        NAMES "${libseccomp_LIB_FILE_NAME}"
        PATHS "${LIBSECCOMP_PREFIX}" "${LIBSECCOMP_PREFIX}/lib" "${LIBSECCOMP_PREFIX}/usr/lib"
        )
    FIND_PATH(
        libseccomp_INC_PATH
        NAMES seccomp.h
        PATHS "${LIBSECCOMP_PREFIX}" "${LIBSECCOMP_PREFIX}/usr/include"
        )
    IF(libseccomp_LIB_FILE_NAME MATCHES "NOTFOUND")
        MESSAGE("-- Libseccomp not found")
    ELSE()
        EXECUTE_PROCESS(
            COMMAND
            bash -c "
                exe=`mktemp`
                echo -e '#include<stdio.h>\n#include<seccomp.h>\nint main(){printf(\"%d.%d\",SCMP_VER_MAJOR,SCMP_VER_MINOR);}' \
                    | gcc -I ${libseccomp_INC_PATH} -xc /dev/stdin -o $exe >/dev/null 2>&1 && $exe
                rc=$?
                rm -f $exe
                exit $rc"
            OUTPUT_VARIABLE libseccomp_VERSION
            RESULT_VARIABLE libseccomp_VERSION_RC
            )
        IF(NOT libseccomp_VERSION_RC EQUAL 0 OR libseccomp_VERSION VERSION_LESS 2.3)
            SET(libseccomp_LIB_PATH "libseccomp_LIB_PATH-NOTFOUND")
            SET(libseccomp_INC_PATH "libseccomp_INC_PATH-NOTFOUND")
            IF (NOT libseccomp_VERSION_RC EQUAL 0)
                MESSAGE("-- failed to compile Libseccomp test program")
            ELSE()
                MESSAGE("-- found Libseccomp in version ${libseccomp_VERSION}, but minimal required version is 2.3")
            ENDIF()
        ENDIF()
    ENDIF()
ENDIF()

IF((NOT DEFINED LIBSECCOMP_BUILD_OWN AND (NOT EXISTS "${libseccomp_LIB_PATH}" OR NOT EXISTS "${libseccomp_LIB_PATH}"))
        OR LIBSECCOMP_BUILD_OWN STREQUAL "YES")

    IF(ARCH STREQUAL "i386")
        SET(EXTRA_FLAGS "-m32")
    ELSEIF(ARCH STREQUAL "x86_64")
        SET(EXTRA_FLAGS "-m64")
    ELSE()
        SET(EXTRA_FLAGS "")
    ENDIF()

    ExternalProject_Add(seccomp_project
        URL https://github.com/seccomp/libseccomp/releases/download/v2.5.4/libseccomp-2.5.4.tar.gz
        URL_HASH SHA256=d82902400405cf0068574ef3dc1fe5f5926207543ba1ae6f8e7a1576351dcbdb
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE

        CONFIGURE_COMMAND
            CFLAGS=${EXTRA_FLAGS} CXXFLAGS=${EXTRA_FLAGS} <SOURCE_DIR>/configure
            --prefix=<INSTALL_DIR>
            --enable-static
        BUILD_COMMAND
            make
        INSTALL_COMMAND
            make install

        BUILD_BYPRODUCTS
            <INSTALL_DIR>/lib/libseccomp.a
        )

    ExternalProject_Get_Property(seccomp_project
        INSTALL_DIR)
    IF(LINK STREQUAL "STATIC")
        SET(libseccomp_LIB_PATH "${INSTALL_DIR}/lib/libseccomp.a")
    ELSE()
        MESSAGE(FATAL_ERROR "-- Can't dynamically link to custom Libseccomp build")
    ENDIF()
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
