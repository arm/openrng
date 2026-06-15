# An overview of OpenRNG

\armplref provides a Random Number Generator (RNG) interface called OpenRNG.
OpenRNG includes the interface to the RNG part of the Vector Statistics Library
(VSL) developed by Intel and shipped for x86 processors as part of oneMKL.
Intel have released this interface, with their documentation, to us under the
CC BY 4.0 license (https://creativecommons.org/licenses/by/4.0/deed.en). This
has allowed us to develop our own implementation of this functionality for
Arm-based systems which enables software portability between architectures with
no code changes.

We try to use the same generators and initializations as documented in the
oneMKL documentation. This means that functions which return bit sequences are
bitwise reproducible between Arm and x86 systems. Functions that return integer
or floating point values might give different answers between Arm and x86
systems because the accuracy of various operations is different between the
two libraries.

\if OPENRNG_STANDALONE

In this page, the hyperlinks to the different sections of the documentation and
those to the VSL functions will not work unless the documentation is built using
doxygen as described in the README file.

\endif

This release does not include all of the random number functions from VSL. The
documentation lists both the implemented and missing functionalities in the
tables in the next sections. We will improve coverage of the missing functions
in future releases.

# Basic Random Number Generators

A Basic Random Number Generator (BRNG) is any source of randomness capable of
producing a uniform distribution in the range [0, 1). The inclusion of 0 in the
range depends on the generator. Some BRNGs can also generate uniformly
distributed bitstreams. OpenRNG supports different types of BRNGs:

 * Pseudorandom number generators
 * Quasirandom number generators
 * Nondeterministic random number generators

See \ref groupBrng for a list of all BRNGs supported by the API. This release
does not include all BRNGs supported by the API. All generators that have not
been implemented have a banner in their documentation which says "Not
implemented in \armplrefmd".


# Pseudorandom number generators

Pseudorandom Number Generators (PRNG) are algorithms that generate sequences
which approximate randomness from an initial seed. PRNG sequences are periodic.
The quality of a PRNG sequence is determined by how closely a subsequence
approximates a nondeterministic source of randomness. See, for example, the
testsuite https://github.com/umontreal-simul/TestU01-2009 for some statistical
tests for PRNGs.

The following is a list of all PRNGs that \armplref implements.

| Generator     | Description                                       |
| :------------ | :------------------------------------------------ |
| ARS5          | An ARS5 counter-based generator                   |
| MCG31m1       | A 31-bit multiplicative congruential generator    |
| MCG59         | A 59-bit multiplicative congruential generator    |
| MRG32K3A      | Combined multiple recursive generator             |
| MT19937       | Mersenne-Twiser generator                         |
| MT2203        | Set of 6024 Mersenne-Twister generators           |
| PHILOX4x32x10 | A Philox counter-based generator                  |
| R250          | Generalized feedback shift register generator     |
| SFMT19937     | SIMD oriented Fast Mersenne-Twister generator     |

All generators produce random number in single or double precision.

The following is a list of all PRNGs that exist in the API but have **not** been
implemented in \armplref. The API will return \ref
VSL_ERROR_FEATURE_NOT_IMPLEMENTED if any of these are selected.

| Generator     | Description                                                        |
| :------------ | :----------------------------------------------------------------- |
| WH            | Set of 273 Wichmann-Hill generators                                |
| ABSTRACT      | Abstract source of randomness based on the user-provided settings  |


# Quasirandom number generators

Quasirandom Number Generators (QRNG) are algorithms that generate sequences of
n-dimensional vectors uniformly distributed on n-dimensional hypercubes. For all
QRNGs in the API, the seed determines the number of dimensions. Each element of
the sequence consists of n-dimensional vectors represented by n consecutive
values in the returned buffer.

The following is a list of all QRNGs that have been implemented in
\armplref.

| Generator     | Description                                               |
| :------------ | :-------------------------------------------------------- |
| NIEDERR       | Up to 318-dimensional low-discrepancy sequences generator |
| SOBOL         | Up to 40-dimensional low-discrepancy sequences generator  |

# Nondeterministic random number generators

Nondeterministic random number generators take entropy from an external source
to generate random numbers. Nondeterministic generators are typically slower
than PRNGs and the sequences are not reproducible.

\armplref implements the only value for nondeterministic generators:

| Generator        | Description                                        |
| :--------------- | :--------------------------------------------------|
| Nondeterministic | Random generator based on hardware entropy sources |

It returns random numbers in single or double precision.

# Random streams

An instance of a BRNG is referred to as a stream. \armplref provides service
functions for creating, initializing, copying, modifying and deleting streams,
see \ref groupService. For example, you create a stream with the PHILOX4X32X10
generator and a single seed as follows:
```
    VSLStreamStatePtr stream;
    int seed = 42;
    int errcode = vslNewStream(&stream, VSL_BRNG_PHILOX4X32X10, seed);
```

See the \ref pi.c example for a complete example of creating and using a
stream.


# Distributions

\armplref can generate random numbers with many common probability
distributions, both discrete and continuous. To access random numbers from the
stream, you must request numbers with a specific distribution, and \armplref
advances your stream in the background. For example, you create a uniform
distribution over the interval [0, 1) as follows:
```
    float randomNumbers[nRandomNumbers];
    const float a = 0.0f;
    const float b = 1.0f;
    int errcode = vsRngUniform(VSL_RNG_METHOD_UNIFORM_STD, stream, nRandomNumbers,
                               &randomNumbers, a, b);
```
This snippet fills the buffer, `randomNumbers`, with random numbers from a
uniform distribution. On return, the amount your stream has advanced by depends
on the distribution method used. See the \ref pi.c example for a complete
example of generating random numbers with a specified probability distribution.

# Continuous distributions

A continuous distribution is any probability distribution over the reals. All
continuous distributions fill pre-allocated buffers. You can request continuous
distributions as either single-precision floating point, `float`, or
double-precision floating point, `double`. The `vsRng` prefix identifies the
single-precision floating point version, while the `vdRng` prefix identifies the
double-precision floating point version.

The following is a list of all continuous distributions available in \armplref.

| Distribution | Description                                |
| :----------- | :----------------------------------------- |
| Cauchy       | Cauchy distribution                        |
| Chi-Square   | Chi-Square distribution                    |
| Exponential  | Exponential distribution                   |
| Gamma        | Gamma distribution                         |
| Gaussian     | Single variable Gaussian distribution      |
| GaussianMV   | Multivariate Gaussian distribution         |
| Gumbel       | Gumbel distribution                        |
| Laplace      | Laplace distribution                       |
| Lognormal    | Lognormal distribution                     |
| Rayleigh     | Rayleigh distribution                      |
| Uniform      | Uniform distribution on the interval [a,b) |
| Weibull      | Weibull distribution                       |

The available continuous distributions are invoked through the following API
functions:

  * \ref vsRngCauchy, \ref vdRngCauchy
  * \ref vsRngChiSquare, \ref vdRngChiSquare
  * \ref vsRngExponential, \ref vdRngExponential
  * \ref vsRngGamma, \ref vdRngGamma
  * \ref vsRngGaussian, \ref vdRngGaussian
  * \ref vsRngGaussianMV, \ref vdRngGaussianMV
  * \ref vsRngGumbel, \ref vdRngGumbel
  * \ref vsRngLaplace, \ref vdRngLaplace
  * \ref vsRngLognormal, \ref vdRngLognormal
  * \ref vsRngRayleigh, \ref vdRngRayleigh
  * \ref vsRngUniform, \ref vdRngUniform
  * \ref vsRngWeibull, \ref vdRngWeibull

The following is a list of all continuous distributions that exist in
the API but have _not_ been implemented in \armplref.

| Distribution      | Description                     |
| :---------------- | :------------------------------ |
| Beta              | Beta distribution               |

They correspond to the following API functions:

  * \ref vsRngBeta, \ref vdRngBeta

These methods return \ref VSL_ERROR_FEATURE_NOT_IMPLEMENTED.

# Discrete distributions

A discrete distribution is any probability distribution that is confined to the
integers. The `viRng` prefix identifies all methods for discrete distributions
which fill pre-allocated buffers of type `int`.

The following is a list of all discrete distributions that are available
in \armplref.

| Distribution | Description                                 |
| :----------- | :------------------------------------------ |
| Bernoulli    | Bernoulli distribution                      |
| Binomial     | Binomial distribution                       |
| Geometric    | Geometric distribution                      |
| Multinomial  | Multinomial distribution                    |
| Poisson      | Poisson distribution                        |
| PoissonV     | Poisson distribution with varying mean      |
| Uniform (int)| Uniform distribution on the interval [a,b)  |
| UniformBits32| Uniformly distributed bits in 32-bit chunks |
| UniformBits64| Uniformly distributed bits in 64-bit chunks |

These distributions correspond to the following API functions:

  * \ref viRngBernoulli
  * \ref viRngBinomial
  * \ref viRngGeometric
  * \ref viRngMultinomial
  * \ref viRngPoisson
  * \ref viRngPoissonV
  * \ref viRngUniform
  * \ref viRngUniformBits
  * \ref viRngUniformBits32
  * \ref viRngUniformBits64

The following is a list of all discrete distributions that exist in the API but
have _not_ been implemented in \armplref.

| Distribution      | Description                            |
| :---------------- | :------------------------------------- |
| Hypergeometric    | Hypergeometric distribution            |
| Negative binomial | Negative binomial distribution         |

These distributions correspond to the following API functions:

  * \ref viRngHypergeometric
  * \ref viRngNegbinomial


The API returns \ref VSL_ERROR_FEATURE_NOT_IMPLEMENTED if any of these functions
is selected.

# Bitstreams

\armplref supports the following methods for generating bitstreams:

 * \ref viRngUniformBits
 * \ref viRngUniformBits32
 * \ref viRngUniformBits64

\ref viRngUniformBits makes no guarantee of the uniformity of the bit sequences
returned. It reinterprets the integer sequence of each generator as a bit
sequence. For some generators, the integer sequence can contain bits that are
always 0, or a collection of bits might exhibit periodic behavior over very
short intervals. The length of the buffer returned is dependent on the
generator's word size (\ref vslGetBrngProperties).

\ref viRngUniformBits32 and \ref viRngUniformBits64 guarantee that the bits have
a uniform distribution. In the 32-bit variant, each element of the buffer needs
to be able to hold 32 bits, and in the 64-bit variant, every element of the
buffer needs to be able to hold 64 bits.


# Error handling

All methods return a negative value on failure. See \ref groupError for a list
of error code values and what each one means.


# Thread safety

\armplref is thread-safe, provided multiple threads do not try to access the
same stream concurrently. To scale to multiple threads, \armplref provides
convenience methods for copying streams and generating disjoint subsequences.
See \ref vslCopyStream, \ref vslLeapfrogStream and \ref vslSkipAheadStream. See
the \ref skipahead.c example for an example of generating disjoint sequences.


# API access

The API definitions are exposed via `openrng.h`. For convenience `armpl.h` also
includes `openrng.h`. See \armplref documentation for instructions on how to build
and link with \armplref.

# Cryptographic purposes

The random number generators implemented in OpenRNG should not be used for
cryptographic purposes as they are not cryptographically secure.
