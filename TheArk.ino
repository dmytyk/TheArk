///////////////////////////////////////////////////////////////////////////////////////
//Terms of use
///////////////////////////////////////////////////////////////////////////////////////
//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
//THE SOFTWARE.
///////////////////////////////////////////////////////////////////////////////////////

#include <SPI.h>
#include <SD.h>

#include <global.h>
#include <base64.h>
#include <WebSocketClient.h>
#include <WebSocketServer.h>
#define _WIFININA_LOGLEVEL_       1
#include <WiFiNINA_Generic.h>
#include <KeyboardController.h> 

// Console Attached
#ifndef TerminalAttached
    // true = terminal attached (send serial messages), false = no terminal attached no messages 
    #define TerminalAttached  true
#endif

#define SECRET_SSID "The Ark Playground"
#define SECRET_PASS "playground"

// global var's
char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;
const short webPort     = 80;
const short socketPort  = 8080;
int status = WL_IDLE_STATUS;
int sectimer = 0;
uint32_t loop_timer;

// WiFi
WiFiServer      WebServer(webPort);
WiFiServer      socketServer(socketPort);
WebSocketServer webSocketServer;
WiFiClient      socketClient;


// USB Host
USBHost usb;
KeyboardController keyboard(usb);


// SD Card
int chipSelect = 4;   //MEM Shield chip select for an MRK 1010
File ARK;             // flie class
String fileName;      // fileName, must be 8.3 name format (8 = name, 3 = extension)
char sdName[20];      // SD Open friendly name, can include directory, must end with a 0


// Data from Barcode Scanner
char receive_buffer[20];
uint8_t receive_buffer_counter;
int receive_byte;


// Battery Monitor
float low_battery_warning = 10.5;          //Set the battery warning at 10.5V (default = 10.5V).
float battery_voltage;
int batterycounter = 0;

// Background
boolean Backgroundinit = true;
int led = 0;


// ARK webpage, gzipped and base64 encoding to save space
char webpage_base64[] = "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";




//FUNCTIONS
// print wifi status
void printWifiStatus()
{
  if(TerminalAttached) {
    Serial.print("SSID: "); Serial.println(WiFi.SSID());
    Serial.print("Signal strength (RSSI): "); Serial.print(WiFi.RSSI()); Serial.println(" dBm");
    Serial.print("IP address: "); Serial.println(WiFi.localIP());
    Serial.print("Gateway: "); Serial.println(WiFi.gatewayIP());
    Serial.print("Netmask: "); Serial.println(WiFi.subnetMask());
    Serial.print("Webpage is at http://"); Serial.print(WiFi.localIP()); Serial.println("/");
    Serial.print("Websocket is at http://"); Serial.print(WiFi.localIP()); Serial.println(":" + (String)socketPort + "/");
  }
}

// Process a reset request
void reset_data(void){
    // clear the file responses box
    webSocketServer.sendData("C:");
}

// check to see if the scanner send some data
void keyPressed() { 
    // get a character
    receive_byte = keyboard.getKey();

    // discard line feeds
    if(receive_byte != 0x0a) {
        //add it to the capture buffer
        receive_buffer[receive_buffer_counter++] = receive_byte;
    }

    // see if it is the end on the capture
    if(receive_byte == 0x0d) {
        // if so end the buffer
        receive_buffer[receive_buffer_counter] = byte(0);

        // format it for sending over the websocket
        String s = String(receive_buffer);
        webSocketServer.sendData("B:" + s);

        // CHECK to see if a specific file is available
        // if the file is available, write to it:
        if(SD.exists(sdName)) {
            // WRITE data to the file
            ARK = SD.open(sdName, FILE_WRITE);
            if (ARK) {
                ARK.print(s);
                ARK.close();
                if(TerminalAttached) {
                  Serial.println(s);
                }
            } else {
                Serial.println("Error Opening: " + String(sdName));
            }
        } else {
            Serial.println("Error File Not Available: " + String(sdName));
        }

        //reset the buffer pointer
        receive_buffer_counter = 0;
    }
}

// list the "/ARK" directory contents  
void listDirectory(File dir) {
    // create a temp file name
    String tempName;
    uint32_t number = 0;
    char numberArray[20];
    
    // make sure we are at the first file in the directory
    dir.rewindDirectory();
    
    // keep display file until we are done
    while (true) {
        File entry =  dir.openNextFile();
        if (! entry) {
          // no more files
          webSocketServer.sendData("O:End of Directory");
          break;
        }
        
        // get the file atributes, and convert to a string
        tempName = entry.name();
        ultoa(entry.size(), numberArray, 10);
        tempName += " - ";
        tempName += String(numberArray);
        
        if(TerminalAttached) {
            Serial.println("File Name: " + tempName);
        }

        // send to server
        webSocketServer.sendData("O:" + tempName);
        
        entry.close();        
    }
}




