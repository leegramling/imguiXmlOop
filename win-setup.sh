#!/usr/bin/env bash

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
THIRDPARTY_DIR="${ROOT_DIR}/thirdparty"
BUILD_DIR="${ROOT_DIR}/build-windows"

SDL_VERSION="${SDL_VERSION:-2.30.5}"
SDL_PACKAGE="SDL2-devel-${SDL_VERSION}-VC.zip"
SDL_URL="https://github.com/libsdl-org/SDL/releases/download/release-${SDL_VERSION}/${SDL_PACKAGE}"

WIN_ARCH_RAW="${WIN_ARCH:-x64}"
case "${WIN_ARCH_RAW,,}" in
    x86|win32)
        SDL_LIB_SUBDIR="x86"
        CMAKE_ARCH="Win32"
        ;;
    x64|win64|amd64)
        SDL_LIB_SUBDIR="x64"
        CMAKE_ARCH="x64"
        ;;
    *)
        echo "Unsupported WIN_ARCH '${WIN_ARCH_RAW}'. Use x64 or x86." >&2
        exit 1
        ;;
esac

GENERATOR="${GENERATOR:-Visual Studio 17 2022}"
BUILD_CONFIG="${BUILD_CONFIG:-Release}"

shopt -s nocasematch
if [[ "${GENERATOR}" == *"Visual Studio"* || "${GENERATOR}" == *"Multi-Config"* || "${GENERATOR}" == *"Xcode"* ]]; then
    IS_MULTI_CONFIG=true
else
    IS_MULTI_CONFIG=false
fi
shopt -u nocasematch

REQUIRED_COMMANDS=(git cmake)

for cmd in "${REQUIRED_COMMANDS[@]}"; do
    if ! command -v "${cmd}" >/dev/null 2>&1; then
        echo "Missing required command: ${cmd}" >&2
        exit 1
    fi
done

download_file() {
    local url="$1"
    local dest="$2"

    if [[ -f "${dest}" ]]; then
        echo "Using cached $(basename "${dest}")"
        return
    fi

    echo "Downloading $(basename "${dest}")..."
    if command -v curl >/dev/null 2>&1; then
        curl -L --retry 3 --retry-delay 2 -o "${dest}" "${url}"
    elif command -v wget >/dev/null 2>&1; then
        wget -O "${dest}" "${url}"
    else
        echo "Neither curl nor wget is available to download ${url}" >&2
        exit 1
    fi
}

