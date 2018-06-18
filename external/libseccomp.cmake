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

ADD_LIBRARY(seccomp STATIC IMPORTED)
SET_PROPERTY(TARGET seccomp
    PROPERTY IMPORTED_LOCATION
        ${INSTALL_DIR}/lib/libseccomp.a
    )
ADD_DEPENDENCIES(seccomp seccomp_project)

INCLUDE_DIRECTORIES(${INSTALL_DIR}/include/)
