#!/usr/bin/env python3
#
# SPDX-FileCopyrightText: <text>Copyright 2024 Arm Limited and/or its affiliates
# <open-source-office@arm.com></text>
#
# SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
#


import argparse
import json
from pathlib import Path


# If running via a REPL, you will need to define __file__.
script_dir = Path(__file__).parent.resolve()


source_template = """\
// This file was created by {script_name}

#include <openrng.h>

int main() {{

    {methods}

    return 0;
}}
"""


def to_source(data):
    methods = '\n    '.join(
        f'{d["method"]}({", ".join("0" for _ in d["parameters"])});'
        for d in data
    )
    return source_template.format(script_name=__file__, methods=methods)


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('output', type=Path)
    args = parser.parse_args()

    data = json.loads((script_dir / 'api.json').read_text())

    output: Path = args.output
    output.write_text(to_source(data))
