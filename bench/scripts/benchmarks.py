#!/usr/bin/env python3
#
# SPDX-FileCopyrightText: <text>Copyright 2024 Arm Limited and/or its affiliates
# <open-source-office@arm.com></text>
#
# SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
#

import json

from pathlib import Path
from typing import Iterable, List, Optional

SCRIPTS_DIR = Path(__file__).parent
DIST_JSON = SCRIPTS_DIR / "distributions.json"


class DistBenchmark:
    def __init__(
        self,
        name: str,
        dtype: Optional[str],
        params: List[str],
        params_type: List[str] = [],
        methods: List[str] = [],
        defaults: list = [],
        elem_size: str = "1",
    ):
        prefix = {"float": "vs", "double": "vd"}.get(dtype or "", "vi")

        self.name = name
        self.func = f"{prefix}Rng{name}"
        self.dtype = dtype
        self.params = params
        self.params_type = dict(zip(params, params_type))
        self.methods = methods
        self.defaults = dict(zip(params, defaults))
        self.elem_size = elem_size

    @property
    def fname(self) -> str:
        suffix = f"_{self.dtype}" if self.dtype else ""
        return self.name.lower() + suffix


def _discover_benchmarks(dist_json: Path) -> Iterable[DistBenchmark]:
    dists = json.loads(dist_json.read_text())
    for dist_name, dist in dists.items():
        params_type = dist.get("params_type", [])
        methods = dist.get("supported_methods", [])
        defaults = dist.get("default_args", [])
        elem_size = dist.get("elem_size", "1")
        if dist["type"] == "continuous":
            yield DistBenchmark(dist_name, "float", dist["params"], params_type, methods, defaults, elem_size)
            yield DistBenchmark(dist_name, "double", dist["params"], params_type, methods, defaults, elem_size)
        else:
            yield DistBenchmark(dist_name, None, dist["params"], params_type, methods, defaults, elem_size)


dist_benchmarks = list(_discover_benchmarks(DIST_JSON))


def find(dist: str, dtype: Optional[str], method: str) -> DistBenchmark:
    for bench in dist_benchmarks:
        if bench.name.lower() == dist.lower():
            if bench.dtype:
                if not dtype:
                    raise RuntimeError(
                        "Define a dtype (float, double) for continuous distributions!"
                    )
                if bench.dtype.lower() != dtype.lower():
                    continue
            if method not in bench.methods:
                raise RuntimeError(
                    f"Distribution {bench.name} does not support method {method}!"
                )
            return bench
    raise RuntimeError("Could not find matching benchmark!")
