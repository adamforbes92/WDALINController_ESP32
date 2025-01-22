void defineWiperFrame() {
  switch (linType) {
    case LIN_WDA:
      // Frame 0:
      // add one to frame count, if 15, roll back to 0
      frameCount++;
      if (frameCount > 15) {
        frameCount = 0;
      }

      wiperFrame[0] = frameCount;  // Bit 0-3: Counter.  Bit 4: KL.15. Bit 5: KL.X. Bit 6: 0. Bit 7: 0.
      // set individual 'bits' on the frame for KL terms (fixed - if we have power, the wipers should be 'active')
      bitWrite(wiperFrame[0], 4, 1);  // KL.15 (Forced 1) - assumed power, so use wipers
      bitWrite(wiperFrame[0], 5, 1);  // KL.X (Forced 1) - assumed power, so use wipers
      bitWrite(wiperFrame[0], 6, 0);  // 0
      bitWrite(wiperFrame[0], 7, 0);  // 0

      // Frame 1
      // set interval speed from static number stored at top (1=1, 2=5, 3=9, 4=13)
      wiperFrame[1] = intervalSpeed;  // Bit 0-3: Counter.  Bit 4: KL.15. Bit 5: KL.X. Bit 6: 0. Bit 7: 0.
      // set individual 'bits' on the frame for wiper direction (based on inputs): single strike, intermittent, SPD1, SPD2
      bitWrite(wiperFrame[1], 4, wiperSingle);  // single strike
      bitWrite(wiperFrame[1], 5, wiperInt);     // intermittent
      bitWrite(wiperFrame[1], 6, wiperPos1);    // SPD1
      bitWrite(wiperFrame[1], 7, wiperPos2);    // SPD2

      // Frame 3 through 5 are 'empty' / not used, so send over 0x00
      // empty frames
      wiperFrame[2] = 0x00;  // empty, empty, empty, empty, empty
      wiperFrame[3] = 0x00;  // empty, empty, empty, empty, empty
      wiperFrame[4] = 0x00;  // empty, empty, empty, empty, empty
      wiperFrame[5] = 0x00;  // empty, empty, empty, empty, empty
      wiperFrame[6] = 0x00;  // empty, empty, empty, empty, empty
      wiperFrame[7] = 0x00;  // empty, empty, empty, empty, empty
      break;
    case LIN_Ford:
      // build the frame - get the easy ones out the way first
      switch (intervalSpeed) {
        case 1:
          wiperFrame[0] = 0xA0;
          break;
        case 2:
          wiperFrame[0] = 0xA1;
          break;
        case 3:
          wiperFrame[0] = 0xA2;
          break;
        case 4:
          wiperFrame[0] = 0xA3;
          break;
        case 5:
          wiperFrame[0] = 0xA4;
          break;
        case 6:
          wiperFrame[0] = 0xA5;
          break;
      }
      //wiperFrame[1] = 0x03;  // speed: 0x00=off; 0x01=int; 0x02=slow; 0x03=fast; 0x04=3x wipes, pause? 0x05=off?
      if (!wiperInt && !wiperPos1 && !wiperPos2) {
        wiperFrame[1] = 0x00;  // off
      }
      if (wiperInt) {
        wiperFrame[1] = 0x01;  // int
      }
      if (wiperPos1) {
        wiperFrame[1] = 0x02;  // slow
      }
      if (wiperPos2) {
        wiperFrame[1] = 0x03;  // fast
      }
      wiperFrame[2] = 0x03;  // set interval speed from static number stored at top (1=1, 2=5, 3=9, 4=13) (ORI 0x03)
      wiperFrame[3] = 0x00;  // ORI 0x00?
      break;
    case LIN_VW:
      /*  ID: 0xF0
        Byte 0: Counter (check rollover)
        Byte 1: Position (0=off; 1=single wipe; 2=int; 4=slow; 8=fast)
        Byte 2: Always 1 (likely interval speed, check(!))
        Byte 3: Always 0
        Byte 4: Always 0 
      */
      frameCount++;
      if (frameCount > 31) {  // check rollover count 0x1F (31)
        frameCount = 10;
      }

      wiperFrame[0] = frameCount;  // Bit 0-3: Counter.  Bit 4: KL.15. Bit 5: KL.X. Bit 6: 0. Bit 7: 0.

      if (!wiperInt && !wiperPos1 && !wiperPos2) {
        wiperFrame[1] = 0x00;  // off
      }
      if (wiperSingle) {
        wiperFrame[1] = 0x01;  // single wipe - tocheck(!)
      }
      if (wiperInt) {
        wiperFrame[1] = 0x02;  // int
      }
      if (wiperPos1) {
        wiperFrame[1] = 0x04;  // slow
      }
      if (wiperPos2) {
        wiperFrame[1] = 0x08;  // fast
      }

      switch (intervalSpeed) {
        case 1:
          wiperFrame[2] = 0x01;
          break;
        case 2:
          wiperFrame[2] = 0x02;
          break;
        case 3:
          wiperFrame[2] = 0x03;
          break;
        case 4:
          wiperFrame[2] = 0x04;
          break;
        default:
          wiperFrame[2] = 0x01;  // set a default - could be that intervalSpeed is corrupt - robustness
          break;
      }

      wiperFrame[3] = 0x00;  // always zero
      wiperFrame[4] = 0x00;  // always zero
      break;
    case LIN_UNKNOWN:
      // kept for future - to be changed to another manufacturer, but do nothing.  Do not add anything to the frames here - if you do you'll end up clearing what's been read...
      break;
    default:
      break;
  }
}

