/*
  ROBOTSHIELD V2.1 - Robot de FUTBOL (control remoto por Bluetooth)
  ESETP N°703 - Puerto Madryn

  El celular (con cualquier app de terminal Bluetooth o joystick simple)
  manda UN CARACTER por vez al modulo HC-05. El Arduino lo lee por Serial
  y mueve los motores segun el comando recibido.

  Comandos:
    F = avanzar        B = retroceder
    L = girar izquierda R = girar derecha
    S = detener         H = bocina

  IMPORTANTE: el HC-05 esta conectado directo a los pines D0/D1 (Serial).
  Hay que DESCONECTAR el HC-05 antes de subir este sketch por USB, o la
  carga va a fallar.

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

#define PIN_BOTON_BOCINA 2   // SW2 - bocina local
#define PIN_BOTON_TURBO  12  // SW3 - alterna modo turbo

#define PIN_LED_ENCENDIDO 4   // fijo mientras el programa corre
#define PIN_LED_ACTIVIDAD 7   // parpadea al recibir un comando
#define PIN_LED_TURBO     13  // fijo mientras el turbo esta activo

#define PIN_BUZZER 8

// ===================== CONSTANTES DE CALIBRACION =====================

const int VELOCIDAD_NORMAL = 150;  // PWM (0-255) en modo normal
const int VELOCIDAD_TURBO  = 255;  // PWM (0-255) en modo turbo

// ===================== VARIABLES DE ESTADO =====================

bool modoTurbo = false;
bool estadoAnteriorBotonTurbo = HIGH;  // para detectar el flanco de apretar

// ===================== SETUP =====================

void setup() {
  pinMode(PIN_MOTOR_IZQ_A, OUTPUT);
  pinMode(PIN_MOTOR_IZQ_B, OUTPUT);
  pinMode(PIN_MOTOR_DER_A, OUTPUT);
  pinMode(PIN_MOTOR_DER_B, OUTPUT);

  pinMode(PIN_BOTON_BOCINA, INPUT_PULLUP);
  pinMode(PIN_BOTON_TURBO, INPUT_PULLUP);

  pinMode(PIN_LED_ENCENDIDO, OUTPUT);
  pinMode(PIN_LED_ACTIVIDAD, OUTPUT);
  pinMode(PIN_LED_TURBO, OUTPUT);

  pinMode(PIN_BUZZER, OUTPUT);

  Serial.begin(9600);  // velocidad de fabrica del HC-05

  detenerMotores();
  digitalWrite(PIN_LED_ENCENDIDO, HIGH);
  tone(PIN_BUZZER, 1500, 150);  // bip corto de "listo"
}

// ===================== LOOP PRINCIPAL =====================

void loop() {
  revisarBotonBocina();
  revisarBotonTurbo();
  revisarBluetooth();
}

// ===================== BOTONES =====================

void revisarBotonBocina() {
  if (digitalRead(PIN_BOTON_BOCINA) == LOW) {
    tocarBocina();
  }
}

void revisarBotonTurbo() {
  bool estadoActual = digitalRead(PIN_BOTON_TURBO);

  // Solo actuar en el flanco: el instante en que el boton pasa de
  // "no apretado" a "apretado". Asi un toque = un solo cambio de modo.
  if (estadoActual == LOW && estadoAnteriorBotonTurbo == HIGH) {
    modoTurbo = !modoTurbo;
    digitalWrite(PIN_LED_TURBO, modoTurbo ? HIGH : LOW);
    delay(50);  // antirrebote simple
  }

  estadoAnteriorBotonTurbo = estadoActual;
}

// ===================== BLUETOOTH =====================

void revisarBluetooth() {
  if (!Serial.available()) {
    return;
  }

  char comando = Serial.read();
  digitalWrite(PIN_LED_ACTIVIDAD, HIGH);

  switch (comando) {
    case 'F':
      avanzar();
      break;
    case 'B':
      retroceder();
      break;
    case 'L':
      girarIzquierda();
      break;
    case 'R':
      girarDerecha();
      break;
    case 'S':
      detenerMotores();
      break;
    case 'H':
      tocarBocina();
      break;
    default:
      // Caracter desconocido (por ejemplo saltos de linea): se ignora.
      break;
  }

  digitalWrite(PIN_LED_ACTIVIDAD, LOW);
}

// ===================== MOVIMIENTOS =====================

int velocidadActual() {
  return modoTurbo ? VELOCIDAD_TURBO : VELOCIDAD_NORMAL;
}

void avanzar() {
  motorIzquierdo(velocidadActual());
  motorDerecho(velocidadActual());
}

void retroceder() {
  motorIzquierdo(-velocidadActual());
  motorDerecho(-velocidadActual());
}

void girarIzquierda() {
  motorIzquierdo(-velocidadActual());
  motorDerecho(velocidadActual());
}

void girarDerecha() {
  motorIzquierdo(velocidadActual());
  motorDerecho(-velocidadActual());
}

void detenerMotores() {
  motorIzquierdo(0);
  motorDerecho(0);
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

// ===================== BOCINA =====================

void tocarBocina() {
  tone(PIN_BUZZER, 2000, 200);
}
