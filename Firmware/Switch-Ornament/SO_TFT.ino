void SO_TFT_SETUP_BL_OFF()
{
  pinMode(TFT_BL, OUTPUT);                                                                        // Pin für Hintergrundbeleuchtung als Ausgang deklarieren und ausschalten um flackern durch Initialisierung zu kaschieren
  digitalWrite(TFT_BL, LOW);
}

void SO_TFT_CS_HIGH()
{
  pinMode(LCD_CS, OUTPUT);                                                                        // LCD_CS und SD_CD Pin Initialisierung und High Schalten, damit zunächst beide Geräte nicht auf SPI Bus hören oder reinquatschen
  digitalWrite(LCD_CS, HIGH);
}

void SO_TFT_SETUP()
{
  pinMode(LCD_type_sel, INPUT_PULLDOWN);

  #if defined (use_tft_eSPI_library)
    if(digitalRead(LCD_type_sel) == LOW) tft.init();                                              // Initialisiere ST7735 als NO96-1608TBBIG11-H13 (ST7735_REDTAB160x80)    (ST7735)
    else tft.init(INITR_GREENTAB160x80);                                                          // Initialisiere ST7735 als NO96-1608THBIG11-H13 (ST7735_GREENTAB160x80)  (ST7735S)
  #elif defined (use_LGFX_library)
    if(digitalRead(LCD_type_sel) == HIGH)
    {
      tft.invertDisplay(true);
    }
    tft.init();
  #endif

  tft.setRotation(3);                                                                             // Bild so drehen, dass es passt
  tft.fillScreen(TFT_BLACK);                                                                      // LCD mit schwarzen Pixeln füllen
  tft.setTextSize(1);                                                                             // kleinste Textgröße definieren
  tft.setTextColor(TFT_WHITE);                                                                    // Weiße Textfarbe
  tft.setCursor(0,0);                                                                             // Cursor in obere links Ecke für nachfolgenden Text
  ledcSetup(BL_CH, BL_Freq, BL_RES);                                                              // Initialisiere PWM Kanal für Hintergrundbeleuchtung
  ledcAttachPin(TFT_BL, BL_CH);                                                                   // PWM Kanal an Pin für Hintergrundbeleuchtung knüpfen
  delay(50);                                                                                      // warte mögliches Flackern durch Initialisierung ab

  brightness_CH = SPIFFS_GET_INT(BL_PWM_VAL);                                                     // lade gespeicherten Wert für Stärke der Hintergrundbeleuchtung
  if(brightness_CH >= 9)                                                                          // nur Werte zwischen 0 und 8 logisch. wenn drüber, zurücksetzen und default Wert speichern
  {
    brightness_CH = BL_PWM_VAL_DEFAULT;
    SPIFFS_PUT(BL_PWM_VAL, brightness_CH);
  }
  ledcWrite(BL_CH, brightness[brightness_CH]);                                                    // anwenden der geladenen Helligkeit
}