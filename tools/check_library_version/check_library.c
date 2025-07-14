/*
 * SPDX-FileCopyrightText: <text>Copyright 2025 Arm Limited and/or its
 * affiliates <open-source-office@arm.com></text>
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

#include <stdio.h>
#include "openrng.h"

int main() {
    int major;
    int minor;
    int patch;

    openRngGetVersion(&major, &minor, &patch);

    // Format the output string as YY.MM
    printf("%d.%02d", major, minor);
    if( patch != -1 ){
        printf(".%d", patch);
    }

    return 0;
}