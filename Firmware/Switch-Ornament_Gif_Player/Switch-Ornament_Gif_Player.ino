// Quelle, auf das dieser Code basiert: https://github.com/tobozo/ESP32-GifPlayer

/*
to-do:
- erkennung ob in Header von .gif datei erste 3 Zeichen "GIF" lauten
- ermiteln ob immer zwischen gifs ein FillScreen folgen muss (nur bei transparenten gif files???)
*/

////////// BIBLIOTHEKEN EINBINDEN //////////
#include <Update.h>                                                                               // Software Update via SD Karte
#include <AnimatedGIF.h>                                                                          // abspielen von GIFs
#include <TFT_eSPI.h>                                                                             // library zur Ansteuerung des Displays
#include "FS.h"                                                                                   // Dateisystem
#include "SPIFFS.h"                                                                               // lesen und Schreiben auf internen Speicher des ESP32
#include "SD.h"                                                                                   // SD Karten Zugriff
#include "SPI.h"                                                                                  // Initialisierung der SPI Schnittstelle (für LCD & SD Kartenzugriff)
#include <esp_task_wdt.h>                                                                         // Watch-Dog-Timer um Abstürze abzufangen

////////// VERSION INFORMATIONEN //////////
const String date = "03.04.2023";
const String version = "1.0";

////////// WDT //////////
#define WDT_TIMEOUT 5//s                                                                          // automatischer Neustart, wenn System 5 sek. lang nicht ansprechbar ist
hw_timer_t *Timer0_Cfg = NULL;                                                                    // deklariere Hardware Timer für WDT
void IRAM_ATTR Timer0_ISR()                                                                       // Cyclischer Aufruf des Timer interrupts um WDT zurückzusetzen
{
  esp_task_wdt_reset();
}

////////// GLOBALE VARIABLEN DEKLARIEREN //////////
TFT_eSPI tft = TFT_eSPI();

#define Button_A 3                                                                                // Button links
#define Button_B 1                                                                                // Button rechts
#define SD_Card_detect 0                                                                          // SD Karten erkennung

#define SCK 6                                                                                     // SPI Pins
#define MOSI 7
#define MISO 2

#define LCD_CS 10                                                                                 // SPI-CS Pin von LCD
#define SD_CS 4

#define DISPLAY_WIDTH 160                                                                         // Display Auflösung
#define DISPLAY_HEIGHT 80

AnimatedGIF gif;

bool Button_A_pressed_event = false;                                                              // true durch Interrupt wenn Button gedrückt wird
bool Button_B_pressed_event = false;

unsigned int Button_A_debounce_timer = 0;                                                         // Software entsprellen von Taster
unsigned int Button_B_debounce_timer = 0;
int Button_debounce_time = 250;//ms                                                               // Zeit in der keine weiteren Button Klicks akzeptiert werden sollen (entprellen)

const int maxLoopIterations =     5;                                                              // Stoppe Gif nach dieser Anzahl an Wiederholungen
const int maxLoopsDuration  =  3000;//ms                                                          // Wenn GIF bereits so lange gespielt wird, gehe nach ende des laufenden loops zum nächsten gif
const int maxGifDuration    = 30000;//ms                                                          // maximale Länge eines gif

static int xOffset = 0;                                                                           // wird genutzt um gif zu zentrieren
static int yOffset = 0;

static int totalFiles = 0;                                                                        // anzahl existierender gif Daten (wird am Anfang immer ermittelt)
static int currentFile = 0;                                                                       // zeigt auf aktuell abgespieltes gif

const int GifFiles_size = 10000;                                                                  // maximale Anzahl an Gifs, magische Grenze = 14579, alles darüber geht nicht (SD Karte nicht gefunden)
String GifFiles[GifFiles_size];                                                                   // merkt sich die Pfade zu jedem Gif auf der SD Karte

bool interrupt_gif = false;                                                                       // interrupt gif, um aktuelles GIF zu überspringen

int brightness[9] = {255, 128, 64, 32, 16, 8, 4, 2, 1};                                           // Helligkeitsstufen (können mit linken Button durchgeschaltet werden)
int brightness_CH = 0;                                                                            // merkt sich aktuell gewählte Helligkeitsstufe
const int BL_Freq = 5000;//HZ                                                                     // PWM Frequenz
const int BL_CH = 0;                                                                              // definiere PWM Kanal
const int BL_RES = 8;//Bit                                                                        // definiere PWM Auflösung
#define BL_PWM_VAL "/BL_PWM_VAL"                                                                  // Pfad, in das gewählte PWM Stufe abgespeichert und bei Start ausgelesen werden sollen
#define BL_PWM_VAL_DEFAULT 0                                                                      // default Wert nach Formatierung des Speichers, erste der vordefinierten Stufen

