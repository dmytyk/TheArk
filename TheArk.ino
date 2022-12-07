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
    // true = terminal attached (send serial messages - for debugging), false = no terminal attached (no messages - production)
    #define TerminalAttached  false
#endif

// user definable to what ever you want
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
boolean ProjectMode = true;

// WiFi
WiFiServer      WebServer(webPort);
WiFiServer      socketServer(socketPort);
WebSocketServer webSocketServer;
WiFiClient      socketClient;


// USB Host
USBHost usb;
KeyboardController keyboard(usb);


// SD Card
int chipSelect = 4;     //MEM Shield chip select for an MRK 1010
File ARK;               // general purpose file class
File ARKSEARCH;         // search file class
String fileName;        // projectName, must be 8.3 name format (8 = name, 3 = extension)
char sdName[20];        // SD Open friendly name, can include directory, must end with a 0
String searchfileName;  // projectName, must be 8.3 name format (8 = name, 3 = extension)
char searchsdName[20];  // SD Open friendly name, can include directory, must end with a 0


// Data from Barcode Scanner
char receive_buffer[20];
uint8_t receive_buffer_counter;
int receive_byte;


// Battery Monitor
float low_battery_warning = 10.5;          //Set the battery warning at 10.5V (default = 10.5V).
float battery_voltage;
int batterycounter = 0;

// Background
boolean Backgroundinit = false;
int led = 0;


// ARK webpage, gzipped and base64 encoding to save space
char webpage_base64[] = "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";




// ** FUNCTIONS
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
// has 2 modes of Operation:
// 1 - Create Project - create and add barcodes to a project
// 2 - Search Projects - searches all projects for a specific barcode
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
       String sbc = String(receive_buffer);
       if(socketClient.connected()) {
           webSocketServer.sendData("B:" + sbc);
       }

       // see what mode we are in, create == true OR display == false
       if(ProjectMode == true) {
           // create/build project
           // CHECK to see if a specific file is available
           // if the file is available, write to it:
           if(SD.exists(sdName)) {
               // WRITE data to the file
               ARK = SD.open(sdName, FILE_WRITE);
               if (ARK) {
                   ARK.print(sbc);
                   ARK.close();
                   if(TerminalAttached) {
                     Serial.println(sbc);
                   }
               } else {
                   if(TerminalAttached) {
                       Serial.println("Error Opening: " + String(sdName));
                   }
               }
           } else {
               if(TerminalAttached) {
                   Serial.println("Error Project Not Available: " + String(sdName));
               }
           }
       } else {
            // search projects
            // clear the read responses box
            webSocketServer.sendData("C:R");

            String tempName;
            boolean notFound = true;

            // open the ARK directory
            ARK = SD.open("/ARK");

            // make sure we are at the first file in the directory
            ARK.rewindDirectory();

            // keep searching files until we find a match or there are no more files in the directory
            while (notFound) {
                // get next directory entry
                File entry =  ARK.openNextFile();
                if (! entry) {
                  break;
                }

                // get the file name, and convert to a string
                tempName = entry.name();
                if(TerminalAttached) {
                    Serial.println("Searching Project Name: " + tempName);
                }

                // build the full directory and file name
                searchfileName = "/ARK/";  // directory
                searchfileName += tempName;  // user create file name (8.3 format), this is the 8
                searchfileName.toCharArray(searchsdName, 20);

                // Open the specific file contents
                if(SD.exists(searchsdName)) {
                    // if the file is available, read it one byte at a time
                    ARKSEARCH = SD.open(searchsdName, FILE_READ);

                    // create the vars we need
                    char readBuffer[20];
                    byte b = 0;
                    int readIndex = 0;
                    String readData;
                    int barcodecount = 1;

                    // keep searching until we find a match or there are no more files
                    while (ARKSEARCH.available() && notFound) {
                        b = ARKSEARCH.read();
                        readBuffer[readIndex++] = b;

                        // if end of line
                        if(b == 0x0d) {
                            // if so end the buffer
                            readBuffer[readIndex] = 0;

                            // format it for comparing to the scanned barcode
                            String rbc = String(readBuffer);
                            if(sbc == rbc) {
                                // found match, display and end
                                // send the found entry
                                webSocketServer.sendData("D:Barcode Found");
                                webSocketServer.sendData("D:Project - " + tempName);
                                webSocketServer.sendData("D:Row - " + String(barcodecount));
                                
                                // stop searching
                                notFound = false;  
                            }

                            // reset the pointer for the next barcode
                            readIndex = 0;

                            // point to next entry in the file
                            barcodecount++;
                        }
                    }

                    // close the SD card
                    ARKSEARCH.close();
                }
                entry.close();
            }
            if(notFound == true) {
                // no entry found
                webSocketServer.sendData("D:Not Found");                
            }

            // no more files
            webSocketServer.sendData("D:End of Search");
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
    
    // keep displaying files until we are done
    while (true) {
        File entry =  dir.openNextFile();
        if (! entry) {
          // no more files
          webSocketServer.sendData("O:End of Directory");
          break;
        }
        
        // get the file attributes, and convert to a string
        tempName = entry.name();
        ultoa(entry.size(), numberArray, 10);
        tempName += " - ";
        tempName += String(numberArray);
        
        if(TerminalAttached) {
            Serial.println("Project Name: " + tempName);
        }

        // send to server
        webSocketServer.sendData("O:" + tempName);
        
        entry.close();        
    }
}




