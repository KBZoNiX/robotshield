/*
  MEDIR BATERIA - sketch de prueba
  ROBOTSHIELD V2.1 - ESETP N°703

  Muestra por el Monitor Serie (9600 baudios), dos veces por segundo:
    - la lectura "cruda" del ADC (0 a 1023)
    - la tension en el pin A0
    - la tension calculada de la bateria

  Antes de usarlo:
    - Robot de Futbol: DESCONECTAR el HC-05 (usa los pines del USB).
    - Bateria conectada en J1 y SW1 ENCENDIDO (el divisor esta
      despues de SW1).
*/

#define PIN_BATERIA A0

// Referencia del ADC (los 5V del Arduino), en volts.
const float TENSION_REFERENCIA_ADC = 5.0;
// (R7 + R8) / R8 = (39K + 10K) / 10K
const float FACTOR_DIVISOR = 4.9;

void setup() {
  Serial.begin(9600);
  Serial.println("Lectura ADC | Tension A0 | Tension bateria");
}

void loop() {
  int lectura = analogRead(PIN_BATERIA);
  float tensionA0 = lectura * TENSION_REFERENCIA_ADC / 1024.0;
  float tensionBateria = tensionA0 * FACTOR_DIVISOR;

  Serial.print(lectura);
  Serial.print("         | ");
  Serial.print(tensionA0, 3);   // 3 decimales
  Serial.print(" V    | ");
  Serial.print(tensionBateria, 2);
  Serial.println(" V");

  delay(500);
}
