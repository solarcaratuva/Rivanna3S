#!/usr/bin/env python3
# MUST INSTALL `cppcheck` TO RUN THIS SCRIPT

import subprocess
import sys


ARGS = [
    "--enable=all",
    "--quiet",
    "--max-ctu-depth=4",
    "--error-exitcode=1",
]

SOURCES = [
    "Core/Src/"
]

INCLUDES = ["-I " + s for s in [
    "Core/Inc/",
]]

SUPPRESSES = ["--suppress=" + s for s in [
    "missingInclude",
    "missingIncludeSystem",
    "unusedFunction",
    "staticFunction",
    "functionStatic",
    "variableScope",
]]


command = "cppcheck " + " ".join(ARGS + SUPPRESSES + INCLUDES + SOURCES)
process = subprocess.run(command, shell=True, check=False, text=True)
sys.exit(process.returncode)
