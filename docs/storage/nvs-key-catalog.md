# SPECTRA NVS Key Catalog (Phase 1.2)

Namespace: `spectra`

## System keys

| Key | Type | Description | Migration rule |
|---|---|---|---|
| `schema_ver` | `i32` | NVS schema version | bump on incompatible format change |

## API contract

Exposed by `components/storage/src/nvs_mgr.c`:
- `nvs_mgr_store_string` / `nvs_mgr_load_string`
- `nvs_mgr_store_blob` / `nvs_mgr_load_blob`
- `nvs_mgr_store_i32` / `nvs_mgr_load_i32`

## Migration strategy

1. Read `schema_ver` at startup.
2. If missing, initialize to current version.
3. On future version bump, run keyed migration steps before normal startup.