extract_archive() {
    local archive_path="$1"
    local destination="$2"

    mkdir -p "${destination}"

    local abs_archive="${archive_path}"
    if [[ ! "${abs_archive}" = /* ]]; then
        abs_archive="$(cd "$(dirname "${archive_path}")" && pwd)/$(basename "${archive_path}")"
    fi

    echo "Extracting $(basename "${archive_path}")..."
    (
        cd "${destination}"
        cmake -E tar xvf "${abs_archive}" >/dev/null
    )
}

clone_or_skip() {
    local repo_url="$1"
    local dest_dir="$2"
    local branch="${3:-}"

    if [[ -d "${dest_dir}/.git" ]]; then
        echo "$(basename "${dest_dir}") already present, skipping clone."
        return
    fi

    mkdir -p "$(dirname "${dest_dir}")"

    local args=("clone" "--depth" "1")
    if [[ -n "${branch}" ]]; then
        args+=("--branch" "${branch}")
    fi
    args+=("${repo_url}" "${dest_dir}")

    echo "Cloning ${repo_url}..."
    git "${args[@]}"
}

ensure_thirdparty_sources() {
    mkdir -p "${THIRDPARTY_DIR}"

    clone_or_skip "https://github.com/ocornut/imgui.git" "${THIRDPARTY_DIR}/imgui"
    clone_or_skip "https://github.com/facebook/yoga.git" "${THIRDPARTY_DIR}/yoga"
    clone_or_skip "https://github.com/leethomason/tinyxml2.git" "${THIRDPARTY_DIR}/tinyxml2"
}

ensure_sdl2() {
    local archive_path="${THIRDPARTY_DIR}/${SDL_PACKAGE}"
    local extracted_dir="${THIRDPARTY_DIR}/SDL2-${SDL_VERSION}"

    if [[ ! -d "${extracted_dir}" ]]; then
        download_file "${SDL_URL}" "${archive_path}"
        extract_archive "${archive_path}" "${THIRDPARTY_DIR}"
    else
        echo "SDL2 ${SDL_VERSION} already extracted."
    fi

    SDL_ROOT_DIR="${extracted_dir}"
    SDL_INCLUDE_DIR="${SDL_ROOT_DIR}/include"
    SDL_LIB_DIR="${SDL_ROOT_DIR}/lib/${SDL_LIB_SUBDIR}"
    SDL_DLL_PATH="${SDL_LIB_DIR}/SDL2.dll"

    if [[ ! -d "${SDL_INCLUDE_DIR}" ]]; then
        echo "SDL2 include directory missing at ${SDL_INCLUDE_DIR}" >&2
        exit 1
    fi

    if [[ ! -d "${SDL_LIB_DIR}" ]]; then
        echo "SDL2 library directory missing at ${SDL_LIB_DIR}" >&2
        exit 1
    fi

    if [[ ! -f "${SDL_DLL_PATH}" ]]; then
        echo "SDL2 runtime (SDL2.dll) not found in ${SDL_LIB_DIR}" >&2
        exit 1
    fi
}

configure_cmake() {
    echo "Configuring project with generator '${GENERATOR}'..."
    mkdir -p "${BUILD_DIR}"

    local cmake_args=(
        "-S" "${ROOT_DIR}"
        "-B" "${BUILD_DIR}"
        "-G" "${GENERATOR}"
        "-DSDL2_INCLUDE_DIRS=${SDL_INCLUDE_DIR}"
        "-DSDL2_LIBRARIES=${SDL_LIB_DIR}/SDL2.lib;${SDL_LIB_DIR}/SDL2main.lib"
        "-DSDL2_CFLAGS_OTHER=/D_CRT_SECURE_NO_WARNINGS"
    )

    if [[ "${GENERATOR}" == *"Visual Studio"* ]]; then
        cmake_args+=("-A" "${CMAKE_ARCH}")
    fi

    if [[ "${IS_MULTI_CONFIG}" == false ]]; then
        cmake_args+=("-DCMAKE_BUILD_TYPE=${BUILD_CONFIG}")
    fi

    cmake "${cmake_args[@]}"
}

build_target() {
    local target="$1"
    echo "Building ${target}..."

    local build_args=("--build" "${BUILD_DIR}" "--target" "${target}")
    if [[ "${IS_MULTI_CONFIG}" == true ]]; then
        build_args+=("--config" "${BUILD_CONFIG}")
    fi

    cmake "${build_args[@]}"
}

copy_sdl_runtime() {
    local destination
    if [[ "${IS_MULTI_CONFIG}" == true ]]; then
        destination="${BUILD_DIR}/${BUILD_CONFIG}"
    else
        destination="${BUILD_DIR}"
    fi

    mkdir -p "${destination}"
    cp "${SDL_DLL_PATH}" "${destination}/SDL2.dll"
    echo "Placed SDL2.dll next to executables in ${destination}"
}

print_summary() {
    local exe_dir
    if [[ "${IS_MULTI_CONFIG}" == true ]]; then
        exe_dir="${BUILD_DIR}/${BUILD_CONFIG}"
    else
        exe_dir="${BUILD_DIR}"
    fi

    cat <<EOF

Build finished successfully.

Executables:
  ${exe_dir}/imgui_builder.exe
  ${exe_dir}/imgui_oop_app.exe

SDL2 runtime copied to:
  ${exe_dir}/SDL2.dll

You can change generator/arch/build config via:
  GENERATOR="Ninja" ./win-setup.sh
  WIN_ARCH=x86 ./win-setup.sh
  BUILD_CONFIG=Debug ./win-setup.sh
EOF
}

main() {
    ensure_thirdparty_sources
    ensure_sdl2
    configure_cmake
    build_target "imgui_builder"
    build_target "imgui_oop_app"
    copy_sdl_runtime
    print_summary
}

main "$@"
