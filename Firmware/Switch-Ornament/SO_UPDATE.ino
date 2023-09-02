void updateFromFS(fs::FS &fs)
{
   File updateBin = fs.open("/SO_update.bin");                                                    // Suche und öffne Datei "SO_update.bin" in Stammverzeichnis
   if (updateBin)                                                                                 // wenn Datei geöffnet werden konnte, diverse eigenschaften prüfen und gegebenenfalls Update durchführen
   {
      ledcWrite(BL_CH, brightness[0]);                                                            // setze maximale Helligkeit, damit Anwender auf jeden Fall Prozess verfolgen kann
      tft.fillScreen(TFT_DARKCYAN);                                                               // Hintergrundfarbe = Dunkel Cyan
      tft.setCursor(0,0);                                                                         // setze Textcursor auf linke obere Ecke
      
      if(updateBin.isDirectory())                                                                 // wenn geöffnetes Element ein Verzeichnis ist, Update nicht durchführen
      {
        tft.println("File is Directory");
        updateBin.close();                                                                        // Verzeichnis schließen
        fs.rmdir("/SO_update.bin");                                                               // entferne das Verzeichnis von SD Karte, damit nach anschließendem Neustart Update nicht erneut getriggert wird
      }
      else
      {
        size_t updateSize = updateBin.size();                                                     // ermittle Größe der Update Datei

        if (updateSize > 0)                                                                       // wenn Datei größer 0 Bytes...
        {
          tft.println("Update file found");
          performUpdate(updateBin, updateSize);                                                   // ... Update durchführen
        }
        else tft.println("Error, file is empty");                                                 // wenn Datei = 0 Bytes, Datei leer, Update abbrechen

        updateBin.close();                                                                        // Datei nach (nicht getätigen) Update schließen
        fs.remove("/SO_update.bin");                                                              // entferne Datei von SD Karte, damit nach anschließendem Neustart Update nicht erneut getriggert wird
      }

      tft.println("restart in 3 sec.");                                                           // neustart des ESP32 in 3 sek.
      delay(3000);
      ESP.restart();
   }
}

void performUpdate(Stream &updateSource, size_t updateSize)
{
   if (Update.begin(updateSize))
   {      
      Update.writeStream(updateSource);                                                           // Funktion schreibt SO_update.bin in Speicher

      if (Update.end())                                                                           // nach Ende des Updates (egal ob mit positiven oder negativen Ergebnis)
      {
         if (Update.isFinished())                                                                 // wurde erfolgreich Beendet, weise Anwender darauf hin
         {
           tft.setTextColor(TFT_GREEN);
           tft.println("Update successful!");
         }
         else                                                                                     // wenn nicht erfolgreich, auch darauf hinweisen
         {
           tft.setTextColor(TFT_RED);
           tft.println("Update ended with Error!");
           tft.setTextColor(TFT_WHITE);
         }
      }
      else tft.println("Error #: " + String(Update.getError()));                                  // unbekannter Fehler, ausgabe einer Fehlermeldung

   }
   else tft.println("not enough space");                                                          // wenn Update Datei zu groß, Update abbrechen und anwender darauf hinweisen
}