# Get started with OpenRNG

OpenRNG is an open-source Random Number Generator library developed at
Arm. OpenRNG includes the interface to the random number generation part
of the Vector Statistics Library (VSL) developed by Intel(R) and
shipped for x86 processors as part of oneMKL[^1].

[^1]: We are grateful to Intel(R) for having released this interface,
    along with their documentation, to us under the CC BY 4.0 license
    (https://creativecommons.org/licenses/by/4.0/deed.en), allowing us to
    develop our own implementation of this functionality for users of
    Arm-based systems, enabling software portability between architectures
    with no code changes.

We have endeavored to ensure that the same generators and initializations are
used as documented in the oneMKL documentation. This means that functions which
return bit sequences are bitwise reproducible between Arm and x86 systems. If an
integer or floating point answer is requested answers may differ as the
precision of various operations is different between the two libraries.

Not all of the random number functions from VSL may be included. These functions
are listed in [IMPLEMENTATION_STATUS.md](./IMPLEMENTATION_STATUS.md) as "not
implemented". We are intending to fill out this coverage in future releases, and
we are very keen to hear from users who find missing functionality that they
would like us to prioritize. If a user would like to request features or for any
other form of assistance, e.g. report a bug, please [raise an
issue](https://gitlab.arm.com/libraries/openrng/-/issues).

This document describes how to build and install OpenRNG, including
the tests and examples provided within the source code. If a user wishes
to contribute to the development of OpenRNG, please see the
[Contribution guidelines](./CONTRIBUTING.md).


## Download, build and install OpenRNG

Building OpenRNG requires either GCC >=11 or Clang >=16. The steps below use
CMake to configure the project and `make` to build it. To configure the project
with a compiler other than your system's default compiler, see the
[CMAKE_LANG_COMPILER](https://cmake.org/cmake/help/latest/variable/CMAKE_LANG_COMPILER.html)
documentation.

Clone the OpenRNG repository:

```
git clone https://git.gitlab.arm.com/libraries/openrng.git
```

Navigate to the folder with the cloned repository and, to install the library in
a folder called `./install`, run the following commands

```
mkdir build
cd build
cmake -DCMAKE_INSTALL_PREFIX=../install ../
make install
```

The install path is configured with `CMAKE_INSTALL_PREFIX`. The default value of
`CMAKE_INSTALL_PREFIX` is system dependent.


## Documentation

A recent version of the documentation can be found with the [Arm Performance
Libraries
documentation](https://developer.arm.com/documentation/101004/2404/Open-Random-Number-Generation--OpenRNG--Reference-Guide?lang=en).
If you need the most recent version of the documentation you can build it
yourself. Building the documentation requires Doxygen and LaTeX to be installed.
Once the necessary requirements are installed, the documentation can be built
with

```
mkdir build
cd build
cmake -DBUILD_DOCS=On ..
make html
```

The documentation can then be found in `build/docs/html`.


## Building and running OpenRNG examples

The library comes with different examples of how the different functions
provided in the interface can be invoked to generate random numbers.

Assuming you have followed the installation instructions above, the examples can
be built from the `examples/` directory with:

```
make OPENRNG_ROOT=../install
```

This will produce an executable for each available example that can be
invoked to run that specific example.