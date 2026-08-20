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

  Este sketch esta dividido en varias pestañas (archivos .ino) dentro de
  esta misma carpeta. El IDE de Arduino las junta todas en un solo
  programa al compilar, asi que las funciones y variables de una pestaña
  se pueden usar en cualquier otra sin necesidad de #include:
    Futbol.ino    - este archivo: configuracion y logica de alto nivel
    Bluetooth.ino - lectura y traduccion de los comandos del HC-05
    Motores.ino   - control de los motores (driver DRV8833)
    Extras.ino    - bocina y boton de turbo

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

// ===================== VARIABLES DE ESTADO =====================

// modoTurbo se usa en mas de una pestaña: Extras.ino lo cambia (boton de
// turbo) y Motores.ino lo lee (para elegir la velocidad). Por eso queda
// declarada aca, en el archivo principal.
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
