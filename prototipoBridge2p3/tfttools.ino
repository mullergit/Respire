void initTFT(){
  tft.init();
  tft.setRotation(1);
  touch_calibrate();  
  tft.fillScreen(TFT_BLACK);
  //tft.fillRect(0, 0, 320, 240, TFT_DARKGREY);
  //tft.fillRect(DISP_X, DISP_Y, DISP_W, DISP_H, TFT_BLACK);
  //tft.drawRect(DISP_X, DISP_Y, DISP_W, DISP_H, TFT_WHITE);
  //tft.fillRect(2, 2, 318, 118, TFT_BLACK);
  tft.drawRect(0, 0, 318, 125, TFT_WHITE);
  //drawKeypad();  

  //tft.fillRect(0, 0, 320, 240, TFT_DARKGREY);
  //tft.fillRect(2, 2, 318, 118, TFT_BLACK);
  //tft.drawRect(0, 0, 320, 125, TFT_WHITE);
    handleTFT2();
  //***********SDcard*****************
  /*
 Serial.print("Initializing SD card...");
  //SPI.pins(int8_t sck, int8_t miso, int8_t mosi, int8_t ss);
  SPI.end();
  SPI.pins(14, 12, 13, 15);
  SPI.begin();
  // see if the card is present and can be initialized:
  //SPI.setHwCs(true);
  if (!SD.begin(15)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    //while (1);
  }
  Serial.println("card initialized.");
  //***********Fim SDcard*****************
            */ 
}
void handleTFT2(){
  double spm = meanpm;
  int sco2 = (int)scd1co2;
  int y1 = 32;
  int y2 = 90;
  int x1 = 40;
  int x2 = 150;
  int x3 = 270;
  int y3 = 152;
  int y4 = 177;
  int y5 = 210;

  tft.drawRect(0, 0, 318, 125, TFT_WHITE);
  
  tft.setFreeFont(FF26);  
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  //tft.setTextFont(0);
  tft.setTextDatum(MC_DATUM);
  //tft.setTextSize(1.8);
  tft.drawString("PM ", x1, y1, GFXFF);
  tft.setFreeFont(FF24);//tft.setFreeFont(FF28);
  tft.drawString(String(spm), x2, y1, GFXFF);
  tft.setFreeFont(FF26);//26
  //tft.setTextDatum(BL_DATUM);
  tft.drawString("ug/m ", x3-5, y1, GFXFF);
  tft.drawString("3 ", x3+30, y1-10, GFXFF);

  
  tft.setFreeFont(FF26);
  //tft.setTextDatum(BC_DATUM);
  tft.drawString("CO2 ", x1, y2);
  tft.setFreeFont(FF24);
  tft.drawString(String(sco2), x2, y2);
  tft.setFreeFont(FF26);
  tft.drawString("ppm ", x3, y2,GFXFF);

  tft.setFreeFont(FF21);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString("STATUS  EXAUSTOR ", x2+10, y3);
  tft.setFreeFont(FF25);
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.drawString("Conexao ", x1, y4);
  tft.drawString("Modo ", x2, y4);
  tft.drawString("Velocidade", x3, y4);

  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  
  tft.setFreeFont(FF22);
  
  if(ventilationMode==1){
    tft.setTextColor(TFT_RED, TFT_BLACK);
  }else tft.setTextColor(TFT_GREENYELLOW, TFT_BLACK);
  tft.drawString("ONLINE ", x1, y5);
  tft.drawString("AUTO ", x2+10, y5);
  tft.drawString("MAX ", x3, y5);
}
void handleTFT(){  
  
  uint16_t t_x = 0, t_y = 0; // To store the touch coordinates

  // Pressed will be set true is there is a valid touch on the screen
  boolean pressed = tft.getTouch(&t_x, &t_y);

  // / Check if any key coordinate boxes contain the touch coordinates
  for (uint8_t b = 0; b < 15; b++) {
    if (pressed && key[b].contains(t_x, t_y)) {
      key[b].press(true);  // tell the button it is pressed
    } else {
      key[b].press(false);  // tell the button it is NOT pressed
    }
  }

  // Check if any key has changed state
  for (uint8_t b = 0; b < 15; b++) {

    if (b < 3) tft.setFreeFont(LABEL1_FONT);
    else tft.setFreeFont(LABEL2_FONT);

    if (key[b].justReleased()) key[b].drawButton();     // draw normal

    if (key[b].justPressed()) {
      key[b].drawButton(true);  // draw invert

      // if a numberpad button, append the relevant # to the numberBuffer
      if (b >= 3) {
        if (numberIndex < NUM_LEN) {
          numberBuffer[numberIndex] = keyLabel[b][0];
          numberIndex++;
          numberBuffer[numberIndex] = 0; // zero terminate
        }
        status(""); // Clear the old status
      }

      // Del button, so delete last char
      if (b == 1) {
        numberBuffer[numberIndex] = 0;
        if (numberIndex > 0) {
          numberIndex--;
          numberBuffer[numberIndex] = 0;//' ';
        }
        status(""); // Clear the old status
      }

      if (b == 2) {
        status("Sent value to serial port");
        Serial.println(numberBuffer);
      }
      // we dont really check that the text field makes sense
      // just try to call
      if (b == 0) {
        status("Value cleared");
        numberIndex = 0; // Reset index to 0
        numberBuffer[numberIndex] = 0; // Place null in buffer
      }

      // Update the number display field
      tft.setTextDatum(TL_DATUM);        // Use top left corner as text coord datum
      tft.setFreeFont(&FreeSans18pt7b);  // Choose a nicefont that fits box
      tft.setTextColor(DISP_TCOLOR);     // Set the font colour

      // Draw the string, the value returned is the width in pixels
      int xwidth = tft.drawString(numberBuffer, DISP_X + 4, DISP_Y + 12);

      // Now cover up the rest of the line up by drawing a black rectangle.  No flicker this way
      // but it will not work with italic or oblique fonts due to character overlap.
      tft.fillRect(DISP_X + 4 + xwidth, DISP_Y + 1, DISP_W - xwidth - 5, DISP_H - 2, TFT_BLACK);

      delay(10); // UI debouncing
    }
  }
}
void handleSDcard(){
  if(millis()-timer4>30000){
    //SPI.setHwCs(true);
    SPI.end();
    SPI.pins(14, 12, 13, 15);
    SPI.begin();
    if (!SD.begin(15)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
   // while (1);
  }
  Serial.println("card initialized.");
      dataString = "";
      dataString += String(contador);
      File dataFile = SD.open("datalog.txt", FILE_WRITE);    
      if (dataFile) {
        dataFile.println(dataString);
        dataFile.close();
        Serial.println(dataString);
      }
      else {
        Serial.println("error opening datalog.txt");
      }
      contador++;         
      timer4=millis();
      SPI.end();
      //SPI.pins(int8_t sck, int8_t miso, int8_t mosi, int8_t ss);
      SPI.pins(6, 7, 8, 0);
      SPI.begin();
      //tft.init();
  }
  
}
void drawKeypad()
{
  // Draw the keys
  for (uint8_t row = 0; row < 5; row++) {
    for (uint8_t col = 0; col < 3; col++) {
      uint8_t b = col + row * 3;

      if (b < 3) tft.setFreeFont(LABEL1_FONT);
      else tft.setFreeFont(LABEL2_FONT);

      key[b].initButton(&tft, KEY_X + col * (KEY_W + KEY_SPACING_X),
                        KEY_Y + row * (KEY_H + KEY_SPACING_Y), // x, y, w, h, outline, fill, text
                        KEY_W, KEY_H, TFT_WHITE, keyColor[b], TFT_WHITE,
                        keyLabel[b], KEY_TEXTSIZE);
      key[b].drawButton();
    }
  }
}

