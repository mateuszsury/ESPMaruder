# Architecture Overview

SPECTRA is structured as an ESP-IDF component-based system with explicit boundaries for runtime safety and incremental delivery.

## High-level layers

- `main/`
  - bootstraps NVS, event loop, system, storage, GUI foundation, CLI
- `components/core_system`
  - system bootstrap and shared startup behavior
- `components/storage`
  - NVS, SPIFFS, SD access, retention helpers
- `components/cli`
  - UART command engine and command handlers
- `components/packet_engine`
  - packet buffer, filter, stats, PCAP writer
- `components/utils`
  - MAC/time/OUI helpers and shared low-level utilities
- `components/radio_*`
  - protocol-specific observability engines (Wi-Fi/BLE/802.15.4)
- `components/gui`
  - GUI event bus and UI runtime (currently foundation stage)
- `components/web`
  - reserved for optional web control plane

## Design principles

- Passive observability first
- Controlled-lab safety posture
- Traceable increments by phase
- Headless build profile for hardware-independent progression

## Build profiles

- `debug`: diagnostics-first local dev
- `release`: optimized artifact baseline
- `headless`: no display/SD hardware dependency