// SETUP
void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(2, OUTPUT);                            // Green LED
  analogReadResolution(12);
  
  receive_buffer_counter = 0;
  
  if(TerminalAttached) {  
    Serial.begin(57600);
    delay(1000);
  } 
  
  // check for the WiFi module:
  if (WiFi.status() == WL_NO_MODULE) {
    if(TerminalAttached) {
      Serial.println("Communication with WiFi module failed!");
    }
    // don't continue
    while (true);
  }
  
  // check and make sure we are using the lastest Firmware
  String fv = WiFi.firmwareVersion();

  // Serial port initialization
  if(TerminalAttached) {
     Serial.println("Version " + String(WIFININA_GENERIC_VERSION));

     // check and make sure we are using the lastest Firmware
     if (fv < WIFI_FIRMWARE_LATEST_VERSION)
     {
         Serial.print("Your current firmware NINA FW v");
         Serial.println(fv);
         Serial.print("Please upgrade the firmware to NINA FW v");
         Serial.println(WIFI_FIRMWARE_LATEST_VERSION);
     }
  }

  // print the network name (SSID);
  if(TerminalAttached) {
    Serial.print("Creating access point named: ");
    Serial.println(ssid);
  }

  // Create open network. Change this line if you want to create an WEP network:
  status = WiFi.beginAP(ssid, pass);
  if (status != WL_AP_LISTENING) {
    if(TerminalAttached) {
      Serial.println("Creating access point failed");
    }
    
    // don't continue
    while (true);
  }

  // start the web and socket servers
  WebServer.begin();
  socketServer.begin();

  // START begin the SD processing
  // see if the SD card is present and can be initialized:
  if(TerminalAttached) {
    Serial.println("Initializing SD card");
  }
  if (!SD.begin(chipSelect)) {
    if(TerminalAttached) {
      Serial.println("Card failed, or not present");
      // don't do anything more:
      while (1);
    }
  }
  if(TerminalAttached) {
    Serial.println("Card Initialized.");
  }

  // MAKE a directory
  // if the SD card is present , make sure we have the "ARK" directory, if not create it
  if(!SD.exists("/ARK")) {
    SD.mkdir("/ARK");
    if(TerminalAttached) {
      Serial.println("Directory Created: /ARK");  
    }
  }
  
  //Load the battery voltage to the battery_voltage variable.
  //The MRK1010 default is a 10 bit analog to digital converter, we need to set it to 12 bit
  //analogRead => 0 = 0V ..... 4095 = 3.3V
  //The voltage divider (1k & 10k) is 1:11.
  //analogRead => 0 = 0V ..... 4095 = 36.3V
  //36.3 / 4095 = 112.81.
  //The variable battery_voltage holds 1050 if the battery voltage is 10.5V.
  analogReadResolution(12);
  battery_voltage = (float)analogRead(4) / 112.81;
  
  // show that we have completed all setup
  digitalWrite(2, HIGH);
  delay(5000);
  digitalWrite(2, LOW);
}




