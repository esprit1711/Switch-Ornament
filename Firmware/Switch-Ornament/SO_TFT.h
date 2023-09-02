#ifndef SO_TFT_H
#define SO_TFT_H

////////// BIBLIOTHEKEN INCLUDIEREN //////////
#if defined(use_tft_eSPI_library)
  #include <TFT_eSPI.h>                                                                           // TFT_eSPI library zur Ansteuerung des Displays
#elif defined (use_LGFX_library)
  #include <LovyanGFX.hpp>                                                                        // LovyanGFX library zur Ansteuerung des Displays
#endif

////////// GLOBALE VARIABLEN ERZEUGEN //////////
const int maxLoopIterations =     5;                                                              // Stoppe Gif nach dieser Anzahl an Wiederholungen
const int maxLoopsDuration  =  3000;//ms                                                          // Wenn GIF bereits so lange gespielt wird, gehe nach ende des laufenden loops zum nächsten gif
const int maxGifDuration    = 30000;//ms                                                          // maximale Länge eines gif

#define LCD_type_sel 9                                                                            // Pin Zustand entscheidet über zu verwendendes Display
#define LCD_CS 10                                                                                 // SPI-CS Pin von LCD

#define DISPLAY_WIDTH 160                                                                         // Display Auflösung
#define DISPLAY_HEIGHT 80

#if defined (use_tft_eSPI_library)
  TFT_eSPI tft = TFT_eSPI();
#elif defined (use_LGFX_library)
  class LGFX : public lgfx::LGFX_Device
  {
    //lgfx::Panel_ST7735      _panel_instance;
    lgfx::Panel_ST7735S     _panel_instance;

    lgfx::Bus_SPI        _bus_instance;
    lgfx::Light_PWM     _light_instance;

    public:
      LGFX(void)
      {
        {
        auto cfg = _bus_instance.config();

        cfg.spi_host = SPI2_HOST;
        cfg.spi_mode = 0;                     //(0 ~ 3)
        cfg.freq_write = 27000000;            //27MHz
        cfg.spi_3wire = true;
        cfg.use_lock = true;
        cfg.dma_channel = SPI_DMA_CH_AUTO;    //(1,2,SPI_DMA_CH_AUTO)
        cfg.pin_sclk = 6;
        cfg.pin_mosi = 7;
        cfg.pin_miso = 2;
        cfg.pin_dc = 5;

        _bus_instance.config(cfg);
        _panel_instance.setBus(&_bus_instance);
        }
        {
        auto cfg = _panel_instance.config();

        cfg.pin_cs = 10;
        cfg.pin_rst = -1;
        cfg.pin_busy = -1;

        cfg.panel_width = 80;
        cfg.panel_height = 160;
        cfg.offset_x = -24;
        cfg.offset_y = 0;
        cfg.offset_rotation = 2;
        cfg.dummy_read_pixel = 8;
        cfg.dummy_read_bits = 1;
        cfg.readable = false;
        cfg.invert = false;
        cfg.rgb_order = false;
        cfg.dlen_16bit = false;
        cfg.bus_shared = true;

        cfg.memory_width = 80;
        cfg.memory_height = 160;

        _panel_instance.config(cfg);
        }
        {
        auto cfg = _light_instance.config();

        cfg.pin_bl = 8;
        cfg.invert = false;
        cfg.freq = 44100;
        cfg.pwm_channel = 7;

        _light_instance.config(cfg);
        _panel_instance.setLight(&_light_instance);
        }
        setPanel(&_panel_instance);
      }
  };

  LGFX tft;
#endif

int brightness[9] = {255, 128, 64, 32, 16, 8, 4, 2, 1};                                           // Helligkeitsstufen (können mit linken Button durchgeschaltet werden)
int brightness_CH = 0;                                                                            // merkt sich aktuell gewählte Helligkeitsstufe
const int BL_Freq = 5000;//HZ                                                                     // PWM Frequenz
const int BL_CH = 0;                                                                              // definiere PWM Kanal
const int BL_RES = 8;//Bit                                                                        // definiere PWM Auflösung
#define BL_PWM_VAL "/BL_PWM_VAL"                                                                  // Pfad, in das gewählte PWM Stufe abgespeichert und bei Start ausgelesen werden sollen
#define BL_PWM_VAL_DEFAULT 0                                                                      // default Wert nach Formatierung des Speichers, erste der vordefinierten Stufen

#if defined (use_LGFX_library)
  #define TFT_BL 8
#endif

////////// FUNKTIONSPROTOTYPEN //////////
void SO_TFT_SETUP_BL_OFF();
void SO_TFT_CS_HIGH();
void SO_TFT_SETUP();

#endif