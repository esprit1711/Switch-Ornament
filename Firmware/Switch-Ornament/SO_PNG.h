#ifndef SO_PNG_H
#define SO_PNG_H

////////// BIBLIOTHEKEN INCLUDIEREN //////////
#include <PNGdec.h>

////////// GLOBALE VARIABLEN ERZEUGEN //////////
static int PngtotalFiles = 0;                                                                     // anzahl existierender png Daten (wird am Anfang immer ermittelt)
static int PngcurrentFile = 0;                                                                    // zeigt auf aktuell dargestelltes PNG
const int PngFiles_size = 5000;                                                                   // maximale Anzahl an Pngs, magische Grenze = 14579, alles darüber geht nicht (SD Karte nicht gefunden)
String PngFiles[PngFiles_size];                                                                   // merkt sich die Pfade zu jedem png auf der SD Karte
File myfile;
PNG png;

////////// FUNKTIONSPROTOTYPEN //////////
void * myOpen(const char *filename, int32_t *size);
void myClose(void *handle);
int32_t myRead(PNGFILE *handle, uint8_t *buffer, int32_t length);
int32_t mySeek(PNGFILE *handle, int32_t position);
void PNGDraw(PNGDRAW *pDraw);
int getPngInventory(const char* basePath);
void pngPlay(char* pngPath);
void PNG_loop();

#endif