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
