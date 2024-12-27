// begin diag
void printBits(byte myByte) {
  // for Serial debug - used to check the binary leaving...
  for (byte mask = 0x80; mask; mask >>= 1) {
    if (mask & myByte)
      Serial.print('1');
    else
      Serial.print('0');
  }
}

void blinkLED(int duration, int flashes, bool test, bool builtIn) {
  if (test) {
    pinMode(pinInt, OUTPUT);
    pinMode(pinPos1, OUTPUT);
    pinMode(pinPos2, OUTPUT);
    for (int i = 0; i < flashes; i++) {
      delay(duration);
      digitalWrite(pinInt, HIGH);
      digitalWrite(pinPos1, HIGH);
      digitalWrite(pinPos2, HIGH);
      delay(duration);
      digitalWrite(pinInt, LOW);
      digitalWrite(pinPos1, LOW);
      digitalWrite(pinPos2, LOW);
    }
  }

  if (builtIn) {
    pinMode(LED_BUILTIN, OUTPUT);
    for (int i = 0; i < flashes; i++) {
      delay(duration);
      digitalWrite(LED_BUILTIN, HIGH);
      delay(duration);
      digitalWrite(LED_BUILTIN, LOW);
    }
  }
  setupPins();
}
// end diag