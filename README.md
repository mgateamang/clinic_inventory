# AIDANS Inventory & Medical Supplies Client (Qt6 / QML)

## Executive Summary (For Engineering Leaders & Hiring Managers)

**`clinic_inventory`** is an enterprise-grade desktop client built with **Qt 6 (C++ backend + declarative QML front-end)**. It provides clinical logistics staff and head nurses with real-time visibility, automated reorder alerts, and audited stock tracking for pharmaceuticals, surgical consumables, and medical equipment.

---

## Architectural Highlights

1. **Clean Separation of Concerns (MVVM Architecture):**
   - **`InventoryController` (C++):** Inherits from `QObject`. Manages asynchronous REST API networking via `QNetworkAccessManager`, configuration serialization (`config.ini`), state machines (`isBusy`, `isServerOnline`), and business rule filtering (identifying items under `reorder_threshold`).
   - **Declarative QML (`Main.qml`):** Pure reactive UI layer. Automatically binds to C++ properties via Qt meta-object notifications (`NOTIFY` signals), guaranteeing snappy 60fps rendering without blocking the UI event loop.
2. **Resilience & Offline Fallbacks:**
   - Features built-in health probing and connection testing.
   - Automatically detects server unavailability, displays non-blocking status badges, and enables live reconfiguration without requiring application restarts.
3. **Audited Stock Modifications:**
   - Every restock or adjustment records timestamped audit reasons directly to the backend ledger.

---

## Key Features
- **All Stock Catalog:** Searchable and filterable medical supply list with units, pricing, and active quantities.
- **Low Stock Alerts:** Automatic identification of mission-critical medicines running low against safety stock thresholds.
- **One-Click Restocking Dialog:** Intuitive restock quantity input and audit reason capture.
- **New Supply Onboarding:** Rapid creation of new inventory items and barcode/SKU registration.
- **Configurable Backend Integration:** Interactive settings dialog with live latency check.

---

## Build and Run

```bash
# Configure with Meson
meson setup build

# Compile
meson compile -C build

# Launch application
./build/clinic_inventory
```
