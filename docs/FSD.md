# Robotshield V2.1 — Especificación Funcional del Software (FSD)

> ESETP N°703, Puerto Madryn — Programación de robots educativos (Fútbol RC y Mini Sumo) sobre Robotshield V2.1 + Arduino UNO.
> Este documento es el plan de diseño de los dos programas. La referencia de hardware completa está en `ROBOTSHIELD_V2.1_CONTEXTO_TECNICO.md`; acá solo se repite lo que el código necesita.

---

## 1. Objetivo y alcance

Escribir **dos sketches independientes**, uno por categoría de robot, que los alumnos (16 años, 1er contacto con robótica) puedan leer de punta a punta y entender **qué hace cada línea y por qué**. No son firmwares de producción: priorizan claridad sobre robustez o eficiencia.

- `src/Futbol/Futbol.ino` — auto controlado por Bluetooth (celular → HC-05).
- `src/MiniSumo/MiniSumo.ino` — autónomo, con máquina de estados.

Ambos:
- Un único archivo `.ino`, sin librerías externas ni archivos `.h` propios (solo `Arduino.h` implícito). Nada de `SoftwareSerial` (ver nota UART).
- Comentarios y mensajes por `Serial` en **español**; nombres de `#define`/funciones en **inglés/convención Arduino estándar** donde corresponda (`pinMode`, `analogWrite`, etc.) pero identificadores propios (variables, funciones) en **español**, que es como se va a explicar en clase.
- Organizados en bloques con comentarios tipo encabezado (`// ===== SECCIÓN =====`) para que se puedan ubicar a simple vista.
- Constantes de calibración agrupadas arriba del todo, con comentario de qué significan y cómo ajustarlas en la práctica.
- Sin `delay()` largos que dejen al robot "ciego" salvo donde se documenta explícitamente que es una maniobra corta y segura (ver Mini Sumo).

**Fuera de alcance (ambos programas):** sensor de batería (A0), uso de I2C, `SoftwareSerial`, control de velocidad fino más allá de dos niveles, cualquier lógica de calibración automática. Se documentan como posibles extensiones para después de que el programa base funcione.

---

## 2. Hardware y pines (resumen — fuente: `ROBOTSHIELD_V2.1_CONTEXTO_TECNICO.md`)

| Pin | Común a ambos | Solo Fútbol | Solo Mini Sumo |
|---|---|---|---|
| D0/D1 | | HC-05 (Serial, sin SoftwareSerial) | |
| D2 | Botón 1 (SW2) | | |
| D3 | Motor 2A (DRV8833) | | |
| D4 | LED 1 | | |
| D5 | | | HC-SR04 ECHO |
| D6 | | | HC-SR04 TRIGGER |
| D7 | LED 2 | | |
| D8 | Buzzer | | |
| D9 | Motor 1A (DRV8833) | | |
| D10 | Motor 1B (DRV8833) | | |
| D11 | Motor 2B (DRV8833) | | |
| D12 | Botón 2 (SW3) | | |
| D13 | LED 3 | | |
| A1/A2/A3 | | | Sensores de borde/línea |

**Motores:** Motor 1 = pines D9(1A)/D10(1B) → asignado como **motor izquierdo**. Motor 2 = pines D3(2A)/D11(2B) → asignado como **motor derecho**. Esta asignación es arbitraria (depende de cómo se armó el chasis); ambos sketches van a tener un comentario bien visible cerca de las funciones de motor indicando "si el robot dobla al revés de lo esperado, invertí esta asignación".

**⚠️ Nota de carga de sketch (Fútbol):** el HC-05 está cableado directo a D0/D1 (UART hardware). Hay que **desenchufar el HC-05** antes de subir el programa por USB, si no, la carga falla o se cuelga.

**⚠️ Alimentación:** Vmax = 10V según el esquemático. No usar packs de batería que superen ese voltaje.

---

## 3. Programa Fútbol RC

### 3.1 Resumen funcional
Robot controlado en tiempo real por Bluetooth. Un celular con una app cualquiera de terminal Bluetooth (o joystick simple que mande caracteres) envía un carácter por vez; el Arduino lo lee de `Serial` (conectado al HC-05) y mueve los motores en consecuencia.

