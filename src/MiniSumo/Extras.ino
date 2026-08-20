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
