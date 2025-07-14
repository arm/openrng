/*
 * SPDX-FileCopyrightText: <text>Copyright 2024-2025 Arm Limited and/or its
 * affiliates <open-source-office@arm.com></text>
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

#include <openrng.h>

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
#include <utils/dynamic_array.hpp>
#include <utils/version.hpp>

#include <cassert>
#include <climits>
#include <cstdint>
#include <cstdio>

extern "C" {

int vdRngBeta(const openrng_int_t, VSLStreamStatePtr, const openrng_int_t,
              double[], const double, const double, const double,
              const double) {
  return VSL_ERROR_FEATURE_NOT_IMPLEMENTED;
}

int vdRngCauchy(const openrng_int_t method, VSLStreamStatePtr stream,
                const openrng_int_t n, double r[], const double a,
                const double beta) {
  auto [generator, error] = openrng::Generator::extract(stream);
  if (error) {
    return error;
  }
  return openrng::distribution::cauchy::fill(method, generator, n, r, a, beta);
}

int vdRngChiSquare(const openrng_int_t, VSLStreamStatePtr, const openrng_int_t,
                   double[], const int) {
  return VSL_ERROR_FEATURE_NOT_IMPLEMENTED;
}

int vdRngExponential(const openrng_int_t method, VSLStreamStatePtr stream,
                     const openrng_int_t n, double r[], const double a,
                     const double beta) {
  auto [generator, error] = openrng::Generator::extract(stream);
  if (error) {
    return error;
  }
  return openrng::distribution::exponential::fill(method, generator, n, r, a,
                                                  beta);
}

int vdRngGamma(const openrng_int_t, VSLStreamStatePtr, const openrng_int_t,
               double[], const double, const double, const double) {
  return VSL_ERROR_FEATURE_NOT_IMPLEMENTED;
}

int vdRngGaussian(const openrng_int_t method, VSLStreamStatePtr stream,
                  const openrng_int_t n, double r[], const double a,
                  const double sigma) {
  auto [generator, error] = openrng::Generator::extract(stream);
  if (error) {
    return error;
  }

  return openrng::distribution::gaussian::fill(method, generator, n, r, a,
                                               sigma);
}

int vdRngGaussianMV(const openrng_int_t method, VSLStreamStatePtr stream,
                    const openrng_int_t n, double r[],
                    const openrng_int_t dimen, const openrng_int_t mstorage,
                    const double *a, const double *t) {
  auto [generator, error] = openrng::Generator::extract(stream);
  if (error) {
    return error;
  }

  return openrng::distribution::gaussianmv::fill(method, generator, n, dimen,
                                                 mstorage, r, a, t);
}

int vdRngGumbel(const openrng_int_t method, VSLStreamStatePtr stream,
                const openrng_int_t n, double r[], const double a,
                const double beta) {
  auto [generator, error] = openrng::Generator::extract(stream);
  if (error) {
    return error;
  }
  return openrng::distribution::gumbel::fill(method, generator, n, r, a, beta);
}

int vdRngLaplace(const openrng_int_t method, VSLStreamStatePtr stream,
                 const openrng_int_t n, double r[], const double a,
                 const double beta) {
  auto [generator, error] = openrng::Generator::extract(stream);
  if (error) {
    return error;
  }
  return openrng::distribution::laplace::fill(method, generator, n, r, a, beta);
}

int vdRngLognormal(const openrng_int_t method, VSLStreamStatePtr stream,
                   const openrng_int_t n, double r[], const double a,
                   const double sigma, const double b, const double beta) {
  auto [generator, error] = openrng::Generator::extract(stream);
  if (error) {
    return error;
  }
  return openrng::distribution::lognormal::fill(method, generator, n, r, a,
                                                sigma, b, beta);
}

int vdRngRayleigh(const openrng_int_t method, VSLStreamStatePtr stream,
                  const openrng_int_t n, double r[], const double a,
                  const double beta) {
  auto [generator, error] = openrng::Generator::extract(stream);
  if (error) {
    return error;
  }
  return openrng::distribution::rayleigh::fill(method, generator, n, r, a,
                                               beta);
}

int vdRngUniform(const openrng_int_t method, VSLStreamStatePtr stream,
                 const openrng_int_t n, double r[], const double a,
                 const double b) {
  auto [generator, error] = openrng::Generator::extract(stream);
  if (error) {
    return error;
  }
  return openrng::distribution::uniform::fill(method, generator, n, r, a, b);
}

int vdRngWeibull(const openrng_int_t method, VSLStreamStatePtr stream,
                 const openrng_int_t n, double r[], const double alpha,
                 const double a, const double beta) {
  auto [generator, error] = openrng::Generator::extract(stream);
  if (error) {
    return error;
  }
  return openrng::distribution::weibull::fill(method, generator, n, r, alpha, a,
                                              beta);
}

int viRngBernoulli(const openrng_int_t method, VSLStreamStatePtr stream,
                   const openrng_int_t n, int r[], const double p) {
  auto [generator, error] = openrng::Generator::extract(stream);
  if (error) {
    return error;
  }
  return openrng::distribution::bernoulli::fill(method, generator, n, r, p);
}

int viRngBinomial(const openrng_int_t method, VSLStreamStatePtr stream,
                  const openrng_int_t n, int r[], const int ntrial,
                  const double p) {
  auto [generator, error] = openrng::Generator::extract(stream);
  if (error) {
    return error;
  }
  return openrng::distribution::binomial::fill(method, generator, n, r, ntrial,
                                               p);
}

int viRngGeometric(const openrng_int_t method, VSLStreamStatePtr stream,
                   const openrng_int_t n, int r[], const double p) {
  auto [generator, error] = openrng::Generator::extract(stream);
  if (error) {
    return error;
  }
  return openrng::distribution::geometric::fill(method, generator, n, r, p);
}

int viRngHypergeometric(const openrng_int_t, VSLStreamStatePtr,
                        const openrng_int_t, int[], const int, const int,
                        const int) {
  return VSL_ERROR_FEATURE_NOT_IMPLEMENTED;
}

int viRngMultinomial(const openrng_int_t, VSLStreamStatePtr,
                     const openrng_int_t, int[], const int, const int,
                     const double[]) {
  return VSL_ERROR_FEATURE_NOT_IMPLEMENTED;
}

int viRngNegBinomial(const openrng_int_t, VSLStreamStatePtr,
                     const openrng_int_t, int[], const double, const double) {
  return VSL_ERROR_FEATURE_NOT_IMPLEMENTED;
}

int viRngNegbinomial(const openrng_int_t, VSLStreamStatePtr,
                     const openrng_int_t, int[], const double, const double) {
  return VSL_ERROR_FEATURE_NOT_IMPLEMENTED;
}

int viRngPoisson(const openrng_int_t method, VSLStreamStatePtr stream,
                 const openrng_int_t n, int r[], const double lambda) {
  auto [generator, error] = openrng::Generator::extract(stream);
  if (error) {
    return error;
  }
  return openrng::distribution::poisson::fill(method, generator, n, r, lambda);
}

int viRngPoissonV(const openrng_int_t, VSLStreamStatePtr, const openrng_int_t,
                  int[], const double[]) {
  return VSL_ERROR_FEATURE_NOT_IMPLEMENTED;
}

int viRngUniform(const openrng_int_t method, VSLStreamStatePtr stream,
                 const openrng_int_t n, int r[], const int a, const int b) {
  auto [generator, error] = openrng::Generator::extract(stream);
  if (error) {
    return error;
  }
  return openrng::distribution::uniform::fill<int32_t>(method, generator, n, r,
                                                       a, b);
}

int viRngUniformBits([[maybe_unused]] const openrng_int_t method,
                     VSLStreamStatePtr stream, const openrng_int_t n,
                     unsigned int buffer[]) {
  assert(method == VSL_RNG_METHOD_UNIFORMBITS_STD);

  auto [generator, error] = openrng::Generator::extract(stream);
  if (error) {
    return error;
  }

  return generator->fillBits(n, buffer);
}

int viRngUniformBits32([[maybe_unused]] const openrng_int_t method,
                       VSLStreamStatePtr stream, const openrng_int_t n,
                       unsigned int buffer[]) {
  assert(method == VSL_RNG_METHOD_UNIFORMBITS32_STD);

  auto [generator, error] = openrng::Generator::extract(stream);
  if (error) {
    return error;
  }

  return generator->fillBits32(n, buffer);
}

int viRngUniformBits64([[maybe_unused]] const openrng_int_t method,
                       VSLStreamStatePtr stream, const openrng_int_t n,
                       openrng_uint64_t buffer[]) {
  assert(method == VSL_RNG_METHOD_UNIFORMBITS64_STD);

  auto [generator, error] = openrng::Generator::extract(stream);
  if (error) {
    return error;
  }

  static_assert(sizeof(uint64_t) == sizeof(openrng_uint64_t));
  return generator->fillBits64(n, reinterpret_cast<uint64_t *>(buffer));
}

int vsRngBeta(const openrng_int_t, VSLStreamStatePtr, const openrng_int_t,
              float[], const float, const float, const float, const float) {
  return VSL_ERROR_FEATURE_NOT_IMPLEMENTED;
}

int vsRngCauchy(const openrng_int_t method, VSLStreamStatePtr stream,
                const openrng_int_t n, float r[], const float a,
                const float beta) {
  auto [generator, error] = openrng::Generator::extract(stream);
  if (error) {
    return error;
  }
  return openrng::distribution::cauchy::fill(method, generator, n, r, a, beta);
}

int vsRngChiSquare(const openrng_int_t, VSLStreamStatePtr, const openrng_int_t,
                   float[], const int) {
  return VSL_ERROR_FEATURE_NOT_IMPLEMENTED;
}

int vsRngExponential(const openrng_int_t method, VSLStreamStatePtr stream,
                     const openrng_int_t n, float r[], const float a,
                     const float beta) {
  auto [generator, error] = openrng::Generator::extract(stream);
  if (error) {
    return error;
  }

  return openrng::distribution::exponential::fill(method, generator, n, r, a,
                                                  beta);
}

int vsRngGamma(const openrng_int_t, VSLStreamStatePtr, const openrng_int_t,
               float[], const float, const float, const float) {
  return VSL_ERROR_FEATURE_NOT_IMPLEMENTED;
}

int vsRngGaussian(const openrng_int_t method, VSLStreamStatePtr stream,
                  const openrng_int_t n, float r[], const float a,
                  const float sigma) {
  auto [generator, error] = openrng::Generator::extract(stream);
  if (error) {
    return error;
  }

  return openrng::distribution::gaussian::fill(method, generator, n, r, a,
                                               sigma);
}

int vsRngGaussianMV(const openrng_int_t method, VSLStreamStatePtr stream,
                    const openrng_int_t n, float r[], const openrng_int_t dimen,
                    const openrng_int_t mstorage, const float *a,
                    const float *t) {
  auto [generator, error] = openrng::Generator::extract(stream);
  if (error) {
    return error;
  }

  return openrng::distribution::gaussianmv::fill(method, generator, n, dimen,
                                                 mstorage, r, a, t);
}

int vsRngGumbel(const openrng_int_t method, VSLStreamStatePtr stream,
                const openrng_int_t n, float r[], const float a,
                const float beta) {
  auto [generator, error] = openrng::Generator::extract(stream);
  if (error) {
    return error;
  }
  return openrng::distribution::gumbel::fill(method, generator, n, r, a, beta);
}

int vsRngLaplace(const openrng_int_t method, VSLStreamStatePtr stream,
                 const openrng_int_t n, float r[], const float a,
                 const float beta) {
  auto [generator, error] = openrng::Generator::extract(stream);
  if (error) {
    return error;
  }

  return openrng::distribution::laplace::fill(method, generator, n, r, a, beta);
}

int vsRngLognormal(const openrng_int_t method, VSLStreamStatePtr stream,
                   const openrng_int_t n, float r[], const float a,
                   const float sigma, const float b, const float beta) {
  auto [generator, error] = openrng::Generator::extract(stream);
  if (error) {
    return error;
  }
  return openrng::distribution::lognormal::fill(method, generator, n, r, a,
                                                sigma, b, beta);
}

int vsRngRayleigh(const openrng_int_t method, VSLStreamStatePtr stream,
                  const openrng_int_t n, float r[], const float a,
                  const float beta) {
  auto [generator, error] = openrng::Generator::extract(stream);
  if (error) {
    return error;
  }
  return openrng::distribution::rayleigh::fill(method, generator, n, r, a,
                                               beta);
}

int vsRngUniform(const openrng_int_t method, VSLStreamStatePtr stream,
                 const openrng_int_t n, float r[], const float a,
                 const float b) {
  auto [generator, error] = openrng::Generator::extract(stream);
  if (error) {
    return error;
  }
  return openrng::distribution::uniform::fill(method, generator, n, r, a, b);
}

int vsRngWeibull(const openrng_int_t method, VSLStreamStatePtr stream,
                 const openrng_int_t n, float r[], const float alpha,
                 const float a, const float beta) {
  auto [generator, error] = openrng::Generator::extract(stream);
  if (error) {
    return error;
  }
  return openrng::distribution::weibull::fill(method, generator, n, r, alpha, a,
                                              beta);
}

int vslCopyStream(VSLStreamStatePtr *newstream,
                  const VSLStreamStatePtr srcstream) {

  auto [srcGenerator, errorExtract] = openrng::Generator::extract(srcstream);
  if (errorExtract) {
    return errorExtract;
  }

  auto [newGenerator, errorCreate] =
      openrng::Generator::create(srcGenerator->brngId, srcGenerator);
  if (errorCreate) {
    return errorCreate;
  }

  *newstream = newGenerator;

  return VSL_ERROR_OK;
}

int vslCopyStreamState(VSLStreamStatePtr dststream,
                       const VSLStreamStatePtr srcstream) {
  auto [srcGenerator, errorExtractSrc] = openrng::Generator::extract(srcstream);
  if (errorExtractSrc) {
    return errorExtractSrc;
  }

  auto [dstGenerator, errorExtractDest] =
      openrng::Generator::extract(dststream);
  if (errorExtractDest) {
    return errorExtractDest;
  }

  auto errorInitialise = dstGenerator->initialise(srcGenerator);
  if (errorInitialise) {
    return errorInitialise;
  }

  return VSL_ERROR_OK;
}

int vslDeleteStream(VSLStreamStatePtr *ptr) {
  if (ptr == nullptr) {
    return VSL_ERROR_NULL_PTR;
  }

  auto [generator, error] = openrng::Generator::extract(*ptr);
  if (error) {
    return error;
  }

  const int ret = openrng::Generator::destroy(generator);
  *ptr = nullptr;
  return ret;
}

int vslGetBrngProperties(const int brngId,
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

int vslGetNumRegBrngs() { return VSL_ERROR_FEATURE_NOT_IMPLEMENTED; }

int vslGetStreamSize(const VSLStreamStatePtr stream) {
  auto [generator, error] = openrng::Generator::extract(stream);
  if (error) {
    return error;
  }

  return generator->stateSize();
}

int vslGetStreamStateBrng(const VSLStreamStatePtr stream) {

  auto [generator, error] = openrng::Generator::extract(stream);
  if (error) {
    return error;
  }

  return generator->brngId;
}

int vslLeapfrogStream(VSLStreamStatePtr stream, const openrng_int_t k,
                      const openrng_int_t nstreams) {
  auto [generator, error] = openrng::Generator::extract(stream);
  if (error) {
    return error;
  }
  return generator->leapfrog(k, nstreams);
}

int vslLoadStreamF(VSLStreamStatePtr *stream, const char *fname) {

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

  return vslLoadStreamM(stream, buffer.data());
}

int vslLoadStreamM(VSLStreamStatePtr *stream, const char *memptr) {
  if (stream == nullptr) {
    return VSL_ERROR_NULL_PTR;
  }

  auto [state, errorState] = openrng::StateWrapperBase::extract(memptr);
  if (errorState) {
    return errorState;
  }

  auto [generator, errorCreate] =
      openrng::Generator::create(state->brngId, state);
  if (errorCreate)
    return errorCreate;

  *stream = generator;

  return VSL_ERROR_OK;
}

int vslNewStream(VSLStreamStatePtr *stream, const openrng_int_t brng,
                 const openrng_uint_t seed) {
#if defined(OPENRNG_ILP64)
  const auto n = uint64_t{seed} >= UINT_MAX ? 2 : 1;
#else
  const int n = 1;
#endif
  return vslNewStreamEx(stream, brng, n,
                        reinterpret_cast<const unsigned int *>(&seed));
}

int vslNewStreamEx(VSLStreamStatePtr *stream, const openrng_int_t brng,
                   const openrng_int_t n, const unsigned int params[]) {

  auto [generator, error] =
      openrng::Generator::create(brng, int64_t{n}, params);
  if (error)
    return error;

  *stream = generator;

  return VSL_ERROR_OK;
}

int vslRegisterBrng(const VSLBRngProperties *) {
  return VSL_ERROR_FEATURE_NOT_IMPLEMENTED;
}

int vslSaveStreamF(const VSLStreamStatePtr stream, const char *fname) {

  const auto [generator, error] = openrng::Generator::extract(stream);
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

int vslSaveStreamM(const VSLStreamStatePtr stream, char *memptr) {
  auto [generator, error] = openrng::Generator::extract(stream);
  if (error) {
    return error;
  }

  if (memptr == nullptr) {
    return VSL_ERROR_NULL_PTR;
  }

  return generator->save(memptr);
}

int vslSkipAheadStream(VSLStreamStatePtr stream, const long long int nskip) {
  // nskip is cast to a uint64_t because of an inconsistency in the interface.
  // We currently don't have any implementations that can test this is the
  // correct behaviour with negative numbers, so this should be good enough for
  // now.
  openrng_uint64_t nskips[1] = {(openrng_uint64_t)nskip};
  const int error = vslSkipAheadStreamEx(stream, 1, nskips);
  if (error == VSL_RNG_ERROR_SKIPAHEADEX_UNSUPPORTED) {
    return VSL_RNG_ERROR_SKIPAHEAD_UNSUPPORTED;
  }
  return error;
}

int vslSkipAheadStreamEx(VSLStreamStatePtr stream, const openrng_int_t n,
                         const openrng_uint64_t nskip[]) {
  auto [generator, error] = openrng::Generator::extract(stream);
  if (error) {
    return error;
  }

  static_assert(sizeof(uint64_t) == sizeof(openrng_uint64_t));
  return generator->skipAhead(n, reinterpret_cast<const uint64_t *>(nskip));
}

int vsldNewAbstractStream(VSLStreamStatePtr *, const openrng_int_t,
                          const double[], const double, const double,
                          const dUpdateFuncPtr) {
  return VSL_ERROR_FEATURE_NOT_IMPLEMENTED;
}

int vsliNewAbstractStream(VSLStreamStatePtr *, const openrng_int_t,
                          const unsigned int[], const iUpdateFuncPtr) {
  return VSL_ERROR_FEATURE_NOT_IMPLEMENTED;
}

int vslsNewAbstractStream(VSLStreamStatePtr *, const openrng_int_t,
                          const float[], const float, const float,
                          const sUpdateFuncPtr) {
  return VSL_ERROR_FEATURE_NOT_IMPLEMENTED;
}

void openRngGetVersion(int32_t *major, int32_t *minor, int32_t *patch) {
  return getLibraryVersion(major, minor, patch);
}
}
