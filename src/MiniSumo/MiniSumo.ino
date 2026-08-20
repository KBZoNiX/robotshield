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

bool estadoAnteriorBotonArmar = HIGH;

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

// ===================== ESTADO: ESPERA =====================

void entrarEnEspera() {
  estadoActual = ESPERA;
  detenerMotores();
  digitalWrite(PIN_LED_ARMADO, LOW);
  digitalWrite(PIN_LED_BUSQUEDA, LOW);
  digitalWrite(PIN_LED_EVASION, LOW);
}

void manejarEspera() {
  bool estadoActualBoton = digitalRead(PIN_BOTON_ARMAR);

  if (estadoActualBoton == LOW && estadoAnteriorBotonArmar == HIGH) {
    delay(50);  // antirrebote simple
    estadoActual = CUENTA_REGRESIVA;
  }

  estadoAnteriorBotonArmar = estadoActualBoton;
}

// ===================== ESTADO: CUENTA_REGRESIVA =====================
//
// El robot todavia no se mueve, asi que no hace falta revisar sensores
// aca: un delay() bloqueante por bip es aceptable.

void manejarCuentaRegresiva() {
  for (int bip = 0; bip < CANTIDAD_BIPS_CUENTA_REGRESIVA; bip++) {
    digitalWrite(PIN_LED_ARMADO, HIGH);
    tone(PIN_BUZZER, 2000, 150);
    delay(DURACION_BIP_CUENTA_REGRESIVA_MS / 2);
    digitalWrite(PIN_LED_ARMADO, LOW);
    delay(DURACION_BIP_CUENTA_REGRESIVA_MS / 2);
  }

  digitalWrite(PIN_LED_ARMADO, HIGH);
  estadoActual = BUSQUEDA;
}

// ===================== ESTADO: BUSQUEDA =====================

void manejarBusqueda() {
  parpadearLed(PIN_LED_BUSQUEDA);

  girarBuscando();

  if (hayRivalDetectado()) {
    estadoActual = ATAQUE;
    digitalWrite(PIN_LED_BUSQUEDA, HIGH);
  }
}

void girarBuscando() {
  // Sin servo, el sensor ultrasonico apunta siempre al frente: "buscar"
  // es girar en el lugar hasta que el sensor detecte algo.
  motorIzquierdo(VELOCIDAD_BUSQUEDA);
  motorDerecho(-VELOCIDAD_BUSQUEDA);
}

// ===================== ESTADO: ATAQUE =====================

void manejarAtaque() {
  digitalWrite(PIN_LED_BUSQUEDA, HIGH);

  if (hayRivalDetectado()) {
    motorIzquierdo(VELOCIDAD_ATAQUE);
    motorDerecho(VELOCIDAD_ATAQUE);
  } else {
    estadoActual = BUSQUEDA;
  }
}

// ===================== ESTADO: EVASION =====================

void manejarEvasion() {
  estadoActual = EVASION;
  digitalWrite(PIN_LED_EVASION, HIGH);

  bool bordeIzquierdo = lecturaBorde(SENSOR_BORDE_IZQUIERDO) > UMBRAL_BORDE;
  bool bordeDerecho = lecturaBorde(SENSOR_BORDE_DERECHO) > UMBRAL_BORDE;

  // Retroceder primero, alejandose del borde en linea recta.
  motorIzquierdo(-VELOCIDAD_EVASION);
  motorDerecho(-VELOCIDAD_EVASION);
  delay(DURACION_RETROCESO_EVASION_MS);

  // Girar alejandose del lado que detecto el borde. Si fue el frontal,
  // o los dos a la vez, se usa una direccion por defecto (derecha).
  if (bordeIzquierdo && !bordeDerecho) {
    motorIzquierdo(VELOCIDAD_EVASION);
    motorDerecho(-VELOCIDAD_EVASION);
  } else {
    motorIzquierdo(-VELOCIDAD_EVASION);
    motorDerecho(VELOCIDAD_EVASION);
  }
  delay(DURACION_GIRO_EVASION_MS);

  digitalWrite(PIN_LED_EVASION, LOW);
  estadoActual = BUSQUEDA;
}

