#!/usr/bin/env python3

import subprocess
import argparse
import sys
import json
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


arg_parser = argparse.ArgumentParser(description="Compile Rivanna3S code in the Docker container.")
arg_parser.add_argument("args", nargs="*", help="Arguments to pass to `cmake --build`.")
arg_parser.add_argument("-c", "--clean", action="store_true", help="Clean the build before compiling.")
arg_parser.add_argument("-s", "--silent", action="store_true", help="Suppress output from the compilation process.")
upload_mode = arg_parser.add_mutually_exclusive_group()
upload_mode.add_argument("--mock", action="store_true", help="Enable UART upload streaming mock on MotorBoard and TopDistBoard.")
upload_mode.add_argument("--can-update", action="store_true", help="Enable CAN firmware updates into the inactive flash bank (requires CAN FD-capable peers).")
arg_parser.add_argument("--install", action="store_true", help="Create the Docker container for the first time.")
arg_parser.add_argument("--nucleo", action="store_true", help="Build TopDistBoard for the NUCLEO-G474RE LED and ST-Link UART pins.")
args = arg_parser.parse_args()

testing_mode = "testing" in args.args
if args.nucleo and testing_mode:
    arg_parser.error("--nucleo selects G474RE and cannot be combined with testing (H743)")
args.args = [arg for arg in args.args if arg != "testing"]
mcu = TESTING_MCU if testing_mode else DEFAULT_MCU
mock_define = "-DUART_UPLOAD_MOCK=ON" if args.mock else "-DUART_UPLOAD_MOCK=OFF"
can_define = "-DCAN_FIRMWARE_UPDATE=ON" if args.can_update else "-DCAN_FIRMWARE_UPDATE=OFF"
nucleo_define = "-DNUCLEO_G474RE=ON" if args.nucleo else "-DNUCLEO_G474RE=OFF"
build_dir = "build"

if args.install:
    create_container()

process = subprocess.run("docker --help", shell=True, capture_output=True, text=True)
if process.returncode != 0:
    print("Docker is not running. Please start Docker and try again.")
    exit(1)

# A container bound to another checkout can otherwise build stale firmware.
inspection = subprocess.run(["docker", "inspect", CONTAINER], capture_output=True, text=True)
if inspection.returncode != 0:
    sys.exit(f"Create the build container first: python3 compile.py --install")
mounts = json.loads(inspection.stdout)[0]["Mounts"]
if not any(mount["Destination"] == CONTAINER_DIR and
           Path(mount["Source"]).resolve() == Path.cwd().resolve() for mount in mounts):
    sys.exit(f"{CONTAINER} is mounted to a different checkout. Run python3 compile.py --install in this checkout first.")

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
    f"-DCMAKE_TOOLCHAIN_FILE=cmake/arm-gcc.cmake -DCMAKE_BUILD_TYPE=Debug -DBOARD={mcu} {mock_define} {can_define} {nucleo_define}"
)
process = subprocess.run(f'docker exec -t {CONTAINER} /bin/bash -c "{command}"', shell=True, capture_output=args.silent, text=True)
if process.returncode != 0:
    print("CMake configuration failed.", file=sys.stderr)
    sys.exit(process.returncode)

command = f"cd {CONTAINER_DIR} && cmake --build {build_dir} {' '.join(args.args)}"
compile_process = subprocess.run(f'docker exec -t {CONTAINER} /bin/bash -c "{command}"', shell=True, capture_output=args.silent, text=True)
if args.silent and compile_process.returncode != 0:
    print("Compilation failed.")

process = subprocess.run(f"docker stop {CONTAINER}", shell=True, capture_output=True, text=True)
if process.returncode != 0:
    print(f"Failed to stop {CONTAINER}")
    exit(1)

exit(compile_process.returncode)