### 3.2 Protocolo de comandos (Bluetooth → Arduino)

Un carácter ASCII por comando, sin necesidad de terminador de línea:

| Carácter | Acción |
|---|---|
| `F` | Avanzar (los dos motores adelante) |
| `B` | Retroceder (los dos motores atrás) |
| `L` | Girar a la izquierda en el lugar (motores en sentidos opuestos) |
| `R` | Girar a la derecha en el lugar |
| `S` | Detener (parada, motores en punto muerto) |
| `H` | Bocina (bip corto de buzzer) |

Cualquier otro carácter recibido se ignora (no rompe el programa). El robot **no tiene "watchdog" de conexión** en la versión base: si se corta el Bluetooth, el robot sigue haciendo lo último que le dijeron. Se documenta como extensión posible (parar si no llega nada en, por ejemplo, 1 segundo) pero no se implementa para no sumar complejidad en la v1.

### 3.3 Controles físicos en la placa
- **Botón 1 (D2):** bocina local (mismo efecto que el comando `H`, pero apretando el botón de la placa en vez de por Bluetooth). Sirve para mostrar que un input puede venir de distintos lugares y disparar la misma función.
- **Botón 2 (D12):** alterna modo **turbo** (velocidad normal ↔ velocidad máxima). Toggle con antirrebote simple.
- **LED 1 (D4):** encendido fijo mientras el programa está corriendo (testigo de "encendido y programado").
- **LED 2 (D7):** parpadea brevemente cada vez que llega un comando por Bluetooth (testigo de actividad/recepción).
- **LED 3 (D13):** encendido mientras el modo turbo está activo.
- **Buzzer (D8):** bip al arrancar (confirmación de "listo") y bip corto en bocina.

### 3.4 Control de motores
Función genérica `motorIzquierdo(int velocidad)` / `motorDerecho(int velocidad)`, rango `-255..255`:
- Positivo → adelante (PWM en el pin "A", LOW en el pin "B").
- Negativo → atrás (LOW en "A", PWM en "B" con `abs(velocidad)`).
- `0` → punto muerto (ambos pines LOW; se elige "coast" en vez de frenado activo por ser más simple de explicar: "sin corriente, no hay fuerza, la rueda gira libre por inercia").

Velocidad definida por dos constantes: `VELOCIDAD_NORMAL` y `VELOCIDAD_TURBO`, aplicadas según el estado del botón 2.

### 3.5 Estructura del `loop()`
1. Si hay botón 1 presionado (con antirrebote) → bocina.
2. Si hay botón 2 presionado (flanco) → alternar turbo.
3. Si `Serial.available()` → leer un carácter → ejecutar el comando correspondiente → parpadeo LED 2.
4. (No hay más lógica: es intencionalmente lineal y sin estados, porque el control remoto no lo necesita.)

---

## 4. Programa Mini Sumo

### 4.1 Resumen funcional
Robot autónomo que debe permanecer dentro del ring, buscar al rival con el sensor ultrasónico y empujarlo hacia afuera, evitando salirse él mismo cuando detecta el borde/línea blanca con los tres sensores de línea.

### 4.2 Máquina de estados

```
        [Botón 1 presionado]
ESPERA ───────────────────────► CUENTA_REGRESIVA
  ▲                                    │ (5 bips, ~5s reglamentarios,
  │ [Botón 2 = parada de emergencia]   │  motores detenidos)
  │                                    ▼
  └──────────────────────────────  BUSQUEDA ◄──────────────┐
                                    │      ▲                 │
                     [rival detectado]   [rival perdido]     │
                                    ▼      │                 │
                                  ATAQUE ──┘                 │
                                                              │
              (en CUALQUIER estado activo, con prioridad     │
               máxima) [borde detectado] ──► EVASION ────────┘
                                          (retrocede + gira,
                                           maniobra corta y fija)
```

