ADD_SUBDIRECTORY(src)

ADD_TEST(
    NAME
        python
    COMMAND
        bash -c "SIO2JAIL_BUILD_PATH='${CMAKE_BINARY_DIR}' python3 -m unittest discover -s '${CMAKE_SOURCE_DIR}/test/testsuits/' -v"
    WORKING_DIRECTORY
        "${CMAKE_SOURCE_DIR}"
    )

ADD_CUSTOM_TARGET(check
    COMMAND
        "${CMAKE_CTEST_COMMAND}" --verbose
    DEPENDS
        sio2jail test-binaries boxes)
