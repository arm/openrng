# Contributing to OpenRNG

This document outlines how to contribute to the project. For an overview of the
project and how to install it, see [README.md](README.md). All instructions
given here will assume you are developing on Linux. On other platforms the steps
would need to be similar.


## Building

See [README.md](README.md) for instructions on building the library. The unit
tests and benchmarks are built by default.


## Unit tests

Unit tests can be run using CMake's CTest tool. The following command from the
build directory, runs all unit tests over ten cores in parallel and gives a
terse summary of the progress.

```console
$ ctest --progress -j 10
Test project /home/user/rng/build
149/149 Test #101: Weibull reference test - double
100% tests passed, 0 tests failed out of 149

Total Test time (real) =   1.33 sec
```

See [the online CTest documentation] for more information on how to use `ctest`.

[the online Ctest documentation]:
https://cmake.org/cmake/help/book/mastering-cmake/chapter/Testing%20With%20CMake%20and%20CTest.html


## Benchmarks

Once the library has been built, all the benchmark cases available to run can be
found by navigating to `build/bench/`. Here, you will find a series of
executables of the form

```
bench_<distribution>[_<precision>]
```

where `<distribution>` is the name of the distribution being benchmarked and
`<precision>` is empty for discrete distributions, or one of `float` or `double`
for the continuous distributions, e.g. bench_gaussian_float. Each executable
takes a different set of arguments based on the type of distribution and its
constitutive parameters. The expected arguments and their meaning can be found
by running the selected benchmark executable without any arguments. For
instance, for `bench_gaussian_float`, the expected arguments are:

 - `brng`: the basic random number generator that generates numbers that are
   uniformly distributed in the interval `[0,1)`. The list of supported
   generators can be found in `bench/include/defines.hpp`. For instance, when
   using the MCG31 generator, the expected value for the `brng` argument will be
   `MCG31`.
 - `method`: the method that transforms the uniformly distributed numbers in the
   interval `[0,1)` into numbers distributed according to a normal distribution.
   For Gaussian, the `method` argument could be `GAUSSIAN_ICDF`,
   `GAUSSIAN_BOXMULLER`, or `GAUSSIAN_BOXMULLER2`.
 - `niters`: the number of times to repeat the same experiment.
 - `nelems`: the size of the buffer to fill with normally distributed numbers.
 - `a`: the average of the normal distribution.
 - `sigma`: the standard deviation of the normal distribution.

For instance, to measure the average time over 100 experiments, it takes to fill
buffers of length 100 with randomly generated numbers that follow a Gaussian
distribution of average equal to 0 and standard deviation of 1, the command to
run is

```
<build_directory>/bench/bench_gaussian_float MCG31 GAUSSIAN_ICDF 100 100 0 1
```

and the output will look like

```
4.4343e-07
```

which is the average runtime to fill the buffer over `niters`
independent trials. Despite the benchmark executables possibly taking
different input arguments, based on the distribution, they all return
only the measured average time as presented above.


## Pull requests

Pull requests can be submitted on [github](https://github.com/arm/openrng).

Every patch must compile successfully and pass all tests. All new functionality
must come with sufficient test coverage. It is good practice to split the
development of new functionality into multiple patches to aid reviewing: present
the initial unoptimized implementation and accompanying tests in one patch and
the optimized implementation in a second patch.

Use the [50/72
rule](https://tbaggery.com/2008/04/19/a-note-about-git-commit-messages.html) for
commit messages.


## Directory structure

```
openrng
|-- armpl
|-- bench
|-- examples
|-- include
|-- src
|   |-- generic
|   |-- math
|   |-- refng
|   |-- utils
|   `-- vsl
|-- test
|   |-- distributions
|   |-- services
|   `-- utils
`-- tools
```
where:

 - `armpl` example [alternative test target](#alternative-test-target).
 - `bench` contains the [benchmark framework](#benchmarks).
 - `examples`  contains source code for simple examples using OpenRNG functions.
 - `include` contains `openrng.h`, the external header. The API is documented
   inline.
 - `src/generic` contains all framework code and reference implementations.
 - `src/math` contains math routines typically provided by `math.h`. We provide
   our own math routines to aid compile-time optimization and to provide lower
   accuracy routines. Architecture specific code is stored in
   `src/math/<arch>`.
 - `src/refng` contains the [reference library](#reference-library).
 - `src/utils` contains code to be shared across `src/`.
 - `src/vsl` contains the definitions of the VSL API and all optimized generators
   and distributions. The VSL methods will call into the framework in
   `src/generic` which can later dispatch back into `src/vsl`. Architecture
   specific code is stored in `src/vsl/<arch>`.
 - `test` contains the [unit tests](#unit-tests).
 - `tools` contains tools to assist with building the project.


## Documenting the API

Documentation for the external API is written as a Doxygen comment immediately
preceding the function's prototype in `include/openrng.h`. Please follow the
format already in place.


## Coding style

The coding style is maintained by `clang-format` 17.0.6. You can install
`clang-format` 17.0.6 with `pip`.

```
$ pip install clang-format==17.0.6
```

Once you have staged/committed all your changes, you can easily check the
formatting of all code with

```
$ git ls-tree -r @ --name-only | grep -E "\.[ch]p?p?$" | xargs ~/.local/bin/clang-format --dry-run
```

If the code needs to be formatted, the above will print the diff to the screen.
The diff can be applied by replacing `--dry-run` with `-i`


## Reference library

We provide a reference library called RefNG for writing unit tests. RefNG
provides an identical interface as VSL, except all function prefixes have been
replaced; `vsl` with `ref` and `v?Rng` with `r?Rng`. RefNG only ever calls the
reference implementations of generators and distributions found in
`src/generic`.


## Alternative test target

The unit tests can be compiled against any library that provides a VSL
interface. Use `-DOPENRNG_TEST_TARGET=<path to test target dir>` at configure
time to specify the test target directory. `OPENRNG_TEST_TARGET` can be either a
relative path or absolute path to a directory containing a `CMakeLists.txt`
file, which defines:

 - A `vsl` target that configures the link and compile lines.
 - An `OPENRNG_TEST_SPEC` variable that configures the tags to be run by ctest.

See `armpl/CMakeLists.txt` for an example configuration using Arm Performance
Libraries (ArmPL). ArmPL ships with OpenRNG. The ArmPL configuration can be used
by running

```
cmake -DOPENRNG_TEST_TARGET=armpl -DARMPL_ROOT=/path/to/armpl/ ..
```

ArmPL can be obtained from
https://developer.arm.com/downloads/-/arm-performance-libraries

## Legal requirements

All code must be compliant with both MIT AND Apache-2.0 WITH LLVM-exception; see
[LICENSE](LICENSE). All code must be copyright-owned by Arm Limited, and the
appropriate copyright notice and license identifier must be present in every
source file.


## No C++ runtime

This project is a C++ project, but to improve portability we choose not to
depend on the C++ runtime. Usage of core features of the C++ language (such as
templates, function overloading, RAII) are permitted, but any STL usage which
introduces a runtime dependency on libc++ or libstdc++ is not permitted. Some
features such as `std::array` are compile-time only, so may be used. This policy
is enforced by the `test_linking_with_c_compiler` build target. If submitting a
patch which uses an STL feature not previously used in the project, please make
a note of this in the pull request.

Note: Memory allocation on the heap is done with `malloc` and `free`. "Placement
new" can be used to construct C++ objects on the heap. `DynamicArray` is a naive
replacement for `std::vector` implemented in `src/utils`.
