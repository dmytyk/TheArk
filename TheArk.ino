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

// To Do
// Remove a barcode
// Format screen for ipad size
// Create label for each barcode (scan barcode, type label, hit save - rewrite current scan process)
// Ability to edit label

// Project description box



/*
 * Remove a barcode
 * 
 * start
have a existing file for reading

[new file format]

b           l  b          l
bbbbbbb(cr)(cr)bbbbbbb(cr)asdcfvgbggghhhjjj(cr)

where:

bbbbbbb = barcode (7 characters max)
(cr) = 0x0d (carriage return)  -  (cr)(cr) =  no label for this barcode
asdcfvgbggghhhjjj = label (20 characters max)
(overhead = 2(cr), 3(bc lb seperator)

[end]

1
open temp file for writing

2
read existing file
if not bc
    save bc and label to temp file
else
    skip bc and label
loop until end

3
now temp file is a copy of existing file - bc and label that was removed

4
truncate the existing file

5
reset temp file seek(0)

6
read temp file for bc and label
write existing file with bc and label
loop until end

7
close temp file
remove temp file
*/



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
String fileName;        // projectName, must be 8.3 name format (8 = name, 3 = extension)
char sdName[20];        // SD Open friendly name, can include directory, can include directory, must end with a 0

File DESC;               // general purpose file class
String descName;        // projectName, must be 8.3 name format (8 = name, 3 = extension)
char sdDesc[20];        // SD Open friendly name of the description for the file name, can include directory, must end with a 0

File ARKSEARCH;         // search file class
String searchfileName;  // projectName, must be 8.3 name format (8 = name, 3 = extension)
char searchsdName[20];  // SD Open friendly name, can include directory, can include directory, must end with a 0
int searchBCposition;   // 1-xx represents the position in the searchscName where the barcode was found

File TEMP;              // temp file class
String tempfileName;    // always Temp.txt
char tempsdName[20];    // SD Open friendly name, can include directory, can include directory, must end with a 0

// Data from Barcode Scanner
char receive_buffer[20];
uint8_t receive_buffer_counter;
int receive_byte;


// Battery Monitor
float low_battery_warning = 5.2;          // Set the battery warning at 5.2V.
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

    // make sure we have a socket if no just drop it
    if(socketClient.connected()) {
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

           // see what mode we are in, create == true OR display == false
           if(ProjectMode == true) {
                // create/build project
                // send the scanned barcode to the user and wait for a label and save button
                webSocketServer.sendData("B:" + sbc);
           } else {
                // search projects
                // clear the read responses box
                webSocketServer.sendData("C:R");

                // send the scanned barcode to the user and wait for a label update button OR a remove barcode button
                webSocketServer.sendData("B:" + sbc);

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
                    if (!entry) {
                      break;
                    }

                    // get the file name, and convert to a string
                    tempName = entry.name();

                    // skip the .des files
                    if(tempName.endsWith(".DES")) {
                        continue;
                    }

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
                        searchBCposition = 0;

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

                                    // calculate the correct position
                                    if(searchBCposition != 0) {
                                        searchBCposition /= 2;
                                        searchBCposition++;   
                                    } else {
                                        searchBCposition = 1;    
                                    }
                                    
                                    // send the found entry
                                    // the searchfileName, searchsdName (with full path) and searchBCposition are global vars and will be used by the update label and remove barcode buttons
                                    webSocketServer.sendData("D:Barcode Found");
                                    webSocketServer.sendData("D:Project - " + tempName);
                                    webSocketServer.sendData("D:Row - " + String(searchBCposition));

                                    // stop searching
                                    notFound = false;
                                } else {
                                    // reset the pointer for the next barcode
                                    readIndex = 0;

                                    // point to next entry in the file
                                    searchBCposition++;
                                }
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

// Save this barcode string
// String sbd - the formatted barcode and label
void saveBarcode(String sbc) {
    // create/build project
    // CHECK to see if a specific file is available
    // if the file is available, write to it:
    if(SD.exists(sdName)) {
       // WRITE data to the file (append)
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
               webSocketServer.sendData("R:Error Opening - " + String(sdName));
           }
       }
    } else {
       if(TerminalAttached) {
           Serial.println("Error Project Not Available: " + String(sdName));
           webSocketServer.sendData("R:Error Project Not Available - " + String(sdName));
       }
    }
}