// ** SETUP
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

    // turn on Green LED solid no WiFi module
    digitalWrite(2, HIGH);

    // don't continue
    while (1) {
    };
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

  // Start the Access Point
  status = WiFi.beginAP(ssid, pass);
  if (status != WL_AP_LISTENING) {
    if(TerminalAttached) {
      Serial.println("Creating access point failed");
    }
    
    // don't continue, but flash the status LED
    while (1) {
      digitalWrite(2, HIGH);
      delay(100);
      digitalWrite(2, LOW);
      delay(100);
    };
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
    // don't continue, but flash the status LED
    while (1) {
      digitalWrite(2, HIGH);
      delay(1000);
      digitalWrite(2, LOW);
      delay(1000);
    };
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

   //The MKR1010 default is a 10 bit analog to digital converter, we need to set it to 12 bit to get a analog read range of 0 - 4096
   analogReadResolution(12);

  //Load the battery voltage to the battery_voltage variable.
  //analogRead => 0 = 0V ..... 4095 = 3.3V
  //The voltage divider (1k & 10k) is 1:11.
  //analogRead => 0 = 0V ..... 4095 = 36.3V
  //36.3 / 4095 = 112.81.
  //The variable battery_voltage holds 1050 if the battery voltage is 10.5V.
  battery_voltage = (float)analogRead(4) / 112.81;
  
  // show that we have completed all setup, flash the Green LED once
  digitalWrite(2, HIGH);
  delay(1000);
  digitalWrite(2, LOW);
}




