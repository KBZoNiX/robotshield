# Robotshield V2.1 — Contexto técnico (ESETP N°703, Puerto Madryn)

> Archivo de referencia para el proyecto de **programación de los robots** (Fútbol y Mini Sumo) en Claude Code. Resume la placa, el mapa de pines, los conectores y los componentes, extraídos del esquemático real (`Robotshield_V2.1.kicad_sch`, Rev 2.1, KiCad 9.0.7) y de `BOM.csv`.

---

## 1. Contexto del proyecto

- **Institución:** ESETP N°703, Puerto Madryn.
- **Placa:** Robotshield V2.1 — shield custom para **Arduino UNO** (encastra sobre los headers estándar).
- **Dos categorías de robot, mismo shield:**
  - **Fútbol** → control remoto por Bluetooth (módulo HC-05).
  - **Mini Sumo** → autónomo, con sensor ultrasónico HC-SR04 y sensores de línea/borde.
- **Driver de motores:** DRV8833, enchufado sobre el Robotshield (conectores J5/J6).

⚠️ **Alimentación máxima indicada en el esquemático: Vmax = 10V.** No alimentar con baterías que superen ese máximo.

---

## 2. Mapa de pines del Arduino UNO

Tabla basada en el esquemático (tabla de pinout y cableado detallado ya coinciden).

| Pin | Función (cableado detallado) | Categoría | PWM |
|---|---|---|---|
| D0 (RX) | UART RX ← salida TX del HC-05 (directo, sin divisor) | Fútbol | |
| D1 (TX) | UART TX → entrada RX del HC-05, **a través de divisor R1(10K)/R2(22K)** | Fútbol | |
| D2 | Botón / pulsador (SW2 en el esquemático) | Común | |
| D3 | Driver de motores — entrada Motor 2A | Común | ✔ |
| D4 | LED (D3 del esquemático, vía R4 470Ω) | Común | |
| D5 | HC-SR04 — ECHO (conector J2, "ULTRA_1") | Mini Sumo | ✔ |
| D6 | HC-SR04 — TRIGGER (conector J2, "ULTRA_1") | Mini Sumo | ✔ |
| D7 | LED (D4 del esquemático, vía R5 470Ω) | Común | |
| D8 | Buzzer (BZ1) | Común | |
| D9 | Driver de motores — entrada Motor 1A | Común | ✔ |
| D10 | Driver de motores — entrada Motor 1B | Común | ✔ |
| D11 | Driver de motores — entrada Motor 2B | Común | ✔ |
| D12 | Botón / pulsador (SW3 en el esquemático) | Común | |
| D13 | LED (D5 del esquemático, vía R6 470Ω) | Común | |
| A0 | Sensor de nivel de batería (opcional, divisor R7/R8) | Común (opcional) | |
| A1 | Sensor de línea/borde IR1 (conector J9) | Mini Sumo | |
| A2 | Sensor de línea/borde IR2 (conector J10) | Mini Sumo | |
| A3 | Sensor de línea/borde IR3 (conector J11) | Mini Sumo | |
| A4 (SDA) | I2C SDA — **o** HC-SR04 alterno TRIGGER (conector J3, "ULTRA_2") | Común / Mini Sumo alt. | |
| A5 (SCL) | I2C SCL — **o** HC-SR04 alterno ECHO (conector J3, "ULTRA_2") | Común / Mini Sumo alt. | |

**Motor → pines de control (DRV8833):**
- **Motor 1 (M1, conector J7):** D9 (1A) / D10 (1B)
- **Motor 2 (M2, conector J8):** D3 (2A) / D11 (2B)

---

## 3. Conectores del Robotshield (según BOM.csv y esquemático)

