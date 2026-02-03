#!/usr/bin/env python3
#
# SPDX-FileCopyrightText: <text>Copyright 2024-2026 Arm Limited and/or its affiliates
# <open-source-office@arm.com></text>
#
# SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
#

import os
import sys
from subprocess import run, PIPE
from pathlib import Path
import re

cc_header = """\
/\*
 \* SPDX-FileCopyrightText: <text>Copyright ([0-9, \-]+) Arm Limited and/or its
 \* affiliates <open-source-office@arm.com></text>
 \*
 \* SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 \*/
"""

py_header= """\
#!/usr/bin/env python3
#
# SPDX-FileCopyrightText: <text>Copyright ([0-9, \-]+) Arm Limited and/or its affiliates
# <open-source-office@arm.com></text>
#
# SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
#
"""

def contents_has_valid_licence(contents, prefix, expected_year):
    n_lines_header  = prefix.count('\n')
    header_re = re.compile(prefix, re.MULTILINE)
    match = header_re.match(''.join(contents[:n_lines_header]))
    return match and expected_year in match.group(1)


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
        cmd = f"git log -1 --format=%cd --date=short {filename}"
        output = run(cmd, shell=True, stdout = PIPE, check=True)
        # The date will be in format YYYY-MM-DD
        yyyy_change = output.stdout.decode().strip()[0:4]
        with open(filename, encoding="utf8") as f:
            contents = f.readlines()
        if not contents_has_valid_licence(contents, header, yyyy_change):
            invalid.append((filename, yyyy_change))

    if len(invalid) > 0:
        print(f"Files with invalid copyright header:")
        print(f"{os.linesep.join([str(f) for f in invalid])}")
        sys.exit(1)

if __name__ == "__main__":
    main()
    sys.exit(0)