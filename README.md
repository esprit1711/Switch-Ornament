# Switch-Ornament
tinyest possible Gif Player that looks like a Nintendo Switch

![alt text](https://github.com/esprit1711/Switch-Ornament/blob/main/Assembly.JPG?raw=true)

Here, you will find everything to Copy my GIF Player, that looks like a Nintendo Switch. This Project use a 0.96" 80x160 Pixel TFT with st7735 Driver.
The Source Code is based on the "ESP32-LGFX-SDCard-GifPlayer" Example of the "AnimatedGIF" library from "bitbank2".
The Inspiration for this Project is the Nintendo Switch Ornamend from scottbez1. He use a slightly bigger Screen, with much higher Resolution but i want the smalest possible Switch.

Part of this Project is a DIY Powerbank only for this device so you can use it as a christmas ornament. Powerbank and Switch are connected over a 65cm long cable with 0.04mm² wires. to achive also the tinyest possible powerbank, i crated a custom Board with Li-Po Charger and Power Switch (you can charge the battery and can still use the device). To use the powerbank also as base, i put 3 magnets inside the switch case and powerbank so it will hold very well together without any bad looking mechanic

The Mainboard is optimised to be ordered at jlcpcb and use the assembly Service. You will find everything jlcpcb need (gerber, BOM and positions file) in the "production" Folder of the KiCad Project. You want to choose a 1.6mm board thickness, the Option "Specify a location" at "Remove Order Number" and check the orientation of every part after upload and check the BOM. You also need to order the tiny Buttons and the TFT so you can assemble it after you got the Boards.

Buttons:

https://de.aliexpress.com/item/1005003752959323.html

0.96" TFT (WELDET TYPE!):

https://de.aliexpress.com/item/1005003806330978.html

After everything is assembled, you need to prepare a USB Cable so the connection will match with the silkscreen. You can solder it directly on the Pads, or use a 1.27mm pinheader. Solder the "BOOT" bridge at the front of the board, connect the USB cable to your PC, open the Firmware with Arduino IDE and upload the Code.
To upload the code, you need to copy the library folder in your local Arduino library folder and install the ESP32 via Board Manager. after you prepared this, you just need to choose the right board (ESP32C3 Dev Module) and Port and set every setting like on the following picture:

![alt text](https://github.com/esprit1711/Switch-Ornament/blob/main/Arduino-Settings.JPG?raw=true)

After a successful upload, disconnect the USB cable and remove the "BOOT" solder bridge. Now, you dont need to solder the Bridge anymore to upload a Code via USB. You can also disconnect the USB Cable (or leave it, if you want to use it as the power source) and power the device via the +5V and GND Pads. The Software is prepared to update the firmware, if a "SO_update.bin" was found on the SD-Card, so in case of an Update, you dont need the USB connection anymore.

Credits:

https://github.com/bitbank2/AnimatedGIF

https://github.com/Bodmer/TFT_eSPI

https://www.youtube.com/watch?v=zJxyTgLjIB8
