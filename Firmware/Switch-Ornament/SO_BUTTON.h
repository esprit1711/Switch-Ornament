#ifndef SO_BUTTON_H
#define SO_BUTTON_H

////////// BIBLIOTHEKEN INCLUDIEREN //////////

////////// GLOBALE VARIABLEN ERZEUGEN //////////
int Button_debounce_time = 100;//ms                                                               // Zeit in der keine weiteren Button Klicks akzeptiert werden sollen (entprellen)

#define Button_A 3                                                                                // Button links
#define Button_B 1                                                                                // Button rechts

bool Button_A_pressed_event = false;                                                              // true durch Interrupt wenn Button gedrückt wird
bool Button_B_pressed_event = false;
unsigned int Button_A_debounce_timer = 0;                                                         // Software entsprellen von Taster
unsigned int Button_B_debounce_timer = 0;

////////// FUNKTIONSPROTOTYPEN //////////
void SO_BUTTON_SETUP();
bool BUTTON_loop();

#endif