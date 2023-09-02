#ifndef SO_SD_H
#define SO_SD_H

////////// BIBLIOTHEKEN INCLUDIEREN //////////
#include "SD.h"                                                                                   // SD Karten Zugriff
#include "SPI.h"                                                                                  // Initialisierung der SPI Schnittstelle (für LCD & SD Kartenzugriff)

////////// GLOBALE VARIABLEN ERZEUGEN //////////
#define SD_Card_detect 0                                                                          // SD Karten erkennung
#define SD_CS 4

////////// FUNKTIONSPROTOTYPEN //////////
void SO_SD_CS_HIGH();
void SO_SD_SETUP();
void SD_Card_missing();

#endif