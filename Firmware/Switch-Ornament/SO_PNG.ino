void * myOpen(const char *filename, int32_t *size)
{
  myfile = SD.open(filename);
  *size = myfile.size();
  return &myfile;
}

void myClose(void *handle)
{
  if (myfile) myfile.close();
}

int32_t myRead(PNGFILE *handle, uint8_t *buffer, int32_t length)
{
  if (!myfile) return 0;
  return myfile.read(buffer, length);
}

int32_t mySeek(PNGFILE *handle, int32_t position)
{
  if (!myfile) return 0;
  return myfile.seek(position);
}

// Function to draw pixels to the display
void PNGDraw(PNGDRAW *pDraw)
{
  uint16_t usPixels[320];
  png.getLineAsRGB565(pDraw, usPixels, PNG_RGB565_BIG_ENDIAN, 0xffffffff);
  tft.pushRect((160 / 2 - pDraw->iWidth / 2), pDraw->y + 0, pDraw->iWidth, 1, usPixels);
}

int getPngInventory(const char* basePath)                                                         // zählt wie viele png Daten gefunden wurden und merkt sich jeden Namen in String Array
{
  tft.fillScreen(TFT_BLACK);
  static File PngRootFolder;                                                                      // zeigt auf Stammverzeichnis der SD Karte
  int PngFiles_counter = 0;                                                                       // zeigt auf Feld in String Array mit Dateinamen und zählt anzahl ermittelter png Daten
  String checkFileType = "";                                                                      // zwischenspeicher um auf Dateityp zu prüfen
  String FileTypeHeader = "";
  PngRootFolder = SD.open(basePath);

  if(!PngRootFolder)                                                                              // prüfe ob png ordner existiert und geöffnet werden kann. wenn nicht, abbruch und aufrufendes Programm über Problem informieren
  {
    log_n("Failed to open directory");
    return -1;
  }
  if(!PngRootFolder.isDirectory())
  {
    log_n("Not a directory");
    return -1;
  }

  File file = PngRootFolder.openNextFile();                                                       // öffne erste Datei in png Ordner

  tft.setTextColor(TFT_WHITE, TFT_BLACK);                                                         // vorbereite Darstellung von Anzahl gefundener png Daten
  tft.setTextSize(2);

  int textPosX = tft.width()/2 - 16;                                                              // Text Zentrieren und ausgabe der ersten Zeile
  int textPosY = tft.height()/2 - 10;

  tft.drawString("PNG Files:", textPosX-40, textPosY-20 );                                        // Ausgabe von "PNG Files:" Text Mitte oben von Bildschirm

  while(file && PngFiles_counter < PngFiles_size)                                                 // so lange daten geöffnet werden können und maximale anzahl an png daten nicht überschritten wurden
  {
    if(!file.isDirectory())                                                                       // prüfen ob nächstes geladenes Element ein Ordner ist. wenn ja, überspringen
    {
      checkFileType = "";                                                                         // Variable zum testen von Dateityp frei machen
      FileTypeHeader = "";
      const char* fileName = file.name();                                                         // zwischenspeichern von Dateinamen zum weiteren Auswerten
      for(int i = strlen(fileName) - 4; i < strlen(fileName); i++) checkFileType += fileName[i];  // die letzten 4 Zeichen in Dateinamen auswerten
      if (checkFileType == ".png" || checkFileType == ".PNG")                                     // prüfen ob Dateiendung wie erwartet ist (.png)
      {
        file.read();                                                                              // überspringe sonderzeichen in PNG Header
        for(int i = 0; i < 3; i++) FileTypeHeader += char(file.read());                           // prüfe, ob Header von Datei mit "PNG" beginnt
        if(FileTypeHeader == ("PNG"))
        {
          PngFiles[PngFiles_counter] = String("/png/") + file.name();                             // merken des Namens der geladenen Datei
          PngFiles_counter++;
          tft.drawString(String(PngFiles_counter), textPosX, textPosY);                           // darstellen der aktuell gezählten Menge
        }
      }
    }
    file = PngRootFolder.openNextFile();                                                          // öffne nächste Datei in png Ordner
  }
  file.close();
  PngRootFolder.close();                                                                          // schließe png Ordner
  log_n("Found %d PNG files", PngFiles_counter);                                                  // ausgabe der Anzahl ermittelter png Daten an Serielle Schnittstelle
  return PngFiles_counter;                                                                        // Anzahl ermittelter png Daten an aufrufendes Programm zurückgeben
}

void pngPlay(char* pngPath)
{
  int rc;
  rc = png.open((const char *) pngPath, myOpen, myClose, myRead, mySeek, PNGDraw);
  if (rc == PNG_SUCCESS)
  {
    log_n("image specs: (%d x %d), %d bpp, pixel type: %d\n", png.getWidth(), png.getHeight(), png.getBpp(), png.getPixelType());
    rc = png.decode(NULL, 0);
  }
  png.close();
}

void PNG_loop()
{
  const char * fileName = PngFiles[PngcurrentFile++%PngtotalFiles].c_str();                       // Vorbereite Pfad zum nächsten gif
  pngPlay((char*)fileName);
  int durationControl = millis() + maxLoopsDuration;                                              // lade "durationControl" mit maximal definierte länge für gif Abspiel Dauer
  while(millis() < durationControl && interrupt_current_play == false)
  {
    if(digitalRead(SD_Card_detect) == HIGH) SD_Card_missing();                                    // wenn SD Karte während des Betriebs raus gezogen wird, schicke Programm in dead end
    if (BUTTON_loop()) break;
  }
}