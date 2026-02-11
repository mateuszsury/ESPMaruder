# SPECTRA CLI Commands (Phase 1.3)

Date: 2026-02-11
Profile note: examples validated on `headless` profile (`COM14`)

## Available commands

- `help`
  - Prints all registered commands with usage.
- `version`
  - Prints project name, firmware version, IDF revision, and active build profile.
- `sysinfo`
  - Prints uptime, heap metrics, and chip temperature.
- `reboot`
  - Performs software restart.
- `set <key> <value>`
  - Stores key/value in NVS namespace `spectra`.
  - Key rules: `[A-Za-z0-9_-]`, max 15 chars.
- `get <key>`
  - Reads key from NVS namespace `spectra`.
- `sd ls <path>`
  - Lists directory content on SD.
- `sd cat <file>`
  - Prints file content from SD.
- `sd rm <file>`
  - Removes file on SD.
- `script <file>`
  - Executes line-by-line command script from SD file.

## Headless behavior

- In `headless` profile, SD is disabled by configuration.
- SD commands and `script` return `sd not mounted` until SD support is enabled and hardware is present.
