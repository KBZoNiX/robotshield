/*
  ROBOTSHIELD V2.1 - Sketch de TEST DE PLACA
  ESETP N°703 - Puerto Madryn

  Sirve para probar el Robotshield ya soldado/cableado ANTES del armado
  final de cualquiera de los dos robots (Futbol o Mini Sumo): usa solo
  los pines comunes a ambas categorias (ver ROBOTSHIELD_V2.1_CONTEXTO_
  TECNICO.md, secciones 2 y 5).

  Que prueba:
    - LEDs (D4, D7, D13)
    - Pulsadores (D2, D12)
    - Sensor ultrasonico HC-SR04 (D5/D6)
    - Motores, driver DRV8833 (D3, D9, D10, D11)
    - Buzzer (D8) - como beep de arranque y de confirmacion
    - Modulo bluetooth HC-05 (D0/D1, Serial) - test de eco

  Como usarlo:
    1. Si el HC-05 esta conectado, DESCONECTARLO antes de subir el
       sketch por USB (comparte los pines D0/D1 con el Monitor Serie:
       si esta conectado, la carga por USB puede fallar).
    2. Subir el sketch y abrir el Monitor Serie a 9600 baudios.
    3. Los tests de LEDs, pulsadores, ultrasonico y motores corren
       una sola vez, automaticamente, narrados por el Monitor Serie.
       Para repetirlos, resetear la placa.
    4. Al terminar, el sketch queda esperando caracteres por Serial
       para el test de bluetooth (ver loop() mas abajo): si no tenes
       el HC-05 a mano todavia, podes probarlo escribiendo caracteres
       en el Monitor Serie. Si queres probar el HC-05 en si, reconectalo
       ahora y mandale caracteres desde una app de terminal bluetooth
       en el celular. Cada caracter recibido prende un LED, hace un
       beep corto, y se devuelve por el mismo canal (eco).
*/

// ===================== PINES =====================

#define PIN_BOTON_1        2   // SW2
#define PIN_MOTOR_2A       3   // Driver DRV8833 - Motor 2
#define PIN_LED_1          4
#define PIN_ULTRA_ECHO     5
#define PIN_ULTRA_TRIGGER  6
#define PIN_LED_2          7
#define PIN_BUZZER         8
#define PIN_MOTOR_1A       9   // Driver DRV8833 - Motor 1
#define PIN_MOTOR_1B       10  // Driver DRV8833 - Motor 1
#define PIN_MOTOR_2B       11  // Driver DRV8833 - Motor 2
#define PIN_BOTON_2        12  // SW3
#define PIN_LED_3          13

const int VELOCIDAD_TEST_MOTOR = 150;  // PWM moderado, alcanza para ver que gira

// ===================== SETUP =====================

void setup() {
  pinMode(PIN_BOTON_1, INPUT_PULLUP);
  pinMode(PIN_BOTON_2, INPUT_PULLUP);

  pinMode(PIN_LED_1, OUTPUT);
  pinMode(PIN_LED_2, OUTPUT);
  pinMode(PIN_LED_3, OUTPUT);

  pinMode(PIN_BUZZER, OUTPUT);

  pinMode(PIN_ULTRA_TRIGGER, OUTPUT);
  pinMode(PIN_ULTRA_ECHO, INPUT);

  pinMode(PIN_MOTOR_1A, OUTPUT);
  pinMode(PIN_MOTOR_1B, OUTPUT);
  pinMode(PIN_MOTOR_2A, OUTPUT);
  pinMode(PIN_MOTOR_2B, OUTPUT);

  Serial.begin(9600);
  delay(500);  // le da tiempo al Monitor Serie a conectarse

  Serial.println();
  Serial.println("===== TEST DE PLACA - ROBOTSHIELD V2.1 =====");
  tone(PIN_BUZZER, 1500, 150);
  delay(300);

  testLeds();
  testPulsadores();
  testUltrasonico();
  testMotores();

  Serial.println();
  Serial.println("===== Tests automaticos terminados =====");
  Serial.println("Test de bluetooth: escribi algo aca (o mandalo por");
  Serial.println("el HC-05 ya reconectado) y deberia hacer eco.");
}