////////// SETUP //////////
void setup()
{
  pinMode(TFT_BL, OUTPUT);                                                                        // Pin für Hintergrundbeleuchtung als Ausgang deklarieren und ausschalten um flackern durch Initialisierung zu kaschieren
  digitalWrite(TFT_BL, LOW);
  esp_task_wdt_init(WDT_TIMEOUT, true);                                                           // Initialisierung des WDT um Abstürze abzufangen
  esp_task_wdt_add(NULL);
  Timer0_Cfg = timerBegin(0, 80, true);                                                           // timer 0, timer divider, aufsteigender Timer (true)
  timerAttachInterrupt(Timer0_Cfg, &Timer0_ISR, true);                                            // aktiviere Timer interrupt, aufruf von Timer0_ISR alle...
  timerAlarmWrite(Timer0_Cfg, 3000, true);                                                        // ... 3 sek. um Timer zurückzusetzen (wenn nicht möglich, Gerät aufgehangen, Hardware Reset wird ausgelöst)
  timerAlarmEnable(Timer0_Cfg);                                                                   // aktiviere WDT

  pinMode(Button_A, INPUT_PULLUP);                                                                // initialisiere linken (A) und rechten (B) Button
  pinMode(Button_B, INPUT_PULLUP);
  pinMode(SD_Card_detect, INPUT_PULLUP);                                                          // initialisiere SD Karten erkennung
  attachInterrupt(digitalPinToInterrupt(Button_A), Button_A_pressed, FALLING);                    // Button erkennung durch Interrupt bei fallender Flanke
  attachInterrupt(digitalPinToInterrupt(Button_B), Button_B_pressed, FALLING);
  pinMode(LCD_CS, OUTPUT);                                                                        // LCD_CS und SD_CD Pin Initialisierung und High Schalten, damit zunächst beide Geräte nicht auf SPI Bus hören oder reinquatschen
  pinMode(SD_CS, OUTPUT);
  digitalWrite(LCD_CS, HIGH);
  digitalWrite(SD_CS, HIGH);
  
  tft.init();                                                                                     // initialisiere LCD gemäß "User_Setup.h" in "TFT_eSPI" library
  tft.setRotation(3);                                                                             // Bild so drehen, dass es passt
  tft.fillScreen(TFT_BLACK);                                                                      // LCD mit schwarzen Pixeln füllen
  tft.setTextSize(1);                                                                             // kleinste Textgröße definieren
  tft.setTextColor(TFT_WHITE);                                                                    // Weiße Textfarbe
  tft.setCursor(0,0);                                                                             // Cursor in obere links Ecke für nachfolgenden Text
  ledcSetup(BL_CH, BL_Freq, BL_RES);                                                              // Initialisiere PWM Kanal für Hintergrundbeleuchtung
  ledcAttachPin(TFT_BL, BL_CH);                                                                   // PWM Kanal an Pin für Hintergrundbeleuchtung knüpfen
  delay(50);                                                                                      // warte mögliches Flackern durch Initialisierung ab

  if ((digitalRead(Button_A) == LOW && digitalRead(Button_B) == LOW) || !SPIFFS.begin())          // wenn während Start linker und rechter Button gleichzeitig gedrückt werden, Werkseinstellungen laden
  SPIFFS_FACTORY_RESET();
  
  brightness_CH = SPIFFS_GET_INT(BL_PWM_VAL);                                                     // lade gespeicherten Wert für Stärke der Hintergrundbeleuchtung
  if(brightness_CH >= 9)                                                                          // nur Werte zwischen 0 und 8 logisch. wenn drüber, zurücksetzen und default Wert speichern
  {
    brightness_CH = BL_PWM_VAL_DEFAULT;
    SPIFFS_PUT(BL_PWM_VAL, brightness_CH);
  }
  ledcWrite(BL_CH, brightness[brightness_CH]);                                                    // anwenden der geladenen Helligkeit

  SPI.begin(SCK, MISO, MOSI, -1);                                                                 // initialisiere SPI Schnittstelle mit den richtigen Pins
  if(!SD.begin(SD_CS) || digitalRead(SD_Card_detect) == HIGH)                                     // initialisiere SD Karte
  {
    SD_Card_missing();                                                                            // aufruf wenn SD Karte nicht gefunden oder initialisiert werden konnte (dead End)
  }
  uint8_t cardType = SD.cardType();                                                               // ermittle diverse SD Karten Eigenschaften und stelle diese auf LCD dar

  tft.println("Release: " + version);
  tft.println("Date: " + date);

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
  delay(1000);

  updateFromFS(SD);                                                                               // prüfe auf "SO_update.bin" in Stammverzeichnis. wenn vorhanden, Update der Firmware

  tft.fillScreen(TFT_BLACK);
  totalFiles = getGifInventory( "/gif" );                                                         // scanne "gif" Ordner auf SD Karte nach vorhandenen gif Daten
  if(totalFiles <= 0) GifInventoryError(totalFiles);

  Button_A_pressed_event = false;                                                                 // eventuelle Ergebnisse von Interrupts während Initialisierung zurücksetzen
  Button_B_pressed_event = false;
}

