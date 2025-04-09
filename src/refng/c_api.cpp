/*
 * SPDX-FileCopyrightText: <text>Copyright 2024-2025 Arm Limited and/or its
 * affiliates <open-source-office@arm.com></text>
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

#include <distributions/bernoulli.hpp>
#include <distributions/binomial.hpp>
#include <distributions/cauchy.hpp>
#include <distributions/exponential.hpp>
#include <distributions/gaussian.hpp>
#include <distributions/gaussianmv.hpp>
#include <distributions/geometric.hpp>
#include <distributions/gumbel.hpp>
#include <distributions/laplace.hpp>
#include <distributions/lognormal.hpp>
#include <distributions/poisson.hpp>
#include <distributions/rayleigh.hpp>
#include <distributions/uniform.hpp>
#include <distributions/weibull.hpp>
#include <generator/generator.hpp>
#include <openrng.h>
#include <refng_vsl.h>
#include <utils/dynamic_array.hpp>

#include <cassert>
#include <climits>
#include <cstdint>
#include <cstdio>

extern "C" {

int rdRngBeta(const refng_int_t, VSLStreamStatePtr, const refng_int_t, double[],
              const double, const double, const double, const double) {
  return VSL_ERROR_FEATURE_NOT_IMPLEMENTED;
}

int rdRngCauchy(const refng_int_t method, VSLStreamStatePtr stream,
                const refng_int_t n, double r[], const double a,
                const double beta) {
  auto [generator, error] = refng::Generator::extract(stream);
  if (error) {
    return error;
  }
  return refng::distribution::cauchy::fill(method, generator, n, r, a, beta);
}

int rdRngChiSquare(const refng_int_t, VSLStreamStatePtr, const refng_int_t,
                   double[], const int) {
  return VSL_ERROR_FEATURE_NOT_IMPLEMENTED;
}

int rdRngExponential(const refng_int_t method, VSLStreamStatePtr stream,
                     const refng_int_t n, double r[], const double a,
                     const double beta) {
  auto [generator, error] = refng::Generator::extract(stream);
  if (error) {
    return error;
  }
  return refng::distribution::exponential::fill(method, generator, n, r, a,
                                                beta);
}

int rdRngGamma(const refng_int_t, VSLStreamStatePtr, const refng_int_t,
               double[], const double, const double, const double) {
  return VSL_ERROR_FEATURE_NOT_IMPLEMENTED;
}

int rdRngGaussian(const refng_int_t method, VSLStreamStatePtr stream,
                  const refng_int_t n, double r[], const double a,
                  const double sigma) {
  auto [generator, error] = refng::Generator::extract(stream);
  if (error) {
    return error;
  }

  return refng::distribution::gaussian::fill(method, generator, n, r, a, sigma);
}

int rdRngGaussianMV(const refng_int_t method, VSLStreamStatePtr stream,
                    const refng_int_t n, double r[], const refng_int_t dimen,
                    const refng_int_t mstorage, const double *a,
                    const double *t) {
  auto [generator, error] = refng::Generator::extract(stream);
  if (error) {
    return error;
  }

  return refng::distribution::gaussianmv::fill(method, generator, n, dimen,
                                               mstorage, r, a, t);
}

int rdRngGumbel(const refng_int_t method, VSLStreamStatePtr stream,
                const refng_int_t n, double r[], const double a,
                const double beta) {
  auto [generator, error] = refng::Generator::extract(stream);
  if (error) {
    return error;
  }
  return refng::distribution::gumbel::fill(method, generator, n, r, a, beta);
}

int rdRngLaplace(const refng_int_t method, VSLStreamStatePtr stream,
                 const refng_int_t n, double r[], const double a,
                 const double beta) {
  auto [generator, error] = refng::Generator::extract(stream);
  if (error) {
    return error;
  }
  return refng::distribution::laplace::fill(method, generator, n, r, a, beta);
}

int rdRngLognormal(const refng_int_t method, VSLStreamStatePtr stream,
                   const refng_int_t n, double r[], const double a,
                   const double sigma, const double b, const double beta) {
  auto [generator, error] = refng::Generator::extract(stream);
  if (error) {
    return error;
  }
  return refng::distribution::lognormal::fill(method, generator, n, r, a, sigma,
                                              b, beta);
}

int rdRngRayleigh(const refng_int_t method, VSLStreamStatePtr stream,
                  const refng_int_t n, double r[], const double a,
                  const double beta) {
  auto [generator, error] = refng::Generator::extract(stream);
  if (error) {
    return error;
  }
  return refng::distribution::rayleigh::fill(method, generator, n, r, a, beta);
}

int rdRngUniform(const refng_int_t method, VSLStreamStatePtr stream,
                 const refng_int_t n, double r[], const double a,
                 const double b) {
  auto [generator, error] = refng::Generator::extract(stream);
  if (error) {
    return error;
  }
  return refng::distribution::uniform::fill(method, generator, n, r, a, b);
}

int rdRngWeibull(const refng_int_t method, VSLStreamStatePtr stream,
                 const refng_int_t n, double r[], const double alpha,
                 const double a, const double beta) {
  auto [generator, error] = refng::Generator::extract(stream);
  if (error) {
    return error;
  }
  return refng::distribution::weibull::fill(method, generator, n, r, alpha, a,
                                            beta);
}

int riRngBernoulli(const refng_int_t method, VSLStreamStatePtr stream,
                   const refng_int_t n, int r[], const double p) {
  auto [generator, error] = refng::Generator::extract(stream);
  if (error) {
    return error;
  }
  return refng::distribution::bernoulli::fill(method, generator, n, r, p);
}

int riRngBinomial(const refng_int_t method, VSLStreamStatePtr stream,
                  const refng_int_t n, int r[], const int ntrial,
                  const double p) {
  auto [generator, error] = refng::Generator::extract(stream);
  if (error) {
    return error;
  }
  return refng::distribution::binomial::fill(method, generator, n, r, ntrial,
                                             p);
}

int riRngGeometric(const refng_int_t method, VSLStreamStatePtr stream,
                   const refng_int_t n, int r[], const double p) {
  auto [generator, error] = refng::Generator::extract(stream);
  if (error) {
    return error;
  }
  return refng::distribution::geometric::fill(method, generator, n, r, p);
}

int riRngHypergeometric(const refng_int_t, VSLStreamStatePtr, const refng_int_t,
                        int[], const int, const int, const int) {
  return VSL_ERROR_FEATURE_NOT_IMPLEMENTED;
}

int riRngMultinomial(const refng_int_t, VSLStreamStatePtr, const refng_int_t,
                     int[], const int, const int, const double[]) {
  return VSL_ERROR_FEATURE_NOT_IMPLEMENTED;
}

int riRngNegBinomial(const refng_int_t, VSLStreamStatePtr, const refng_int_t,
                     int[], const double, const double) {
  return VSL_ERROR_FEATURE_NOT_IMPLEMENTED;
}

int riRngNegbinomial(const refng_int_t, VSLStreamStatePtr, const refng_int_t,
                     int[], const double, const double) {
  return VSL_ERROR_FEATURE_NOT_IMPLEMENTED;
}

int riRngPoisson(const refng_int_t method, VSLStreamStatePtr stream,
                 const refng_int_t n, int r[], const double lambda) {
  auto [generator, error] = refng::Generator::extract(stream);
  if (error) {
    return error;
  }
  return refng::distribution::poisson::fill(method, generator, n, r, lambda);
}

int riRngPoissonV(const refng_int_t, VSLStreamStatePtr, const refng_int_t,
                  int[], const double[]) {
  return VSL_ERROR_FEATURE_NOT_IMPLEMENTED;
}

int riRngUniform(const refng_int_t method, VSLStreamStatePtr stream,
                 const refng_int_t n, int r[], const int a, const int b) {
  auto [generator, error] = refng::Generator::extract(stream);
  if (error) {
    return error;
  }
  return refng::distribution::uniform::fill<int32_t>(method, generator, n, r, a,
                                                     b);
}

int riRngUniformBits([[maybe_unused]] const refng_int_t method,
                     VSLStreamStatePtr stream, const refng_int_t n,
                     unsigned int buffer[]) {
  assert(method == VSL_RNG_METHOD_UNIFORMBITS_STD);

  auto [generator, error] = refng::Generator::extract(stream);
  if (error) {
    return error;
  }

  return generator->fillBits(n, buffer);
}

int riRngUniformBits32([[maybe_unused]] const refng_int_t method,
                       VSLStreamStatePtr stream, const refng_int_t n,
                       unsigned int buffer[]) {
  assert(method == VSL_RNG_METHOD_UNIFORMBITS32_STD);

  auto [generator, error] = refng::Generator::extract(stream);
  if (error) {
    return error;
  }

  return generator->fillBits32(n, buffer);
}

int riRngUniformBits64([[maybe_unused]] const refng_int_t method,
                       VSLStreamStatePtr stream, const refng_int_t n,
                       refng_uint64_t buffer[]) {
  assert(method == VSL_RNG_METHOD_UNIFORMBITS64_STD);

  auto [generator, error] = refng::Generator::extract(stream);
  if (error) {
    return error;
  }

  static_assert(sizeof(uint64_t) == sizeof(refng_uint64_t));
  return generator->fillBits64(n, reinterpret_cast<uint64_t *>(buffer));
}

int rsRngBeta(const refng_int_t, VSLStreamStatePtr, const refng_int_t, float[],
              const float, const float, const float, const float) {
  return VSL_ERROR_FEATURE_NOT_IMPLEMENTED;
}

int rsRngCauchy(const refng_int_t method, VSLStreamStatePtr stream,
                const refng_int_t n, float r[], const float a,
                const float beta) {
  auto [generator, error] = refng::Generator::extract(stream);
  if (error) {
    return error;
  }
  return refng::distribution::cauchy::fill(method, generator, n, r, a, beta);
}

int rsRngChiSquare(const refng_int_t, VSLStreamStatePtr, const refng_int_t,
                   float[], const int) {
  return VSL_ERROR_FEATURE_NOT_IMPLEMENTED;
}

int rsRngExponential(const refng_int_t method, VSLStreamStatePtr stream,
                     const refng_int_t n, float r[], const float a,
                     const float beta) {
  auto [generator, error] = refng::Generator::extract(stream);
  if (error) {
    return error;
  }

  return refng::distribution::exponential::fill(method, generator, n, r, a,
                                                beta);
}

int rsRngGamma(const refng_int_t, VSLStreamStatePtr, const refng_int_t, float[],
               const float, const float, const float) {
  return VSL_ERROR_FEATURE_NOT_IMPLEMENTED;
}

int rsRngGaussian(const refng_int_t method, VSLStreamStatePtr stream,
                  const refng_int_t n, float r[], const float a,
                  const float sigma) {
  auto [generator, error] = refng::Generator::extract(stream);
  if (error) {
    return error;
  }

  return refng::distribution::gaussian::fill(method, generator, n, r, a, sigma);
}

int rsRngGaussianMV(const refng_int_t method, VSLStreamStatePtr stream,
                    const refng_int_t n, float r[], const refng_int_t dimen,
                    const refng_int_t mstorage, const float *a,
                    const float *t) {
  auto [generator, error] = refng::Generator::extract(stream);
  if (error) {
    return error;
  }

  return refng::distribution::gaussianmv::fill(method, generator, n, dimen,
                                               mstorage, r, a, t);
}

int rsRngGumbel(const refng_int_t method, VSLStreamStatePtr stream,
                const refng_int_t n, float r[], const float a,
                const float beta) {
  auto [generator, error] = refng::Generator::extract(stream);
  if (error) {
    return error;
  }
  return refng::distribution::gumbel::fill(method, generator, n, r, a, beta);
}

int rsRngLaplace(const refng_int_t method, VSLStreamStatePtr stream,
                 const refng_int_t n, float r[], const float a,
                 const float beta) {
  auto [generator, error] = refng::Generator::extract(stream);
  if (error) {
    return error;
  }

  return refng::distribution::laplace::fill(method, generator, n, r, a, beta);
}

int rsRngLognormal(const refng_int_t method, VSLStreamStatePtr stream,
                   const refng_int_t n, float r[], const float a,
                   const float sigma, const float b, const float beta) {
  auto [generator, error] = refng::Generator::extract(stream);
  if (error) {
    return error;
  }
  return refng::distribution::lognormal::fill(method, generator, n, r, a, sigma,
                                              b, beta);
}

int rsRngRayleigh(const refng_int_t method, VSLStreamStatePtr stream,
                  const refng_int_t n, float r[], const float a,
                  const float beta) {
  auto [generator, error] = refng::Generator::extract(stream);
  if (error) {
    return error;
  }
  return refng::distribution::rayleigh::fill(method, generator, n, r, a, beta);
}

int rsRngUniform(const refng_int_t method, VSLStreamStatePtr stream,
                 const refng_int_t n, float r[], const float a, const float b) {
  auto [generator, error] = refng::Generator::extract(stream);
  if (error) {
    return error;
  }
  return refng::distribution::uniform::fill(method, generator, n, r, a, b);
}

int rsRngWeibull(const refng_int_t method, VSLStreamStatePtr stream,
                 const refng_int_t n, float r[], const float alpha,
                 const float a, const float beta) {
  auto [generator, error] = refng::Generator::extract(stream);
  if (error) {
    return error;
  }
  return refng::distribution::weibull::fill(method, generator, n, r, alpha, a,
                                            beta);
}

int refCopyStream(VSLStreamStatePtr *newstream,
                  const VSLStreamStatePtr srcstream) {

  auto [srcGenerator, errorExtract] = refng::Generator::extract(srcstream);
  if (errorExtract) {
    return errorExtract;
  }

  auto [newGenerator, errorCreate] =
      refng::Generator::create(srcGenerator->brngId, srcGenerator);
  if (errorCreate) {
    return errorCreate;
  }

  *newstream = newGenerator;

  return VSL_ERROR_OK;
}

int refCopyStreamState(VSLStreamStatePtr dststream,
                       const VSLStreamStatePtr srcstream) {
  auto [srcGenerator, errorExtractSrc] = refng::Generator::extract(srcstream);
  if (errorExtractSrc) {
    return errorExtractSrc;
  }

  auto [dstGenerator, errorExtractDest] = refng::Generator::extract(dststream);
  if (errorExtractDest) {
    return errorExtractDest;
  }

  auto errorInitialise = dstGenerator->initialise(srcGenerator);
  if (errorInitialise) {
    return errorInitialise;
  }

  return VSL_ERROR_OK;
}

int refDeleteStream(VSLStreamStatePtr *ptr) {
  if (ptr == nullptr) {
    return VSL_ERROR_NULL_PTR;
  }

  auto [generator, error] = refng::Generator::extract(*ptr);
  if (error) {
    return error;
  }

  const int ret = refng::Generator::destroy(generator);
  *ptr = nullptr;
  return ret;
}

int refGetBrngProperties(const int brngId,
                         VSLBRngProperties *const properties) {
  if (brngId >= VSL_BRNG_MT2203 && brngId < VSL_BRNG_MT2203 + 6024) {
    properties->StreamStateSize = 0;
    properties->InitStream = nullptr;
    properties->sBRng = nullptr;
    properties->dBRng = nullptr;
    properties->iBRng = nullptr;
    properties->NSeeds = 70;
    properties->IncludesZero = 1;
    properties->WordSize = 4;
    properties->NBits = 32;
    return VSL_ERROR_OK;
  }

  switch (brngId) {
  case VSL_BRNG_MCG31:
    properties->StreamStateSize = 0;
    properties->InitStream = nullptr;
    properties->sBRng = nullptr;
    properties->dBRng = nullptr;
    properties->iBRng = nullptr;
    properties->NSeeds = 1;
    properties->IncludesZero = 0;
    properties->WordSize = 4;
    properties->NBits = 31;
    break;

  case VSL_BRNG_R250:
    properties->StreamStateSize = 0;
    properties->InitStream = nullptr;
    properties->sBRng = nullptr;
    properties->dBRng = nullptr;
    properties->iBRng = nullptr;
    properties->NSeeds = 252;
    properties->IncludesZero = 1;
    properties->WordSize = 4;
    properties->NBits = 32;
    break;

  case VSL_BRNG_MRG32K3A:
    properties->StreamStateSize = 0;
    properties->InitStream = nullptr;
    properties->sBRng = nullptr;
    properties->dBRng = nullptr;
    properties->iBRng = nullptr;
    properties->NSeeds = 6;
    properties->IncludesZero = 1;
    properties->WordSize = 4;
    properties->NBits = 32;
    break;

  case VSL_BRNG_MT19937:
  case VSL_BRNG_SFMT19937:
    properties->StreamStateSize = 0;
    properties->InitStream = nullptr;
    properties->sBRng = nullptr;
    properties->dBRng = nullptr;
    properties->iBRng = nullptr;
    properties->NSeeds = 625;
    properties->IncludesZero = 1;
    properties->WordSize = 4;
    properties->NBits = 32;
    break;

  case VSL_BRNG_PHILOX4X32X10:
    properties->StreamStateSize = 0;
    properties->InitStream = nullptr;
    properties->sBRng = nullptr;
    properties->dBRng = nullptr;
    properties->iBRng = nullptr;
    properties->NSeeds = 8;
    properties->IncludesZero = 1;
    properties->WordSize = 4;
    properties->NBits = 32;
    break;

  case VSL_BRNG_NONDETERM:
    properties->StreamStateSize = 0;
    properties->InitStream = nullptr;
    properties->sBRng = nullptr;
    properties->dBRng = nullptr;
    properties->iBRng = nullptr;
    properties->NSeeds = 1;
    properties->IncludesZero = 1;
    properties->WordSize = 4;
    properties->NBits = 32;
    break;

  case VSL_BRNG_MCG59:
    properties->StreamStateSize = 0;
    properties->InitStream = nullptr;
    properties->sBRng = nullptr;
    properties->dBRng = nullptr;
    properties->iBRng = nullptr;
    properties->NSeeds = 1;
    properties->IncludesZero = 0;
    properties->WordSize = 8;
    properties->NBits = 59;
    break;

  case VSL_BRNG_SOBOL:
    properties->StreamStateSize = 0;
    properties->InitStream = nullptr;
    properties->sBRng = nullptr;
    properties->dBRng = nullptr;
    properties->iBRng = nullptr;
    properties->NSeeds = 2604;
    properties->IncludesZero = 0;
    properties->WordSize = 4;
    properties->NBits = 32;
    break;

  case VSL_BRNG_ARS5:
    properties->StreamStateSize = 0;
    properties->InitStream = nullptr;
    properties->sBRng = nullptr;
    properties->dBRng = nullptr;
    properties->iBRng = nullptr;
    properties->NSeeds = 8;
    properties->IncludesZero = 1;
    properties->WordSize = 4;
    properties->NBits = 32;
    break;

  case VSL_BRNG_WH:
  case VSL_BRNG_NIEDERR:
  case VSL_BRNG_IABSTRACT:
  case VSL_BRNG_DABSTRACT:
  case VSL_BRNG_SABSTRACT:
    return VSL_ERROR_FEATURE_NOT_IMPLEMENTED;

  default:
    return VSL_RNG_ERROR_INVALID_BRNG_INDEX;
  }
  return VSL_ERROR_OK;
}

int refGetNumRegBrngs() { return VSL_ERROR_FEATURE_NOT_IMPLEMENTED; }

int refGetStreamSize(const VSLStreamStatePtr stream) {
  auto [generator, error] = refng::Generator::extract(stream);
  if (error) {
    return error;
  }

  return generator->stateSize();
}

int refGetStreamStateBrng(const VSLStreamStatePtr stream) {

  auto [generator, error] = refng::Generator::extract(stream);
  if (error) {
    return error;
  }

  return generator->brngId;
}

int refLeapfrogStream(VSLStreamStatePtr stream, const refng_int_t k,
                      const refng_int_t nstreams) {
  auto [generator, error] = refng::Generator::extract(stream);
  if (error) {
    return error;
  }
  return generator->leapfrog(k, nstreams);
}

int refLoadStreamF(VSLStreamStatePtr *stream, const char *fname) {

  const auto fileHandle = fopen(fname, "rb");
  if (fileHandle == nullptr) {
    return VSL_RNG_ERROR_FILE_OPEN;
  }

  // Get the file size.
  fseek(fileHandle, 0, SEEK_END);
  const auto size = ftell(fileHandle);
  fseek(fileHandle, 0, SEEK_SET);

  if (size < 0) {
    fclose(fileHandle);
    return VSL_RNG_ERROR_FILE_READ;
  }

  DynamicArray<char> buffer(size);
  const auto bytesRead = fread(buffer.data(), 1, size, fileHandle);
  if (bytesRead == 0) {
    fclose(fileHandle);
    return VSL_RNG_ERROR_FILE_READ;
  }

  const auto errorClose = fclose(fileHandle);
  if (errorClose) {
    return VSL_RNG_ERROR_FILE_CLOSE;
  }

  return refLoadStreamM(stream, buffer.data());
}

int refLoadStreamM(VSLStreamStatePtr *stream, const char *memptr) {
  if (stream == nullptr) {
    return VSL_ERROR_NULL_PTR;
  }

  auto [state, errorState] = refng::StateWrapperBase::extract(memptr);
  if (errorState) {
    return errorState;
  }

  auto [generator, errorCreate] =
      refng::Generator::create(state->brngId, state);
  if (errorCreate)
    return errorCreate;

  *stream = generator;

  return VSL_ERROR_OK;
}

int refNewStream(VSLStreamStatePtr *stream, const refng_int_t brng,
                 const refng_uint_t seed) {
#if defined(OPENRNG_ILP64)
  const auto n = uint64_t{seed} >= UINT_MAX ? 2 : 1;
#else
  const int n = 1;
#endif
  return refNewStreamEx(stream, brng, n,
                        reinterpret_cast<const unsigned int *>(&seed));
}

int refNewStreamEx(VSLStreamStatePtr *stream, const refng_int_t brng,
                   const refng_int_t n, const unsigned int params[]) {

  auto [generator, error] = refng::Generator::create(brng, int64_t{n}, params);
  if (error)
    return error;

  *stream = generator;

  return VSL_ERROR_OK;
}

int refRegisterBrng(const VSLBRngProperties *) {
  return VSL_ERROR_FEATURE_NOT_IMPLEMENTED;
}

int refSaveStreamF(const VSLStreamStatePtr stream, const char *fname) {

  const auto [generator, error] = refng::Generator::extract(stream);
  if (error) {
    return error;
  }

  const auto size = generator->stateSize();
  if (size <= 0) {
    return size;
  }

  DynamicArray<char> buffer(size);
  if (buffer.data() == nullptr) {
    return VSL_ERROR_MEM_FAILURE;
  }

  const auto errorSave = generator->save(buffer.data());
  if (errorSave) {
    return errorSave;
  }

  const auto fileHandle = fopen(fname, "wb");
  if (fileHandle == nullptr) {
    return VSL_RNG_ERROR_FILE_OPEN;
  }

  const auto bytesWritten = fwrite(buffer.data(), 1, size, fileHandle);
  if (bytesWritten == 0) {
    fclose(fileHandle);
    return VSL_RNG_ERROR_FILE_WRITE;
  }

  const auto errorClose = fclose(fileHandle);
  if (errorClose) {
    return VSL_RNG_ERROR_FILE_CLOSE;
  }

  return VSL_ERROR_OK;
}

int refSaveStreamM(const VSLStreamStatePtr stream, char *memptr) {
  auto [generator, error] = refng::Generator::extract(stream);
  if (error) {
    return error;
  }

  if (memptr == nullptr) {
    return VSL_ERROR_NULL_PTR;
  }

  return generator->save(memptr);
}

int refSkipAheadStream(VSLStreamStatePtr stream, const long long int nskip) {
  // nskip is cast to a uint64_t because of an inconsistency in the interface.
  // We currently don't have any implementations that can test this is the
  // correct behaviour with negative numbers, so this should be good enough for
  // now.
  refng_uint64_t nskips[1] = {(refng_uint64_t)nskip};
  const int error = refSkipAheadStreamEx(stream, 1, nskips);
  if (error == VSL_RNG_ERROR_SKIPAHEADEX_UNSUPPORTED) {
    return VSL_RNG_ERROR_SKIPAHEAD_UNSUPPORTED;
  }
  return error;
}

int refSkipAheadStreamEx(VSLStreamStatePtr stream, const refng_int_t n,
                         const refng_uint64_t nskip[]) {
  auto [generator, error] = refng::Generator::extract(stream);
  if (error) {
    return error;
  }

  static_assert(sizeof(uint64_t) == sizeof(refng_uint64_t));
  return generator->skipAhead(n, reinterpret_cast<const uint64_t *>(nskip));
}

int refdNewAbstractStream(VSLStreamStatePtr *, const refng_int_t,
                          const double[], const double, const double,
                          const dUpdateFuncPtr) {
  return VSL_ERROR_FEATURE_NOT_IMPLEMENTED;
}

int refiNewAbstractStream(VSLStreamStatePtr *, const refng_int_t,
                          const unsigned int[], const iUpdateFuncPtr) {
  return VSL_ERROR_FEATURE_NOT_IMPLEMENTED;
}

int refsNewAbstractStream(VSLStreamStatePtr *, const refng_int_t, const float[],
                          const float, const float, const sUpdateFuncPtr) {
  return VSL_ERROR_FEATURE_NOT_IMPLEMENTED;
}
}
