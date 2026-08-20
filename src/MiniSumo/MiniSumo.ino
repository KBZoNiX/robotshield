/*
  ROBOTSHIELD V2.1 - Robot de MINI SUMO (autonomo)
  ESETP N°703 - Puerto Madryn

  El robot funciona como una maquina de estados:

    ESPERA -> (boton 1) -> CUENTA_REGRESIVA -> BUSQUEDA <-> ATAQUE
                                                    |    ^
                                                    v    |
                                                 EVASION

  - ESPERA: parado, esperando que aprieten el boton 1 para armarlo.
  - CUENTA_REGRESIVA: 5 bips (~5 segundos reglamentarios) sin moverse.
  - BUSQUEDA: gira en el lugar buscando al rival con el sensor ultrasonico.
  - ATAQUE: encontro al rival, avanza recto a fondo.
  - EVASION: algun sensor de borde detecto el limite del ring, retrocede
    y gira para alejarse. Se revisa en CADA vuelta del loop, antes que
    cualquier otra cosa, porque no salirse del ring es lo mas importante.

  El boton 2 funciona en cualquier momento como parada de emergencia y
  vuelve el robot a ESPERA.

  Este sketch esta dividido en varias pestañas (archivos .ino) dentro de
  esta misma carpeta. El IDE de Arduino las junta todas en un solo
  programa al compilar, asi que las funciones y variables de una pestaña
  se pueden usar en cualquier otra sin necesidad de #include:
    MiniSumo.ino        - este archivo: configuracion, enum Estado y loop()
    MaquinaDeEstado.ino - que hace el robot en cada estado
    Sensores.ino        - lectura "cruda" de sensores (borde, ultrasonico, boton de emergencia)
    Motores.ino         - control de los motores (driver DRV8833, igual que en Futbol)
    Extras.ino          - funciones sueltas (LED parpadeante)

  El enum Estado y la variable estadoActual quedan declarados en este
  archivo, y no en MaquinaDeEstado.ino, aunque son parte de la logica de
  decision: este es el archivo "principal" del sketch (el IDE siempre lo
  compila primero) y el switch de loop(), mas abajo, los necesita ya
  declarados.

  Ver docs/FSD.md para el detalle completo del diseño.
*/

// ===================== PINES =====================

// Motores (driver DRV8833). Motor 1 = motor izquierdo, Motor 2 = motor
// derecho. Si el robot dobla para el lado contrario al esperado, es la
// asignacion izquierda/derecha la que esta invertida: cambiar aca.
#define PIN_MOTOR_IZQ_A 9   // Motor 1A
#define PIN_MOTOR_IZQ_B 10  // Motor 1B
#define PIN_MOTOR_DER_A 3   // Motor 2A
#define PIN_MOTOR_DER_B 11  // Motor 2B

#define PIN_BOTON_ARMAR       2   // SW2 - arma el robot (ESPERA -> cuenta)
#define PIN_BOTON_EMERGENCIA  12  // SW3 - parada de emergencia

#define PIN_LED_ARMADO    4   // parpadea en cuenta regresiva, fijo si esta corriendo
#define PIN_LED_BUSQUEDA  7   // parpadea buscando, fijo si encontro al rival
#define PIN_LED_EVASION   13  // parpadea durante la maniobra de evasion

#define PIN_BUZZER 8

#define PIN_ULTRA_TRIGGER 6
#define PIN_ULTRA_ECHO    5

// Sensores de borde/linea. OJO: estos nombres asumen una disposicion
// tipica (dos en las esquinas delanteras, uno al frente/centro), pero
// la ubicacion real depende de como cada grupo armo el chasis.
// VERIFICAR y ajustar estos tres #define contra el armado real antes
// de competir.
#define SENSOR_BORDE_IZQUIERDO A1
#define SENSOR_BORDE_DERECHO   A2
#define SENSOR_BORDE_FRONTAL   A3

// ===================== CONSTANTES DE CALIBRACION =====================

const int VELOCIDAD_BUSQUEDA = 130;  // PWM al girar buscando al rival
const int VELOCIDAD_ATAQUE   = 255;  // PWM al embestir
const int VELOCIDAD_EVASION  = 180;  // PWM al retroceder/girar escapando del borde

const int DISTANCIA_DETECCION_CM = 30;  // rango del ultrasonido para "rival visible"

// AJUSTAR EN CANCHA: valor de analogRead() a partir del cual se
// considera que un sensor de borde "ve" la linea blanca. Poner
// MODO_DEBUG_SENSORES en true, abrir el Monitor Serie y anotar los
// valores sobre la superficie negra y sobre la linea blanca; el
// umbral va a mitad de camino entre ambos.
const int UMBRAL_BORDE = 500;
const bool MODO_DEBUG_SENSORES = false;

const unsigned long DURACION_RETROCESO_EVASION_MS = 300;
const unsigned long DURACION_GIRO_EVASION_MS = 300;
const unsigned long DURACION_BIP_CUENTA_REGRESIVA_MS = 1000;
const int CANTIDAD_BIPS_CUENTA_REGRESIVA = 5;

// ===================== ESTADOS =====================

enum Estado {
  ESPERA,
  CUENTA_REGRESIVA,
  BUSQUEDA,
  ATAQUE,
  EVASION
};

Estado estadoActual = ESPERA;

// ===================== SETUP =====================

void setup() {
  pinMode(PIN_MOTOR_IZQ_A, OUTPUT);
  pinMode(PIN_MOTOR_IZQ_B, OUTPUT);
  pinMode(PIN_MOTOR_DER_A, OUTPUT);
  pinMode(PIN_MOTOR_DER_B, OUTPUT);

  pinMode(PIN_BOTON_ARMAR, INPUT_PULLUP);
  pinMode(PIN_BOTON_EMERGENCIA, INPUT_PULLUP);

  pinMode(PIN_LED_ARMADO, OUTPUT);
  pinMode(PIN_LED_BUSQUEDA, OUTPUT);
  pinMode(PIN_LED_EVASION, OUTPUT);

  pinMode(PIN_BUZZER, OUTPUT);

  pinMode(PIN_ULTRA_TRIGGER, OUTPUT);
  pinMode(PIN_ULTRA_ECHO, INPUT);

  // Los sensores de borde se leen con analogRead(), no hace falta pinMode.

  if (MODO_DEBUG_SENSORES) {
    Serial.begin(9600);
  }

  detenerMotores();
  entrarEnEspera();
}

// ===================== LOOP PRINCIPAL =====================

void loop() {
  if (MODO_DEBUG_SENSORES) {
    imprimirLecturasDeBorde();
  }

  // La parada de emergencia tiene prioridad absoluta, en cualquier estado.
  if (botonEmergenciaPresionado()) {
    entrarEnEspera();
    return;
  }

  switch (estadoActual) {
    case ESPERA:
      manejarEspera();
      break;
    case CUENTA_REGRESIVA:
      manejarCuentaRegresiva();
      break;
    case BUSQUEDA:
    case ATAQUE:
      // En BUSQUEDA y ATAQUE, lo primero es siempre chequear el borde:
      // no salirse del ring es mas importante que buscar o atacar.
      if (hayBordeDetectado()) {
        manejarEvasion();
      } else if (estadoActual == BUSQUEDA) {
        manejarBusqueda();
      } else {
        manejarAtaque();
      }
      break;
    default:
      // EVASION nunca queda "activo" de una vuelta de loop() a la
      // siguiente: manejarEvasion() hace toda la maniobra de una vez
      // (ver 4.5 del FSD) y termina dejando estadoActual en BUSQUEDA.
      break;
  }
}
