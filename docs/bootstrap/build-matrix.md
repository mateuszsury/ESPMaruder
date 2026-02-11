# SPECTRA Build Matrix (Phase 1.1 Bootstrap)

## Profiles

| Profile | SDKCONFIG_DEFAULTS | Use case |
|---|---|---|
| debug | `sdkconfig.defaults;sdkconfig.debug.defaults` | Local development and diagnostics |
| release | `sdkconfig.defaults;sdkconfig.release.defaults` | Reproducible firmware artifact for controlled lab use |
| headless | `sdkconfig.defaults;sdkconfig.headless.defaults` | Bring-up without connected display and SD card hardware |

## Configure and Build

WSL isolated build (recommended for parallel instances):

```bash
./scripts/wsl/build_instance.sh wsl01 debug
./scripts/wsl/build_instance.sh wsl02 release
./scripts/wsl/build_instance.sh wsl03 headless
```

By default, the script picks ESP-IDF in this order:
- `SPECTRA_IDF_ROOT` (if set)
- `~/esp-idf-v5.5-spectra`
- `~/esp-idf-5.5.1`
- `~/esp-idf`

Use ESP-IDF `v5.5+` for ESP32-C5 `v1.0` hardware compatibility.
Example override:

```bash
SPECTRA_IDF_ROOT=~/esp-idf-v5.5-spectra ./scripts/wsl/build_instance.sh wsl10 headless
```

Each instance uses separate:
- build directory in `out/build/<instance>-<profile>`
- sdkconfig file in `out/sdkconfig/<instance>-<profile>.sdkconfig`
- `esp32c5` preview mode handled by script (`idf.py --preview`)

SD support in storage is enabled by default.
The `headless` profile overrides this and sets `CONFIG_SPECTRA_STORAGE_ENABLE_SD=n`.
The `headless` profile also disables GUI foundation and hardware path:
- `CONFIG_SPECTRA_GUI_ENABLE=n`
- `CONFIG_SPECTRA_GUI_ENABLE_HW_DRIVERS=n`
Optional toggle in menuconfig (if needed):

```bash
idf.py menuconfig
# SPECTRA Storage -> Enable SD card mount and file operations
```

Headless:

```powershell
idf.py set-target esp32c5
idf.py -D SDKCONFIG_DEFAULTS="sdkconfig.defaults;sdkconfig.headless.defaults" reconfigure build
```

Debug:

```powershell
idf.py set-target esp32c5
idf.py -D SDKCONFIG_DEFAULTS="sdkconfig.defaults;sdkconfig.debug.defaults" reconfigure build
```

Release:

```powershell
idf.py set-target esp32c5
idf.py -D SDKCONFIG_DEFAULTS="sdkconfig.defaults;sdkconfig.release.defaults" reconfigure build
```

## Flash

```powershell
idf.py -p COMx flash monitor
```

## Artifact naming rule

Use: `spectra-<version>-<profile>-esp32c5.bin`

Example:
- `spectra-0.1.0-bootstrap-debug-esp32c5.bin`
- `spectra-0.1.0-bootstrap-release-esp32c5.bin`
- `spectra-0.1.0-bootstrap-headless-esp32c5.bin`