| Conector | Tipo / huella | Pines | Función | Uso |
|---|---|---|---|---|
| J1 | Header 1x2 | VBAT+, GND | Entrada de batería | Común — **respetar polaridad** |
| J2 | Socket 1x4 | GND, D6(Trig), D5(Echo), +5V | "ULTRA_1 / ENC" — sensor ultrasónico 1 (o encoder) | Mini Sumo |
| J3 | Socket 1x5 | GND, —, A5/SCL, A4/SDA, +5V | "ULTRA_2 / I2C" — sensor ultrasónico alterno o bus I2C | Mini Sumo (alternativo) / Común (I2C) |
| J4 | Socket 1x4 | GND, +5V, D0(RX), D1(TX vía divisor) | "BT / UART" — módulo HC-05 | Fútbol |
| J5 | Socket 1x6 | D3, D11, VCC, GND, D10, D9 | "driver_in" — entradas de control al DRV8833 | Común |
| J6 | Socket 1x6 | — | "driver_out" — salidas del DRV8833 hacia J7/J8 | Común |
| J7 | Header 1x2 | M1+, M1− | Motor 1 | Común |
| J8 | Header 1x2 | M2+, M2− | Motor 2 | Común |
| J9 | Header 1x3 | GND, A1, +5V | Sensor de línea/borde IR1 | Mini Sumo |
| J10 | Header 1x3 | GND, A2, +5V | Sensor de línea/borde IR2 | Mini Sumo |
| J11 | Header 1x3 | GND, A3, +5V | Sensor de línea/borde IR3 | Mini Sumo |
| J12 | Header 1x8 | IOREF, RESET, 3V3, 5V, Vin, GND, GND | Header "Power" del Arduino (mecánico, no se cablea a mano) | — |
| J13 | Header 1x6 | A0–A3, SDA/A4, SCL/A5 | Header "Analog" del Arduino | — |
| J14 | Header 1x10 | SCL/A5, SDA/A4, AREF, NC, D13, D12, D11, D10, D9, D8 | Header "Digital/PWM" del Arduino | — |
| J15 | Header 1x8 | D7, D6, D5, D4, D3, D2, D1(TX), D0(RX) | Header "Digital/PWM" del Arduino | — |

*(J12–J15 son los headers hembra que hacen de interfaz mecánica/eléctrica con el Arduino UNO; no requieren cableado adicional, se muestran para referencia de continuidad de señal.)*

---

## 4. Componentes comunes (de BOM.csv)

| Ref. | Descripción | Cant. | Notas |
|---|---|---|---|
| SW1 | Interruptor de encendido (toggle 2P2T 7x7mm) | 1 | Corta VBAT → VCC. Pieza más alta del PCB. |
| D2 | Diodo 1N4007 | 1 | Protección de polaridad inversa, entre VCC y Vin. |
| D1 | LED 3mm | 1 | Indicador de encendido, junto a R3 (1K). |
| C1, C2 | Capacitores 100µF | 2 | Filtrado en VCC y Vin. |
| SW2, SW3 | Pulsadores 6mm | 2 | Botones de usuario (D2 y D12). |
| D3, D4, D5 | LEDs 5mm | 3 | LEDs de estado (D4, D7, D13). |
| R4, R5, R6 | Resistencias 470Ω | 3 | Limitadoras de corriente de D3/D4/D5. |
| BZ1 | Buzzer 12x9.5mm | 1 | D8. |
| R1 | 10K (0805) | 1 | Divisor de tensión TX→HC-05 (junto a R2). |
| R2 | 22K (0805) | 1 | Divisor de tensión TX→HC-05 (junto a R1). |
| R7 | 39K (0805) | 1 | Divisor sensor de batería (con R8), opcional. |
| R8 | 10K (0805) | 1 | Divisor sensor de batería (con R7), opcional. |
| C3 | 100nF (0805) | 1 | Filtro del sensor de batería en A0. |

**Módulos según categoría (no están en el Robotshield, se enchufan aparte):**
- **HC-05** (Bluetooth) → J4 — solo Fútbol.
- **HC-SR04** (ultrasónico) → J2 (o J3 alternativo) — solo Mini Sumo.
- **DRV8833** (driver de motores) → J5/J6 — común a ambas categorías.