//------------------------------------------------------------------------------------------

void touch_calibrate()
{
  uint16_t calData[5];
  uint8_t calDataOK = 0;

  // check file system exists
  if (!SPIFFS.begin()) {
    Serial.println("Formating file system");
    SPIFFS.format();
    SPIFFS.begin();
  }

  // check if calibration file exists and size is correct
  if (SPIFFS.exists(CALIBRATION_FILE)) {
    if (REPEAT_CAL)
    {
      // Delete if we want to re-calibrate
      SPIFFS.remove(CALIBRATION_FILE);
    }
    else
    {
      File f = SPIFFS.open(CALIBRATION_FILE, "r");
      if (f) {
        if (f.readBytes((char *)calData, 14) == 14)
          calDataOK = 1;
        f.close();
      }
    }
  }

  if (calDataOK && !REPEAT_CAL) {
    // calibration data valid
    tft.setTouch(calData);
  } else {
    // data not valid so recalibrate
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(20, 0);
    tft.setTextFont(2);
    tft.setTextSize(1);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);

    tft.println("Touch corners as indicated");

    tft.setTextFont(1);
    tft.println();

    if (REPEAT_CAL) {
      tft.setTextColor(TFT_RED, TFT_BLACK);
      tft.println("Set REPEAT_CAL to false to stop this running again!");
    }

    tft.calibrateTouch(calData, TFT_MAGENTA, TFT_BLACK, 15);

    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.println("Calibration complete!");

    // store data
    File f = SPIFFS.open(CALIBRATION_FILE, "w");
    if (f) {
      f.write((const unsigned char *)calData, 14);
      f.close();
    }
  }
}

//------------------------------------------------------------------------------------------

// Print something in the mini status bar
void status(const char *msg) {
  tft.setTextPadding(240);
  //tft.setCursor(STATUS_X, STATUS_Y);
  tft.setTextColor(TFT_WHITE, TFT_DARKGREY);
  tft.setTextFont(0);
  tft.setTextDatum(TC_DATUM);
  tft.setTextSize(1);
  tft.drawString(msg, STATUS_X, STATUS_Y);
}
