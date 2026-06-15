/*
 * SPDX-FileCopyrightText: <text>Copyright 2024-2026 Arm Limited and/or its
 * affiliates <open-source-office@arm.com></text>
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

#pragma once

#include <openrng.h>

#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>

namespace openrng::bench {
const std::unordered_map<std::string_view, int> defines = {
    {"MCG31",                     VSL_BRNG_MCG31                          },
    {"R250",                      VSL_BRNG_R250                           },
    {"MRG32K3A",                  VSL_BRNG_MRG32K3A                       },
    {"MCG59",                     VSL_BRNG_MCG59                          },
    {"WH",                        VSL_BRNG_WH                             },
    {"SOBOL",                     VSL_BRNG_SOBOL                          },
    {"NIEDERR",                   VSL_BRNG_NIEDERR                        },
    {"MT19937",                   VSL_BRNG_MT19937                        },
    {"MT2203",                    VSL_BRNG_MT2203                         },
    {"IABSTRACT",                 VSL_BRNG_IABSTRACT                      },
    {"DABSTRACT",                 VSL_BRNG_DABSTRACT                      },
    {"SABSTRACT",                 VSL_BRNG_SABSTRACT                      },
    {"SFMT19937",                 VSL_BRNG_SFMT19937                      },
    {"NONDETERM",                 VSL_BRNG_NONDETERM                      },
    {"ARS5",                      VSL_BRNG_ARS5                           },
    {"PHILOX4X32X10",             VSL_BRNG_PHILOX4X32X10                  },
    {"UNIFORMBITS_STD",           VSL_RNG_METHOD_UNIFORMBITS_STD          },
    {"UNIFORMBITS32_STD",         VSL_RNG_METHOD_UNIFORMBITS32_STD        },
    {"UNIFORMBITS64_STD",         VSL_RNG_METHOD_UNIFORMBITS64_STD        },
    {"UNIFORM_STD",               VSL_RNG_METHOD_UNIFORM_STD              },
    {"UNIFORM_STD_ACCURATE",      VSL_RNG_METHOD_UNIFORM_STD_ACCURATE     },
    {"GAMMA_GNORM",               VSL_RNG_METHOD_GAMMA_GNORM              },
    {"GAMMA_GNORM_ACCURATE",      VSL_RNG_METHOD_GAMMA_GNORM_ACCURATE     },
    {"GAUSSIAN_BOXMULLER",        VSL_RNG_METHOD_GAUSSIAN_BOXMULLER       },
    {"GAUSSIAN_BOXMULLER2",       VSL_RNG_METHOD_GAUSSIAN_BOXMULLER2      },
    {"GAUSSIAN_ICDF",             VSL_RNG_METHOD_GAUSSIAN_ICDF            },
    {"GAUSSIANMV_BOXMULLER",      VSL_RNG_METHOD_GAUSSIANMV_BOXMULLER     },
    {"GAUSSIANMV_BOXMULLER2",     VSL_RNG_METHOD_GAUSSIANMV_BOXMULLER2    },
    {"GAUSSIANMV_ICDF",           VSL_RNG_METHOD_GAUSSIANMV_ICDF          },
    {"EXPONENTIAL_ICDF",          VSL_RNG_METHOD_EXPONENTIAL_ICDF         },
    {"EXPONENTIAL_ICDF_ACCURATE", VSL_RNG_METHOD_EXPONENTIAL_ICDF_ACCURATE},
    {"LAPLACE_ICDF",              VSL_RNG_METHOD_LAPLACE_ICDF             },
    {"RAYLEIGH_ICDF",             VSL_RNG_METHOD_RAYLEIGH_ICDF            },
    {"RAYLEIGH_ICDF_ACCURATE",    VSL_RNG_METHOD_RAYLEIGH_ICDF_ACCURATE   },
    {"WEIBULL_ICDF",              VSL_RNG_METHOD_WEIBULL_ICDF             },
    {"WEIBULL_ICDF_ACCURATE",     VSL_RNG_METHOD_WEIBULL_ICDF_ACCURATE    },
    {"GUMBEL_ICDF",               VSL_RNG_METHOD_GUMBEL_ICDF              },
    {"BERNOULLI_ICDF",            VSL_RNG_METHOD_BERNOULLI_ICDF           },
    {"BINOMIAL_BTPE",             VSL_RNG_METHOD_BINOMIAL_BTPE            },
    {"GEOMETRIC_ICDF",            VSL_RNG_METHOD_GEOMETRIC_ICDF           },
    {"CAUCHY_ICDF",               VSL_RNG_METHOD_CAUCHY_ICDF              },
    {"POISSON_PTPE",              VSL_RNG_METHOD_POISSON_PTPE             },
    {"POISSON_POISNORM",          VSL_RNG_METHOD_POISSON_POISNORM         },
    {"POISSONV_POISNORM",         VSL_RNG_METHOD_POISSONV_POISNORM        },
    {"MULTINOMIAL_MULTPOISSON",   VSL_RNG_METHOD_MULTINOMIAL_MULTPOISSON  },
    {"CHISQUARE_CHI2GAMMA",       VSL_RNG_METHOD_CHISQUARE_CHI2GAMMA      },
};

const auto get_define(const std::string &string_define) {
  auto define = defines.find(string_define);
  if (define != defines.end()) {
    return define->second;
  }
  throw std::invalid_argument("Unknown define: " + string_define);
}
} // namespace openrng::bench
