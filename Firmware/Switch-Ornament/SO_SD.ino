void SO_SD_CS_HIGH()
{
  pinMode(SD_CS, OUTPUT);
  digitalWrite(SD_CS, HIGH);
}

void SO_SD_SETUP()
{
  tft.setCursor(0,0);
  for(int i = 0; i < 6; i++) tft.println();
  pinMode(SD_Card_detect, INPUT_PULLUP);                                                          // initialisiere SD Karten erkennung
  if(!SD.begin(SD_CS) || digitalRead(SD_Card_detect) == HIGH)                              // initialisiere SD Karte
  {
    SD_Card_missing();                                                                            // aufruf wenn SD Karte nicht gefunden oder initialisiert werden konnte (dead End)
  }
  uint8_t cardType = SD.cardType();                                                               // ermittle diverse SD Karten Eigenschaften und stelle diese auf LCD dar

  if(cardType == CARD_NONE)
  {
    SD_Card_missing();
  }

  tft.print("SD Card Type: ");
  if(cardType == CARD_MMC)
  {
      tft.println("MMC");
  }
  else if(cardType == CARD_SD)
  {
    tft.println("SDSC");
  }
  else if(cardType == CARD_SDHC)
  {
    tft.println("SDHC");
  }
  else
  {
    tft.println("UNKNOWN");
  }

  uint64_t cardSize = SD.cardSize() / (1024 * 1024);
  tft.printf("SD Card Size: %lluMB\n", cardSize);
}

void SD_Card_missing()                                                                            // aufruf der Funktion wenn SD Karte nicht (mehr) vorhanden ist (dead end)
{
  Button_A_pressed_event = false;                                                                 // eventuelle Ergebnisse von Interrupts zurücksetzen
  Button_B_pressed_event = false;
  tft.fillScreen(TFT_RED);
  tft.setTextColor(TFT_BLACK);
  tft.setTextSize(2);
  tft.setCursor(35,7);
  tft.println("SD Card");
  tft.setCursor(33,27);
  tft.println("Missing!");
  tft.setTextSize(1);
  tft.setCursor(17,50);
  tft.println("Inset Card and Press");
  tft.setCursor(27,60);
  tft.println("Button to restart");
  while(1)
  {
    if(Button_A_pressed_event == true || Button_B_pressed_event == true) ESP.restart();           // warte auf drücken von linken oder rechten Button um neustart zu triggern
  }
}