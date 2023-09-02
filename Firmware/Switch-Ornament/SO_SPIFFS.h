#ifndef SO_SPIFFS_H
#define SO_SPIFFS_H

////////// BIBLIOTHEKEN INCLUDIEREN //////////
#include "FS.h"                                                                                   // Dateisystem
#include "SPIFFS.h"                                                                               // lesen und Schreiben auf internen Speicher des ESP32

////////// GLOBALE VARIABLEN ERZEUGEN //////////

////////// FUNKTIONSPROTOTYPEN //////////
void SO_SPIFFS_SETUP();
void SPIFFS_FACTORY_RESET();
void fill_SPIFFS(String Info_Text, String file_to_open, int data_to_put);
bool SPIFFS_PUT(String file_to_open, int data_to_put);
int SPIFFS_GET_INT(String file_to_open);

#endif