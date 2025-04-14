#!/usr/bin/env python3
#
# SPDX-FileCopyrightText: <text>Copyright 2024-2025 Arm Limited and/or its affiliates
# <open-source-office@arm.com></text>
#
# SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
#


import argparse
import json
import re
import textwrap

from pathlib import Path

import benchmarks


def generate_cpp(bench):
    # We read in 5 common parameters (argv[0], brng, method, niters, nelems)
    # plus any extra parameters that the distribution expects
    common_params = 5
    num_params = len(bench.params) + common_params

    # Parsing of any extra arguments that the distribution expects
    # NOTE: we currently read these in as doubles, this _may_ change
    param_args = []
    for i, p in enumerate(bench.params):
        ptype = bench.params_type.get(p, "const auto")
        pval = f"std::stod(argv[{i + common_params}])"

        if re.match("^\w+\[.+\]$", p):
            pname, psize = p[:-1].split("[")
            param_args.append(f"std::vector<{ptype}> {pname}_vec({psize}, {pval});")
            param_args.append(f"{ptype} *{pname} = {pname}_vec.data();")
        else:
            param_args.append(f"{ptype} {p} = {pval};")

    parse_args = ("\n" + " " * 16).join(param_args)

    if bench.params:
        help_args = " " + " ".join(p.split("[")[0] for p in bench.params)
        func_args = ", " + ", ".join(p.split("[")[0] for p in bench.params)
    else:
        help_args, func_args = "", ""

    return textwrap.dedent(
        f"""
            #include "bench.hpp"
            #include "defines.hpp"

            #include <openrng.h>

            #include <cstdlib>
            #include <iostream>
            #include <string>
            #include <vector>

            int main(int argc, char* argv[]) {{
                if (argc != {num_params}) {{
                    std::cerr << "usage: " << argv[0] << " brng method niters nelems{help_args}" << std::endl;
                    return EXIT_FAILURE;
                }}
                typedef {bench.dtype or "double"} dtype;
                const auto brng   = openrng::bench::get_define(argv[1]);
                const auto method = openrng::bench::get_define(argv[2]);
                const auto niters = std::stoll(argv[3]);
                const auto nelems = std::stoi(argv[4]);
                {parse_args}
                const auto result = openrng::bench::distribution({bench.func}, niters, brng, method, nelems, {bench.elem_size}{func_args});
                std::cout << result << std::endl;
                return EXIT_SUCCESS;
            }}
        """
    ).lstrip()


def main():
    parser = argparse.ArgumentParser(
        description="Generate a set of C++ benchmarks "
        "(one per-distribution) & save them to a directory."
    )
    parser.add_argument("output_dir", help="output directory for generated C++ files")
    args = parser.parse_args()

    output_dir = Path(args.output_dir)
    for bench in benchmarks.dist_benchmarks:
        output_path = output_dir / (bench.fname + ".cpp")
        output_path.write_text(generate_cpp(bench))


if __name__ == "__main__":
    main()