////////// FIRMWARE UPDATE VIA SD-KARTE //////////
void updateFromFS(fs::FS &fs)
{
   File updateBin = fs.open("/SO_update.bin");                                                    // Suche und öffne Datei "SO_update.bin" in Stammverzeichnis
   if (updateBin)                                                                                 // wenn Datei geöffnet werden konnte, diverse eigenschaften prüfen und gegebenenfalls Update durchführen
   {
      ledcWrite(BL_CH, brightness[0]);                                                            // setze maximale Helligkeit, damit Anwender auf jeden Fall Prozess verfolgen kann
      tft.fillScreen(TFT_DARKCYAN);                                                               // Hintergrundfarbe = Dunkel Cyan
      tft.setCursor(0,0);                                                                         // setze Textcursor auf linke obere Ecke
      
      if(updateBin.isDirectory())                                                                 // wenn geöffnetes Element ein Verzeichnis ist, Update nicht durchführen
      {
        tft.println("File is Directory");
        updateBin.close();                                                                        // Verzeichnis schließen
        fs.rmdir("/SO_update.bin");                                                               // entferne das Verzeichnis von SD Karte, damit nach anschließendem Neustart Update nicht erneut getriggert wird
      }
      else
      {
        size_t updateSize = updateBin.size();                                                     // ermittle Größe der Update Datei

        if (updateSize > 0)                                                                       // wenn Datei größer 0 Bytes...
        {
          tft.println("Update file found");
          performUpdate(updateBin, updateSize);                                                   // ... Update durchführen
        }
        else tft.println("Error, file is empty");                                                 // wenn Datei = 0 Bytes, Datei leer, Update abbrechen

        updateBin.close();                                                                        // Datei nach (nicht getätigen) Update schließen
        fs.remove("/SO_update.bin");                                                              // entferne Datei von SD Karte, damit nach anschließendem Neustart Update nicht erneut getriggert wird
      }

      tft.println("restart in 3 sec.");                                                           // neustart des ESP32 in 3 sek.
      delay(3000);
      ESP.restart();
   }
}

void performUpdate(Stream &updateSource, size_t updateSize)
{
   if (Update.begin(updateSize))
   {      
      Update.writeStream(updateSource);                                                           // Funktion schreibt SO_update.bin in Speicher

      if (Update.end())                                                                           // nach Ende des Updates (egal ob mit positiven oder negativen Ergebnis)
      {
         if (Update.isFinished())                                                                 // wurde erfolgreich Beendet, weise Anwender darauf hin
         {
           tft.setTextColor(TFT_GREEN);
           tft.println("Update successful!");
         }
         else                                                                                     // wenn nicht erfolgreich, auch darauf hinweisen
         {
           tft.setTextColor(TFT_RED);
           tft.println("Update ended with Error!");
           tft.setTextColor(TFT_WHITE);
         }
      }
      else tft.println("Error #: " + String(Update.getError()));                                  // unbekannter Fehler, ausgabe einer Fehlermeldung

   }
   else tft.println("not enough space");                                                          // wenn Update Datei zu groß, Update abbrechen und anwender darauf hinweisen
}

////////// BUTTON INTERRUPT HANDLING //////////
void Button_A_pressed()                                                                           // Interrupt wenn Button gedrückt wird, wird an entsprechender Stelle in Code verarbeitet und Variable zurückgesetzt
{
  Button_A_pressed_event = true;
}

