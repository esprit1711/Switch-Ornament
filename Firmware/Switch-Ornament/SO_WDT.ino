void SO_WDT_SETUP()
{
  esp_task_wdt_init(WDT_TIMEOUT, true);                                                           // Initialisierung des WDT um Abstürze abzufangen
  esp_task_wdt_add(NULL);
  Timer0_Cfg = timerBegin(0, 80, true);                                                           // timer 0, timer divider, aufsteigender Timer (true)
  timerAttachInterrupt(Timer0_Cfg, &Timer0_ISR, true);                                            // aktiviere Timer interrupt, aufruf von Timer0_ISR alle...
  timerAlarmWrite(Timer0_Cfg, 3000, true);                                                        // ... 3 sek. um Timer zurückzusetzen (wenn nicht möglich, Gerät aufgehangen, Hardware Reset wird ausgelöst)
  timerAlarmEnable(Timer0_Cfg);                                                                   // aktiviere WDT
}