// go to a specific position in the searched file
void removeBarcode() {
    // if the file is available , remove the requested barcode:
    if(SD.exists(searchsdName)) {
       // WRITE data to the file (append)
       ARKSEARCH = SD.open(searchsdName, FILE_READ);
       ARKSEARCH.seek(0);
       if (ARKSEARCH) {
            // start at first barcode
            int searchCount = 1;
            byte crFlag = 0;
            byte b = 0;

            // create the Temp.txt file, make sure it is empty
            tempfileName = "/ARK/Temp.txt";
            tempfileName.toCharArray(tempsdName, 20);
            TEMP = SD.open(tempsdName, O_WRITE | O_TRUNC);
            
            // keep searching until we find a match or there are no more barcodes
            while (ARKSEARCH.available()) {
                // get a byte
                b = ARKSEARCH.read();
                
                // see if we are searched position, if not save it in the temp file
                if(searchCount != searchBCposition) {
                    TEMP.write(b);
                }
                
                // check for carriage returns
                if(b == 0x0d) {
                    crFlag++;

                    // if we are at the second one bump the search count, reset the carriage return flag so we can look for the next barcode/label set
                    if(crFlag == 2) {
                        searchCount++;
                        crFlag = 0;    
                    }
                }                    
            }

            // we have skipped the barcode / label so now lets clean up send debug
            //webSocketServer.sendData("D:Search Count - " + String(searchCount));

            // close the search files
            ARKSEARCH.close();
            TEMP.close();

            // reopen them, empty the search file and start the temp from the beginning
            ARKSEARCH = SD.open(searchsdName, O_WRITE | O_TRUNC);
            TEMP = SD.open(tempsdName, O_READ);
            TEMP.seek(0);

            // transfer all the data from the temp to the requested search file
            while (TEMP.available()) {
                b = TEMP.read();
                ARKSEARCH.write(b);              
            }
            
            // close the search files
            ARKSEARCH.close();
            TEMP.close();

            // remove the temp file
            //SD.remove(tempsdName);
       } else {
           if(TerminalAttached) {
               Serial.println("Error Opening: " + String(searchsdName));
               webSocketServer.sendData("R:Error Opening - " + String(searchsdName));
           }
       }
    } else {
       if(TerminalAttached) {
           Serial.println("Error Search Project Not Available: " + String(searchsdName));
           webSocketServer.sendData("R:Error Search Project Not Available - " + String(searchsdName));
       }
    }
}

