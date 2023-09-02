void SO_BUTTON_SETUP()
{
  pinMode(Button_A, INPUT_PULLUP);                                                                // initialisiere linken (A) und rechten (B) Button
  pinMode(Button_B, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(Button_A), Button_A_pressed, FALLING);                    // Button erkennung durch Interrupt bei fallender Flanke
  attachInterrupt(digitalPinToInterrupt(Button_B), Button_B_pressed, FALLING);
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

bool BUTTON_loop()
{
  static unsigned int Button_hold_timer = 0;
  static unsigned int TFT_BL_Change_timer = 0;

  if (digitalRead(Button_A) == HIGH && digitalRead(Button_B) == HIGH) Button_hold_timer = millis();
  else if(Button_hold_timer + 1000 < millis())
  {
    if(TFT_BL_Change_timer + 400 < millis())
    {
      if(digitalRead(Button_A) == LOW && digitalRead(Button_B) == HIGH)
      {
        brightness_CH += 1;                                                                       // helligkeitsstufe durchschalten
        if(brightness_CH >= 8) brightness_CH = 8;                                                 // wenn bei letzter Stufe angelangt, wieder mit erster Stufe weiter machen
        ledcWrite(BL_CH, brightness[brightness_CH]);                                              // neue Helligkeit anwenden
        SPIFFS_PUT(BL_PWM_VAL, brightness_CH);                                                    // merke neuen Zustand in internen Speicher
        TFT_BL_Change_timer = millis();
      }
      else if(digitalRead(Button_B) == LOW && digitalRead(Button_A) == HIGH)
      {
        brightness_CH -= 1;                                                                       // helligkeitsstufe durchschalten
        if(brightness_CH <= 0) brightness_CH = 0;                                                 // wenn bei letzter Stufe angelangt, wieder mit erster Stufe weiter machen
        ledcWrite(BL_CH, brightness[brightness_CH]);                                              // neue Helligkeit anwenden
        SPIFFS_PUT(BL_PWM_VAL, brightness_CH);                                                    // merke neuen Zustand in internen Speicher
        TFT_BL_Change_timer = millis();
      }
    }
  }
  if(Button_A_pressed_event == true)                                                            // wenn linker Button gedrückt wurde, vorherige Datei abspielen
  {
    if(interrupt_current_play == false && Button_A_debounce_timer < millis() && digitalRead(Button_A) == LOW)
    {                                                                                           // debounce maßnahmen und sicheres erkennen von Button events
      Button_A_debounce_timer = millis() + Button_debounce_time;                                // merke Zeit wie lange keine weitere Eingaben erkannt werden sollen
      interrupt_current_play = true;
      return true;
    }
  }
  else if(Button_B_pressed_event == true)                                                       // wenn rechter Button gedrückt wurde, aktuelle Datei unterbrechen und mit nächste weiter machen
  {
    if(interrupt_current_play == false && Button_B_debounce_timer < millis() && digitalRead(Button_B) == LOW)
    {
      Button_B_debounce_timer = millis() + Button_debounce_time;
      interrupt_current_play = true;
      return true;
    }
  }
  return false;
}