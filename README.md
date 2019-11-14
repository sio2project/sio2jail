sio2jail
========


building
--------

You need a CMake, a C/C++ compiler with multilib support and python2. Any
external libraries sio2jail use (see below) can be either installed
system-wide, or downloaded and built during the process. To build sio2jail and
install files to ~/local directory run:

    mkdir build && cd build
    
    cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=$HOME/local ..
    make && make install

Our sio2jail uses some external libraries and programs:
  * libcap
  * libseccomp (>= 2.3.0)
  * libtclap
  * scdoc (for generating man pages)

some of which you can install (e.g. on Debian) with:

    apt-get install libcap-dev libtclap-dev libseccomp-dev

By default sio2jail searches for this libraries in system paths and in case they
aren't found their sources are downloaded and libraries are built in working
directory. You can tune this behaviour with cmake options:

    -D<DEPENDENCY>_PREFIX=<PATH>
    -D<DEPENDENCY>_BUILD_OWN=YES|NO

where DEPENDENCY is one of
  * LIBCAP
  * LIBSECCOMP
  * LIBTCLAP
  * SCDOC

You can also control whether to generate man pages with option (YES by default):

    -DWITH_DOCS=YES|NO

and whether to install boxes scripts (NO by default):

    -DWITH_BOXES=YES|NO

To control whether sio2jail binary is statically or dynamically linked use
option (STATIC by default):

    -DLINK=STATIC|DYNAMIC

There is also a possibility to control whether output binary should run on other
architecture than the default one (or force given architecture):

    -DARCH=i386|x86_64

Note, that when using ARCH other than build host architecture it may be necessary
(depending on libraries installation) to build sio2jail with custom libseccomp (more
precisely with flag -DLIBSECCOMP\_BUILD\_OWN=YES).

For example, to skip man pages, use libtclap from /opt/tclap directory and
ignore system libseccomp run:

    cmake -DWITH_DOCS=NO -DLIBTCLAP_PREFIX=/opt/tclap -DLIBSECCOMP_BUILD_OWN=YES ..

running
-------

You may need to run

    sysctl -w kernel.perf_event_paranoid=-1

Additionally, if you want to use sandboxing on older kernels, you'll need to run

    sysctl -w kernel.unprivileged_userns_clone=1

For both settings, you may also put these options in your /etc/sysctl.conf.
This will make the settings persist across reboots.

running tests
-------------

To run test suit use 'check' target, e.g in build directory run:

    make check

notes for developers
--------------------

To manually run clang-format on each file run:

    make clang-format

inside build directory.

To manually run clang-tidy on each source file run:

    make clang-tidy

or to use automatically fix errors:

    make clang-tidy-fix

inside build directory.

There is possibility to enable running clang-tidy automatically during
compilation on each file (can significantly slow down compilation):

    -DWITH_CLANG_TIDY=YES
