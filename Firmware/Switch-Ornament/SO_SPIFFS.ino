void SO_SPIFFS_SETUP()
{
  if ((digitalRead(Button_A) == LOW && digitalRead(Button_B) == LOW) || !SPIFFS.begin())          // wenn während Start linker und rechter Button gleichzeitig gedrückt werden, Werkseinstellungen laden
  SPIFFS_FACTORY_RESET();
}

void SPIFFS_FACTORY_RESET()                                                                       // formatieren des Speichers und setzen von default Parametern
{
  ledcWrite(BL_CH, brightness[0]);                                                                // setze maximale Helligkeit, damit Anwender auf jeden Fall Prozess verfolgen kann
  tft.fillScreen(TFT_BLACK);
  tft.setCursor(0,0);
  tft.setTextSize(1);
  
  tft.println("factory reset!");
  tft.print("format filesystem... ");
  SPIFFS.format();                                                                                // formatiere internes Dateisystem
  SPIFFS.begin();                                                                                 // initialisiere internes Dateisystem nach formatieren
  tft.setTextColor(TFT_GREEN);
  tft.println("done!");
  tft.setTextColor(TFT_WHITE);

  fill_SPIFFS("TFT_BL", BL_PWM_VAL, BL_PWM_VAL_DEFAULT);                                          // schreibe default Wert für Helligkeit
  
  tft.println("factory reset done!");
  tft.println("Restart in 3 sec.");
  delay(3000);
  ESP.restart();                                                                                  // neustart des ESP32 nach 3 sek.
}

void fill_SPIFFS(String Info_Text, String file_to_open, int data_to_put)                          // schreibt Integer Wert in definierte Datei in Internen Speicher während formatierung
{
  tft.print("create ");
  tft.print(Info_Text);
  tft.print(" file... ");
  if (SPIFFS_PUT(file_to_open, data_to_put) == true)
  {
    tft.setTextColor(TFT_GREEN);
    tft.println("done!");
  }
  else
  {
    tft.setTextColor(TFT_RED);
    tft.println("failed!");
  }
  tft.setTextColor(TFT_WHITE);
}

bool SPIFFS_PUT(String file_to_open, int data_to_put)                                             // schreibt Integer Wert in definierte Datei (Aufruf während Programmablauf)
{
  File f = SPIFFS.open(file_to_open, FILE_WRITE);                                                 // öffne Datei mit schreibrechten
  if (f)                                                                                          // wenn Datei geöffnet werden konnte,...
  {
    f.print(String(data_to_put));                                                                 // ... schreibe neuen Integer Wert als String rein
    f.close();
    return true;
  }
  else return false;
}

int SPIFFS_GET_INT(String file_to_open)                                                           // liest Integer Wert aus definierte Datei (Aufruf während Programmablauf)
{
  String data_out;
  File f = SPIFFS.open(file_to_open);                                                             // öffne Datei mit leserechten
  if (f)                                                                                          // wenn Datei geöffnet werden konnte,...
  {
    data_out = f.readString();                                                                    // ... lese Integer Wert als Strin aus
    f.close();
  }
  return(data_out.toInt());                                                                       // Rückgabe des ausgelesenen Wert als Integer
}