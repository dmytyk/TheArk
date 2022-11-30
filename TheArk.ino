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

// global var's
char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;
const short webPort     = 80;
const short socketPort  = 8080;
int status = WL_IDLE_STATUS;


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


// Background
boolean Backgroundinit = true;
uint8_t led = 0;


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
  pinMode(2, OUTPUT);                                   // Green LED
  pinMode(3, OUTPUT);                                   // Red LED
  
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
  
  // Background Process 3 - heart beat
  if(led == 0) {
    led = 1;
    digitalWrite(LED_BUILTIN, HIGH);
  } else {
    led = 0;
    digitalWrite(LED_BUILTIN, LOW);
  }
}
