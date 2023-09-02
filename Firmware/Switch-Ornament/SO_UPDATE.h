#ifndef SO_UPDATE_H
#define SO_UPDATE_H

////////// BIBLIOTHEKEN INCLUDIEREN //////////
#include <Update.h>                                                                               // Software Update via SD Karte

////////// GLOBALE VARIABLEN ERZEUGEN //////////

////////// FUNKTIONSPROTOTYPEN //////////
void updateFromFS(fs::FS &fs);
void performUpdate(Stream &updateSource, size_t updateSize);

#endif