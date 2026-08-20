// ===================== BLUETOOTH =====================
//
// Lectura y traduccion de los comandos que llegan del HC-05 por Serial.
// Ver el comentario de cabecera de Futbol.ino para la lista de comandos.

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
