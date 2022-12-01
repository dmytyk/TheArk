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
            while (ARK.available()) {
                b = ARK.read();
                readBuffer[readIndex++] = b;
                if(b == 0x0d) {
                    // if so end the buffer
                    readBuffer[readIndex] = 0;

                    // format it for sending over the websocket
                    String rs = String(readBuffer);
                    webSocketServer.sendData("O:" + rs);

                    // reset the pointer for the next barcode
                    readIndex = 0;
                }
            }
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
