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
