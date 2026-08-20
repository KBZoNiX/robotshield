// ===================== ESTADO: ESPERA =====================

bool estadoAnteriorBotonArmar = HIGH;

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