void Button_B_pressed()
{
  Button_B_pressed_event = true;
}

////////// SPIFFS (INTERNER SPEICHER) //////////
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

////////// SD-KARTEN ERROR HANDLING //////////
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

////////// GIF HANDLING //////////
int getGifInventory(const char* basePath)                                                         // zählt wie viele Gif Daten gefunden wurden und merkt sich jeden Namen in String Array
{
  static File GifRootFolder;                                                                      // zeigt auf Stammverzeichnis der SD Karte
  int GifFiles_counter = 0;                                                                       // zeigt auf Feld in String Array mit Dateinamen und zählt anzahl ermittelter gif Daten
  String checkFileType = "";                                                                      // zwischenspeicher um auf Dateityp zu prüfen
  GifRootFolder = SD.open(basePath);

  if(!GifRootFolder)                                                                              // prüfe ob gif ordner existiert und geöffnet werden kann. wenn nicht, abbruch und aufrufendes Programm über Problem informieren
  {
    log_n("Failed to open directory");
    return -1;
  }
  if(!GifRootFolder.isDirectory())
  {
    log_n("Not a directory");
    return -1;
  }

  File file = GifRootFolder.openNextFile();                                                       // öffne erste Datei in gif Ordner

  tft.setTextColor(TFT_WHITE, TFT_BLACK);                                                         // vorbereite Darstellung von Anzahl gefundener gif Daten
  tft.setTextSize(2);

  int textPosX = tft.width()/2 - 16;                                                              // Text Zentrieren und ausgabe der ersten Zeile
  int textPosY = tft.height()/2 - 10;

  tft.drawString("GIF Files:", textPosX-40, textPosY-20 );

  while(file && GifFiles_counter < GifFiles_size)                                                 // so lange daten geöffnet werden können und maximale anzahl an gif daten nicht überschritten wurden
  {
    if(!file.isDirectory())                                                                       // prüfen ob nächstes geladenes Element ein Ordner ist. wenn ja, überspringen
    {
      checkFileType = "";                                                                         // Variable zum testen von Dateityp frei machen
      const char* fileName = file.name();                                                         // zwischenspeichern von Dateinamen zum weiteren Auswerten
      for(int i = strlen(fileName) - 4; i < strlen(fileName); i++) checkFileType += fileName[i];  // die letzten 4 Zeichn in Dateinamen auswerten
      if (checkFileType == ".gif" || checkFileType == ".GIF")                                     // prüfen ob Dateiendung wie erwartet ist (.gif)
      {
        GifFiles[GifFiles_counter] = String("/gif/") + file.name();                               // merken des Namens der geladenen Datei
        GifFiles_counter++;
        tft.drawString(String(GifFiles_counter), textPosX, textPosY);                             // darstellen der aktuell gezählten Menge
      }
    }
    file.close();
    file = GifRootFolder.openNextFile();                                                          // öffne nächste Datei in gif Ordner
  }
  GifRootFolder.close();                                                                          // schließe gif Ordner
  log_n("Found %d GIF files", GifFiles_counter);                                                  // ausgabe der Anzahl ermittelter gif Daten an Serielle Schnittstelle
  return GifFiles_counter;                                                                        // Anzahl ermittelter gif Daten an aufrufendes Programm zurückgeben
}

void GifInventoryError(int totalFiles)                                                            // dead end, wenn kein gif Ordner gefunden wurde, oder Inhalt leer ist
{
  Button_A_pressed_event = false;                                                                 // eventuelle Ergebnisse von Interrupts zurücksetzen
  Button_B_pressed_event = false;
  tft.fillScreen(TFT_RED);
  tft.setTextColor(TFT_BLACK);
  tft.setTextSize(2);
  tft.setCursor(20,7);
  tft.println("gif folder");
  if(totalFiles < 0)                                                                              // gif Ordner fehlt = ERROR code -1
  {
    tft.setCursor(33,27);
    tft.println("Missing!");
  }
  else if(totalFiles == 0)                                                                        // gif Ordner vorhanden, aber leer oder keine .gif Daten gefunden
  {
    tft.setCursor(45,27);
    tft.println("Empty!");
  }
  tft.setTextSize(1);
  tft.setCursor(27,50);
  tft.println("create gif folder");
  tft.setCursor(8,60);
  tft.println("and fill with .gif files");
  while(1)
  {
    if(Button_A_pressed_event == true || Button_B_pressed_event == true) ESP.restart();           // warte auf drücken von linken oder rechten Button um neustart zu triggern
  }
}

