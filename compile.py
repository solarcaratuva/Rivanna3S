#!/usr/bin/env python3

import subprocess
import argparse
import re
import sys
import shlex
from pathlib import Path

CONTAINER = "Rivanna3S_compile"
HOST_DIR = "$(pwd)"
CONTAINER_DIR = "/root/code"
DEFAULT_MCU = "STM32G474RET6"
TESTING_MCU = "STM32H743ZITX"
HIL_MCU = "STM32U5A9NJH6Q" # drivers not done for this yet

def container_exists(name):
    result = subprocess.run(f"docker ps -a --format '{{{{.Names}}}}'", shell=True, capture_output=True, text=True)
    return name in result.stdout.splitlines()

def create_container():
    if container_exists(CONTAINER):
        print(f"Container {CONTAINER} already exists. Deleting it...")
        subprocess.run(f"docker rm -f {CONTAINER}", shell=True, check=True)
    print(f"Creating container {CONTAINER}...")
    subprocess.run(
        f'docker create --name {CONTAINER} --init -it -v "{HOST_DIR}/:{CONTAINER_DIR}:Z" --platform linux/amd64 ghcr.io/solarcaratuva/rivanna3s_compile_env',
        shell=True,
        check=True
    )
    print("Container created successfully.")

# Match explicit severity labels after a file location or tool prefix.
DIAGNOSTIC = re.compile(r"^.+?:\s*(fatal error|error|warning|note):")
# GNU ld symbol/library failures and memory overflow messages; collect2 reports linker failure.
LINKER_ERROR = re.compile(r"undefined reference|multiple definition|cannot find |"
                          r"region .* overflowed|will not fit in region|ld(?:\.exe)?:.*(?:error|failed)|collect2: error:")
# CMake build progress from Ninja ([12/340]) or Make ([ 50%]).
PROGRESS_LINE = re.compile(r"^\[\s*\d+(?:/\d+|%)\]")
# ANSI colors: red errors, yellow warnings, blue notes.
SECTION_COLORS = {"Linker errors": "31", "Compiler errors": "31", "Warnings": "33", "Notes": "34"}
ANSI_ESCAPE = re.compile(r"\x1b\[[0-?]*[ -/]*[@-~]")


def process_compilation_output(output, color=False, errors_only=False):
    sections = {title: [] for title in SECTION_COLORS}
    current = None
    for line in ANSI_ESCAPE.sub("", output).splitlines():
        match = DIAGNOSTIC.match(line)
        # Explicit warnings/notes take precedence over keywords in their messages.
        if match and match.group(1) in ("warning", "note"):
            section = {"warning": "Warnings", "note": "Notes"}[match.group(1)]
        elif LINKER_ERROR.search(line):
            section = "Linker errors"
        elif match:
            section = "Compiler errors"
        else:
            # GCC prints the source line and caret beneath a diagnostic.
            if current is not None and re.match(r"^\s*(?:\d+)?\s*\|", line):
                current.append(line)
            else:
                current = None
            continue
        current = [line]
        sections[section].append(current)

    result = []
    for title, diagnostics in sections.items():
        if not diagnostics or (errors_only and title in ("Warnings", "Notes")):
            continue
        code = SECTION_COLORS[title]
        result.append(f"\033[1;{code}m{title}\033[0m" if color else title)
        result.extend(dict.fromkeys("\n".join(lines) for lines in diagnostics))
    has_errors = bool(sections["Compiler errors"] or sections["Linker errors"])
    return "\n".join(result), has_errors


def run_compilation(command, silent=False, raw=False):
    log_path = Path("build/compile.log")
    log_path.parent.mkdir(exist_ok=True)
    output = []
    with log_path.open("w") as log_file:
        process = subprocess.Popen(command, stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                                   text=True, errors="replace", bufsize=1)
        for line in process.stdout:
            log_file.write(line)
            output.append(line)
            if raw or (not silent and PROGRESS_LINE.match(line)):
                print(line, end="", flush=True)
        process.stdout.close()
        returncode = process.wait()

    if not raw:
        summary, has_errors = process_compilation_output("".join(output), color=sys.stdout.isatty(), errors_only=silent)
        # Do not hide a failure just because only warnings (or nothing) matched.
        if returncode and not has_errors:
            print("".join(output), end="")
        elif summary:
            print(summary)
    print("Compilation failed." if returncode else "Compilation succeeded.")
    print(f"Full output: {log_path}")
    return returncode


def main():
    arg_parser = argparse.ArgumentParser(description="Compile Rivanna3S code in the Docker container.")
    arg_parser.add_argument("args", nargs="*", help="Arguments to pass to `cmake --build`.")
    arg_parser.add_argument("-c", "--clean", action="store_true", help="Clean the build before compiling.")
    output_mode = arg_parser.add_mutually_exclusive_group()
    output_mode.add_argument("-s", "--silent", action="store_true", help="Show only errors and the build result; keep full output in build/compile.log.")
    output_mode.add_argument("--raw", action="store_true", help="Show unfiltered build output live instead of a diagnostic summary.")
    arg_parser.add_argument("--install", action="store_true", help="Create the Docker container for the first time.")
    args = arg_parser.parse_args()

    testing_mode = "testing" in args.args
    args.args = [arg for arg in args.args if arg != "testing"]
    mcu = TESTING_MCU if testing_mode else DEFAULT_MCU
    build_dir = "build"

    if args.install:
        create_container()

    process = subprocess.run("docker --help", shell=True, capture_output=True, text=True)
    if process.returncode != 0:
        print("Docker is not running. Please start Docker and try again.")
        return process.returncode

    process = subprocess.run(f"docker start {CONTAINER}", shell=True, capture_output=True, text=True)
    if process.returncode != 0:
        print(f"Failed to start {CONTAINER}")
        return process.returncode

    if args.clean:
        command = f"rm -rf {CONTAINER_DIR}/{build_dir}/"
        process = subprocess.run(f'docker exec -t {CONTAINER} /bin/bash -c "{command}"', shell=True, capture_output=True, text=True)
        if process.returncode != 0:
            print("Failed to clean the build.")
            subprocess.run(["docker", "stop", CONTAINER], capture_output=True)
            return process.returncode

    command = (
        f"cd {CONTAINER_DIR} && cmake -S . -B {build_dir} -G Ninja "
        f"-DCMAKE_TOOLCHAIN_FILE=cmake/arm-gcc.cmake -DCMAKE_BUILD_TYPE=Debug -DBOARD={mcu}"
    )
    process = subprocess.run(f'docker exec -t {CONTAINER} /bin/bash -c "{command}"', shell=True, capture_output=True, text=True)
    if not args.silent or process.returncode != 0:
        print(process.stdout + process.stderr, end="")
    if process.returncode != 0:
        print("CMake configuration failed.")
        subprocess.run(["docker", "stop", CONTAINER], capture_output=True)
        return process.returncode

    command = f"cd {CONTAINER_DIR} && cmake --build {build_dir} {shlex.join(args.args)}"
    compile_returncode = run_compilation(
        ["docker", "exec", CONTAINER, "/bin/bash", "-c", command],
        silent=args.silent, raw=args.raw)

    process = subprocess.run(f"docker stop {CONTAINER}", shell=True, capture_output=True, text=True)
    if process.returncode != 0:
        print(f"Failed to stop {CONTAINER}")
        return process.returncode

    return compile_returncode


if __name__ == "__main__":
    sys.exit(main())
