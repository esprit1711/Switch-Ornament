#ifndef SO_WDT_H
#define SO_WDT_H

////////// BIBLIOTHEKEN INCLUDIEREN //////////
#include <esp_task_wdt.h>                                                                         // Watch-Dog-Timer um Abstürze abzufangen

////////// GLOBALE VARIABLEN ERZEUGEN //////////
#define WDT_TIMEOUT 5//s                                                                          // automatischer Neustart, wenn System 5 sek. lang nicht ansprechbar ist

hw_timer_t *Timer0_Cfg = NULL;                                                                    // deklariere Hardware Timer für WDT
void IRAM_ATTR Timer0_ISR()                                                                       // Cyclischer Aufruf des Timer interrupts um WDT zurückzusetzen
{
  esp_task_wdt_reset();
}

////////// FUNKTIONSPROTOTYPEN //////////
void SO_WDT_SETUP();

#endif