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
    #define TerminalAttached  false
#endif

#define SECRET_SSID "The Ark Playground"
#define SECRET_PASS "playground"

// global vars
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

       // format it for sending over the websocket, only sent it id we are connected, the user could be scanning with no connection
       String s = String(receive_buffer);
       if(socketClient.connected()) {
           webSocketServer.sendData("B:" + s);
       }

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
               if(TerminalAttached) {
                   Serial.println("Error Opening: " + String(sdName));
               }
           }
       } else {
           if(TerminalAttached) {
               Serial.println("Error File Not Available: " + String(sdName));
           }
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
  
  receive_buffer_counter = 0;
  
  if(TerminalAttached) {  
    Serial.begin(9600);
    delay(2000);
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
    while (1);
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
    }
    // don't do anything more:
    while (1);
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
  delay(1000);
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

      // make sure the socket is also stopped, in case we want to reconnect
      socketClient.stop();
      delay(100);
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

  if(!socketClient.connected()) {
    // we are not connect to a socket so keep listening
    socketClient = socketServer.available();
    
    if (socketClient.connected() && webSocketServer.handshake(socketClient)) {
       if(TerminalAttached) {
           Serial.print("\n--Websocket connected to: ");
           Serial.print(socketClient.remoteIP());
           Serial.print(":");
           Serial.println(socketClient.remotePort());
       }
    } else {
        socketClient.stop();
        delay(100);
    }
  } else {
    // we have a good socket so process the connection
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
      } else if (cmd == "ReSet") {
            // do local reset stuff

            // send reset response to tell webpage to reset
            webSocketServer.sendData("Z:" + cmd);
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
        if(micros() - loop_timer > 2000000) {
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
                   // too low send error message
                   if(socketClient.connected()) {
                       webSocketServer.sendData("L:" + String(battery_voltage));
                   }
               } else {
                   // send the normal battery update
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