// Save this barcode string
// String bcLabel - the formatted barcode label
void updateBCLabel(String bcLabel) {
    // if the file is available , remove the requested barcode:
    if(SD.exists(searchsdName)) {
       // WRITE data to the file (append)
       ARKSEARCH = SD.open(searchsdName, FILE_READ);
       ARKSEARCH.seek(0);
       if (ARKSEARCH) {
            // start at first barcode
            int searchCount = 1;
            int updatecrFlag = 0;
            int savecrFlag = 0;
            boolean savedNewLabelFlag = false;
            byte b = 0;

            // create the Temp.txt file, make sure it is empty
            tempfileName = "/ARK/Temp.txt";
            tempfileName.toCharArray(tempsdName, 20);
            TEMP = SD.open(tempsdName, O_WRITE | O_TRUNC);

            // keep searching until we find a match or there are no more barcodes
            while (ARKSEARCH.available()) {
                // get a byte
                b = ARKSEARCH.read();

                // see if we are searched position, if not save it in the temp file
                if(searchCount == searchBCposition) {
                    // see if we are on the barcode (save it), label (update it), or just transferring data until done
                    switch(updatecrFlag) {
                        // barcode
                        case 0:
                            TEMP.write(b);
                            if(b == 0x0d) {
                                updatecrFlag++;
                            }
                        break;
                        // label
                        case 1:
                            // we are discarding the old label by not saving it until we are at the end of it
                            if(b == 0x0d) {
                                // indicate we are done
                                updatecrFlag++;
                                searchCount++;
                            } else {
                                // have we saved the new label
                                if(!savedNewLabelFlag) {
                                    // save the new label, this is a one time action
                                    
                                    webSocketServer.sendData("R:Debug - " + String(bcLabel));
                                    
                                    int labellength = bcLabel.length();

                                    webSocketServer.sendData("R:Debug - " + String(labellength));
                                    
                                    for(int i = 0; i <= (labellength - 1); i++) {
                                        TEMP.write(bcLabel.charAt(i));
                                    }

                                    // indicate it is saved
                                    savedNewLabelFlag = true;
                                }
                            }
                        break;
                        // new label has been saved so just keep working until all bytes have been transferred
                        default:
                            TEMP.write(b);
                        break;
                    }
                } else {
                    // save the byte 
                    TEMP.write(b);
                    
                    // check for carriage returns
                    if(b == 0x0d) {
                        savecrFlag++;
    
                        // if we are at the second one bump the search count, reset the carriage return flag so we can look for the next barcode/label set
                        if(savecrFlag == 2) {
                            searchCount++;
                            savecrFlag = 0;    
                        }
                    }
                }
            }

            // we have skipped the barcode / label so now lets clean up send debug
            //webSocketServer.sendData("D:Search Count - " + String(searchCount));

            // close the search files
            ARKSEARCH.close();
            TEMP.close();

            // reopen them, empty the search file and start the temp from the beginning
            ARKSEARCH = SD.open(searchsdName, O_WRITE | O_TRUNC);
            TEMP = SD.open(tempsdName, O_READ);
            TEMP.seek(0);

            // transfer all the data from the temp to the requested search file
            while (TEMP.available()) {
                b = TEMP.read();
                ARKSEARCH.write(b);
            }

            // close the search files
            ARKSEARCH.close();
            TEMP.close();

            // remove the temp file
            //SD.remove(tempsdName);
       } else {
           if(TerminalAttached) {
               Serial.println("Error Opening: " + String(searchsdName));
               webSocketServer.sendData("R:Error Opening - " + String(searchsdName));
           }
       }
    } else {
       if(TerminalAttached) {
           Serial.println("Error Search Project Not Available: " + String(searchsdName));
           webSocketServer.sendData("R:Error Search Project Not Available - " + String(searchsdName));
       }
    }
}




// ** SETUP
void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(0, OUTPUT);                            // Green LED
  
  receive_buffer_counter = 0;
  
    Serial.begin(9600);
    delay(2000); 

  // check for the WiFi module:
  if (WiFi.status() == WL_NO_MODULE) {
    if(TerminalAttached) {
      Serial.println("Communication with WiFi module failed!");
    }

    // turn on Green LED solid no WiFi module
    digitalWrite(0, HIGH);

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
      digitalWrite(0, HIGH);
      delay(100);
      digitalWrite(0, LOW);
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
      digitalWrite(0, HIGH);
      delay(1000);
      digitalWrite(0, LOW);
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
  digitalWrite(0, HIGH);
  delay(1000);
  digitalWrite(0, LOW);
}




