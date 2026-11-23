// ===================== BATERIA =====================
//
// Medicion de la tension de la bateria por A0, a traves del divisor
// R7 (39K) / R8 (10K) y el capacitor de filtro C3 (100nF).
//
// Este archivo es IDENTICO en Futbol y en MiniSumo, y trae toda su
// configuracion. En el programa principal solo hay que agregar UNA
// linea en setup():
//
//   mostrarNivelBateria();
//
// La bateria se mide una sola vez, al encender o resetear el robot,
// con los motores detenidos.

// ===================== CONFIGURACION =====================

#define PIN_BATERIA A0

// Los 3 LEDs de la placa (son los mismos pines en los dos robots).
#define LED_NIVEL_1 4   // D4
#define LED_NIVEL_2 7   // D7
#define LED_NIVEL_3 13  // D13

// Referencia del ADC (los 5V del Arduino), en volts.
const float TENSION_REFERENCIA_ADC = 5.0;
// Relacion del divisor: (R7 + R8) / R8 = (39K + 10K) / 10K
const float FACTOR_DIVISOR = 4.9;

// Umbrales para 2 celdas 18650 en serie (2S). Ver el tutorial.
const float TENSION_LLENA = 7.8;      // mas de 7,8 V: 3 LEDs
const float TENSION_MEDIA = 7.4;      // mas de 7,4 V: 2 LEDs
const float TENSION_BAJA  = 7.1;      // mas de 7,1 V: 1 LED
                                      // 7,1 V o menos: CRITICA
const float TENSION_SIN_BATERIA = 3.0;  // menos: alimentado por USB

// ===================== LECTURA =====================

float leerTensionBateria() {
  int lectura = analogRead(PIN_BATERIA);

  // Paso 1: de numero del ADC (0 a 1023) a volts en el pin A0.
  float tensionA0 = lectura * TENSION_REFERENCIA_ADC / 1024.0;

  // Paso 2: de volts en A0 a volts en la bateria (deshacer el divisor).
  return tensionA0 * FACTOR_DIVISOR;
}

// ===================== INDICACION AL ENCENDER =====================

void mostrarNivelBateria() {
  float tensionBateria = leerTensionBateria();

  // Sin bateria (SW1 apagado, alimentado solo por USB): no se muestra
  // nada, asi no suena la alarma cada vez que se sube un programa.
  if (tensionBateria < TENSION_SIN_BATERIA) {
    return;
  }

  // Bateria critica: el LED 1 parpadea 5 veces con bip en cada parpadeo.
  if (tensionBateria <= TENSION_BAJA) {
    for (int i = 0; i < 5; i++) {
      digitalWrite(LED_NIVEL_1, HIGH);
      bipBateria(200);
      digitalWrite(LED_NIVEL_1, LOW);
      delay(200);
    }
    return;
  }

  // Cuantos LEDs prender. Se compara de MAYOR a menor: el primer
  // if que se cumple decide.
  int cantidadLeds;
  if (tensionBateria > TENSION_LLENA) {
    cantidadLeds = 3;
  } else if (tensionBateria > TENSION_MEDIA) {
    cantidadLeds = 2;
  } else {
    cantidadLeds = 1;
  }

  // Llena / media / baja: "barra" de LEDs fija + un bip por LED.
  digitalWrite(LED_NIVEL_1, cantidadLeds >= 1 ? HIGH : LOW);
  digitalWrite(LED_NIVEL_2, cantidadLeds >= 2 ? HIGH : LOW);
  digitalWrite(LED_NIVEL_3, cantidadLeds >= 3 ? HIGH : LOW);
  for (int i = 0; i < cantidadLeds; i++) {
    bipBateria(80);
    delay(150);
  }

  delay(1500);  // tiempo para mirar los LEDs

  // Apagar los LEDs
  digitalWrite(LED_NIVEL_1, LOW);
  digitalWrite(LED_NIVEL_2, LOW);
  digitalWrite(LED_NIVEL_3, LOW);
}

// ===================== BIP =====================

// Suena el buzzer durante "duracionMs" milisegundos y espera a que
// termine antes de seguir.
void bipBateria(int duracionMs) {
  tone(PIN_BUZZER, 2000, duracionMs);
  delay(duracionMs);
}
