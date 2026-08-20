# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this repository is

Educational robotics project for ESETP N°703 (Puerto Madryn). A custom Arduino UNO shield ("Robotshield V2.1") is used by secondary-school electronics students (~16 years old) to build one of two robots:

- **Fútbol** — remote-controlled via Bluetooth (HC-05).
- **Mini Sumo** — autonomous, ultrasonic + edge/line sensors.

Both robots share the same shield and pin map but run completely independent Arduino sketches.

## Source of truth for hardware and design

Before touching pins, sensors, or behavior, read:

- `ROBOTSHIELD_V2.1_CONTEXTO_TECNICO.md` — the authoritative pin map, connector list (J1–J15), BOM, and hardware notes, reconstructed from the real KiCad schematic (`hardware/Robotshield_V2.1.kicad_sch`) and `hardware/BOM.csv`.
- `docs/FSD.md` — the functional spec for both sketches: Bluetooth command protocol, the Mini Sumo state machine, calibration constants, and the rationale behind design choices (e.g. why `delay()` is acceptable in specific spots).

Do not re-derive the pin map from scratch — the schematic historically had a real discrepancy between its summary pinout table and its detailed wiring (now resolved); `ROBOTSHIELD_V2.1_CONTEXTO_TECNICO.md` §2 is the resolved, trustworthy version.

## Code architecture

```
src/Futbol/
  Futbol.ino      — setup()/loop(), high-level dispatch, pin #defines (main file — always compiled first)
  Bluetooth.ino   — reading/parsing commands from the HC-05
  Motores.ino     — DRV8833 motor-driving functions
  Extras.ino      — horn button + physical horn, turbo button

src/MiniSumo/
  MiniSumo.ino         — setup()/loop(), pin #defines, calibration consts, enum Estado + estadoActual (main file — always compiled first)
  MaquinaDeEstado.ino  — per-state handlers (manejarEspera, manejarBusqueda, manejarAtaque, manejarEvasion, ...)
  Motores.ino          — DRV8833 motor-driving functions (identical to Futbol's)
  Sensores.ino         — raw sensor reads (edge/line sensors, HC-SR04, emergency button)
  Extras.ino           — LED blink helper
```

Each sketch is split into several `.ino` files **within its own sketch folder**, purely for readability — the audience is students reading one short file at a time, not a codebase optimized for reuse. This works because the Arduino IDE/CLI concatenates every `.ino` file in a sketch folder into a single compilation unit before building: the file matching the folder name (the sketch's "main" file) is always concatenated first, then the rest in alphabetical order. Two consequences worth knowing before editing:

- **Function calls across files are always safe**, regardless of which file defines vs. calls a function — Arduino auto-generates forward prototypes for every function in every tab.
- **Global variables, `const`s, and `enum`s are NOT auto-prototyped** — they're only visible after their point of definition in the concatenated output. This is why all `#define` pins, calibration constants, and (for MiniSumo) `enum Estado`/`estadoActual` live in each sketch's main file: that file is guaranteed to be compiled first, and its own `setup()`/`loop()` needs them already declared. Don't move `enum Estado`/`estadoActual` into `MaquinaDeEstado.ino` even though they're conceptually "decision logic" — `MiniSumo.ino`'s `loop()` references them and would fail to compile if they were declared later.

There are still **no shared headers or libraries between the two sketches** (Futbol vs. MiniSumo) — that rule is unchanged, it just now applies at the sketch-folder level rather than the single-file level. `motorIzquierdo`/`motorDerecho`/`aplicarVelocidadMotor`/`detenerMotores` are intentionally duplicated byte-for-byte between `Futbol/Motores.ino` and `MiniSumo/Motores.ino`. When editing one, mirror any relevant fix into the other's `Motores.ino` instead of extracting a shared library.

**Language convention:** comments, `Serial` messages, and identifiers (variables/functions) are in Spanish; `#define` names follow standard Arduino/English casing conventions (`pinMode`, `analogWrite`, etc. are untouched). Keep this consistent — the code is explicitly written for Spanish-speaking students to read.

**Futbol** — linear, stateless command dispatch: `Bluetooth.ino`'s `revisarBluetooth()` reads one ASCII char at a time from `Serial` (fed by the HC-05 over hardware UART D0/D1 — no `SoftwareSerial`), maps it to a movement function in `Motores.ino`. Two physical buttons (horn, turbo toggle) are polled by functions in `Extras.ino`, called every iteration from `loop()` in `Futbol.ino`.

**MiniSumo** — explicit `enum Estado` state machine (`ESPERA → CUENTA_REGRESIVA → BUSQUEDA ⇄ ATAQUE`, with `EVASION` interrupting from either), declared in `MiniSumo.ino` and driven by a `switch` in that same file's `loop()`; the actual per-state behavior lives in `MaquinaDeEstado.ino`. Edge-sensor checks (`Sensores.ino`'s `hayBordeDetectado()`) run with top priority on every `loop()` iteration before any state-specific logic — this is a safety property enforced by statement order in `MiniSumo.ino`'s `loop()`, not just style: don't reorder it, and don't move it into a called function without preserving that same ordering. The `EVASION` maneuver runs synchronously to completion inside `manejarEvasion()` (`MaquinaDeEstado.ino`, blocking `delay()`s) rather than persisting as a state across loop iterations — this is deliberate (see FSD §4.5), don't try to make it interruptible without also updating the FSD's rationale.

## Build / compile verification

No test suite (Arduino sketches, not a library). To verify a sketch compiles, use `arduino-cli` (not installed by default in this environment — install via the official install script if needed):

```bash
arduino-cli core install arduino:avr   # once, after install
arduino-cli compile --fqbn arduino:avr:uno src/Futbol
arduino-cli compile --fqbn arduino:avr:uno src/MiniSumo
```

Each sketch's main `.ino` filename must match its containing folder name (`Futbol/Futbol.ino`, `MiniSumo/MiniSumo.ino`) — this is an Arduino IDE/CLI requirement, not a stylistic choice. The other `.ino` files in the same folder (`Motores.ino`, `Bluetooth.ino`, etc.) are auto-included regardless of their own names — just add/edit them, no manifest to update.

## Hardware constraints that affect code changes

- **HC-05 occupies D0/D1 (hardware UART)** — any code change that adds `Serial` debug output to `Futbol.ino` would collide with Bluetooth communication. `MiniSumo.ino` can safely use `Serial` for debugging since it has no Bluetooth module.
- **Max supply voltage is 10V** per the schematic — not a code concern, but relevant if adding any battery-voltage-dependent logic (the optional battery sensor on A0 is not currently used in either sketch).
- **A4/A5 are shared** between I2C and the alternate HC-SR04 header (J3) — mutually exclusive, not both usable at once.
- Motor left/right assignment (`PIN_MOTOR_IZQ_*` = Motor 1, `PIN_MOTOR_DER_*` = Motor 2) is arbitrary and chassis-dependent; both sketches flag this near the `#define`s rather than hardcode an assumption that's guaranteed to be wrong for some builds.
