sio2jail
========


building
--------

You need a CMake, a C/C++ compiler with multilib support, python2 and a
number of libraries (see below). To build sio2jail and install binary
files into ./bin/ directory run:

    mkdir build && cd build
    
    cmake -DCMAKE_BUILD_TYPE=Release ..
    make && make install

Our sio2jail uses some external libraries:
  * libcap
  * libseccomp (>= 2.3.0)
  * libtclap

which you can install (e.g. on Debian) with:

    apt-get install libcap-dev libtclap-dev libseccomp-dev

You may need to run

  sysctl -w kernel.perf\_event\_paranoid=-1
  sysctl -w kernel.unprivileged\_userns\_clone=1

or to enable these options in your /etc/sysctl.conf in order
to run sio2jail.

running tests
-------------

To run test suit, firstly build project and install files into ./bin/
directory. Then then run `main.py` executable, e.g:

    ./test/testsuits/main.py

Remember that many tests will work only on 3.\* kernels versions.
