#!/usr/bin/env python3

import subprocess
import argparse

CONTAINER = "Rivanna3S_compile"
HOST_DIR = "$(pwd)"
CONTAINER_DIR = "/root/code"


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
args = arg_parser.parse_args()

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
    command = f"rm -rf {CONTAINER_DIR}/build/"
    process = subprocess.run(f'docker exec -t {CONTAINER} /bin/bash -c "{command}"', shell=True, capture_output=True, text=True)
    if process.returncode != 0:
        print("Failed to clean the build.")
        exit(1)

command = f"cd {CONTAINER_DIR} && cmake -S . -B build -G Ninja -DCMAKE_TOOLCHAIN_FILE=toolchains/arm-gcc.cmake -DCMAKE_BUILD_TYPE=Debug"
process = subprocess.run(f'docker exec -t {CONTAINER} /bin/bash -c "{command}"', shell=True, capture_output=args.silent, text=True)
if args.silent and process.returncode != 0:
    print("CMake configuration failed.")

command = f"cd {CONTAINER_DIR} && cmake --build build {' '.join(args.args)}"
compile_process = subprocess.run(f'docker exec -t {CONTAINER} /bin/bash -c "{command}"', shell=True, capture_output=args.silent, text=True)
if args.silent and compile_process.returncode != 0:
    print("Compilation failed.")

process = subprocess.run(f"docker stop {CONTAINER}", shell=True, capture_output=True, text=True)
if process.returncode != 0:
    print(f"Failed to stop {CONTAINER}")
    exit(1)

exit(compile_process.returncode)
