void basicInit() {
#if stateDebug
  Serial.begin(115200);
  Serial.println(F("Ford LIN Wiper Controller Initialising..."));
#endif

#if stateDebug
  Serial.println(F("Preferences Initialising..."));
#endif
  preferences.begin("settings", false);
  intervalSpeed = preferences.getInt("intervalSpeed", 0);
  if (intervalSpeed != 0) {
    lastIntervalSpeed = intervalSpeed;
  }
#if stateDebug
  Serial.println(intervalSpeed);
#endif

  setupPins();  // setup pins for IO
  LIN.begin(linBaud);

#if stateDebug
  Serial.println(F("Ford LIN Wiper Controller Initialised, LIN started!"));
#endif

#if stateDebug
  Serial.println(F("Setting up buttons as inputs..."));
#endif
  setupButtons();
#if stateDebug
  Serial.println(F("Buttons setup!"));
#endif
}

void setupPins() {
  // setup the pins for input/output
  pinMode(pinCS, OUTPUT);
  pinMode(pinWake, OUTPUT);

  // drive CS & Wake high to use the LIN chip
  digitalWrite(pinCS, HIGH);
  digitalWrite(pinWake, HIGH);

// setup the wiper position inputs
#if testLED
  pinMode(pinInt, OUTPUT);
  pinMode(pinPos1, OUTPUT);
  pinMode(pinPos2, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  blinkLED(500, intervalSpeed, true, false);
#endif

  // reset buttons if testLED is used (can be removed if 'testLED' is not used but keeping here for solidness)
  pinMode(pinInt, INPUT_PULLUP);
  pinMode(pinPos1, INPUT_PULLUP);
  pinMode(pinPos2, INPUT_PULLUP);
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