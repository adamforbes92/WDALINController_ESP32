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
    for (int i = 0; i < flashes; i++) {
#if serialDebug
      Serial.println(F("Flashing onboard LEDs..."));
#endif
      delay(duration);
      digitalWrite(pinIntLED, LOW);
      digitalWrite(pinPos1LED, LOW);
      digitalWrite(pinPos2LED, LOW);
      delay(duration);
      digitalWrite(pinIntLED, HIGH);
      digitalWrite(pinPos1LED, HIGH);
      digitalWrite(pinPos2LED, HIGH);
    }
#if serialDebug
    Serial.println(F("Flashing onboard LEDs Complete!"));
#endif
  }

  if (builtIn) {
#if serialDebug
    Serial.println(F("Flashing builtin LED..."));
#endif
    pinMode(LED_BUILTIN, OUTPUT);
    for (int i = 0; i < flashes; i++) {
      delay(duration);
      digitalWrite(LED_BUILTIN, HIGH);
      delay(duration);
      digitalWrite(LED_BUILTIN, LOW);
    }
#if serialDebug
    Serial.println(F("Flashing builtin LED Complete!"));
#endif
  }
}
// end diag