// ** MAIN LOOP
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

  if(client)if(client)
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
                // send the WEB Page to the browser
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

  // wait for the javascript to start the socket connection
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
    if(Backgroundinit == false) {

       // clear out any existing socket data
       String data = webSocketServer.getData();
       if(TerminalAttached) {
           Serial.println("Websocket Flushed");
       }

       if(TerminalAttached) {
           Serial.println("Background Init Complete");
       }

       // indicate that the we are ready to process commands
       Backgroundinit = true;
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
      } else if (cmd == "CrtPrj") {
            ProjectMode = true;
            webSocketServer.sendData("R:Project Mode - Create/Build ");
      } else if (cmd == "DspPrj") {
            ProjectMode = false;
            webSocketServer.sendData("R:Project Mode - Search");
      } else if (cmd == "ReSet") {
            // do local reset stuff
            // reset the file name
            fileName = "";
            fileName.toCharArray(sdName, 20);

            // send reset response to tell webpage to reset
            webSocketServer.sendData("Z:");
      } else if (cmd == "Change") {
            // build the full directory and file name
            fileName = "/ARK/";  // directory
            fileName += usrVal;  // user create file name (8.3 format), this is the 8
            fileName += ".txt";  // this is the 3
            if(TerminalAttached) {
                Serial.println("Projectname: " + fileName);
            }

            // CREATE a file if it does not exists
            fileName.toCharArray(sdName, 20);
            if(!SD.exists(sdName)) {
              ARK = SD.open(sdName, FILE_WRITE);
              if(ARK) {
                if(TerminalAttached) {
                    Serial.println("Project Created: " + fileName);
                }
                // send the 8.3 part of the file name to the user as Active Projectname
                webSocketServer.sendData("F:" + fileName.substring(5));
                ARK.close();
              } else {
                if(TerminalAttached) {
                    Serial.println("Project NOT Created: " + fileName);
                }
              }
            } else {
              if(TerminalAttached) {
                  Serial.println("Project Opened: " + fileName);
              }
              // send the 8.3 part of the file name to the user as Active Projectname
              webSocketServer.sendData("F:" + fileName.substring(5));
            }
      } else if (cmd == "ListDir") {
          // DISPLAY specific directory contents
          if(SD.exists("/ARK")) {

            // clear the file responses box
            webSocketServer.sendData("C:O");

            ARK = SD.open("/ARK");
            listDirectory(ARK); 
          } else {
            webSocketServer.sendData("O:/ARK directory not found!");  
          }
      } else if (cmd == "ListFile") {
          // clear the file responses box
          webSocketServer.sendData("C:O");
          
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
                    barcodecount++;

                    // reset the pointer for the next barcode
                    readIndex = 0;
                }
            }
            // send the barcode count
            webSocketServer.sendData("O:Barcode Count [ " + String(barcodecount) + " ]");

            // close the SD card
            ARK.close();
          } else {
            webSocketServer.sendData("O:Project Name Not Set!");
          }
       } else if (cmd == "RmvDir") {   
            // clear the file responses box
            webSocketServer.sendData("C:O");
                                 
            // REMOVE directory
            // if the SD card is present , make sure we have the "/ARK" directory, if not create it
            if(SD.exists("/ARK")) {
                SD.remove("/ARK");
                if(TerminalAttached) {
                    Serial.println("Project Removed: /ARK");
                }
                webSocketServer.sendData("O:Directory Removed");
            } else {
                webSocketServer.sendData("O:Directory Not Found");    
            }
       } else if (cmd == "RmvFile") {
            // clear the file responses box
            webSocketServer.sendData("C:O");
                                 
            // REMOVE file
            // if the SD card is present , make sure we have the "/ARK" directory, if not create it
            if(SD.exists(sdName)) {
                SD.remove(sdName);
                if(TerminalAttached) {
                    Serial.println("Project Removed:");
                }
                webSocketServer.sendData("O:Project Removed");
            } else {
                webSocketServer.sendData("O:Project Not Found");
            }
       } else if (cmd == "ClrFile") {
            // clear the file responses box
            webSocketServer.sendData("C:O");
                                 
            // REMOVE file
            // if the SD card is present , make sure we have the "/ARK" directory, if not create it
            if(SD.exists(sdName)) {
                SD.remove(sdName);
                ARK = SD.open(sdName, FILE_WRITE);
                if(TerminalAttached) {
                    Serial.println("Project Emptied:");
                }
                ARK.close();
                webSocketServer.sendData("O:Project Empited");
            } else {
                webSocketServer.sendData("O:Project Not Found");
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
            // flash the heart beat - this indicate that the main loop is running normally
            if(led == 0) {
                digitalWrite(LED_BUILTIN, HIGH);
                led = 1;
            } else {
                digitalWrite(LED_BUILTIN, LOW);
                led = 0;
            }

           // send the battery status every 10 seconds
           if(batterycounter++ > 10) {
               if (battery_voltage < low_battery_warning) {
                   // too low send error message - this will also cause the javascript to sound the beep tone and display the voltage in RED text
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
