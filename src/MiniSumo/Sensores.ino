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
