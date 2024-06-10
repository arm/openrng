#!/usr/bin/env python3
#
# SPDX-FileCopyrightText: <text>Copyright 2024 Arm Limited and/or its affiliates
# <open-source-office@arm.com></text>
#
# SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
#

import os
import sys
from subprocess import run, PIPE
from pathlib import Path

cc_header = """\
/*
 * SPDX-FileCopyrightText: <text>Copyright 2024 Arm Limited and/or its
 * affiliates <open-source-office@arm.com></text>
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */
"""

py_header= """\
#!/usr/bin/env python3
#
# SPDX-FileCopyrightText: <text>Copyright 2024 Arm Limited and/or its affiliates
# <open-source-office@arm.com></text>
#
# SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
#
"""

def contents_has_valid_licence(contents, prefix):
    n_lines_header = prefix.count('\n')
    return ''.join(contents[:n_lines_header]) == prefix


def get_files_with_extension(extension):
    cmd = "git ls-tree -r @ --name-only"
    ret = run(cmd, shell=True, stdout = PIPE, check=True)
    all_files = [Path(file) for file in ret.stdout.decode().strip().splitlines()]
    return filter(lambda file: file.suffix in extension, all_files)

def main():

    CODE_FILES = ('.c', '.cpp', '.h', '.hpp', '.py')
    invalid = []
    for filename in get_files_with_extension(CODE_FILES):
        header = py_header if filename.suffix == ".py" else cc_header
        with open(filename, encoding="utf8") as f:
            contents = f.readlines()
        if not contents_has_valid_licence(contents, header):
            invalid.append(filename)

    if len(invalid) > 0:
        print(f"Files with invalid copyright header:")
        print(f"{os.linesep.join([str(f) for f in invalid])}")
        sys.exit(1)

if __name__ == "__main__":
    main()
    sys.exit(0)