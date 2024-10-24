# OpenRNG Changelog

All notable changes to this project will be documented in this file.


## Unreleased

### Added

### Performance

### Changed

### Deprecated

### Removed

### Fixed

### Security


## 24.10

### Added
  - Full support for the SOBOL interface, which now includes custom polynomials
    and direction numbers.

  - Shared object versioning. The CMake properties,
    [VERSION](https://cmake.org/cmake/help/latest/prop_tgt/VERSION.html) and
    [SOVERSION](https://cmake.org/cmake/help/latest/prop_tgt/VERSION.html) are
    now set. VERSION will be updated on every release and SOVERSION will be
    updated whenever there is a breaking ABI change.

  - Support for a local install of catch2 when building the unit tests. If your
    build is configured with -DOPENRNG_LOCAL_CATCH=On, CMake will look for
    catch2 locally rather than pulling and building a version from github.

### Performance
  - The performance of MT19937 skip-ahead has had a considerable improvement,
    particularly when skipping ahead a number of values that is a power of 2.


## 24.04

This is the initial release of OpenRNG, all functionality is new.

### Added

#### Generators:
  - Reference versions: MCG31, R250, MRG32K3A, MCG59, SOBOL, MT19937, SFMT19937,
  PHILOX4X32X10.
  - Neon optimizations: MCG31, MRG32K3A, MCG59, MT19937, SFMT19937,
  PHILOX4X32X10.
  - NONDETERM for AArch64 and x86-64.

#### Distributions:
  - References versions: Cauchy, Exponential, Gaussian, Gumbel, Laplace,
  Lognormal, Rayleigh, Uniform, Weibull, Bernoulli, Binomial, Geometric,
  Poisson, Uniform, UniformBits, UniformBits32, UniformBits64.
  - Neon optimizations: Binomial, Exponential, Gaussian, Geometric, Laplace,
  Poisson, Rayleigh, Weibull.
  - SVE optimizations: Exponential, Gaussian, Geometric, Rayleigh.

#### Tooling:
  - Benchmark framework.
  - Unit test framework.
  - Scalar maths functions for log, cospi and sinpi.
  - Neon and SVE optimizations for log and erfinv.
