/*
 * SPDX-FileCopyrightText: <text>Copyright 2024-2026 Arm Limited and/or its
 * affiliates <open-source-office@arm.com></text>
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/generators/catch_generators_range.hpp>
#include <catch2/matchers/catch_matchers_contains.hpp>
#include <functional>
#include <map>
#include <openrng.h>
#include <random>
#include <refng_vsl.h>
#include <string>
#include <supports_autogen.h>
#include <utils/detect_trng.hpp>
#include <utils/names_map.hpp>

// Header for getpid
#ifdef _WIN32
#include <process.h>
#else
#include <unistd.h>
#endif

namespace {

// Always seed rand with the process ID, this avoids collisions when tests are
// run concurrently. VSL could have been used here, but we didn't provide
// viRngUniform at the time of writing.
static std::mt19937 rng(getpid());
static std::uniform_int_distribution<int> dist(0, 25);

// Returns a random filename.
//
// The returned filename will be
//
//    tmpFileForTesting-XXXXXXXXXXXXXXXX
//
// where X is a random character in the range [A, Z]. std::tmpnam() would have
// been sufficient if it didn't come with a linker warning.
//
// This method assumes rand has been seeded with a unique seed for this process.
//
std::string tmpFileName() {
  std::string result = "tmpFileForTesting-";
  for (int i = 0; i < 16; i++) {
    auto u = dist(rng);
    result.push_back(u + 'A');
  }
  return result;
}

// Creates a temporary file in the current working directory to test
// serialisation and deserialisation. Provided vslSaveStreamF and vslLoadStreamF
// don't fail, the file will be deleted.
void saveLoadStreamF(VSLStreamStatePtr *newStream,
                     VSLStreamStatePtr oldStream) {
  int errcode;

  const auto filename = tmpFileName();
  CAPTURE(filename);

  errcode = vslSaveStreamF(oldStream, filename.c_str());
  REQUIRE(errcode == VSL_ERROR_OK);

  errcode = vslLoadStreamF(newStream, filename.c_str());
  REQUIRE(errcode == VSL_ERROR_OK);

  std::remove(filename.c_str());
}

void saveLoadStreamM(VSLStreamStatePtr *newStream,
                     VSLStreamStatePtr oldStream) {
  int errcode;

  std::vector<char> stateBuffer(vslGetStreamSize(oldStream));
  errcode = vslSaveStreamM(oldStream, stateBuffer.data());
  REQUIRE(errcode == VSL_ERROR_OK);

  errcode = vslLoadStreamM(newStream, stateBuffer.data());
  REQUIRE(errcode == VSL_ERROR_OK);
}

std::map<std::string, std::function<decltype(saveLoadStreamM)>> saveLoadMap{
    {"memory", saveLoadStreamM},
    {"file",   saveLoadStreamF}
};

} // namespace

TEST_CASE("Save/load stream basic") {
  VSLStreamStatePtr oldStream, newStream;
  VSLBRngProperties brngProperties;
  const int n = 1000;
  int errcode;

  const auto brngId = GENERATE(NEWSTREAM_SUPPORTED);
  const auto variant = GENERATE("memory", "file");
  const auto saveLoadStream = saveLoadMap[variant];

  CAPTURE(brngToString(brngId), variant);

  errcode = vslGetBrngProperties(brngId, &brngProperties);
  REQUIRE(errcode == VSL_ERROR_OK);
  const int bufferSize = brngProperties.WordSize / sizeof(uint32_t) * n;

  errcode = vslNewStream(&oldStream, brngId, 0);
  REQUIRE(errcode == VSL_ERROR_OK);

  saveLoadStream(&newStream, oldStream);

  std::vector<uint32_t> oldBuffer(bufferSize);
  std::vector<uint32_t> newBuffer(bufferSize);

  errcode = viRngUniformBits(VSL_RNG_METHOD_UNIFORMBITS_STD, oldStream, n,
                             oldBuffer.data());
  REQUIRE(errcode == VSL_ERROR_OK);

  errcode = viRngUniformBits(VSL_RNG_METHOD_UNIFORMBITS_STD, newStream, n,
                             newBuffer.data());
  REQUIRE(errcode == VSL_ERROR_OK);

  REQUIRE(oldBuffer == newBuffer);

  errcode = vslDeleteStream(&oldStream);
  REQUIRE(errcode == VSL_ERROR_OK);

  errcode = vslDeleteStream(&newStream);
  REQUIRE(errcode == VSL_ERROR_OK);
}

TEST_CASE("Save/load stream with intermediate deallocation") {
  VSLStreamStatePtr oldStream, newStream;
  VSLBRngProperties brngProperties;
  const int n = 1000;
  int errcode;

  const auto brngId = GENERATE(NEWSTREAM_SUPPORTED);

  errcode = vslGetBrngProperties(brngId, &brngProperties);
  REQUIRE(errcode == VSL_ERROR_OK);
  const int bufferSize = brngProperties.WordSize / sizeof(uint32_t) * n;

  errcode = vslNewStream(&oldStream, brngId, 0);
  REQUIRE(errcode == VSL_ERROR_OK);

  std::vector<uint32_t> oldBuffer(bufferSize);
  std::vector<uint32_t> newBuffer(bufferSize);

  // Fake a skipahead of n elements (some generators do not support skipahead)
  errcode = viRngUniformBits(VSL_RNG_METHOD_UNIFORMBITS_STD, oldStream, n,
                             oldBuffer.data());
  REQUIRE(errcode == VSL_ERROR_OK);

  const auto filename = tmpFileName();
  CAPTURE(filename);

  errcode = vslSaveStreamF(oldStream, filename.c_str());
  REQUIRE(errcode == VSL_ERROR_OK);

  errcode = viRngUniformBits(VSL_RNG_METHOD_UNIFORMBITS_STD, oldStream, n,
                             oldBuffer.data());
  REQUIRE(errcode == VSL_ERROR_OK);

  errcode = vslDeleteStream(&oldStream);
  REQUIRE(errcode == VSL_ERROR_OK);

  errcode = vslLoadStreamF(&newStream, filename.c_str());
  REQUIRE(errcode == VSL_ERROR_OK);

  std::remove(filename.c_str());

  errcode = viRngUniformBits(VSL_RNG_METHOD_UNIFORMBITS_STD, newStream, n,
                             newBuffer.data());
  REQUIRE(errcode == VSL_ERROR_OK);

  REQUIRE(oldBuffer == newBuffer);

  errcode = vslDeleteStream(&newStream);
  REQUIRE(errcode == VSL_ERROR_OK);
}

TEST_CASE("Save/load stream state skip ahead") {
  VSLStreamStatePtr oldStream, newStream;
  VSLBRngProperties brngProperties;
  const int n = 1000;
  const int nSkip = 1000;
  const int oldSeed = 0;
  const int newSeed = 0xff;
  int errcode;

  const auto brngId = GENERATE(SKIPAHEAD_SUPPORTED);
  const auto variant = GENERATE("memory", "file");
  const auto saveLoadStream = saveLoadMap[variant];

  CAPTURE(brngToString(brngId), variant);

  errcode = vslGetBrngProperties(brngId, &brngProperties);
  REQUIRE(errcode == VSL_ERROR_OK);
  const int bufferSize = brngProperties.WordSize / sizeof(uint32_t) * n;

  errcode = vslNewStream(&oldStream, brngId, oldSeed);
  REQUIRE(errcode == VSL_ERROR_OK);

  errcode = vslSkipAheadStream(oldStream, nSkip);
  REQUIRE(errcode == VSL_ERROR_OK);

  saveLoadStream(&newStream, oldStream);

  std::vector<uint32_t> oldBuffer(bufferSize);
  std::vector<uint32_t> newBuffer(bufferSize);

  errcode = viRngUniformBits(VSL_RNG_METHOD_UNIFORMBITS_STD, oldStream, n,
                             oldBuffer.data());
  REQUIRE(errcode == VSL_ERROR_OK);

  errcode = viRngUniformBits(VSL_RNG_METHOD_UNIFORMBITS_STD, newStream, n,
                             newBuffer.data());
  REQUIRE(errcode == VSL_ERROR_OK);

  REQUIRE(oldBuffer == newBuffer);

  errcode = vslDeleteStream(&oldStream);
  REQUIRE(errcode == VSL_ERROR_OK);

  errcode = vslDeleteStream(&newStream);
  REQUIRE(errcode == VSL_ERROR_OK);
}

TEST_CASE("Save/load stream state leap frog") {
  VSLStreamStatePtr oldStream, newStream;
  VSLBRngProperties brngProperties;
  const int n = 1000;
  const int threadId = 11;
  const int threadCount = 64;
  const int oldSeed = 0;
  const int newSeed = 0xff;
  int errcode;

  const auto brngId = GENERATE(LEAPFROG_SUPPORTED);
  const auto variant = GENERATE("memory", "file");
  const auto saveLoadStream = saveLoadMap[variant];

  CAPTURE(brngToString(brngId), variant);

  errcode = vslGetBrngProperties(brngId, &brngProperties);
  REQUIRE(errcode == VSL_ERROR_OK);
  const int bufferSize = brngProperties.WordSize / sizeof(uint32_t) * n;

  errcode = vslNewStream(&oldStream, brngId, oldSeed);
  REQUIRE(errcode == VSL_ERROR_OK);
  saveLoadStream(&newStream, oldStream);

  std::vector<uint32_t> oldBuffer(bufferSize);
  std::vector<uint32_t> newBuffer(bufferSize);

  errcode = viRngUniformBits(VSL_RNG_METHOD_UNIFORMBITS_STD, oldStream, n,
                             oldBuffer.data());
  REQUIRE(errcode == VSL_ERROR_OK);

  errcode = viRngUniformBits(VSL_RNG_METHOD_UNIFORMBITS_STD, newStream, n,
                             newBuffer.data());
  REQUIRE(errcode == VSL_ERROR_OK);

  REQUIRE(oldBuffer == newBuffer);

  errcode = vslDeleteStream(&oldStream);
  REQUIRE(errcode == VSL_ERROR_OK);

  errcode = vslDeleteStream(&newStream);
  REQUIRE(errcode == VSL_ERROR_OK);
}

TEST_CASE("Save/load stream state nullptrs", "[openrng_only]") {
  VSLStreamStatePtr stream;
  const int nullBufferSize = 4096;
  int errcode;

  const auto brngId = GENERATE(NEWSTREAM_SUPPORTED, NEWSTREAM_NONDETERM);
  CAPTURE(brngToString(brngId));

  errcode = vslNewStream(&stream, brngId, 0);
  ALLOW_NONDETERM_UNSUPPORTED_IF_NO_TRNG(brngId, errcode);
  REQUIRE(errcode == VSL_ERROR_OK);

  std::vector<char> stateBuffer(vslGetStreamSize(stream));
  errcode = vslSaveStreamM(stream, stateBuffer.data());
  REQUIRE(errcode == VSL_ERROR_OK);

  SECTION("Save buffer nullptr") {
    errcode = vslSaveStreamM(stream, nullptr);
    REQUIRE(errcode == VSL_ERROR_NULL_PTR);
  }

  SECTION("Save stream nullptr") {
    errcode = vslSaveStreamM(nullptr, stateBuffer.data());
    REQUIRE(errcode == VSL_ERROR_NULL_PTR);
  }

  SECTION("Load buffer nullptr") {
    errcode = vslLoadStreamM(&stream, nullptr);
    REQUIRE(errcode == VSL_ERROR_NULL_PTR);
  }

  // This isn't the documented behaviour, but it makes sense to error here
  // rather than SEGFAULT.
  SECTION("Load stream nullptr") {
    errcode = vslLoadStreamM(nullptr, stateBuffer.data());
    REQUIRE(errcode == VSL_ERROR_NULL_PTR);
  }

  errcode = vslDeleteStream(&stream);
  REQUIRE(errcode == VSL_ERROR_OK);
}

TEST_CASE("Save/load stream state null buffer") {
  VSLStreamStatePtr stream;
  int errcode;

  const auto brngId = GENERATE(NEWSTREAM_SUPPORTED, NEWSTREAM_NONDETERM);
  CAPTURE(brngToString(brngId));

  std::vector<char> nullBuffer(4096, 0);
  std::vector<char> stateBuffer(4096);

  SECTION("Save nullBuffer") {
    errcode = vslSaveStreamM(nullBuffer.data(), stateBuffer.data());
    REQUIRE(errcode == VSL_RNG_ERROR_BAD_STREAM);
  }

  SECTION("Load nullBuffer") {
    std::vector<char> nullBuffer(4096, 0);
    errcode = vslLoadStreamM(&stream, nullBuffer.data());

    //
    // At the time of writing, if building via git, the GIT_HASH check fails,
    // resulting in BAD_MEM_FORMAT. If building without git, the BRNG index
    // check fails, resulting in INVALID_BRNG_INDEX. What matters is an error
    // occurred, the exact value isn't that important.
    //
    std::vector<openrng_int_t> errors{
        {VSL_RNG_ERROR_BAD_MEM_FORMAT, VSL_RNG_ERROR_INVALID_BRNG_INDEX}
    };

    REQUIRE_THAT(errors, Catch::Matchers::Contains(errcode));
  }
}
