# Robotshield V2.1 — ESETP N°703

Proyecto de programación de robots educativos sobre la placa **Robotshield V2.1** (shield para Arduino UNO). Cada grupo arma uno de estos dos robots y le carga su programa correspondiente:

- 🎮 **Fútbol** — controlado a distancia por Bluetooth desde el celular.
- 🤖 **Mini Sumo** — autónomo, busca al rival y evita salirse del ring.

## Qué hay en este repositorio

```
robotshield/
├── ROBOTSHIELD_V2.1_CONTEXTO_TECNICO.md   → mapa de pines y hardware de la placa
├── docs/
│   └── FSD.md                             → diseño completo de los dos programas
├── hardware/                              → esquemático, PCB y BOM (KiCad)
└── src/
    ├── Futbol/Futbol.ino                  → programa del robot de Fútbol
    └── MiniSumo/MiniSumo.ino              → programa del robot de Mini Sumo
```

Si algo de este README no alcanza para entender una decisión de diseño (por qué un pin es tal cosa, por qué el robot se comporta así), la explicación completa está en **`docs/FSD.md`**.

---

## 1. Lo que necesitás antes de empezar

- **Arduino IDE** (versión 2.x) instalado en la notebook del grupo. Descarga: https://www.arduino.cc/en/software
- Cable USB para conectar el Arduino UNO a la notebook.
- El Robotshield ya armado y con el DRV8833 enchufado (J5/J6).
- Batería para el robot — **⚠️ no debe superar los 10V** (ver `ROBOTSHIELD_V2.1_CONTEXTO_TECNICO.md`).
- **Fútbol:** módulo HC-05 enchufado en J4, y en el celular alguna app de terminal Bluetooth (o joystick simple que mande caracteres por Bluetooth clásico — no BLE).
- **Mini Sumo:** sensor HC-SR04 en J2 y los tres sensores de línea/borde en J9/J10/J11.

No hace falta instalar ninguna librería adicional: los dos programas usan solamente funciones estándar de Arduino.

---

## 2. Configurar el Arduino IDE (una sola vez por notebook)

1. Abrir el Arduino IDE.
2. Conectar el Arduino UNO por USB.
3. Arriba, en el selector de placa, elegir **Arduino Uno**.
4. Al lado, elegir el **puerto serie** donde apareció el Arduino (en Windows suele ser `COMx`, en Linux/Mac `/dev/ttyUSBx` o `/dev/cu.usbmodemxxxx`).

Si el Arduino no aparece en la lista de puertos, probar con otro cable USB (muchos cables solo cargan y no transmiten datos) o reinstalar el driver CH340/FTDI según el clon de Arduino que tengan.

---

## 3. Robot de Fútbol

### 3.1 Antes de subir el programa

**⚠️ Desconectar el HC-05 del Robotshield antes de subir el sketch por USB.** El módulo Bluetooth está cableado directo a los pines D0/D1, que son los mismos que usa el Arduino para programarse por USB — si el HC-05 queda enchufado, la carga va a fallar o quedarse colgada.

### 3.2 Subir el sketch

1. Abrir `src/Futbol/Futbol.ino` con el Arduino IDE (doble clic al archivo).
2. Con el HC-05 desconectado, hacer clic en **Subir** (flecha →).
3. Esperar el mensaje de "Subida completa".
4. **Volver a conectar el HC-05** al Robotshield.

### 3.3 Probar el control remoto

1. En el celular, emparejar el HC-05 por Bluetooth (contraseña por defecto suele ser `1234` o `0000`).
2. Abrir la app de terminal Bluetooth y conectarse al HC-05.
3. Mandar estos caracteres, uno por vez, y verificar que el robot responda:

| Carácter | Acción esperada |
|---|---|
| `F` | Avanza |
| `B` | Retrocede |
| `L` | Gira a la izquierda |
| `R` | Gira a la derecha |
| `S` | Se detiene |
| `H` | Suena la bocina |