- **ESPERA:** motores parados, LED 1 apagado. Esperando que se apriete el botón 1 para armar el robot (así los alumnos lo pueden ubicar en el ring sin que arranque solo).
- **CUENTA_REGRESIVA:** 5 bips de buzzer + parpadeo de LED 1, uno por segundo (cuenta regresiva reglamentaria de mini sumo antes de que el robot pueda moverse). Motores detenidos todo este tiempo.
- **BUSQUEDA:** el robot gira en el lugar lentamente mientras mide distancia con el ultrasónico en cada vuelta de `loop()`. LED 2 parpadea. Sin servo (el sensor es fijo), por eso "buscar" = girar hasta que el sensor "vea" algo dentro del rango.
- **ATAQUE:** rival detectado dentro de `DISTANCIA_DETECCION_CM` → avanza recto a fondo. LED 2 fijo. Si el rival deja de estar en rango, vuelve a BUSQUEDA.
- **EVASION:** cualquier sensor de borde se activa → retrocede un tiempo fijo corto y gira alejándose del lado que detectó el borde, luego vuelve a BUSQUEDA. LED 3 parpadea durante la maniobra.
- La verificación de borde se hace **al principio de cada vuelta de `loop()`, antes que cualquier otra lógica de estado**, porque no salirse del ring es más importante que atacar o buscar (regla de seguridad, no solo de puntaje).
- **Botón 2 (D12), en cualquier momento:** corta motores y vuelve a ESPERA (parada de emergencia/manual, para poder levantar el robot de la mesa sin tener que apagarlo).

### 4.3 Sensores

**Ultrasónico (HC-SR04, D6=Trigger/D5=Echo):** función `medirDistanciaCM()` con `pulseIn()` y timeout (para no bloquear el programa si no hay eco). Devuelve un número grande o -1 si no hay lectura válida, que se trata como "no hay rival visible".

**Sensores de borde (A1/A2/A3):** se nombran `SENSOR_BORDE_IZQUIERDO` (A1), `SENSOR_BORDE_DERECHO` (A2) y `SENSOR_BORDE_FRONTAL` (A3) — nombres que asumen una disposición típica (dos en las esquinas delanteras, uno al centro/frente), **pero la disposición física real depende de cómo cada grupo armó su chasis**. Hay un comentario bien visible instruyendo a verificar/ajustar estos tres `#define` contra el armado real antes de competir.

Lectura por `analogRead()` comparado contra `UMBRAL_BORDE` (no `digitalRead`, para que sirva tanto con sensores de salida analógica como con los de salida digital con comparador integrado). **Hay que calibrar este umbral en la cancha real**: el programa incluye un modo de depuración (`MODO_DEBUG_SENSORES`, una constante `bool` arriba del todo) que, si está en `true`, imprime por `Serial` los tres valores de borde cada medio segundo, para que los alumnos midan "valor sobre negro" vs. "valor sobre la línea blanca" con el Monitor Serie y fijen el umbral a mitad de camino entre ambos.

Dirección de evasión según qué sensor se activó:
- Izquierdo → retrocede y gira a la derecha.
- Derecho → retrocede y gira a la izquierda.
- Frontal (o más de uno a la vez) → retrocede y gira a la derecha (dirección por defecto).

### 4.4 Botones/LEDs/Buzzer — resumen
| Elemento | Uso |
|---|---|
| Botón 1 (D2) | Armar robot: ESPERA → CUENTA_REGRESIVA |
| Botón 2 (D12) | Parada de emergencia: cualquier estado → ESPERA |
| LED 1 (D4) | Parpadea en cuenta regresiva; fijo mientras el robot está armado/corriendo |
| LED 2 (D7) | Parpadea en BUSQUEDA; fijo en ATAQUE |
| LED 3 (D13) | Parpadea durante EVASION |
| Buzzer (D8) | 5 bips en CUENTA_REGRESIVA únicamente |

### 4.5 Sobre el uso de `delay()`
El programa evita `delay()` largos porque cortan la lectura de sensores. Excepciones deliberadas y documentadas en el código:
- Los bips de la cuenta regresiva (el robot no se mueve en ese estado, no hay nada que monitorear).
- La maniobra de evasión (retroceder + girar): es corta (cientos de milisegundos), fija, y su objetivo es precisamente alejarse del borde sin depender de sensores durante esos instantes — es el patrón estándar en robots de mini sumo de nivel introductorio.

