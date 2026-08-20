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

// ===================== BOCINA =====================

void tocarBocina() {
  tone(PIN_BUZZER, 2000, 200);
}
