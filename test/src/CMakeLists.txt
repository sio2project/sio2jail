SET(CMAKE_C_FLAGS "-std=gnu99 -lm -static -O2")
SET(CMAKE_C_FLAGS_DEBUG "")
SET(CMAKE_C_FLAGS_RELEASE "")
SET(CMAKE_CXX_FLAGS "-std=c++11 -lm -static -O2")
SET(CMAKE_CXX_FLAGS_DEBUG "")
SET(CMAKE_CXX_FLAGS_RELEASE "")

# Time tests
ADD_EXECUTABLE(1-sec-prog 1-sec-prog.c)
ADD_EXECUTABLE(1-sec-evil 1-sec-evil.c)
ADD_EXECUTABLE(infinite-loop infinite-loop.c)
SET_TARGET_PROPERTIES(1-sec-prog
                      1-sec-evil
                      infinite-loop
                      PROPERTIES COMPILE_FLAGS "-m32"
                                 LINK_FLAGS "-m32")

# Memory limit tests
ADD_EXECUTABLE(leak-tiny_32 leak-tiny.c)
ADD_EXECUTABLE(leak-huge_32 leak-huge.c)
ADD_EXECUTABLE(leak-dive_32 leak-dive.c)
SET_TARGET_PROPERTIES(leak-tiny_32
                      leak-huge_32
                      leak-dive_32
                      PROPERTIES COMPILE_FLAGS "-m32"
                                 LINK_FLAGS "-m32")
ADD_EXECUTABLE(leak-tiny_64 leak-tiny.c)
ADD_EXECUTABLE(leak-huge_64 leak-huge.c)
ADD_EXECUTABLE(leak-dive_64 leak-dive.c)
SET_TARGET_PROPERTIES(leak-tiny_64
                      leak-huge_64
                      leak-dive_64 PROPERTIES COMPILE_FLAGS "-m64"
                                              LINK_FLAGS "-m64")
# Summing programs
ADD_EXECUTABLE(sum_c    sum_c.c)
ADD_EXECUTABLE(sum_cxx  sum_cxx.cc)
SET_TARGET_PROPERTIES(sum_c
                      sum_cxx PROPERTIES COMPILE_FLAGS "-m32"
                                         LINK_FLAGS "-m32")

# Other
ADD_EXECUTABLE(stderr-write stderr-write.c)

ADD_CUSTOM_TARGET(test-binaries
    DEPENDS
        1-sec-prog 1-sec-evil infinite-loop
        leak-tiny_32 leak-huge_32 leak-dive_32
        leak-tiny_64 leak-huge_64 leak-dive_64
        sum_c sum_cxx stderr-write)