### 4.6 Constantes de calibración (agrupadas arriba del sketch)
```cpp
const int VELOCIDAD_BUSQUEDA   = ...; // PWM al girar buscando
const int VELOCIDAD_ATAQUE     = ...; // PWM al embestir
const int VELOCIDAD_EVASION    = ...; // PWM al retroceder/girar en evasión
const int DISTANCIA_DETECCION_CM = 30; // rango del ultrasonido para considerar "rival visible"
const int UMBRAL_BORDE         = ...; // AJUSTAR EN CANCHA — ver 4.3
const unsigned long DURACION_EVASION_MS = ...; // duración de la maniobra de escape
const bool MODO_DEBUG_SENSORES = false; // true = imprime lecturas de borde por Serial
```

---

## 5. Consideraciones comunes de implementación

- **Antirrebote de botones:** lectura por flanco (guardar el estado anterior, comparar) + `INPUT_PULLUP` (se asume que SW2/SW3 conectan el pin a GND al presionar, como es habitual en este tipo de shield económico; si no responden, revisar si en la placa están cableados al revés).
- **PWM:** `analogWrite()` estándar de Arduino, 0-255, en los 4 pines de motor (D3, D9, D10, D11, todos con PWM disponible según el esquemático).
- **`Serial.begin(9600)`** en ambos sketches (velocidad por defecto de fábrica del HC-05; en Mini Sumo se usa solo para depuración por USB, nunca para Bluetooth).
- **No se usan interrupciones ni `millis()` como reloj maestro** salvo donde se indica (cuenta regresiva y parpadeos usan `millis()` no bloqueante; ver 4.5 para las excepciones con `delay()`).
- **Validación:** no se dispone de `arduino-cli` en este entorno, así que la compilación final se debe verificar en el Arduino IDE (o `arduino-cli` si el usuario lo instala) antes de subir a la placa real.

---

## 6. Estructura de archivos del repositorio

```
robotshield/
├── ROBOTSHIELD_V2.1_CONTEXTO_TECNICO.md
├── docs/
│   └── FSD.md                      (este documento)
├── hardware/
│   └── ...                         (ya existente)
└── src/
    ├── Futbol/
    │   └── Futbol.ino
    └── MiniSumo/
        └── MiniSumo.ino
```

(Cada sketch va en su propia carpeta porque el Arduino IDE exige que el nombre del archivo `.ino` coincida con el de la carpeta que lo contiene.)

---

## 7. Plan de trabajo

1. ✅ Confirmar mapa de pines definitivo (resuelto en `ROBOTSHIELD_V2.1_CONTEXTO_TECNICO.md`).
2. ✅ Definir protocolo Bluetooth, estructura de estados de Mini Sumo, idioma y layout de archivos (este documento).
3. Escribir `src/Futbol/Futbol.ino`.
4. Escribir `src/MiniSumo/MiniSumo.ino`.
5. Revisión de lectura cruzada: ¿un alumno de 16 años que nunca programó un robot puede seguir el flujo leyendo de arriba a abajo?
6. Entrega al usuario con checklist de qué falta verificar en la placa física antes de la primera carga (desconectar HC-05, calibrar `UMBRAL_BORDE`, confirmar asignación motor izq/der).

---

## 8. Checklist de puesta en marcha (para los alumnos)

- [ ] Verificar que la batería no supere los 10V.
- [ ] Desconectar el HC-05 antes de subir el sketch de Fútbol por USB.
- [ ] Después de subir el sketch, probar cada motor por separado (¿el robot dobla para el lado esperado? Si no, invertir la asignación izquierda/derecha en el código).
- [ ] Mini Sumo: con `MODO_DEBUG_SENSORES = true`, abrir el Monitor Serie y anotar los valores de los 3 sensores de borde sobre la superficie negra y sobre la línea blanca; fijar `UMBRAL_BORDE` a mitad de camino; volver a poner `MODO_DEBUG_SENSORES = false` antes de competir.
- [ ] Mini Sumo: confirmar que `SENSOR_BORDE_IZQUIERDO`/`DERECHO`/`FRONTAL` corresponden a la ubicación física real de cada sensor en el chasis.
- [ ] Probar el botón de parada de emergencia (D12 en Mini Sumo) antes de la primera prueba en cancha.
