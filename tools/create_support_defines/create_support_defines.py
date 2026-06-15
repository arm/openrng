#!/usr/bin/env python3
#
# SPDX-FileCopyrightText: <text>Copyright 2024-2026 Arm Limited and/or its affiliates
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
import argparse

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
    features = data["features"]
    statuses = data["statuses"]
    generatorDataList = data["generators"]

    for genData in generatorDataList:
        for feature, status in genData["featureStatus"].items():
            if feature not in features:
                print(f'"{feature}" not recognised in {genData["method"]}')
                exit(1)
            if status not in statuses:
                print(f'"{status}" not recognised in {genData["method"]}')
                exit(1)


def transformData(data, incompatible_generators):
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
    features = data["features"]
    statuses = data["statuses"]
    generatorDataList = data["generators"]

    def is_compatible_generator(method):
        for exclude_gen in incompatible_generators:
            # If the method starts with a string contained in the
            # list of incompatible generators, we do not want to include
            # it in the list for a given #define.
            # We can't do an exact match because we want `MT2203`
            # to also exclude generators like `MT2203 + 3`
            if method.startswith(exclude_gen):
                return False
        return True

    return {
        feature: {
            status: [
                f'VSL_BRNG_{gen["method"]}'
                for gen in generatorDataList
                if gen["featureStatus"].get(feature) == status
                and is_compatible_generator(gen["method"])
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
    output = template.format(defines="\n".join(defines))
    print(output)


def main():

    parser = argparse.ArgumentParser()

    parser.add_argument(
        "--incompatible_gens",
        type=str,
        required=True,
        help="Comma-separated list of generators to exclude",
    )

    args = parser.parse_args()

    incompatible_generators = []
    if args.incompatible_gens.strip():
        incompatible_generators = args.incompatible_gens.split(";")

    data = json.loads(Path(f"{SCRIPTS_DIR}/generators.json").read_text())

    checkInput(data)
    featureSupport = transformData(data, incompatible_generators)
    outputDefines(featureSupport)


if __name__ == "__main__":
    main()
