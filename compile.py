#!/usr/bin/env python3

import subprocess
import argparse
import re
import shutil
import sys

CONTAINER = "Rivanna3S_compile"
HOST_DIR = "$(pwd)"
CONTAINER_DIR = "/root/code"
DEFAULT_MCU = "STM32G474RET6"
TESTING_MCU = "STM32H743ZITX"
HIL_MCU = "STM32U5A9NJH6Q" # drivers not done for this yet

# ---------------------------------------------------------------- output filtering

ANSI_RE = re.compile(r"\x1b\[[0-9;?]*[A-Za-z]")
NINJA_PROGRESS_RE = re.compile(r"^\[\d+/\d+\]")
ERROR_RE = re.compile(r"^(?P<loc>[^\s:][^:]*:\d+(?::\d+)?): (?:fatal )?error: (?P<msg>.*)$")
WARNING_RE = re.compile(r"^(?P<loc>[^\s:][^:]*:\d+(?::\d+)?): warning: (?P<msg>.*)$")
NOTE_RE = re.compile(r"^(?P<loc>[^\s:][^:]*:\d+(?::\d+)?): note: (?P<msg>.*)$")
LINKER_RE = re.compile(
    r"undefined reference to|multiple definition of|cannot find -l|"
    r"region `.*' overflowed|will not fit in region|collect2: error|"
    r"ld returned \d+ exit status|arm-none-eabi-ld:"
)

USE_COLOR = sys.stdout.isatty()

def paint(text, code):
    return f"\033[{code}m{text}\033[0m" if USE_COLOR else text

RED, YELLOW, CYAN, DIM, BOLD = "31", "33", "36", "2", "1"


class Entry:
    def __init__(self, text):
        self.text = text
        self.count = 1
        self.notes = []

    def add_note(self, note):
        if note not in self.notes:
            self.notes.append(note)


def parse_output(lines):
    """Returns (linker_errors, errors, warnings, orphan_notes); each of the first
    three is an insertion-ordered dict of text -> Entry (deduplicated)."""
    linker, errors, warnings = {}, {}, {}
    orphan_notes = set()
    last = None

    def record(bucket, text):
        nonlocal last
        if text in bucket:
            bucket[text].count += 1
        else:
            bucket[text] = Entry(text)
        last = bucket[text]

    for line in lines:
        line = line.strip()
        if ERROR_RE.match(line):
            record(errors, line)
        elif WARNING_RE.match(line):
            record(warnings, line)
        elif LINKER_RE.search(line):
            record(linker, line)
        elif NOTE_RE.match(line):
            if last is not None:
                last.add_note(line)
            else:
                orphan_notes.add(line)
    return linker, errors, warnings, orphan_notes


def print_section(title, bucket, color):
    if not bucket:
        return
    print(paint(f"\n=== {title} ({len(bucket)}) ===", f"{BOLD};{color}"))
    for entry in bucket.values():
        suffix = f" (x{entry.count})" if entry.count > 1 else ""
        print(paint(entry.text + suffix, color))
        for note in entry.notes:
            print(paint(f"    {note}", DIM))


def process_compilation_output(lines, returncode):
    """Print a filtered summary. Returns True if anything useful was found."""
    linker, errors, warnings, orphan_notes = parse_output(lines)

    print_section("LINKER ERRORS", linker, RED)
    print_section("COMPILE ERRORS", errors, RED)
    print_section("COMPILE WARNINGS", warnings, YELLOW)

    found_errors = bool(linker or errors)
    if returncode != 0 and not found_errors:
        # Filter missed whatever went wrong, so show everything.
        print(paint("\nBuild failed but no errors were recognized; raw output follows:\n", RED))
        print("\n".join(lines))
        return False

    n_notes = sum(len(e.notes) for b in (linker, errors, warnings) for e in b.values()) + len(orphan_notes)
    print(paint(
        f"\n{len(linker)} linker error(s), {len(errors)} error(s), "
        f"{len(warnings)} warning(s), {n_notes} note(s) "
        f"(identical diagnostics collapsed; use --verbose for raw output)",
        CYAN,
    ))
    if orphan_notes:
        print(paint("Notes not attached to a diagnostic:", DIM))
        for note in sorted(orphan_notes):
            print(paint(f"    {note}", DIM))
    return True