void sendLINFrame() {
  LIN_Master_Base::frame_t Type;
  LIN_Master_Base::error_t error;
  uint8_t recvWiperData[8];

  if (LIN.getState() == LIN_Master_Base::STATE_DONE) {
    error = LIN.getError();

    if (error != LIN_Master_Base::NO_ERROR) {
      hasError = true;
    } else {
      hasError = false;
    }

#if serialDebug
    if (error != LIN_Master_Base::NO_ERROR) {
      Serial.print(", err=0x");
      Serial.println(error, HEX);
    }
#endif
    // reset state machine & error
    LIN.resetStateMachine();
    LIN.resetError();

  }  // if LIN frame finished

  switch (linType) {
    case LIN_WDA:
#if ledOnLIN
      digitalWrite(LED_BUILTIN, HIGH);
#endif
      LIN.sendMasterRequest(LIN_Master_Base::LIN_V2, linWiperID, 8, wiperFrame);  // WDA wiper uses 8(!) bytes to send over
#if ledOnLIN
      digitalWrite(LED_BUILTIN, LOW);
#endif
      break;
    case LIN_Ford:
#if ledOnLIN
      digitalWrite(LED_BUILTIN, HIGH);
#endif
      LIN.sendMasterRequest(LIN_Master_Base::LIN_V2, linWiperID, 4, wiperFrame);  // Ford wiper uses 4(!) bytes to send over
#if ledOnLIN
      digitalWrite(LED_BUILTIN, LOW);
#endif
      break;
    case LIN_VW:
#if ledOnLIN
      digitalWrite(LED_BUILTIN, HIGH);
#endif
      LIN.sendMasterRequest(LIN_Master_Base::LIN_V1, linWiperID, 5, wiperFrame);  // VW wiper uses 5(!) bytes to send over
#if ledOnLIN
      digitalWrite(LED_BUILTIN, LOW);
#endif
      break;
    case LIN_UNKNOWN:
#if ledOnLIN
      digitalWrite(LED_BUILTIN, HIGH);
#endif
      LIN.receiveSlaveResponseBlocking(LIN_Master_Base::LIN_V2, linWiperID, 8, recvWiperData);  // assume other manufacturers uses 8 bytes
#if ledOnLIN
      digitalWrite(LED_BUILTIN, LOW);
#endif  // print result
#if serialDebug
      Serial.print(LIN.nameLIN);
      Serial.print(", response, ID=0x");
      Serial.print(linWiperID, HEX);
      if (error != LIN_Master_Base::NO_ERROR) {
        Serial.print(", err=0x");
        Serial.println(error, HEX);
      } else {
        Serial.print(", data=");
        for (uint8_t i = 0; (i < 8); i++) {
          Serial.print("0x");
          Serial.print((int)recvWiperData[i], HEX);
          Serial.print(" ");
        }
        Serial.println();
      }
#endif
      break;
    default:
      break;
  }

#if serialDebug
  Serial.print(linWiperID, HEX);
  if (error != LIN_Master_Base::NO_ERROR) {
    Serial.print(", Err = 0x");
    Serial.println(error, HEX);
    hasError = true;
  } else {
    Serial.print(", Data = ");
    for (uint8_t i = 0; (i < 8); i++) {
      Serial.print("0x");
      Serial.print(wiperFrame[i], HEX);
      Serial.print(" ");
    }
    Serial.println();
  }
#endif
}