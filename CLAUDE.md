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
src/Futbol/Futbol.ino      — Bluetooth RC sketch
src/MiniSumo/MiniSumo.ino  — autonomous sketch
```

Each sketch is a **single self-contained `.ino` file** by design — no shared headers or libraries between them, even though they duplicate the same small motor-driving helpers (`motorIzquierdo`/`motorDerecho`/`aplicarVelocidadMotor`). This is intentional: the audience is students reading one file top-to-bottom, not a codebase optimized for reuse. When editing one sketch, do not "fix" the duplication by extracting a shared library — mirror any relevant fix into the other sketch instead if it applies there too.

**Language convention:** comments, `Serial` messages, and identifiers (variables/functions) are in Spanish; `#define` names follow standard Arduino/English casing conventions (`pinMode`, `analogWrite`, etc. are untouched). Keep this consistent — the code is explicitly written for Spanish-speaking students to read.

**Futbol.ino** — linear, stateless command dispatch: reads one ASCII char at a time from `Serial` (fed by the HC-05 over hardware UART D0/D1 — no `SoftwareSerial`), maps it to a movement function. Two physical buttons (horn, turbo toggle) are polled directly in `loop()`.

**MiniSumo.ino** — explicit `enum Estado` state machine (`ESPERA → CUENTA_REGRESIVA → BUSQUEDA ⇄ ATAQUE`, with `EVASION` interrupting from either) driven by a `switch` in `loop()`. Edge-sensor checks run with top priority on every loop iteration before any state-specific logic — this is a safety property, not just style: don't reorder it. The `EVASION` maneuver runs synchronously to completion inside `manejarEvasion()` (blocking `delay()`s) rather than persisting as a state across loop iterations — this is deliberate (see FSD §4.5), don't try to make it interruptible without also updating the FSD's rationale.

## Build / compile verification

No test suite (Arduino sketches, not a library). To verify a sketch compiles, use `arduino-cli` (not installed by default in this environment — install via the official install script if needed):

```bash
arduino-cli core install arduino:avr   # once, after install
arduino-cli compile --fqbn arduino:avr:uno src/Futbol
arduino-cli compile --fqbn arduino:avr:uno src/MiniSumo
```

Each sketch's `.ino` filename must match its containing folder name (`Futbol/Futbol.ino`, `MiniSumo/MiniSumo.ino`) — this is an Arduino IDE/CLI requirement, not a stylistic choice.

## Hardware constraints that affect code changes

- **HC-05 occupies D0/D1 (hardware UART)** — any code change that adds `Serial` debug output to `Futbol.ino` would collide with Bluetooth communication. `MiniSumo.ino` can safely use `Serial` for debugging since it has no Bluetooth module.
- **Max supply voltage is 10V** per the schematic — not a code concern, but relevant if adding any battery-voltage-dependent logic (the optional battery sensor on A0 is not currently used in either sketch).
- **A4/A5 are shared** between I2C and the alternate HC-SR04 header (J3) — mutually exclusive, not both usable at once.
- Motor left/right assignment (`PIN_MOTOR_IZQ_*` = Motor 1, `PIN_MOTOR_DER_*` = Motor 2) is arbitrary and chassis-dependent; both sketches flag this near the `#define`s rather than hardcode an assumption that's guaranteed to be wrong for some builds.
