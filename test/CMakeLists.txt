ADD_SUBDIRECTORY(src)

ADD_TEST(NAME python
    COMMAND
        ${CMAKE_SOURCE_DIR}/test/testsuits/main.py
    )
