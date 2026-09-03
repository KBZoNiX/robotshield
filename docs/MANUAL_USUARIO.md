# Manual de usuario — Robotshield V2.1

Guía corta de los tres programas del repositorio. Para instalación del Arduino IDE y primeros pasos, ver `README.md`; para el detalle de diseño, `docs/FSD.md`.

## 1. Fútbol (`src/Futbol/Futbol.ino`)

Control remoto por Bluetooth (HC-05). El celular manda un carácter por vez y el robot lo traduce en movimiento:

| Carácter | Acción |
|---|---|
| `F` | Avanzar |
| `B` | Retroceder |
| `L` | Girar izquierda |
| `R` | Girar derecha |
| `S` | Detener |
| `H` | Bocina |

Además: botón D2 = bocina física, botón D12 = alterna modo turbo (más velocidad, LED D13 fijo mientras está activo).

**Recordar:** desconectar el HC-05 antes de subir el sketch por USB (comparte pines D0/D1 con la programación).

## 2. Mini Sumo (`src/MiniSumo/MiniSumo.ino`)

Autónomo, máquina de estados:

```
ESPERA → (botón D2) → CUENTA_REGRESIVA → BUSQUEDA ⇄ ATAQUE
                                              ↕
                                          EVASION
```

- **ESPERA:** parado, esperando el botón D2 para armarse.
- **CUENTA_REGRESIVA:** 5 bips (~5 s reglamentarios), sin moverse.
- **BUSQUEDA:** gira en el lugar hasta que el sensor ultrasónico detecta al rival.
- **ATAQUE:** avanza recto a fondo mientras el rival sigue detectado.
- **EVASION:** algún sensor de borde vio el límite del ring → retrocede y gira para alejarse, después vuelve a BUSQUEDA. Se revisa en cada vuelta del loop con prioridad absoluta.

El botón D12 es parada de emergencia en cualquier momento (vuelve a ESPERA).

## 3. Test de placa (`src/TestPlaca/TestPlaca.ino`)

Sketch de diagnóstico para probar el Robotshield ya soldado/cableado, **antes** de armar Fútbol o Mini Sumo. No depende de la lógica de ninguno de los dos robots. Con el Monitor Serie a 9600 baudios, corre automáticamente y en orden:

1. LEDs (D4, D7, D13).
2. Pulsadores (D2, D12) — 10 s de margen cada uno.
3. Sensor ultrasónico HC-SR04 (5 mediciones).
4. Motores (D3/D9/D10/D11) — cada uno adelante y atrás.

Al terminar, queda haciendo eco de lo que llegue por Serial o por el HC-05 (si lo reconectás), útil para confirmar que el módulo Bluetooth responde.

---

## FAQ

**¿Cómo arreglo la dirección de un motor (gira al revés de lo esperado)?**
En el `.ino` principal del sketch (`Futbol.ino` o `MiniSumo.ino`), cerca del inicio, están:
```c
#define PIN_MOTOR_IZQ_A 9
#define PIN_MOTOR_IZQ_B 10
#define PIN_MOTOR_DER_A 3
#define PIN_MOTOR_DER_B 11
```
- Si el robot **gira sobre sí mismo hacia el lado contrario** al esperado (izquierda/derecha invertidas), intercambiá el par completo `IZQ` ↔ `DER`.
- Si **un solo motor** gira al revés (adelante hace que esa rueda vaya atrás), intercambiá sus dos pines `A`/`B` entre sí (por ejemplo `PIN_MOTOR_IZQ_A` ↔ `PIN_MOTOR_IZQ_B`).
- La asignación motor 1/motor 2 = izquierda/derecha es arbitraria y depende del chasis armado por cada grupo — no hay un valor "correcto" universal.
- En Mini Sumo, este arreglo también se aplica en `TestPlaca.ino` si el diagnóstico ya mostró el motor invertido antes del armado final.

**¿Cómo cambio la velocidad de búsqueda o de ataque del Mini Sumo?**
En `MiniSumo.ino`:
```c
const int VELOCIDAD_BUSQUEDA = 100;  // PWM al girar buscando al rival
const int VELOCIDAD_ATAQUE   = 255;  // PWM al embestir
const int VELOCIDAD_EVASION  = 180;  // PWM al retroceder/girar escapando del borde
```
Son valores PWM de 0 a 255. Subir `VELOCIDAD_BUSQUEDA` hace que gire más rápido buscando (pero puede pasar de largo al rival); `VELOCIDAD_ATAQUE` normalmente conviene dejarlo a fondo (255).

**¿Cómo cambio la distancia a la que el Mini Sumo detecta al rival?**
En `MiniSumo.ino`:
```c
const int DISTANCIA_DETECCION_CM = 40;  // rango del ultrasonido para "rival visible"
```
Bajarlo hace que necesite tener al rival más cerca para entrar en ATAQUE; subirlo lo hace "ver" más lejos (máximo práctico del HC-SR04 ~3-4 m, pero en cancha conviene no pasar de 60-80 cm para no reaccionar a cosas fuera del ring).

**¿Cómo cambio la velocidad del Fútbol (normal y turbo)?**
En `Futbol/Motores.ino`:
```c
const int VELOCIDAD_NORMAL = 150;  // PWM (0-255) en modo normal
const int VELOCIDAD_TURBO  = 255;  // PWM (0-255) en modo turbo
```

**¿Cómo calibro los sensores de borde del Mini Sumo?**
1. En `MiniSumo.ino`, poner `MODO_DEBUG_SENSORES` en `true` y subir el sketch.
2. Abrir el Monitor Serie (9600 baudios).
3. Anotar los tres valores con el robot sobre la superficie negra, y de nuevo sobre la línea blanca.
4. Elegir `UMBRAL_BORDE` a mitad de camino entre ambos grupos de valores.
5. Volver `MODO_DEBUG_SENSORES` a `false` antes de competir (con el Monitor Serie abierto el robot reacciona más lento).

**¿Cómo verifico que los sensores de borde están asignados al pin correcto?**
`SENSOR_BORDE_IZQUIERDO`, `SENSOR_BORDE_DERECHO` y `SENSOR_BORDE_FRONTAL` (en `MiniSumo.ino`, pines A1/A2/A3) asumen una ubicación típica que puede no coincidir con el chasis real de cada grupo — confirmar contra el armado real y corregir el `#define` si hace falta.

**¿Por qué el Mini Sumo a veces vuelve a BUSQUEDA apenas empieza a atacar?**
El sensor ultrasónico a veces da una lectura suelta sin eco aunque el rival siga ahí. `manejarAtaque()` (en `MaquinaDeEstado.ino`) tolera hasta `PERDIDAS_PARA_VOLVER_A_BUSCAR` (3) lecturas fallidas seguidas antes de dar por perdido al rival — si el problema persiste, revisar el cableado del HC-SR04 antes de subir ese número.

**¿Por qué no puedo subir el sketch de Fútbol (o el de Test de placa) por USB?**
El HC-05 está enchufado y ocupa los pines D0/D1, los mismos que usa la carga por USB. Desconectarlo, subir el sketch, y volver a conectarlo recién después.

**¿Por qué el Mini Sumo se sale del ring o no evade a tiempo?**
Revisar, en orden: `UMBRAL_BORDE` mal calibrado (ver arriba), sensores en el pin equivocado (ver arriba), o `CHEQUEAR_BORDE` en `false` (sirve para probar sin los sensores conectados, pero **tiene que estar en `true`** antes de competir).
