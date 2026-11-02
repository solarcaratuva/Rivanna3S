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

# Keep location/column and source context; identical diagnostics are shown once.
DIAGNOSTIC = re.compile(r"^.+?:\s*(fatal error|error|warning|note):")
LINKER_ERROR = re.compile(r"undefined reference|multiple definition|cannot find |"
                          r"region .* overflowed|will not fit in region|ld(?:\.exe)?:.*(?:error|failed)|collect2: error:")
ANSI_ESCAPE = re.compile(r"\x1b\[[0-?]*[ -/]*[@-~]")


def process_compilation_output(output, color=False):
    sections = {"Linker errors": [], "Compiler errors": [], "Warnings": [], "Notes": []}
    current = None
    for line in ANSI_ESCAPE.sub("", output).splitlines():
        match = DIAGNOSTIC.match(line)
        if LINKER_ERROR.search(line):
            section = "Linker errors"
        elif match:
            severity = match.group(1)
            section = {"warning": "Warnings", "note": "Notes"}.get(severity, "Compiler errors")
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
        if not diagnostics:
            continue
        code = "31" if "errors" in title else "33" if "warnings" in title.lower() else "34"
        result.append(f"\033[1;{code}m{title}\033[0m" if color else title)
        result.extend(dict.fromkeys("\n".join(lines) for lines in diagnostics))
    return "\n".join(result)


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
            if raw or (not silent and re.match(r"^\[\s*\d+(?:/\d+|%)\]", line)):
                print(line, end="", flush=True)
        process.stdout.close()
        returncode = process.wait()

    if not raw:
        summary = process_compilation_output("".join(output), color=sys.stdout.isatty())
        # Do not hide a failure just because only warnings (or nothing) matched.
        if returncode and not any(title in summary for title in ("Compiler errors", "Linker errors")):
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
    output_mode.add_argument("-s", "--silent", action="store_true", help="Hide routine progress; still show diagnostics and the build result.")
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
        exit(1)

    process = subprocess.run(f"docker start {CONTAINER}", shell=True, capture_output=True, text=True)
    if process.returncode != 0:
        print(f"Failed to start {CONTAINER}")
        exit(1)

    if args.clean:
        command = f"rm -rf {CONTAINER_DIR}/{build_dir}/"
        process = subprocess.run(f'docker exec -t {CONTAINER} /bin/bash -c "{command}"', shell=True, capture_output=True, text=True)
        if process.returncode != 0:
            print("Failed to clean the build.")
            exit(1)

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
        exit(1)

    return compile_returncode


if __name__ == "__main__":
    sys.exit(main())