---

## 5. Notas prácticas para la programación

- **Conflicto de UART con el HC-05:** el módulo Bluetooth está cableado directo a los pines *hardware* D0/D1 (no hay `SoftwareSerial`). Esto significa que **hay que desconectar el HC-05 (o su línea hacia el shield) para subir un sketch por USB**, porque el módulo interfiere con la programación. Tenerlo en cuenta en cualquier guía o script de carga.
- **Divisor de tensión solo en TX (D1):** el HC-05 recibe en su RX una señal reducida a ~3.4V (R1/R2). La salida del HC-05 hacia el RX del Arduino (D0) va directa, sin divisor.
- **A4/A5 son un recurso compartido:** funcionan como bus I2C **o** como HC-SR04 alterno (J3) — no se pueden usar ambos usos a la vez.
- **J2 ("ULTRA_1 / ENC") también es de doble propósito:** pensado para ultrasónico o para un encoder, comparte D5/D6.
- **Sensor de batería (A0, opcional):** `Vout = Vbat / 5` (máximo 24V en packs de 5S). Para convertir la lectura de `analogRead(A0)` a voltios reales de batería: `Vbat = (lectura * 5.0 / 1023.0) * 5`.
- **Pines PWM disponibles en el shield:** D3, D5, D6, D9, D10, D11 (marcados con `*` en el esquemático).

### Bloque de referencia para el código

```cpp
// ---- Común a ambas categorías ----
#define PIN_BOTON_1     2   // SW2 (pulsador)
#define PIN_MOTOR_2A    3   // Driver DRV8833
#define PIN_LED_1       4   // D3
#define PIN_LED_2       7   // D4
#define PIN_BUZZER      8   // BZ1
#define PIN_MOTOR_1A    9   // Driver DRV8833
#define PIN_MOTOR_1B    10  // Driver DRV8833
#define PIN_MOTOR_2B    11  // Driver DRV8833
#define PIN_BOTON_2     12  // SW3 (pulsador)
#define PIN_LED_3       13  // D5
#define PIN_BATERIA     A0  // Opcional, Vbat = lectura * 5.0/1023.0 * 5

// ---- Solo Fútbol ----
// HC-05 en Serial (D0/D1) — usar Serial.begin(), sin SoftwareSerial
// Recordar desconectar el módulo al subir sketches por USB

// ---- Solo Mini Sumo ----
#define PIN_ULTRA1_ECHO    5
#define PIN_ULTRA1_TRIGGER 6
#define PIN_SENSOR_LINEA_1 A1
#define PIN_SENSOR_LINEA_2 A2
#define PIN_SENSOR_LINEA_3 A3
// A4/A5: HC-SR04 alterno (J3) o I2C — uso exclusivo, no simultáneo
```

---

## 6. Referencia mecánica (breve)

| Pieza | Cant. | Notas |
|---|---|---|
| Cuerpo, Frente, Tapa batería | 1 c/u | Piezas 3D impresas |
| Motor TT amarillo | 2 | Uno a cada lado del Cuerpo |
| Rueda | 2 | En el eje de cada motor |
| Arduino Uno + Robotshield + DRV8833 | 1 c/u | Apilados sobre el Cuerpo |
| HC-SR04 | 1 | Se inserta en el Frente — solo Mini Sumo |
| HC-05 | 1 | Se enchufa directo en el Robotshield — solo Fútbol |

*(El Frente incluye el hueco para el HC-SR04 aunque no se use en la unidad de Fútbol.)*

---

## 7. Archivos fuente del proyecto

- `Robotshield_V2.1.kicad_sch` (esquemático, Rev 2.1)
- `BOM.csv`
- `diagramas_bloques.html` (diagramas de bloques por categoría)
- Imágenes 3D de referencia (armado mecánico, despiece etiquetado)
