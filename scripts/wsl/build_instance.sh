#!/usr/bin/env bash
set -euo pipefail

INSTANCE_ID="${1:-wsl01}"
PROFILE="${2:-debug}"

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "${SCRIPT_DIR}/../.." && pwd)"
OUT_ROOT="${PROJECT_ROOT}/out"
BUILD_DIR="${OUT_ROOT}/build/${INSTANCE_ID}-${PROFILE}"
SDKCONFIG_PATH="${OUT_ROOT}/sdkconfig/${INSTANCE_ID}-${PROFILE}.sdkconfig"
if [[ -n "${SPECTRA_IDF_ROOT:-}" ]]; then
  IDF_ROOT="${SPECTRA_IDF_ROOT}"
elif [[ -d "${HOME}/esp-idf-v5.5-spectra" ]]; then
  IDF_ROOT="${HOME}/esp-idf-v5.5-spectra"
elif [[ -d "${HOME}/esp-idf-5.5.1" ]]; then
  IDF_ROOT="${HOME}/esp-idf-5.5.1"
else
  IDF_ROOT="${HOME}/esp-idf"
fi

if [[ -n "${IDF_PATH:-}" && "${IDF_PATH}" != "${IDF_ROOT}" ]]; then
  unset IDF_PATH
fi

if [[ -z "${IDF_PATH:-}" ]]; then
  if [[ -f "${IDF_ROOT}/export.sh" ]]; then
    # shellcheck source=/dev/null
    source "${IDF_ROOT}/export.sh" >/dev/null
  else
    echo "ERROR: IDF_PATH is not set and ${IDF_ROOT}/export.sh was not found."
    exit 1
  fi
fi

case "${PROFILE}" in
  debug)
    SDKCONFIG_DEFAULTS="sdkconfig.defaults;sdkconfig.debug.defaults"
    ;;
  release)
    SDKCONFIG_DEFAULTS="sdkconfig.defaults;sdkconfig.release.defaults"
    ;;
  headless)
    SDKCONFIG_DEFAULTS="sdkconfig.defaults;sdkconfig.headless.defaults"
    ;;
  *)
    echo "ERROR: Unknown profile '${PROFILE}'. Expected: debug | release | headless"
    exit 2
    ;;
esac

mkdir -p "$(dirname "${SDKCONFIG_PATH}")" "${BUILD_DIR}"
cd "${PROJECT_ROOT}"

echo "[SPECTRA] instance=${INSTANCE_ID} profile=${PROFILE}"
echo "[SPECTRA] build_dir=${BUILD_DIR}"
echo "[SPECTRA] sdkconfig=${SDKCONFIG_PATH}"
echo "[SPECTRA] defaults=${SDKCONFIG_DEFAULTS}"
echo "[SPECTRA] idf_path=${IDF_PATH}"

if [[ -f "${SDKCONFIG_PATH}" ]]; then
  if grep -Eq '^CONFIG_IDF_TARGET_ESP32C5_MP_VERSION=y$' "${SDKCONFIG_PATH}"; then
    echo "[SPECTRA] Existing sdkconfig uses legacy ESP32-C5 MP variant; regenerating for current baseline."
    rm -f "${SDKCONFIG_PATH}"
  fi
fi

COMMON_ARGS=(
  --preview
  -B "${BUILD_DIR}"
  -D "SDKCONFIG=${SDKCONFIG_PATH}"
  -D "SDKCONFIG_DEFAULTS=${SDKCONFIG_DEFAULTS}"
  -D "SPECTRA_BUILD_PROFILE=${PROFILE}"
)

if [[ ! -f "${SDKCONFIG_PATH}" ]]; then
  idf.py "${COMMON_ARGS[@]}" set-target esp32c5 reconfigure build
else
  idf.py "${COMMON_ARGS[@]}" reconfigure build
fi

echo "[SPECTRA] Build completed."
