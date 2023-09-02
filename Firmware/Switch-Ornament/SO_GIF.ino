int getGifInventory(const char* basePath)                                                         // zählt wie viele Gif Daten gefunden wurden und merkt sich jeden Namen in String Array
{
  tft.fillScreen(TFT_BLACK);
  static File GifRootFolder;                                                                      // zeigt auf Stammverzeichnis der SD Karte
  int GifFiles_counter = 0;                                                                       // zeigt auf Feld in String Array mit Dateinamen und zählt anzahl ermittelter gif Daten
  String checkFileType = "";                                                                      // zwischenspeicher um auf Dateityp zu prüfen
  String FileTypeHeader = "";
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

  tft.drawString("GIF Files:", textPosX-40, textPosY-20 );                                        // Ausgabe von "GIF Files:" Text Mitte oben von Bildschirm

  while(file && GifFiles_counter < GifFiles_size)                                                 // so lange daten geöffnet werden können und maximale anzahl an gif daten nicht überschritten wurden
  {
    if(!file.isDirectory())                                                                       // prüfen ob nächstes geladenes Element ein Ordner ist. wenn ja, überspringen
    {
      checkFileType = "";                                                                         // Variable zum testen von Dateityp frei machen
      FileTypeHeader = "";
      const char* fileName = file.name();                                                         // zwischenspeichern von Dateinamen zum weiteren Auswerten
      for(int i = strlen(fileName) - 4; i < strlen(fileName); i++) checkFileType += fileName[i];  // die letzten 4 Zeichen in Dateinamen auswerten
      if (checkFileType == ".gif" || checkFileType == ".GIF")                                     // prüfen ob Dateiendung wie erwartet ist (.gif)
      {
        for(int i = 0; i < 3; i++) FileTypeHeader += char(file.read());                           // prüfe, ob Header von Datei mit "GIF" beginnt
        if(FileTypeHeader == "GIF")
        {
          GifFiles[GifFiles_counter] = String("/gif/") + file.name();                             // merken des Namens der geladenen Datei
          GifFiles_counter++;
          tft.drawString(String(GifFiles_counter), textPosX, textPosY);                           // darstellen der aktuell gezählten Menge
        }
      }
    }
    file = GifRootFolder.openNextFile();                                                          // öffne nächste Datei in gif Ordner
  }
  file.close();
  GifRootFolder.close();                                                                          // schließe gif Ordner
  log_n("Found %d GIF files", GifFiles_counter);                                                  // ausgabe der Anzahl ermittelter gif Daten an Serielle Schnittstelle
  return GifFiles_counter;                                                                        // Anzahl ermittelter gif Daten an aufrufendes Programm zurückgeben
}

void GifInventoryError(int GiftotalFiles)                                                         // dead end, wenn kein gif Ordner gefunden wurde, oder Inhalt leer ist
{
  tft.fillScreen(TFT_BLACK);

  Button_A_pressed_event = false;                                                                 // eventuelle Ergebnisse von Interrupts zurücksetzen
  Button_B_pressed_event = false;
  tft.fillScreen(TFT_RED);
  tft.setTextColor(TFT_BLACK);
  tft.setTextSize(2);
  tft.setCursor(20,7);
  tft.println("gif folder");
  if(GiftotalFiles < 0)                                                                           // gif Ordner fehlt = ERROR code -1
  {
    tft.setCursor(33,27);
    tft.println("Missing!");
  }
  else if(GiftotalFiles == 0)                                                                     // gif Ordner vorhanden, aber leer oder keine .gif Daten gefunden
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

int gifPlay(char* gifPath)
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

  if(lastFile != GifcurrentFile)                                                                  // wenn neues gif beginnt, ausgabe des Dateinamens vie Serielle Schnittstelle
  {
    log_n("Playing %s [%d,%d] with offset [%d,%d]", gifPath, w, h, xOffset, yOffset );
    lastFile = GifcurrentFile;
  }

  while (gif.playFrame(true, &frameDelay))                                                        // abspielen des nächsten Frames, merken der dafür benötigten Zeit
  {
    if(digitalRead(SD_Card_detect) == HIGH) SD_Card_missing();                                    // wenn herausziehen der SD Karte erkannt wird, Programm ins dead end schicken

    then += frameDelay;                                                                           // merke gesamt benötigte Zeit für aktuell abgespieltes gif
    if(then > maxGifDuration) break;                                                              // wenn gif länger als maximal definierte Zeit läuft, loop unterbrechen
    if (BUTTON_loop()) break;
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

static void TFTDraw(int x, int y, int w, int h, uint16_t* lBuf)
{
  tft.pushRect( x+xOffset, y+yOffset, w, h, lBuf );
}

void GIF_loop()
{
  const char * fileName = GifFiles[GifcurrentFile++%GiftotalFiles].c_str();                       // Vorbereite Pfad zum nächsten gif

  int loops = maxLoopIterations;                                                                  // lade "loops" mit Wert für maximale anzahl an Durchgänge pro gif
  int durationControl = maxLoopsDuration;                                                         // lade "durationControl" mit maximal definierte länge für gif Abspiel Dauer

  while(loops-->0 && durationControl > 0 && interrupt_current_play == false)                      // decrementiere loops und prüfe ob noch größer 0 | gif abspiellänge schon länger als vorher definiert? | Abspielen von Gif durch Button überspringen?
  {
    durationControl -= gifPlay((char*)fileName);                                                  // gif abspielen und Dauer in "durationControl" abziehen
    gif.reset();                                                                                  // zurücksetzen der gif State machine
  }
}