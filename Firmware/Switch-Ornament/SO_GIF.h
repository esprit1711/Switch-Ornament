#ifndef SO_GIF_H
#define SO_GIF_H

////////// BIBLIOTHEKEN INCLUDIEREN //////////
#include <AnimatedGIF.h>                                                                          // abspielen von GIFs

////////// GLOBALE VARIABLEN ERZEUGEN //////////
AnimatedGIF gif;
static int xOffset = 0;                                                                           // wird genutzt um gif zu zentrieren
static int yOffset = 0;
static int GiftotalFiles = 0;                                                                     // anzahl existierender gif Daten (wird am Anfang immer ermittelt)
static int GifcurrentFile = 0;                                                                    // zeigt auf aktuell abgespieltes gif
const int GifFiles_size = 5000;                                                                   // maximale Anzahl an Gifs, magische Grenze = 14579, alles darüber geht nicht (SD Karte nicht gefunden)
String GifFiles[GifFiles_size];                                                                   // merkt sich die Pfade zu jedem Gif auf der SD Karte

////////// FUNKTIONSPROTOTYPEN //////////
int getGifInventory(const char* basePath);
void GifInventoryError(int GiftotalFiles);
int gifPlay(char* gifPath);
static void * GIFOpenFile(const char *fname, int32_t *pSize);
static void GIFCloseFile(void *pHandle);
static int32_t GIFReadFile(GIFFILE *pFile, uint8_t *pBuf, int32_t iLen);
static int32_t GIFSeekFile(GIFFILE *pFile, int32_t iPosition);
void GIFDraw(GIFDRAW *pDraw);
static void TFTDraw(int x, int y, int w, int h, uint16_t* lBuf);
void GIF_loop();

#endif