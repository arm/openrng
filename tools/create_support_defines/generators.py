#!/usr/bin/env python3
#
# SPDX-FileCopyrightText: <text>Copyright 2024-2025 Arm Limited and/or its affiliates
# <open-source-office@arm.com></text>
#
# SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
#

import json

from dataclasses import dataclass
from pathlib import Path
from typing import Iterable

GEN_JSON = Path(__file__).parent / "generators.json"


@dataclass
class Generator:
    name: str
    to_benchmark: bool
    newStream: str = "unsupported"
    newStreamEx: str = "unsupported"
    leapfrog: str = "unsupported"
    skipAhead: str = "unsupported"
    skipAheadEx: str = "unsupported"
    uniformBits32: str = "unsupported"
    uniformBits64: str = "unsupported"


def _discover(gen_json: Path) -> Iterable[Generator]:
    """Build up a list of Generators from a generators.json file.

    Feature statuses are recorded as class attributes, defaulting to
    unsupported if not provided.
    """
    data = json.loads(gen_json.read_text())
    for gen in data["generators"]:
        to_benchmark = gen.get("to_benchmark", True)
        yield Generator(gen["method"].lower(), to_benchmark, **gen["featureStatus"])


generators = list(_discover(GEN_JSON))