# ---------------------------------------------------------------- running commands

def run_in_container(command, mode):
    """Run a command in the container, streaming output.
    mode: 'verbose' (print raw), 'normal' (progress line only), 'silent' (nothing).
    Returns (returncode, list_of_clean_lines)."""
    proc = subprocess.Popen(
        ["docker", "exec", "-t", CONTAINER, "/bin/bash", "-c", command],
        stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True, errors="replace",
    )
    lines = []
    show_progress = mode == "normal" and sys.stdout.isatty()
    for raw in proc.stdout:
        line = ANSI_RE.sub("", raw).rstrip("\r\n")
        lines.append(line)
        if mode == "verbose":
            print(line)
        elif show_progress and NINJA_PROGRESS_RE.match(line):
            width = shutil.get_terminal_size().columns - 1
            print("\r\033[K" + line[:width], end="", flush=True)
    proc.wait()
    if show_progress:
        print("\r\033[K", end="", flush=True)  # clear progress line
    return proc.returncode, lines


def report(step, returncode, lines, mode):
    if mode == "verbose":
        if returncode != 0:
            print(f"{step} failed.")
        return
    process_compilation_output(lines, returncode)
    if returncode != 0:
        print(paint(f"{step} failed.", f"{BOLD};{RED}"))


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


# ---------------------------------------------------------------- main

arg_parser = argparse.ArgumentParser(description="Compile Rivanna3S code in the Docker container.")
arg_parser.add_argument("args", nargs="*", help="Arguments to pass to `cmake --build`.")
arg_parser.add_argument("-c", "--clean", action="store_true", help="Clean the build before compiling.")
arg_parser.add_argument("-s", "--silent", action="store_true", help="No live progress; only print the filtered error/warning summary.")
arg_parser.add_argument("-v", "--verbose", action="store_true", help="Stream the raw compiler output with no filtering.")
arg_parser.add_argument("--install", action="store_true", help="Create the Docker container for the first time.")
args = arg_parser.parse_args()

mode = "verbose" if args.verbose else "silent" if args.silent else "normal"

testing_mode = "testing" in args.args
args.args = [arg for arg in args.args if arg != "testing"]
mcu = TESTING_MCU if testing_mode else DEFAULT_MCU
build_dir = "build"

if args.install:
    create_container()

process = subprocess.run("docker --help", shell=True, capture_output=True, text=True)
if process.returncode != 0:
    print("Docker is not running. Please start Docker and try again.")
    sys.exit(1)

process = subprocess.run(f"docker start {CONTAINER}", shell=True, capture_output=True, text=True)
if process.returncode != 0:
    print(f"Failed to start {CONTAINER}")
    sys.exit(1)

exit_code = 0
try:
    if args.clean:
        rc, _ = run_in_container(f"rm -rf {CONTAINER_DIR}/{build_dir}/", "silent")
        if rc != 0:
            print("Failed to clean the build.")
            sys.exit(1)

    configure_cmd = (
        f"cd {CONTAINER_DIR} && cmake -S . -B {build_dir} -G Ninja "
        f"-DCMAKE_TOOLCHAIN_FILE=cmake/arm-gcc.cmake -DCMAKE_BUILD_TYPE=Debug -DBOARD={mcu}"
    )
    rc, lines = run_in_container(configure_cmd, mode)
    if rc != 0:
        # CMake output isn't compiler output; always show it raw on failure.
        if mode != "verbose":
            print("\n".join(lines))
        print("CMake configuration failed.")
        sys.exit(rc)

    build_cmd = f"cd {CONTAINER_DIR} && cmake --build {build_dir} {' '.join(args.args)}"
    exit_code, lines = run_in_container(build_cmd, mode)
    report("Compilation", exit_code, lines, mode)
finally:
    process = subprocess.run(f"docker stop {CONTAINER}", shell=True, capture_output=True, text=True)
    if process.returncode != 0:
        print(f"Failed to stop {CONTAINER}")

sys.exit(exit_code)