// MAIN LOOP
void loop() {
  // Background Process 1 - poll the barcode scanner
  // poll the USB Host device, in this case the Barcode Reader
  usb.Task(); 

  // Background Process 2 - process the web interface
  // compare the previous status to the current status
  if (status != WiFi.status()) {
    // it has changed update the variable
    status = WiFi.status();

    if (status == WL_AP_CONNECTED) {
      // a device has connected to the AP
      if(TerminalAttached) {
        Serial.println("Device connected to AP");
      }
      printWifiStatus();
    } else {
      // a device has disconnected from the AP, and we are back in listening mode
      if(TerminalAttached) {
        Serial.println("Device disconnected from AP");
      }
    }
  }

  WiFiClient client = WebServer.available();   // listen for incoming clients

  if(client)
  {
     if(TerminalAttached) {
      Serial.print("New client: "); 
      Serial.print(client.remoteIP()); 
      Serial.print(" : "); 
      Serial.println(client.remotePort());
     }
      String header = "";

      while (client.connected()) {            // loop while the client's connected
        while(client.available())
        {
            char ch = client.read();
  
            if (ch != '\r') 
            {
            header += ch;
            }
        
            if (header.substring(header.length() - 2) == "\n\n") 
            {
            if(TerminalAttached) {
               Serial.print(header.substring(0, header.length() - 1));
            }
          
            if (header.indexOf("GET / HTTP") > -1) 
            {
                const int webpage_base64_length = sizeof(webpage_base64);
                const int webpage_gz_length = base64_dec_len(webpage_base64, webpage_base64_length);
                char webpage_gz[webpage_gz_length];
                base64_decode(webpage_gz, webpage_base64, webpage_base64_length);
                int packetsize = 1024;
                int done = 0;
                
                client.println("HTTP/1.1 200 OK\nContent-Type: text/html\nContent-Encoding: gzip\n");
            
                while (webpage_gz_length > done) 
                {
                    if (webpage_gz_length - done < packetsize) {
                
                    packetsize = webpage_gz_length - done;
                    }
              
                    client.write(webpage_gz + done, packetsize * sizeof(char));
                    done = done + packetsize;
                }
                if(TerminalAttached) {
                   Serial.println("--Interface webpage sent");
                }
            } 
            else 
            {
                client.println("HTTP/1.1 404 Not Found\nContent-Type: text/plain; charset=utf-8\n\n404 Not Found\n");
                if(TerminalAttached) {
                   Serial.println("--Page not found");
                }
            }
          
            client.stop();
            if(TerminalAttached) {
               Serial.println("--Client disconnected");
            }
        }
      }
    }
  }

  if(!socketClient.connected()) 
  {
    socketClient = socketServer.available();
    
    if (socketClient.connected() && webSocketServer.handshake(socketClient)) 
    {
       if(TerminalAttached) {
           Serial.print("\n--Websocket connected to: ");
           Serial.print(socketClient.remoteIP());
           Serial.print(":");
           Serial.println(socketClient.remotePort());
       }
    } 
    else 
    {
        socketClient.stop();
        delay(100);
    }
  }

  if(socketClient.connected()) 
  {
    // Background Init - setup the background tasks, runs only once
    if(Backgroundinit == true) {
        Backgroundinit = false;

       String data = webSocketServer.getData();
       if(TerminalAttached) {
           Serial.println("Websocket Flushed");
       }

       if(TerminalAttached) {
           Serial.println("Background Init Complete");
       }
    }
    
    // see if we have a command/request from the user 
    String data = webSocketServer.getData();
    if (data.length() > 0) 
    {
      // get tools to parse incoming request
      char buf[data.length()+1];
      data.toCharArray(buf, sizeof(buf));
      char *token;
      char *pter = buf;
      byte i = 0;
      String cmd;
      String usrVal; 
      while ((token = strtok_r(pter, ":", &pter)))
      {
        switch(i) {
          case 0:
            cmd = token;
            break;
          case 1:
            usrVal = token;
            break;
        }
        i++;
      }
      if(TerminalAttached) {
        Serial.println("CMD: " + String(cmd));
        Serial.println("UsrVal: " + String(usrVal));
      }

      // process command
      if(cmd == "Ledon") {
            digitalWrite(2, HIGH);
            webSocketServer.sendData("R:" + cmd);
      } else if (cmd == "Ledoff") {
            digitalWrite(2, LOW);
            webSocketServer.sendData("R:" + cmd);
      } else if (cmd == "Rst") {
            reset_data();
            webSocketServer.sendData("R:" + cmd);
      } else if (cmd == "Change") {
            // build the full directory and file name
            fileName = "/ARK/";  // directory
            fileName += usrVal;  // user create file name (8.3 format), this is the 8
            fileName += ".txt";  // this is the 3
            if(TerminalAttached) {
                Serial.println("Filename: " + fileName);
            }

            // send the 8.3 part of the file name to the user as Active Filename
            webSocketServer.sendData("F:" + fileName.substring(5));

            // CREATE a file if it does not exists
            fileName.toCharArray(sdName, 20);
            if(!SD.exists(sdName)) {
              ARK = SD.open(sdName, FILE_WRITE);
              if(ARK) {
                if(TerminalAttached) {
                    Serial.println("File Created: " + fileName);
                }
                ARK.close();
              } else {
                if(TerminalAttached) {
                    Serial.println("File NOT Created: " + fileName);
                }
              }
            } else {
              if(TerminalAttached) {
                  Serial.println("File Opened: " + fileName);
              }
            }
      } else if (cmd == "ListDir") {
          // DISPLAY specific directory contents
          if(SD.exists("/ARK")) {

            // clear the file responses box
            webSocketServer.sendData("C:");

            ARK = SD.open("/ARK");
            listDirectory(ARK); 
          } else {
            webSocketServer.sendData("O:/ARK directory not found!");  
          }
      } else if (cmd == "ListFile") {
          // clear the file responses box
          webSocketServer.sendData("C:");
          
          // DISPLAY specific file contents
          if(SD.exists(sdName)) {
            // if the file is available, read it one byte at a time
            ARK = SD.open(sdName, FILE_READ);

            // create the vars we need
            char readBuffer[20];
            byte b = 0;
            int readIndex = 0;
            String readData;
            int barcodecount = 0;
            while (ARK.available()) {
                b = ARK.read();
                readBuffer[readIndex++] = b;
                if(b == 0x0d) {
                    // if so end the buffer
                    readBuffer[readIndex] = 0;

                    // format it for sending over the websocket
                    String rs = String(readBuffer);
                    webSocketServer.sendData("O:" + rs);

                    // count this barcode
                    barcodecount++;;

                    // reset the pointer for the next barcode
                    readIndex = 0;
                }
            }
            // send the barcode count
            webSocketServer.sendData("O:Barcode Count [ " + String(barcodecount) + " ]");

            // close the SD card
            ARK.close();
          } else {
            webSocketServer.sendData("O:File Name Not Set!");
          }
       } else if (cmd == "RmvDir") {   
            // clear the file responses box
            webSocketServer.sendData("C:");
                                 
            // REMOVE directory
            // if the SD card is present , make sure we have the "/ARK" directory, if not create it
            if(SD.exists("/ARK")) {
                SD.remove("/ARK");
                if(TerminalAttached) {
                    Serial.println("File Removed: /ARK");  
                }
                webSocketServer.sendData("O:Directory Removed");
            } else {
                webSocketServer.sendData("O:Directory Not Found");    
            }
       } else if (cmd == "RmvFile") {
            // clear the file responses box
            webSocketServer.sendData("C:");
                                 
            // REMOVE file
            // if the SD card is present , make sure we have the "/ARK" directory, if not create it
            if(SD.exists(sdName)) {
                SD.remove(sdName);
                if(TerminalAttached) {
                    Serial.println("File Removed:");  
                }
                webSocketServer.sendData("O:File Removed");
            } else {
                webSocketServer.sendData("O:File Not Found");    
            }
       } else if (cmd == "ClrFile") {
            // clear the file responses box
            webSocketServer.sendData("C:");
                                 
            // REMOVE file
            // if the SD card is present , make sure we have the "/ARK" directory, if not create it
            if(SD.exists(sdName)) {
                SD.remove(sdName);
                ARK = SD.open(sdName, FILE_WRITE);
                if(TerminalAttached) {
                    Serial.println("File Emptied:");  
                }
                ARK.close();
                webSocketServer.sendData("O:File Empited");
            } else {
                webSocketServer.sendData("O:File Not Found");    
            }
      } else {
          webSocketServer.sendData("E:" + cmd);
      }
    }
  }


  // Background Process 3 - get battery voltage
  //The battery voltage is needed for compensation.
  //A complementary filter is used to reduce noise.
  //1410.1 = 112.81 / 0.08.
  battery_voltage = battery_voltage * 0.92 + ((float)analogRead(4) / 1410.1);
  

  // Background Process 4 - make a 1 second timer
  switch(sectimer) {
    case 0:
        loop_timer = micros();
        sectimer = 1;
    break;
    case 1:
        // 1 second has passed so do functions
        if(micros() - loop_timer > 1000000) {
            // flash the heart beat
            if(led == 0) {
                digitalWrite(LED_BUILTIN, HIGH);
                led = 1;
            } else {
                digitalWrite(LED_BUILTIN, LOW);
                led = 0;
            }

            // send the battery status every 15 seconds
            if(batterycounter++ > 10) {
                if (battery_voltage < low_battery_warning) {
                    if(socketClient.connected()) {
                        webSocketServer.sendData("L:" + String(battery_voltage));
                    }    
                } else {
                    // too low send error message  
                    if(socketClient.connected()) {
                        webSocketServer.sendData("N:" + String(battery_voltage));
                    }
                }
                batterycounter = 0;
            }

            // reset the timer
            sectimer = 0;
        }
    break;
  }
}
