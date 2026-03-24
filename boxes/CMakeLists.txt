SET(LEGACY_BOX_SCRIPTS
    ${CMAKE_SOURCE_DIR}/boxes/minimal/build_minbase.sh
    ${CMAKE_SOURCE_DIR}/boxes/busybox/build_trixie.sh
    ${CMAKE_SOURCE_DIR}/boxes/python/build_python2.7.sh
    ${CMAKE_SOURCE_DIR}/boxes/python/build_python3_minimal.sh
    ${CMAKE_SOURCE_DIR}/boxes/python/build_python3_9_numpy.sh
    ${CMAKE_SOURCE_DIR}/boxes/utils.sh
)

# Build legacy test boxes using individual scripts
ADD_CUSTOM_COMMAND(
    OUTPUT
        minimal.tar.gz
    COMMAND
        bash ${CMAKE_SOURCE_DIR}/boxes/minimal/build_minbase.sh ${CMAKE_CURRENT_BINARY_DIR}
    DEPENDS
        ${LEGACY_BOX_SCRIPTS}
    WORKING_DIRECTORY
        ${CMAKE_CURRENT_BINARY_DIR}
)

ADD_CUSTOM_COMMAND(
    OUTPUT
        busybox.tar.gz
    COMMAND
        bash ${CMAKE_SOURCE_DIR}/boxes/busybox/build_trixie.sh ${CMAKE_CURRENT_BINARY_DIR}
    DEPENDS
        ${LEGACY_BOX_SCRIPTS}
    WORKING_DIRECTORY
        ${CMAKE_CURRENT_BINARY_DIR}
)

ADD_CUSTOM_COMMAND(
    OUTPUT
        python2.tar.gz
    COMMAND
        bash ${CMAKE_SOURCE_DIR}/boxes/python/build_python2.7.sh ${CMAKE_CURRENT_BINARY_DIR}
    DEPENDS
        ${LEGACY_BOX_SCRIPTS}
    WORKING_DIRECTORY
        ${CMAKE_CURRENT_BINARY_DIR}
)

ADD_CUSTOM_COMMAND(
    OUTPUT
        python3.tar.gz
    COMMAND
        bash ${CMAKE_SOURCE_DIR}/boxes/python/build_python3_minimal.sh ${CMAKE_CURRENT_BINARY_DIR}
    DEPENDS
        ${LEGACY_BOX_SCRIPTS}
    WORKING_DIRECTORY
        ${CMAKE_CURRENT_BINARY_DIR}
)

ADD_CUSTOM_COMMAND(
    OUTPUT
        python3_9.tar.gz
    COMMAND
        bash ${CMAKE_SOURCE_DIR}/boxes/python/build_python3_9_numpy.sh ${CMAKE_CURRENT_BINARY_DIR}
    DEPENDS
        ${LEGACY_BOX_SCRIPTS}
    WORKING_DIRECTORY
        ${CMAKE_CURRENT_BINARY_DIR}
)

SET(TEST_BOX_ARCHIVES
    minimal.tar.gz
    busybox.tar.gz
    python2.tar.gz
    python3.tar.gz
    python3_9.tar.gz
)

ADD_CUSTOM_COMMAND(
    OUTPUT
        boxes_extraction_stamp.txt
    COMMAND
        for archive in ${TEST_BOX_ARCHIVES} \; do
            tar -xf "$$archive" \;
        done
        && touch boxes_extraction_stamp.txt
    DEPENDS
        ${TEST_BOX_ARCHIVES}
    WORKING_DIRECTORY
        ${CMAKE_CURRENT_BINARY_DIR}
)

ADD_CUSTOM_TARGET(boxes
    DEPENDS
        boxes_extraction_stamp.txt
)

IF(NOT DEFINED WITH_BOXES)
    SET(WITH_BOXES "NO")
ENDIF()
IF(NOT WITH_BOXES MATCHES "YES|NO")
    MESSAGE(FATAL_ERROR "WITH_BOXES should be one of YES, NO")
ENDIF()
IF(WITH_BOXES STREQUAL "YES")
    INSTALL(FILES
        build_all.sh
        utils.sh
        DESTINATION "${CMAKE_INSTALL_FULL_DATADIR}/sio2jail/boxes"
        PERMISSIONS OWNER_READ OWNER_EXECUTE)
    INSTALL(DIRECTORY
        minimal busybox cpp python java kotlin
        DESTINATION "${CMAKE_INSTALL_FULL_DATADIR}/sio2jail/boxes"
        FILE_PERMISSIONS OWNER_READ OWNER_EXECUTE)
ENDIF()
