INCLUDE_DIRECTORIES(./)

FILE(GLOB_RECURSE sources *.cc)

ADD_EXECUTABLE(sio2jail ${sources})
TARGET_LINK_LIBRARIES(sio2jail seccomp rt pthread)
ADD_DEPENDENCIES(sio2jail libcap libtclap)

INSTALL(TARGETS sio2jail DESTINATION "${CMAKE_INSTALL_FULL_BINDIR}")

FILE(GLOB_RECURSE all_sources *.cc *.h *.hpp)

ADD_CUSTOM_TARGET(
    clang-format
    COMMAND
        clang-format -style=file -i ${all_sources}
    WORKING_DIRECTORY
        ${CMAKE_CURRENT_SOURCE_DIR})

SET(clang_tidy_cxx_flags -x;c++;-std=gnu++14)

GET_PROPERTY(include_directories DIRECTORY PROPERTY INCLUDE_DIRECTORIES)
FOREACH(include_directory IN LISTS include_directories)
    SET(clang_tidy_cxx_flags ${clang_tidy_cxx_flags};-I${include_directory})
ENDFOREACH()

ADD_CUSTOM_TARGET(
    clang-tidy
    COMMAND
        clang-tidy ${sources} -- ${clang_tidy_cxx_flags}
    WORKING_DIRECTORY
        ${CMAKE_CURRENT_SOURCE_DIR})

ADD_CUSTOM_TARGET(
    clang-tidy-fix
    COMMAND
        clang-tidy ${sources} -fix -fix-errors -- ${clang_tidy_cxx_flags}
    WORKING_DIRECTORY
        ${CMAKE_CURRENT_SOURCE_DIR})