int gifPlay( char* gifPath )
{
  gif.begin(BIG_ENDIAN_PIXELS);

  if(!gif.open(gifPath, GIFOpenFile, GIFCloseFile, GIFReadFile, GIFSeekFile, GIFDraw))            // öffne gif mit hilfe der nachfolgenden Funktionen
  {
    log_n("Could not open gif %s", gifPath);                                                      // logging Ausgabe über Serielle Schnittstelle, welches gif nicht geladen werden konnte
    return maxLoopsDuration;                                                                      // wenn gif nicht geöffnet werden kann, maßnahme um zum nächsten gif zu springen (so tun als sei das gif schon lange genug gelaufen)
  }

  int frameDelay = 0;                                                                             // ermittle anzeigedauer des aktuell gespielten Frames, damit dies zur gesamtdauer dazu addiert werden kann
  int then = 0;                                                                                   // merkt sich gesamte Laufzeit des gif
  static int lastFile = -1;                                                                       // zeigt auf zuletzt abgespieltes gif

  int w = gif.getCanvasWidth();                                                                   // zentrieren des gif
  int h = gif.getCanvasHeight();
  xOffset = ( tft.width()  - w ) /2;
  yOffset = ( tft.height() - h ) /2;

  if(lastFile != currentFile)                                                                     // wenn neues gif beginnt, ausgabe des Dateinamens vie Serielle Schnittstelle
  {
    log_n("Playing %s [%d,%d] with offset [%d,%d]", gifPath, w, h, xOffset, yOffset );
    lastFile = currentFile;
  }

  while (gif.playFrame(true, &frameDelay))                                                        // abspielen des nächsten Frames, merken der dafür benötigten Zeit
  {
    if(digitalRead(SD_Card_detect) == HIGH) SD_Card_missing();                                    // wenn herausziehen der SD Karte erkannt wird, Programm ins dead end schicken

    then += frameDelay;                                                                           // merke gesamt benötigte Zeit für aktuell abgespieltes gif
    if(then > maxGifDuration) break;                                                              // wenn gif länger als maximal definierte Zeit läuft, loop unterbrechen

    if(Button_A_pressed_event == true)                                                            // wenn linker Button gedrückt wurde, helligkeitsstufen durchschalten
    {
      Button_A_pressed_event = false;                                                             // zurücksetzen von merker (wird durch interrupt gesetzt)
      if(interrupt_gif == false && Button_A_debounce_timer < millis() && digitalRead(Button_A) == LOW)
      {                                                                                           // debounce maßnahmen und sicheres erkennen von Button events
        Button_A_debounce_timer = millis() + Button_debounce_time;                                // merke Zeit wie lange keine weitere Eingaben erkannt werden sollen
        brightness_CH += 1;                                                                       // helligkeitsstufe durchschalten
        if(brightness_CH >= 9) brightness_CH = 0;                                                 // wenn bei letzter Stufe angelangt, wieder mit erster Stufe weiter machen
        ledcWrite(BL_CH, brightness[brightness_CH]);                                              // neue Helligkeit anwenden
        SPIFFS_PUT(BL_PWM_VAL, brightness_CH);                                                    // merke neuen Zustand in internen Speicher

        //currentFile = currentFile - 2;                                                          // alternative zum regeln der Helligkeit, vorheriges gif abspielen
        //if(currentFile < 0) currentFile = totalFiles - 1;
        //interrupt_gif = true;
        //break;
      }
    }
    else if(Button_B_pressed_event == true)                                                       // wenn rechter Button gedrückt wurde, aktuelles gif unterbrechen und mit nächstes weiter machen
    {
      Button_B_pressed_event = false;
      if(interrupt_gif == false && Button_B_debounce_timer < millis() && digitalRead(Button_B) == LOW)
      {
        Button_B_debounce_timer = millis() + Button_debounce_time;
        interrupt_gif = true;
        break;
      }
    }
  }

  gif.close();

  return then;
}

static void * GIFOpenFile(const char *fname, int32_t *pSize)
{
  static File FSGifFile;
  FSGifFile = SD.open(fname);
  if(FSGifFile)
  {
    *pSize = FSGifFile.size();
    return (void *)&FSGifFile;
  }
  return NULL;
}

