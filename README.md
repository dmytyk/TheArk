# The Ark by  Daves Hack Shack

![Daves Hack Shack](/Images/Hackshack.png)


<h1>Inventory Project using an Access Point Web Interface with a Barcode Scanner</h1>

I created a YouTube video that provides additional details and a demonstration, the Link is: [The Ark Video](https://youtu.be/7wXU4oj81e4)

The Ark uses 4 key components an Arduino MKR1010 controller, Arduino MKR MEM Shield (with 16GB SD card formatted as FAT32), BT 2D Barcode Scanner and a Micro Type B to USB Female Hub cable.  A prototype was constructed:

![Prototype](/Images/Prototype.jpg)
![Scanner](/Images/Scanner.jpg)

I plan on encasing the prototype in a project box for production use.

#### Basic Operation

> - Power on The Ark (if the /ARK directory is not found it will create it automatically)
> - Connect to the Access Point (you will get served a Web Page Interface)
> - Enter a Project Name (this will create a project with the name entered if it does not exist)
> - Start scanner barcodes (That's It!)

#### Project Operations

> - Change - Create/Change the Active Project using the name provided
> - Change - Update Project Description
> - Show All Projects - list all projects in the /ARK directory
> - Display Project - show all scanner barcodes
> - Remove Directory - Remove the /ARK directory (not recommend)
> - Remove Project - Delete an Active Project
> - Clear Project - empty the contents of a project
> - Download Project - Transfer the contents of the Project Responses area to the attached device 
> - Project Mode - switch between the Create Project (manage projects) and Display Project (search for a barcode) modes
> - Remove Barcode - This will remove a barcode, and it's label
> - Update Label - this will update a barcodes label

#### Project Notes

> - The Barcode Scanner can be recharged from The ARK by removing the USB dongle and attaching it to the ARK via the USB cable it comes with
> - The Ark has a power monitor circuit with a low voltage threshold set to 10.50v, once reached the attached device will beep and the voltage display will turn RED
> - The WEB page is HTML and javascript, it is gzipped and base 64 encoded to save code space
> - On startup if Green LED stay lit we did not find the WiFi module
> - On startup if Green LED is flashing every 100ms the Access Point did not start
> - On startup if Green LED is flashing every 1000ms the SD Card did not start

#### Arduino Software Notes

> - Libraries needed : <SPI.h>, <SD.h>, <global.h>, <base64.h>, <WebSocketClient.h>, <WebSocketServer.h>, <WiFiNINA_Generic.h> and <KeyboardController.h>
> - YouTube video that shows how to encode a WEB Page for the Arduino : [WEB Page Encoding](https://youtu.be/oKCXiYc311A) skip to the 2:22 mark


#### Schematic for the project

![The Ark Schematic](/Images/TheArk_Schematic.png)