4. Si el robot dobla para el lado contrario al que esperaban, es la asignación de motor izquierdo/derecho la que está invertida. En el código, cerca del principio, están los `#define PIN_MOTOR_IZQ_A` etc. — ver el comentario ahí mismo para saber cómo corregirlo.
5. El botón físico en D2 (SW2) hace sonar la bocina, y el botón en D12 (SW3) activa/desactiva el modo turbo (más velocidad, LED D13 prendido mientras está activo).

Más detalle del protocolo y del diseño en `docs/FSD.md`, sección 3.

---

## 4. Robot de Mini Sumo

### 4.1 Subir el sketch

1. Abrir `src/MiniSumo/MiniSumo.ino` con el Arduino IDE.
2. Conectar el Arduino por USB y subirlo con **Subir** (no hace falta desconectar nada para este robot).

### 4.2 Calibrar los sensores de borde (obligatorio antes de competir)

Los sensores de línea/borde vienen con un umbral de fábrica en el código (`UMBRAL_BORDE`) que **casi seguro no va a servir tal cual** para la superficie real de la cancha. Hay que calibrarlo:

1. En el código, cambiar la línea `const bool MODO_DEBUG_SENSORES = false;` a `true`.
2. Volver a subir el sketch.
3. Abrir el **Monitor Serie** del Arduino IDE (velocidad 9600).
4. Con el robot apoyado sobre la superficie negra de la cancha, anotar los tres valores que aparecen.
5. Apoyar el robot sobre la línea blanca del borde y anotar los tres valores de nuevo.
6. Elegir un valor de `UMBRAL_BORDE` que quede a mitad de camino entre lo que midieron sobre negro y sobre blanco.
7. Volver a poner `MODO_DEBUG_SENSORES` en `false` y subir el sketch de nuevo (con el Monitor Serie abierto el robot gasta tiempo mandando texto en vez de reaccionar rápido — para competir tiene que quedar en `false`).

### 4.3 Verificar la ubicación de los sensores

En el código, `SENSOR_BORDE_IZQUIERDO`, `SENSOR_BORDE_DERECHO` y `SENSOR_BORDE_FRONTAL` asumen dónde está montado cada sensor en el chasis. Confirmar que coincide con cómo armó cada grupo su robot; si no, cambiar los pines (A1/A2/A3) en esos `#define` para que coincidan con la ubicación real.

### 4.4 Cómo correrlo

1. Apoyar el robot en el ring.
2. Apretar el botón D2 (SW2) para armarlo.
3. El robot va a hacer una cuenta regresiva de 5 bips (no se mueve todavía) y después arranca a buscar solo.
4. El botón D12 (SW3) funciona en cualquier momento como **parada de emergencia**: lo detiene y lo vuelve a dejar esperando. Probarlo antes de la primera prueba en cancha.

Más detalle de la máquina de estados (búsqueda, ataque, evasión) en `docs/FSD.md`, sección 4.

---

## 5. Problemas comunes

| Problema | Posible causa |
|---|---|
| La carga del sketch de Fútbol falla o se cuelga | El HC-05 sigue conectado — desenchufarlo y volver a intentar |
| El robot no aparece en la lista de puertos | Cable USB solo de carga, o falta el driver del chip USB-serie |
| El robot dobla para el lado contrario | Asignación motor izquierdo/derecho invertida — ver comentario en el código junto a los `#define` de motores |
| El Mini Sumo se sale del ring o no reacciona a la línea | `UMBRAL_BORDE` mal calibrado, o los sensores están asignados al pin equivocado — repetir el paso 4.2 |
| El HC-05 no aparece para emparejar | Verificar alimentación (LED del HC-05 titilando) y que esté en J4 con la polaridad correcta |

Para cualquier otra duda sobre el diseño del código (por qué está estructurado así, qué hace cada función), la referencia completa es `docs/FSD.md`.
