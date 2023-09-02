/*
Changelog:
1.1:
- Support for alternative ST7735 Displays (if Color ist wrong, try GPIO09 connect to VCC (alternative Display Type))

1.2:
- Check for GIF Header. Ignor file if Header ist not "GIF"
- alternative library (chose before compile code)

1.3:
- PNG support
- Left Button trigger play previous file instead of changing the Brightness of the Screen

1.4:
- Clean Code, so it should be now better to maintain
- Fix a problem when use the LGFX library (use a variable from eSPI to initialise the SD Card (old testing stuff))
- improve Version Information and SD Card Info Screen. Its now cleaner and stay for 2 instead of 1 second
- change brightness level by push and hold left (reduce) or right (increase) button for more than 1 second
- speed up initialisation slightly
*/

////////// DISPLAY TREIBER //////////
//chose if you want to use the TFT_eSPI (recommended) or the LGFX library
#define use_tft_eSPI_library
//#define use_LGFX_library

////////// VERSION INFORMATIONEN //////////
const String date = "02.09.2023";
const String version = "1.4";

////////// BIBLIOTHEKEN EINBINDEN //////////
#include "SO_TFT.h"
#include "SO_SD.h"
#include "SO_BUTTON.h"
#include "SO_GIF.h"
#include "SO_UPDATE.h"
#include "SO_PNG.h"
#include "SO_SPIFFS.h"
#include "SO_WDT.h"

////////// GLOBALE VARIABLEN DEKLARIEREN //////////
bool interrupt_current_play = false;                                                                // interrupt gif, um aktuelles GIF zu überspringen

////////// SETUP //////////
void setup()
{
  //Serial.begin(115200);
  SO_TFT_SETUP_BL_OFF();
  SO_WDT_SETUP();
  SO_BUTTON_SETUP();
  SO_TFT_CS_HIGH();                                                                                 // ChipSelect Pins zu Begin HIGH um SPI Bus nicht zu blockieren
  SO_SD_CS_HIGH();
  SO_SPIFFS_SETUP();
  SO_TFT_SETUP();

  SO_SD_SETUP();

  tft.setCursor(0,0);
  tft.println("Switch Ornament");
  tft.println("by esprit1711");
  tft.println();
  tft.println("Release: " + version);
  tft.println("Date: " + date);
  delay(2000);

  updateFromFS(SD);                                                                                 // prüfe auf "SO_update.bin" in Stammverzeichnis. wenn vorhanden, Update der Firmware

  GiftotalFiles = getGifInventory("/gif");                                                          // scanne "gif" Ordner auf SD Karte nach vorhandenen gif Daten
  PngtotalFiles = getPngInventory("/png");
  if(GiftotalFiles <= 0 && PngtotalFiles <= 0) GifInventoryError(GiftotalFiles);

  Button_A_pressed_event = false;                                                                   // eventuelle Ergebnisse von Interrupts während Initialisierung zurücksetzen
  Button_B_pressed_event = false;
}

////////// LOOP //////////
void loop()
{
  static int FileTypeRotation = 0;
  if (GiftotalFiles <= 0) FileTypeRotation = 1;                                                     // sollte es keine gif Files geben, mache sofort mit PNG weiter

  tft.fillScreen(TFT_BLACK);                                                                        // Bildschirm schwarz füllen um Pixel Leichen vom jeweils vorherigen gif zu vermeiden
  if(digitalRead(SD_Card_detect) == HIGH) SD_Card_missing();                                        // wenn SD Karte während des Betriebs raus gezogen wird, schicke Programm in dead end

  if(FileTypeRotation == 0) GIF_loop();
  else if (FileTypeRotation == 1) PNG_loop();

  FileTypeRotation = fileRotationHandling(FileTypeRotation);                                        // schalte immer wieder um zwischen gif und png

  Button_A_pressed_event = false;
  Button_B_pressed_event = false;
  interrupt_current_play = false;                                                                   // Variable die den Abbruch der aktuellen Datei anweist wieder zurücksetzen
}

////////// FUNKTIONEN //////////
int fileRotationHandling(int FileTypeRotation)
{
  if(interrupt_current_play == true)                                                                // wenn abspielen der aktuellen Datei angewiesen wurde, feststellen, ob vor oder zurück
  {
    if(Button_A_pressed_event == true)                                                              // zurück schalten
    {
      if ((FileTypeRotation == 0 && PngtotalFiles > 0) || (FileTypeRotation == 1 && GiftotalFiles > 0)) // aktuelle unterbrochene Datei = gif und es gibt mindestens ein PNG, oder aktuell unterbrochene Datei PNG und es gibt mindestens 1 gif
      {
        PngcurrentFile = PngcurrentFile - 1;
        if(PngcurrentFile < 0) PngcurrentFile = PngtotalFiles - 1;
        GifcurrentFile = GifcurrentFile - 1;
        if(GifcurrentFile < 0) GifcurrentFile = GiftotalFiles - 1;
      }
      else if (FileTypeRotation == 0 && PngtotalFiles <= 0)                                         // aktuell gespielte Datei = gif und es gibt kein PNG
      {
        GifcurrentFile = GifcurrentFile - 2;
        if(GifcurrentFile < 0) GifcurrentFile = GiftotalFiles - 1;
      }
      else if (FileTypeRotation == 1 && GiftotalFiles <= 0)                                         // aktuell gespielte Datei = PNG und es gibt kein gif
      {
        PngcurrentFile = PngcurrentFile - 2;
        if(PngcurrentFile < 0) PngcurrentFile = PngtotalFiles - 1;
      }
    }
  }
  if (FileTypeRotation == 0 && PngtotalFiles > 0) FileTypeRotation = 1;                             // wenn letzte abgespielte Datei = gif, schalte auf PNG...
  else if (FileTypeRotation == 1 && GiftotalFiles > 0) FileTypeRotation = 0;                        // ... ansonsten von PNG auf gif
  return FileTypeRotation;
}