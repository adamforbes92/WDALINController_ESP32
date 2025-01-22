void basicInit() {
#if serialDebug
  Serial.begin(115200);
  Serial.println(F("LIN Wiper Controller Initialising..."));
#endif

#if serialDebug
  Serial.println(F("Preferences Initialising..."));
#endif
  preferences.begin("settings", false);
  intervalSpeed = preferences.getInt("intervalSpeed", 1);
  if (intervalSpeed != 0) {
    lastIntervalSpeed = intervalSpeed;
  }
#if serialDebug
  Serial.println(F("Interval Speed (from preferences):" );
  Serial.print(intervalSpeed);
#endif

#if serialDebug
  Serial.println(F("IO Initialising..."));
#endif
  setupPins();  // setup pins for IO
#if serialDebug
  Serial.println(F("IO Initialised!"));
#endif

#if serialDebug
  Serial.println(F("LIN Initialising..."));
#endif
  LIN.begin(linBaud);
#if serialDebug
  Serial.println(F("LIN Initialised!"));
#endif

#if serialDebug
  Serial.println(F("Setting up buttons as inputs..."));
#endif
  setupButtons();
#if serialDebug
  Serial.println(F("Buttons setup!"));
#endif

#if serialDebug
  Serial.println(F("LIN Wiper Controller Initialised!"));
#endif
}

void setupPins() {
  // setup the wiper position inputs
  pinMode(pinIntLED, OUTPUT);
  pinMode(pinPos1LED, OUTPUT);
  pinMode(pinPos2LED, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);

#if testLED
  blinkLED(500, 4, true, false);
#endif

  // reset LED to pull high
  digitalWrite(pinIntLED, HIGH);
  digitalWrite(pinPos1LED, HIGH);
  digitalWrite(pinPos2LED, HIGH);

  // reset buttons if testLED is used (can be removed if 'testLED' is not used but keeping here for solidness)
  pinMode(pinIntNeg, INPUT_PULLUP);
  pinMode(pinPos1Neg, INPUT_PULLUP);
  pinMode(pinPos2Neg, INPUT_PULLUP);

  pinMode(pinIntPos, INPUT_PULLDOWN);
  pinMode(pinPos1Pos, INPUT_PULLDOWN);
  pinMode(pinPos2Pos, INPUT_PULLDOWN);
}

void setupButtons() {
  //setup buttons / inputs
  btnInt.attachSingleClick(intSingle);           // call intSingle on a single click (single wipe)
  btnInt.attachSingleNotHeld(intSingleRelease);  // call intSingleRelease on a single click release (clears flags)
  btnInt.attachDoubleClick(intAdjustSpeed);      // call adjustSpeed on double-click on intermediate - will +4 on the speed until overflow
  btnInt.attachContinuousHold(intHold);          // call intHold if the button is 'always down'
  btnInt.attachContinuousNotHeld(intRelease);    // call intRelease if the button is now not held (clears flags)
  btnPos1.attachContinuousHold(pos1Hold);        // call pos1Hold if the button is 'always down'
  btnPos1.attachContinuousNotHeld(pos1Release);  // call pos1Release if the button is now not held (clears flags)
  btnPos2.attachContinuousHold(pos2Hold);        // call pos1Hold if the button is 'always down'
  btnPos2.attachContinuousNotHeld(pos2Release);  // call pos2Release if the button is now not held (clears flags)
}

void LED() {
  // for following inputs if enabled
  if (wiperInt) {
    digitalWrite(pinIntLED, LOW);
  } else {
    digitalWrite(pinIntLED, HIGH);
  }

  if (wiperPos1) {
    digitalWrite(pinPos1LED, LOW);
  } else {
    digitalWrite(pinPos1LED, HIGH);
  }

  if (wiperPos2) {
    digitalWrite(pinPos2LED, LOW);
  } else {
    digitalWrite(pinPos2LED, HIGH);
  }

  if (hasError) {
    digitalWrite(LED_BUILTIN, HIGH);
  } else {
    digitalWrite(LED_BUILTIN, LOW);
  }
}