static void GIFCloseFile(void *pHandle)
{
  File *f = static_cast<File *>(pHandle);
  if(f != NULL)
     f->close();
}

static int32_t GIFReadFile(GIFFILE *pFile, uint8_t *pBuf, int32_t iLen)
{
  int32_t iBytesRead;
  iBytesRead = iLen;
  File *f = static_cast<File *>(pFile->fHandle);
  if((pFile->iSize - pFile->iPos) < iLen) iBytesRead = pFile->iSize - pFile->iPos - 1;
  if(iBytesRead <= 0) return 0;
  iBytesRead = (int32_t)f->read(pBuf, iBytesRead);
  pFile->iPos = f->position();
  return iBytesRead;
}

static int32_t GIFSeekFile(GIFFILE *pFile, int32_t iPosition)
{
  int i = micros();
  File *f = static_cast<File *>(pFile->fHandle);
  f->seek(iPosition);
  pFile->iPos = (int32_t)f->position();
  i = micros() - i;
  return pFile->iPos;
}

void GIFDraw(GIFDRAW *pDraw)
{
  uint8_t *s;
  uint16_t *d, *usPalette, usTemp[320];
  int x, y, iWidth;

  iWidth = pDraw->iWidth;
  if(iWidth > tft.width()) iWidth = tft.width();
  usPalette = pDraw->pPalette;
  y = pDraw->iY + pDraw->y;

  s = pDraw->pPixels;
  if(pDraw->ucDisposalMethod == 2)
  {
    for(x=0; x<iWidth; x++)
    {
      if(s[x] == pDraw->ucTransparent) s[x] = pDraw->ucBackground;
    }
    pDraw->ucHasTransparency = 0;
  }
  if(pDraw->ucHasTransparency)
  {
    uint8_t *pEnd, c, ucTransparent = pDraw->ucTransparent;
    int x, iCount;
    pEnd = s + iWidth;
    x = 0;
    iCount = 0;
    while(x < iWidth)
    {
      c = ucTransparent-1;
      d = usTemp;
      while(c != ucTransparent && s < pEnd)
      {
        c = *s++;
        if(c == ucTransparent) s--;
        else
        {
            *d++ = usPalette[c];
            iCount++;
        }
      }
      if(iCount)
      {
        TFTDraw(pDraw->iX+x, y, iCount, 1, (uint16_t*)usTemp);
        x += iCount;
        iCount = 0;
      }
      c = ucTransparent;
      while(c == ucTransparent && s < pEnd)
      {
        c = *s++;
        if(c == ucTransparent) iCount++;
        else s--;
      }
      if(iCount)
      {
        x += iCount;
        iCount = 0;
      }
    }
  }
  else
  {
    s = pDraw->pPixels;
    for (x=0; x<iWidth; x++) usTemp[x] = usPalette[*s++];
    TFTDraw(pDraw->iX, y, iWidth, 1, (uint16_t*)usTemp);
  }
}

static void TFTDraw(int x, int y, int w, int h, uint16_t* lBuf )
{
  tft.pushRect( x+xOffset, y+yOffset, w, h, lBuf );
}

////////// LOOP //////////
void loop()
{
  tft.fillScreen(TFT_BLACK);                                                                      // Bildschirm schwarz füllen um Pixel Leichen vom jeweils vorherigen gif zu vermeiden
  if(digitalRead(SD_Card_detect) == HIGH) SD_Card_missing();                                      // wenn SD Karte während des Betriebs raus gezogen wird, schicke Programm in dead end
  const char * fileName = GifFiles[currentFile++%totalFiles].c_str();                             // Vorbereite Pfad zum nächsten gif

  int loops = maxLoopIterations;                                                                  // lade "loops" mit Wert für maximale anzahl an Durchgänge pro gif
  int durationControl = maxLoopsDuration;                                                         // lade "durationControl" mit maximal definierte länge für gif Abspiel Dauer

  while(loops-->0 && durationControl > 0 && interrupt_gif == false)                               // decrementiere loops und prüfe ob noch größer 0 | gif abspiellänge schon länger als vorher definiert? | Abspielen von Gif durch Button überspringen?
  {
    durationControl -= gifPlay((char*)fileName);                                                  // gif abspielen und Dauer in "durationControl" abziehen
    gif.reset();                                                                                  // zurücksetzen der gif State machine
  }
  interrupt_gif = false;                                                                          // Variable die den Abbruch der aktuellen gif Schleife anweist wieder zurücksetzen
}