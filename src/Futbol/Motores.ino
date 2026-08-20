// ===================== CONSTANTES DE CALIBRACION =====================

const int VELOCIDAD_NORMAL = 150;  // PWM (0-255) en modo normal
const int VELOCIDAD_TURBO  = 255;  // PWM (0-255) en modo turbo

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
//
// motorIzquierdo/motorDerecho/aplicarVelocidadMotor son identicas a las
// de MiniSumo/Motores.ino: el driver DRV8833 es el mismo en ambas
// categorias. Si corrigen algo aca, repliquen el cambio alla.

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