// ===================== LOOP: TEST DE BLUETOOTH (ECO) =====================

void loop() {
  if (Serial.available() > 0) {
    char c = Serial.read();

    digitalWrite(PIN_LED_1, HIGH);
    tone(PIN_BUZZER, 2000, 80);
    Serial.write(c);
    delay(100);
    digitalWrite(PIN_LED_1, LOW);
  }
}

// ===================== TEST: LEDS =====================

void testLeds() {
  Serial.println();
  Serial.println("--- Test de LEDs ---");

  Serial.println("LED 1 (D4) deberia encender...");
  digitalWrite(PIN_LED_1, HIGH);
  delay(400);
  digitalWrite(PIN_LED_1, LOW);

  Serial.println("LED 2 (D7) deberia encender...");
  digitalWrite(PIN_LED_2, HIGH);
  delay(400);
  digitalWrite(PIN_LED_2, LOW);

  Serial.println("LED 3 (D13) deberia encender...");
  digitalWrite(PIN_LED_3, HIGH);
  delay(400);
  digitalWrite(PIN_LED_3, LOW);
}

// ===================== TEST: PULSADORES =====================

void testPulsadores() {
  Serial.println();
  Serial.println("--- Test de pulsadores ---");
  esperarBoton("Boton 1 (D2)", PIN_BOTON_1);
  esperarBoton("Boton 2 (D12)", PIN_BOTON_2);
}

void esperarBoton(const char* nombre, int pin) {
  Serial.print(nombre);
  Serial.println(": presionalo (10s de margen)...");

  unsigned long inicio = millis();
  while (millis() - inicio < 10000) {
    if (digitalRead(pin) == LOW) {
      Serial.print(nombre);
      Serial.println(": OK");
      tone(PIN_BUZZER, 2000, 100);
      delay(300);  // antirrebote simple
      return;
    }
  }

  Serial.print(nombre);
  Serial.println(": no se detecto, revisar cableado.");
}

// ===================== TEST: SENSOR ULTRASONICO =====================

void testUltrasonico() {
  Serial.println();
  Serial.println("--- Test de sensor ultrasonico (HC-SR04) ---");

  for (int i = 0; i < 5; i++) {
    digitalWrite(PIN_ULTRA_TRIGGER, LOW);
    delayMicroseconds(2);
    digitalWrite(PIN_ULTRA_TRIGGER, HIGH);
    delayMicroseconds(10);
    digitalWrite(PIN_ULTRA_TRIGGER, LOW);

    unsigned long duracionPulso = pulseIn(PIN_ULTRA_ECHO, HIGH, 20000);

    if (duracionPulso == 0) {
      Serial.println("Sin eco (sensor no conectado o fuera de rango).");
    } else {
      int distanciaCM = duracionPulso / 58;
      Serial.print("Distancia: ");
      Serial.print(distanciaCM);
      Serial.println(" cm");
    }

    delay(500);
  }
}

// ===================== TEST: MOTORES =====================

void testMotores() {
  Serial.println();
  Serial.println("--- Test de motores (driver DRV8833) ---");
  testUnMotor("Motor 1", PIN_MOTOR_1A, PIN_MOTOR_1B);
  testUnMotor("Motor 2", PIN_MOTOR_2A, PIN_MOTOR_2B);
}

void testUnMotor(const char* nombre, int pinA, int pinB) {
  Serial.print(nombre);
  Serial.println(": adelante...");
  analogWrite(pinA, VELOCIDAD_TEST_MOTOR);
  digitalWrite(pinB, LOW);
  delay(500);

  digitalWrite(pinA, LOW);
  digitalWrite(pinB, LOW);
  delay(300);

  Serial.print(nombre);
  Serial.println(": atras...");
  digitalWrite(pinA, LOW);
  analogWrite(pinB, VELOCIDAD_TEST_MOTOR);
  delay(500);

  digitalWrite(pinA, LOW);
  digitalWrite(pinB, LOW);
  delay(300);
}
