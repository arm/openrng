# An Overview of OpenRNG

# Introduction to OpenRNG

OpenRNG is the random number generator (RNG) interface provided in \armplref.
OpenRNG includes the interface to the random number generation part of the
Vector Statistics library (VSL) library developed by Intel(R) and shipped for
x86 processors as part of oneMKL. We are grateful to Intel(R) for having
released this interface, along with their documentation, to us under the CC BY
4.0 license (https://creativecommons.org/licenses/by/4.0/deed.en), allowing us
to develop our own implementation of this functionality for users of Arm-based
systems, enabling software portability between architectures with no code
changes.

We have endeavoured to ensure that the same generators and initializations are
used as documented in the oneMKL documentation. This means that functions which
return bit sequences are bitwise reproducible between Arm and x86 systems. If an
integer or floating point answer is requested answers may differ as the
precision of various operations is different between the two libraries.

Note that in this release not all of the random number functions from VSL have
been included. These functions are listed in the documentation as not
implemented. We are intending to fill out this coverage in future releases, and
we are very keen to hear from users who find missing functionality that they
would like us to prioritize, see \feedback.

# Basic Random Number Generators

A basic random number generator (BRNG) is any source of randomness capable of
producing a uniform distribution in the range [0, 1). The inclusion of 0 in the
range depends on the generator. Some BRNGs can also generate uniformly
distributed bit streams. OpenRNG supports three types of BRNGs:

 * Pseudorandom number generators
 * Quasirandom number generators
 * Nondeterministic random number generators

See \ref groupBrng for a list of all BRNGs supported by the API. Note that not
all BRNGs supported by the API have been implemented at this time. All
generators that have not been implemented will have a banner in their respective
documentation which says "Not implemented in \armplref".


# Pseudorandom Number Generators

Pseudorandom number generators (PRNG) are algorithms that generate sequences
which approximate randomness from an initial seed. PRNG sequences are periodic,
but the quality of a PRNG sequence is determined by how closely a subsequence
approximates a nondeterministic source of randomness. There are numerous
statistical tests for PRNGs, and several test suites that implement these tests.
See, for example, https://github.com/umontreal-simul/TestU01-2009.

The following is a list of all PRNGs that have been implemented in \armplref.

 * \ref VSL_BRNG_MCG31
 * \ref VSL_BRNG_R250
 * \ref VSL_BRNG_MRG32K3A
 * \ref VSL_BRNG_MCG59
 * \ref VSL_BRNG_MT19937
 * \ref VSL_BRNG_SFMT19937
 * \ref VSL_BRNG_NONDETERM
 * \ref VSL_BRNG_PHILOX4X32X10

The following is a list of all PRNGs that exist in the API but have _not_ been
implemented in \armplref. If any of these values are used, the API will return
\ref VSL_ERROR_FEATURE_NOT_IMPLEMENTED.

 * \ref VSL_BRNG_WH
 * \ref VSL_BRNG_MT2203
 * \ref VSL_BRNG_IABSTRACT
 * \ref VSL_BRNG_DABSTRACT
 * \ref VSL_BRNG_SABSTRACT
 * \ref VSL_BRNG_ARS5


# Quasirandom Number Generators

Quasirandom number generators (QRNG) are algorithms that generate sequences of
n-dimensional vectors uniformly distributed on n-dimensional hypercubes. For all
QRNGs in the API, the seed determines the number of dimensions. Each element of
the sequence consists of n-dimensional vectors represented by n consecutive
values in the returned buffer.

The following is a list of all QRNGs that have been implemented in \armplref.

 * \ref VSL_BRNG_SOBOL

The following is a list of all QRNGs that exist in the API but have _not_ been
implemented in \armplref. If any of these values are used, the API will return
\ref VSL_ERROR_FEATURE_NOT_IMPLEMENTED.

 * \ref VSL_BRNG_NIEDERR


# Nondeterministic Random Number Generators

Nondeterministic random number generators are generators that take entropy from
an external source to generate random numbers. Nondeterministic generators are
typically slower than PRNGs and the sequences are not reproducible.

\armplref implements the only value for nondeterministic generators:

 * \ref VSL_BRNG_NONDETERM

# Random Streams

An instance of a BRNG is referred to as a stream. OpenRNG provides service
functions for creating, initializing, copying, modifying and deleting streams,
see \ref groupService. For example, a stream with the PHILOX4X32X10 generator
and a single seed can be created as follows:

    VSLStreamStatePtr stream;
    int errcode = vslNewStream(&stream, VSL_BRNG_PHILOX4X32X10, 42);

See the \ref pi.c example, for a complete example of creating and using a
stream.


# Distributions

OpenRNG can generate random numbers with many common probability distributions,
both discrete and continuous. To access random numbers from the stream, you must
request numbers with a specific distribution, and OpenRNG will advance your
stream in the background. For example, a uniform distribution on [0, 1) can be
generated as follows:

    float randomNumbers[nRandomNumbers];
    int errcode = vsRngUniform(VSL_RNG_METHOD_UNIFORM_STD, stream, nRandomNumbers,
                               &randomNumbers, 0, 1);

This snippet will fill the buffer, `randomNumbers`, with random numbers from a
uniform distribution. On return, the amount your stream has advanced by will be
dependent on the distribution method used. See the \ref pi.c example for a
complete example of generating random numbers with a specified probability
distribution.

# Continuous distributions

A continuous distribution is any probability distribution over the reals. All
continuous distributions fill pre-allocated buffers. Continuous distributions
can be requested as either single-precision floating point, `float,` or
double-precision floating point, `double`. The single-precision floating point
version can be identified by the `vsRng` prefix, and the double-precision
floating point version can be identified by the `vdRng` prefix.

The following is a list of all continuous distributions that have been
implemented in \armplref.

  * \ref vsRngCauchy, \ref vdRngCauchy
  * \ref vsRngExponential, \ref vdRngExponential
  * \ref vsRngGaussian, \ref vdRngGaussian
  * \ref vsRngGumbel, \ref vdRngGumbel
  * \ref vsRngLaplace, \ref vdRngLaplace
  * \ref vsRngLognormal, \ref vdRngLognormal
  * \ref vsRngRayleigh, \ref vdRngRayleigh
  * \ref vsRngUniform, \ref vdRngUniform
  * \ref vsRngWeibull, \ref vdRngWeibull

The following is a list of all continuous distributions that exist in the API
but have _not_ been implemented in \armplref. If any of these methods are used,
they will return \ref VSL_ERROR_FEATURE_NOT_IMPLEMENTED.

  * \ref vsRngBeta, \ref vdRngBeta
  * \ref vsRngChiSquare, \ref vdRngChiSquare
  * \ref vsRngGamma, \ref vdRngGamma
  * \ref vsRngGaussianMV, \ref vdRngGaussianMV

# Discrete distributions

A discrete distribution is any probability distribution that's confined to the
integers. All discrete distributions fill pre-allocated buffers of type `int`
and their methods can be identified by the `viRng` prefix.

The following is a list of all discrete distributions that have been implemented
in \armplref.

  * \ref viRngBernoulli
  * \ref viRngBinomial
  * \ref viRngGeometric
  * \ref viRngPoisson
  * \ref viRngUniform
  * \ref viRngUniformBits
  * \ref viRngUniformBits32
  * \ref viRngUniformBits64

The following is a list of all discrete distributions that exist in the API but
have _not_ been implemented in \armplref. If any of these methods are used, they
will return \ref VSL_ERROR_FEATURE_NOT_IMPLEMENTED.

  * \ref viRngHypergeometric
  * \ref viRngMultinomial
  * \ref viRngNegbinomial
  * \ref viRngPoissonV

# Bit streams

OpenRNG supports 3 methods for generating bit streams: \ref viRngUniformBits,
\ref viRngUniformBits32 and \ref viRngUniformBits64.

\ref viRngUniformBits makes no guarantee of the uniformity of the bit sequences
returned; it reinterprets the integer sequence of each generator as a bit
sequence. For some generators, the integer sequence can contain bits that will
always be 0, or a collection of bits may exhibit periodic behavior over very
short intervals. The length of the buffer returned is dependent on the
generator's word size (\ref vslGetBrngProperties).

\ref viRngUniformBits32 and \ref viRngUniformBits64 guarantee that the bits will
have a uniform distribution. In the 32 bit variant, each element needs to be
able to hold 32 bits, and in the 64 bit variant, every element needs to be able
to hold 64 bits. They have almost identical output, there are very few reasons
to chose one over the other.


# Error handling

All methods return a negative value on failure. See \ref groupError for a list
of error code values and what each one means.


# Thread Safety

OpenRNG is thread-safe, provided multiple threads don't try to access the same
stream concurrently. In order to scale to multiple threads, OpenRNG provides
convenience methods for copying streams and generating disjoint subsequences.
See \ref vslCopyStream, \ref vslLeapfrogStream and \ref vslSkipAheadStream. See
the \ref skipahead.c example for an example of generating disjoint sequences.


# API Access

The API definitions are exposed via `openrng.h`. For convenience `armpl.h` also
includes `openrng.h`. See \accesspl for
instructions on how to build and link with \armplref.