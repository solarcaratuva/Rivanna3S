#!/usr/bin/env python3
# MUST INSTALL `cppcheck` TO RUN THIS SCRIPT

import subprocess
import sys


ARGS = [
    "--enable=all",
    "--quiet",
    "--max-ctu-depth=4",
    "--error-exitcode=1",
    "--check-level=exhaustive",
]

SOURCES = [
    "BottomDistBoard/Src/",
    "MotorBoard/Src/",
    "RelayBoard/Src/",
    "TelemetryBoard/Src/",
    "TopDistBoard/Src/",
    "Common/Drivers/Src/",
    "Common/Libs/Src/"
]

INCLUDES = ["-I " + s for s in [
    "BottomDistBoard/Inc/",
    "MotorBoard/Inc/",
    "RelayBoard/Inc/",
    "TelemetryBoard/Inc/",
    "TopDistBoard/Inc/",
    "Common/Drivers/Inc/",
    "Common/Libs/Inc/"
]]

SUPPRESSES = ["--suppress=" + s for s in [
    "missingInclude",
    "missingIncludeSystem",
    "unusedFunction",
    "staticFunction",
    "functionStatic",
    "variableScope",
    "passedByValue",
]]


command = "cppcheck " + " ".join(ARGS + SUPPRESSES + INCLUDES + SOURCES)
process = subprocess.run(command, shell=True, check=False, text=True)
sys.exit(process.returncode)
