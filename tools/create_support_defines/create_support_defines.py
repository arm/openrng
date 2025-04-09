#!/usr/bin/env python3
#
# SPDX-FileCopyrightText: <text>Copyright 2024-2025 Arm Limited and/or its affiliates
# <open-source-office@arm.com></text>
#
# SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
#


"""
Reads generator.json and prints a C header file to stdout. The header file
defines a list of generators for each feature-status pair. Example output might
be

  #pragma once
  #define NEWSTREAM_SUPPORTED VSL_BRNG_MCG31, VSL_BRNG_MRG32K3A, VSL_BRNG_R250, VSL_BRNG_MT19937
  #define NEWSTREAM_NOTIMPLEMENTED VSL_BRNG_MCG59, VSL_BRNG_WH, VSL_BRNG_SOBOL, VSL_ETC...
  #define LEAPFROG_SUPPORTED VSL_BRNG_MCG31
  #define LEAPFROG_UNSUPPORTED VSL_BRNG_MRG32K3A, VSL_BRNG_R250
  #define LEAPFROG_NOTIMPLEMENTED VSL_BRNG_MT19937
  #define SKIPAHEAD_SUPPORTED VSL_BRNG_MCG31, VSL_BRNG_MRG32K3A
  #define SKIPAHEAD_UNSUPPORTED VSL_BRNG_R250
  #define SKIPAHEAD_NOTIMPLEMENTED VSL_BRNG_MT19937
  #define SKIPAHEADEX_SUPPORTED VSL_BRNG_MRG32K3A
  #define SKIPAHEADEX_UNSUPPORTED VSL_BRNG_MCG31, VSL_BRNG_R250
  #define SKIPAHEADEX_NOTIMPLEMENTED VSL_BRNG_MT19937
  #define UNIFORMBITS32_SUPPORTED VSL_BRNG_MT19937
  #define UNIFORMBITS32_UNSUPPORTED VSL_BRNG_MCG31, VSL_BRNG_MRG32K3A, VSL_BRNG_R250
  #define UNIFORMBITS64_SUPPORTED VSL_BRNG_MT19937
  #define UNIFORMBITS64_UNSUPPORTED VSL_BRNG_MCG31, VSL_BRNG_MRG32K3A, VSL_BRNG_R250
"""

import json
from pathlib import Path
import platform

SCRIPTS_DIR = Path(__file__).resolve().parent

template = """\
#pragma once
{defines}
"""


def checkInput(data):
    """
    Check that there are no unlisted features or statuses being used to describe
    a generator.
    """
    features = data['features']
    statuses = data['statuses']
    generatorDataList = data['generators']

    for genData in generatorDataList:
        for feature, status in genData["featureStatus"].items():
            if feature not in features:
                print(f'"{feature}" not recognised in {genData["method"]}')
                exit(1)
            if status not in statuses:
                print(f'"{status}" not recognised in {genData["method"]}')
                exit(1)


def transformData(data):
    """
    Transform input into a more useable mapping of the form

        {
            "leapfrog": {
                "supported": [ "VSL_BRNG_MCG31", "VSL_BRNG_MCG59" ],
                "unsupported": [ "VSL_BRNG_R250" ],
                "notImplemented": []
            },
            "skipAhead": {
                ...
            },
            ...
        }

    """
    features = data['features']
    statuses = data['statuses']
    generatorDataList = data['generators']

    # Because the MT2203 does not satisfy bitwise reproducibility with any
    # other VSL library and we rely on its separate statistical testing,
    # unless on aarch64, MT2203 is not tested against refng.
    include_mt2203 = platform.machine() == "aarch64"

    return {
        feature: {
            status: [
                f'VSL_BRNG_{gen["method"]}'
                for gen in generatorDataList
                if gen["featureStatus"].get(feature) == status and
                (include_mt2203 or not gen["method"].startswith("MT2203"))
            ]
            for status in statuses
        }
        for feature in features
    }


def outputDefines(featureSupport):
    defines = []
    for feature, statuses in featureSupport.items():
        for status in statuses:
            if not statuses[status]:
                continue
            defines.append(
                f"#define {feature.upper()}_{status.upper()} {', '.join(statuses[status])}"
            )
    output = template.format(defines='\n'.join(defines))
    print(output)


def main():
    data = json.loads(Path(f'{SCRIPTS_DIR}/generators.json').read_text())

    checkInput(data)
    featureSupport = transformData(data)
    outputDefines(featureSupport)


if __name__ == "__main__":
    main()
