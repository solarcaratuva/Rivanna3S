#!/usr/bin/env python3

import subprocess
import argparse
import os

CONTAINER = "Rivanna3S_compile"
HOST_DIR = "$(pwd)"
CONTAINER_DIR = "/root/code"
DEFAULT_MCU = "STM32G474RET6"
DEFAULT_BUILD_DIR = "build"
APP_TARGETS = {
    "bottomdist": "BottomDistBoard.elf",
    "motor": "MotorBoard.elf",
    "relay": "RelayBoard.elf",
    "telemetry": "TelemetryBoard.elf",
    "topdist": "TopDistBoard.elf",
}


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


arg_parser = argparse.ArgumentParser(description="Compile Rivanna3S code in the Docker container.")
arg_parser.add_argument("args", nargs="*", help="Arguments to pass to `cmake --build`.")
arg_parser.add_argument("-c", "--clean", action="store_true", help="Clean the build before compiling.")
arg_parser.add_argument("-s", "--silent", action="store_true", help="Suppress output from the compilation process.")
arg_parser.add_argument("--install", action="store_true", help="Create the Docker container for the first time.")
arg_parser.add_argument("--mcu", default=DEFAULT_MCU, help="MCU target (Drivers/<MCU>).")
arg_parser.add_argument("--testing", action="store_true", help="Build for STM32H743ZITX (testing).")
arg_parser.add_argument("--app", help="App target (bottomdist, motor, relay, telemetry, topdist, or full CMake target).")
arg_parser.add_argument("--list-apps", action="store_true", help="List available app targets and exit.")
arg_parser.add_argument("--build-dir", help="CMake build directory (default: build or build-<MCU> for non-default MCU).")
args = arg_parser.parse_args()

if args.testing:
    args.mcu = "STM32H743ZITX"

if args.list_apps:
    print("Available app targets:")
    for key in sorted(APP_TARGETS):
        print(f"  {key} -> {APP_TARGETS[key]}")
    raise SystemExit(0)

build_dir = args.build_dir
if not build_dir:
    if args.mcu == DEFAULT_MCU:
        build_dir = DEFAULT_BUILD_DIR
    else:
        build_dir = f"{DEFAULT_BUILD_DIR}-{args.mcu}"

app_target = None
if args.app:
    normalized = args.app.lower().replace("board", "")
    app_target = APP_TARGETS.get(normalized, args.app)

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
    f"-DCMAKE_TOOLCHAIN_FILE=cmake/arm-gcc.cmake -DCMAKE_BUILD_TYPE=Debug -DBOARD={args.mcu}"
)
process = subprocess.run(f'docker exec -t {CONTAINER} /bin/bash -c "{command}"', shell=True, capture_output=args.silent, text=True)
if args.silent and process.returncode != 0:
    print("CMake configuration failed.")

build_args = list(args.args)
if app_target:
    has_target = any(arg in ("-t", "--target") or arg.startswith("--target=") for arg in build_args)
    if not has_target:
        build_args.extend(["--target", app_target])

command = f"cd {CONTAINER_DIR} && cmake --build {build_dir} {' '.join(build_args)}"
compile_process = subprocess.run(f'docker exec -t {CONTAINER} /bin/bash -c "{command}"', shell=True, capture_output=args.silent, text=True)
if args.silent and compile_process.returncode != 0:
    print("Compilation failed.")

process = subprocess.run(f"docker stop {CONTAINER}", shell=True, capture_output=True, text=True)
if process.returncode != 0:
    print(f"Failed to stop {CONTAINER}")
    exit(1)

exit(compile_process.returncode)