// ===================== SENSORES: BORDE / LINEA =====================

int lecturaBorde(int pinSensor) {
  return analogRead(pinSensor);
}

bool hayBordeDetectado() {
  return lecturaBorde(SENSOR_BORDE_IZQUIERDO) > UMBRAL_BORDE ||
         lecturaBorde(SENSOR_BORDE_DERECHO) > UMBRAL_BORDE ||
         lecturaBorde(SENSOR_BORDE_FRONTAL) > UMBRAL_BORDE;
}

void imprimirLecturasDeBorde() {
  static unsigned long ultimaImpresion = 0;

  if (millis() - ultimaImpresion < 500) {
    return;
  }
  ultimaImpresion = millis();

  Serial.print("Borde izq: ");
  Serial.print(lecturaBorde(SENSOR_BORDE_IZQUIERDO));
  Serial.print("  Borde der: ");
  Serial.print(lecturaBorde(SENSOR_BORDE_DERECHO));
  Serial.print("  Borde frontal: ");
  Serial.println(lecturaBorde(SENSOR_BORDE_FRONTAL));
}

// ===================== SENSOR: ULTRASONICO (RIVAL) =====================

bool hayRivalDetectado() {
  int distancia = medirDistanciaCM();
  return distancia > 0 && distancia <= DISTANCIA_DETECCION_CM;
}

int medirDistanciaCM() {
  digitalWrite(PIN_ULTRA_TRIGGER, LOW);
  delayMicroseconds(2);
  digitalWrite(PIN_ULTRA_TRIGGER, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_ULTRA_TRIGGER, LOW);

  // Timeout de 20ms (~3.4m de alcance) para no bloquear el programa si
  // no llega eco.
  unsigned long duracionPulso = pulseIn(PIN_ULTRA_ECHO, HIGH, 20000);

  if (duracionPulso == 0) {
    return -1;  // no hubo eco: no hay nada detectado
  }

  return duracionPulso / 58;  // formula estandar del HC-SR04, resultado en cm
}

// ===================== BOTON DE EMERGENCIA =====================

bool botonEmergenciaPresionado() {
  return digitalRead(PIN_BOTON_EMERGENCIA) == LOW;
}

// ===================== CONTROL DE MOTORES (DRV8833) =====================
//
// Cada motor se maneja con dos pines "A" y "B". Para el driver DRV8833:
//   - Adelante:     A = PWM (velocidad), B = LOW
//   - Atras:        A = LOW, B = PWM (velocidad)
//   - Punto muerto: A = LOW, B = LOW (sin corriente, la rueda gira libre)
//
// velocidad va de -255 a 255: positivo = adelante, negativo = atras,
// 0 = detenido.

void motorIzquierdo(int velocidad) {
  aplicarVelocidadMotor(PIN_MOTOR_IZQ_A, PIN_MOTOR_IZQ_B, velocidad);
}

void motorDerecho(int velocidad) {
  aplicarVelocidadMotor(PIN_MOTOR_DER_A, PIN_MOTOR_DER_B, velocidad);
}

void aplicarVelocidadMotor(int pinA, int pinB, int velocidad) {
  if (velocidad > 0) {
    analogWrite(pinA, velocidad);
    digitalWrite(pinB, LOW);
  } else if (velocidad < 0) {
    digitalWrite(pinA, LOW);
    analogWrite(pinB, -velocidad);
  } else {
    digitalWrite(pinA, LOW);
    digitalWrite(pinB, LOW);
  }
}

void detenerMotores() {
  motorIzquierdo(0);
  motorDerecho(0);
}

// ===================== UTILIDAD: LED PARPADEANTE =====================

void parpadearLed(int pin) {
  static unsigned long ultimoCambio = 0;
  static bool encendido = false;

  if (millis() - ultimoCambio < 200) {
    return;
  }
  ultimoCambio = millis();

  encendido = !encendido;
  digitalWrite(pin, encendido ? HIGH : LOW);
}