// ** MAIN LOOP
void loop() {
  // Background Process 1 - poll the barcode scanner
  // poll the USB Host device, in this case the Barcode Reader
  if(!TerminalAttached) {  
    usb.Task();
  }
  
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
            digitalWrite(0, HIGH);
            webSocketServer.sendData("R:" + cmd);
      } else if (cmd == "Ledoff") {
            digitalWrite(0, LOW);
            webSocketServer.sendData("R:" + cmd);
      } else if (cmd == "CrtPrj") {
            ProjectMode = true;
            webSocketServer.sendData("R:Project Mode - Create/Build ");
      } else if (cmd == "DspPrj") {
            ProjectMode = false;
            webSocketServer.sendData("R:Project Mode - Search");
      } else if (cmd == "UpdLab") {
            // searchsdName - sd name (full path)
            // searchBCposition - position in the file we found the barcode label
            // call update barcode label function with the updated label string
            updateBCLabel(usrVal);

            // tell the user
            webSocketServer.sendData("R:Barcode Label Updated");
      } else if (cmd == "RmvBcode") {
            // searchsdName - sd name (full path)
            // searchBCposition - position in the file we found the barcode
            removeBarcode();

            // tell the user
            webSocketServer.sendData("R:Barcode Removed");
      } else if (cmd == "UpdDesc") {
            // create/build project
            // CHECK to see if a specific file is available
            // if the file is available, truncate it and write the new description:
            if(SD.exists(sdDesc)) {
                DESC = SD.open (sdDesc, O_READ | O_WRITE | O_TRUNC);
                if(DESC) {
                    DESC.print(usrVal);
                    webSocketServer.sendData("S:" + usrVal);
                    webSocketServer.sendData("R:Description Updated");
                } else {
                    webSocketServer.sendData("R:Description Updated Failed");    
                }
                DESC.close();
            }
      } else if (cmd == "SaveBC") {
            // call save barcode function with the barcode/label combo string
            saveBarcode(usrVal);
            webSocketServer.sendData("R:Barcode Saved");
      } else if (cmd == "ReSet") {
            // do local reset stuff
            // reset the file name
            fileName = "";
            fileName.toCharArray(sdName, 20);

            descName = "";
            descName.toCharArray(sdDesc, 20);

            // send reset response to tell webpage to reset
            webSocketServer.sendData("Z:");
      } else if (cmd == "Select") {
            // build the full directory and file name
            fileName = "/ARK/";  // directory

            fileName += usrVal;  // user create file name (8.3 format), this is the 8
            descName = fileName;

            fileName += ".txt";  // this is the 3
            descName += ".des";

            if(TerminalAttached) {
                Serial.println("Projectname: " + fileName);
            }

            // CREATE a file if it does not exists
            fileName.toCharArray(sdName, 20);
            descName.toCharArray(sdDesc, 20);
            if(!SD.exists(sdName)) {
              ARK = SD.open(sdName, FILE_WRITE);
              DESC = SD.open(descName, FILE_WRITE);
              if(ARK) {
                if(TerminalAttached) {
                    Serial.println("Project Created: " + fileName);
                }
                // send the 8.3 part of the file name to the user as Active Projectname
                webSocketServer.sendData("F:" + fileName.substring(5));
                ARK.close();
                DESC.close();
              } else {
                if(TerminalAttached) {
                    Serial.println("Project NOT Created: " + fileName);
                }
              }
            } else {
                if(TerminalAttached) {
                  Serial.println("Project Opened: " + fileName);
                }
                // send the 8.3 part of the file name to the user as Active Project Name
                webSocketServer.sendData("F:" + fileName.substring(5));

                //now read the description for the file so it can be displayed
                // create the vars we need
                boolean readError = false;
                char readBuffer[100];
                byte b = 0;
                int readIndex = 0;
                String readData;
                DESC = SD.open(sdDesc, FILE_READ);
                DESC.seek(0);
                while (DESC.available()) {
                     b = DESC.read();
                     readBuffer[readIndex++] = b;
                     if(readIndex >= 100) {
                        readError = true;
                        webSocketServer.sendData("S:Error Description too Long");
                        break;
                     }
                }
                if(!readError) {
                    // if so end the buffer
                    readBuffer[readIndex] = 0;

                    // format it for sending over the websocket
                    String rs = String(readBuffer);
                    webSocketServer.sendData("S:" + rs);
                    if(TerminalAttached) {
                      Serial.println("Project Description: " + rs);
                    }
                }
                DESC.close();
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
            // 31 string of 7+3+20+1 + 0x00
            char readBuffer[32];
            byte b = 0;
            int readIndex = 0;
            String readData;
            int barcodecount = 0;
            boolean buildBC = true;
            while (ARK.available()) {
                if(readIndex >= 31) {
                    webSocketServer.sendData("O:Error Reading BCS - Too Long");
                    break;
                }

                // get a character from the file
                b = ARK.read();
                readBuffer[readIndex++] = b;

                // build the barcode
                if(buildBC) {
                    if(b == 0x0d) {
                        // format it for sending over the websocket
                        readBuffer[--readIndex] = 0x20; // space
                        readBuffer[readIndex++] = 0x2D; // dash
                        readBuffer[readIndex++] = 0x20; // space
                        buildBC = false;
                    }
                // build the label
                } else {
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
                        buildBC = true;
                    }
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
                SD.remove(descName);
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
