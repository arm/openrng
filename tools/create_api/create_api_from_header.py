#!/usr/bin/env python3
#
# SPDX-FileCopyrightText: <text>Copyright 2024-2025 Arm Limited and/or its affiliates
# <open-source-office@arm.com></text>
#
# SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
#

from argparse import ArgumentParser
import json
from pathlib import Path
import re
from subprocess import run
import sys

def separate_arguments(arguments_str: str) -> list:
    arguments = []
    parameters = arguments_str.split(',')
    for parameter in parameters:
        parameter = parameter.strip()
        words = parameter.split(' ')

        ## Let's transform arrays passed as [] into pointers
        if words[-1].endswith('[]'):
            words[-1] = '*' + words[-1][:-2]

        if words[-1].startswith('*'):
            words[-1] = words[-1][1:]
            words[-2] = words[-2] + '*'

        arguments.append({'name':words[-1],'type':' '.join(words[:-1])})
    return arguments

def create_api_from_header(include_file: Path) -> list:

    # We know the function declaration in OpenRNG returns an int and
    # they are all of the form `int *`. Therefore this script does not
    # support functions that return anything apart from int.
    re_fct = '^int\s+([a-zA-Z0-9]+)\(([a-zA-Z\*\[\],\s]+)\);\n$'
    re_fct = re.compile(re_fct)

    re_multiline_fct = '^int\s+([a-zA-Z0-9]+)\(([a-zA-Z0-9*_\[\],\s]+),\n$'
    re_multiline_fct = re.compile(re_multiline_fct)

    otherlines_fct = '\s+([a-zA-Z0-9*_\[\],\s]+)(,|\);)\n'
    otherlines_fct = re.compile(otherlines_fct)

    lines = []
    with open(include_file, "r") as f:
        lines = f.readlines()

    functions = []

    l_c = 0
    while l_c < len(lines) :
        one_line_fct = re_fct.match(lines[l_c])
        more_than_one_line = re_multiline_fct.match(lines[l_c])
        if one_line_fct:
            details = {}
            details['returnType' ] = 'int'
            details['name'] = one_line_fct.group(1)

            arguments = []
            if one_line_fct.group(2) != "void":
                arguments = separate_arguments(one_line_fct.group(2))

            details['parameters'] = arguments
            functions.append(details)
        if more_than_one_line:
            details = {}
            details['returnType' ] = 'int'
            details['name'] = more_than_one_line.group(1)

            assert more_than_one_line.group(2) != "void", f"{more_than_one_line.group(2)}"
            arguments = separate_arguments(more_than_one_line.group(2))

            next_line = l_c + 1
            done_at_least_once = False
            finished_def = False
            while( finished_def == False and otherlines_fct.match(lines[next_line])):
                matched = otherlines_fct.match(lines[next_line])
                extra_args =separate_arguments(matched.group(1))
                arguments.extend(extra_args)
                if matched.group(2) == ');':
                    finished_def=True
                done_at_least_once = True
                next_line += 1

            l_c = next_line - 1
            assert done_at_least_once, f"{lines[next_line]}"

            details['parameters'] = arguments
            functions.append(details)
        l_c +=1

    return  functions

def print_differences(ref_fcts, gen_fcts):
    def rigenerate_declaration(d: dict)->str:
        pars = ', '.join([f"{arg['type']} {arg['name']}" for arg in d['parameters']])
        return f"{d['returnType']} {d['name']}({pars});"


    s_rf = set(map(rigenerate_declaration,ref_fcts))
    s_gf = set(map(rigenerate_declaration,gen_fcts))

    print("Functions in static json but not generated from header")
    print( s_rf.difference(s_gf) )

    print("Functions from header but not in static json")
    print( s_gf.difference(s_rf) )

if __name__ == "__main__":
    parser = ArgumentParser()

    parser.add_argument("includefile", type=Path, help='Path to include file for openrng')
    parser.add_argument("--output", type=Path)

    args = parser.parse_args()

    input_file: Path = args.includefile.resolve()

    fcts = create_api_from_header(input_file)

    # Check against the static openrng.h.json file
    if args.output:
        output_file = output / f'{input_file.name}.json'
        output_file.write_text(json.dumps(fcts, indent=2))
    else:
        ref_path = Path('./openrng.h.json')
        ref_fcts = json.loads(ref_path.read_text())

        if fcts != ref_fcts:
            print_differences(ref_fcts, fcts)
            sys.exit(1)

    sys.exit(0)