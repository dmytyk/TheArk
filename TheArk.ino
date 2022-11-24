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

#include <global.h>
#include <base64.h>
#include <WebSocketClient.h>
#include <WebSocketServer.h>
#define _WIFININA_LOGLEVEL_       1
#include <WiFiNINA_Generic.h>
#include "network_parameters.h"

// global var
char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;
const byte IPLastByte  = 99;
const short webPort     = 80;
const short socketPort  = 8080;
int status = WL_IDLE_STATUS;

// WiFi
WiFiServer      WebServer(webPort);
WiFiServer      socketServer(socketPort);
WebSocketServer webSocketServer;
WiFiClient      socketClient;

// Console Attached
#ifndef TerminalAttached
    // true = terminal attached (send serial messages), false = no terminal attached no messages 
    #define TerminalAttached  true
#endif


// Data from Barcode Scanner
uint8_t receive_buffer[20], receive_buffer_counter, receive_byte;

// Filename
String filename;

uint8_t led;

// Background
boolean Backgroundinit = true;

// YMC32 webpage, gzipped and base64 encoding to save space
char webpage_base64[] = "H4sICCqmf2MEAHdlYnBhZ2UuaHRtbADtWuluw0QQ/o/EO4QgcSiAnTuBFsl3HB+J7TiODQj5tuMzvh3Eu7NO05bSUsqNgJXqemd3Z2e+mZ3Z7PrmPXyD7dQt0fOKKPzy3Xduuv+9UI/d274d9wGlB8qNZ+vW9f1SL/witL/chnrrZkkZWz06ruy4SLK2J7V5YUc30KXLj8dEdqH3TE/Pcru47ZeF8+mi/+P20I+DXmaHt/3cS7LCLIuebyZxv+dltnPbf/9J57xor9wfi5FYbe+7B9ojXTeDOzE/NZMwyT7vvU9MiSlJfvG07/dPq+lLvJwkLj519MgP2897SObr4Se9XI/zT3M7853nDJ8SLL/6rPYdPy/0osxf4p8muV/4Sfx5TzfyJCwL+4vnnYok/bw3nKbNC22h7RQvNn7/XJQSCG0ChbIk/H2yLODfLYth26lRFkUS/z5RXoVlPIHfIktoW0n8Rwgzhf8YYRznj5Bm8RugeUr4zEyiSI+tn5fGSDLLBisMSOJbX7xlLW4yEGxekvnaboRgyAvNqW5Zfux+3hulTff3Qpfatwrv6p0/s5Rz/2wDp3m5h1lmeSdCmvhxYWcv4Wo3xad66LsAftPuOr1u0B+5OTDqbwLw0v7pj8F5E8xoWL4Ccu35hf0ayMB1wN8rIA+nv4zy6K9CObSvLvobnfQ3Yiza1n8Q4r/Wj6nMtuP/Aso30P0e5672oy3YTbfT6d4f+oJE0TNDPc9v+z9K6vc7psdtm26Edu/C97b/wrbIuZQv7o0H1LyzzNWK9/ye8syu1OctVmeVPNXj2/6o3+v0Bm93evfvpbhONfzJVFdrjGFgDDDvjfGlZBdlegMZX4KtpfXClICc/Vr5viT90I71yO54vtLtxo9TsB0t2hRI3Bmx3/Ot275zHQ2U8c+gZTjpf/lBbOTpFzd3a+M64q7S74G+Xa/+vamu2fS+OYnN0DcDsP+1Y6sz47X9o27cx/0vMQ9kSiDqXf8/EodHO41/aqcvRTtPkzi3/6zpvrzpANUzW7/ik10n7DB+rD2A9kjIkhrUJ/0LO8BrDkhgkSRx2AJw7rn+VTiheoYllp3/WdM9w8nQM7ObsMPpsXbF6ZFwj9Psj8MJkLpAAqiPFBCBQL2jPA9Jjz95/lURSfFJv4clcWyb3Qa8J11U/KNDFAaCQBn7pn6Zg+1+JH9+4Q8aL5aXuG+f9On/iunfYsmLKcFMP4lX+cth7MHEz4c/7n2vQ5+QXoLgxTD6wOuBy5NN9UMcvaN/BALnzs6LHgoqD7Hzca5HpX9W7vufg4+CX2m/SXIw8imfu8bH/MCCDj/NBteE/pOEAHr2NvxvV8pxnmh1T/zNajnOc7064qNil9qbVXOcN+v2QgDKzcxPi5+MDO2iBzJrZWe9216/zj+HoH5vANZ2bCX1Z2Fyt4Q+85K86CQBTf3PF/AChvpfvMAoMQPw77YXl2H4QruhF9/qoZ5FXRe77vWQ0vKTj/qWXuif6907VOvVF4ae27PJJxBUCiICisJJbC3TKoLkanBwE6FGCF1lFSdQXISGayWH6AL0o3BXLRAXafmBoOgI4ns1p0QTaiEIIu5zJoaaqzFmDjC8guYNMtlV3lI8Q+1CQHAUdHUFRKMgmuURRBQmFLszcZumaa+hNihS4nNv46DoGFpOXHM9rgfJFJraNEKDmfd1N3gvjDaiTgs1wSFrH3eXU2oSC+w8EvYKPaNq0XRyDTFzJlhXjdsQxzMpu84qkYSlcJozfH1a1SU6HHPIRB+eIDxCs5W7NaOjTyUeBBE1No2pLRFJE8Ap2DMbb+0MlI2bNGszJM/jwI0hyEKRQasPKa/M+HhxNiBoyfNlwZfeeTx0IFCWVQGe5wFtIQSQnZmwAo0wibGiC4JWMQohGrS2F0dzl5DLM81khIWrdLkdce5GISViPKlVX5z6J2F7hNe0neytkJrs8ckBmc0jxOT1oeOdkeGxXpU7scFlAYJ8FTXPx5W/W5uAU8uEKyGoIHYlyGpQSv503go7YG/XdTRePwrjfXQeaEDMQRSNlWgsTOezqpN9MB6B5xQiNi4QHd+7noB0pfMVDZERdKjQ8FYr3QsVeAp4EsE4Fq3R6ULCgF2Qu4JiJ4IQRBZz6xSluHAd0OfFkfGo6jgJZsezGJ/0nGeAgt7UpefoWseGyBBJePS4do0WSffShMZDzcXlqSIjDcHgfBQh7OqspTAZnIckt4soMgT/iTaxaL0yMURe4UrGIDQwsT5KMJdmCbRMNXdVIR6BmtrsmAhoI81niGuaGRmaZ7RiWAwW6JGJJhhZOTscSeZ2shHo+IRv0aLKcEJDeZWfNjVfN4Lo1AtNmNbCnq02i40qD6JVRbIh7sFTznLn6G6mZuI2xdBzzKGjDIIaZn1s2403MOkoG0ETK8nNGp/FtnYsNXGCuAAr9YJYO2xMSygaFVVVzfKdQMbd8/mgH+LlQoMTf0mz8jRCjyTCR2Id7tdjaHZUcSRqdB+nQKU5CgKGMYFoQLPAhWplEEjrKQRlbGIErOQz0WkLwX6wLnFd0iUHmq19VmUJNVagAVj7Q0Xcr1loSUTCUNXovIJmwrlup6dgDg0KQobjIMzH0JmWuS3DH6BZq7HOICBdxHGpliURuVYffMW0prLPb/eHTrf04hMEvNcEpl7SUucrd56yF+jWpYlwn5EBKg3Wnj8gkLIchAwSqDJB5xGBEAln4xFG7LjKkVY7KiCnorKaB2eIkFbKFjlBgYmJ3gktOCsJthY3r/xwLlLLBTLKzfl5M9OqHaCm0UZGtF28Vgb8oTlF1mRondaziN0dmmybxvuFXum0M5Uqdp7G1rbagudaXDJluXDsbVX6JbgrcJVjOuccaCseG5PEtnigjId55icY0fhVtQzNmiH0Ve5A86QeAQ/dQUsRoEt4KhEiCcaSBYdkTe1X+Ho9MMThasesrZBIdgSL0SeXqPeUJAw8OoJ5g1koB3NH0LC8TfQRb1CTdsTA8JpQDE2v1odA1U/JEhNX0nRKmtruyIAFyTMJpsqxJMCZ0mSaQTIGQ3sbZqvZQ3PNnDVJFzK6ldkVrkX7KTNpWinHRUbZZKUeU8WRXR9IIqp12sDZA+zRW1QWtntyHwgBnsi8IyW4K1DM1g6SxqbOyASXqzieGM3Z1LJi3fop42z5dloPlmF8qN1T7rcr3pb2qR3VE0Mxlmmu2qeLr6AJggrFpOX2jXoJ94GQgOeqUlN2Gs2QboFgB7zzFXy6PBrhzs8Ee1mmZB5pRsqVVjtnz8PDEFfzZrORlNTZnvjDSDkFBQPjXFVSdWjwx8lkF9rsVhhQuWMMqI2yYdYJnSnG2KQ5297ws/A8KqPdiTvsF9xxDLfKNj0mkr4sj8xCau31dtpOo40yxGdexdqpXdnANfEEjlxMxMIN4SJ8NUaX+szxA1eWSpBjEA7ZZVQzj2N3mk59P9ryWu4IrIqOcpylg7PIaJsVhehyi5spiUosteQwAWFB6nWFfKSW8SY/sxLSCup2j41Ui6LsFT6ydtY6GKYIGKrgxzgwWYZsI2ksCtqOjs+tlICUS8FCaWNjl8sxQm3kU5iZDTMLhqwgmHnUqrawtQBLp6S2iIShgbnnBOGU7te5V/rDIjgIa2EPbw4UonEHaUMQqOhnB2MvCsq8dE9zmptnW0XSWjzkxnTJE0FzxjZrOG5HO48PWlH3h8bMBqllbJ156syDdDkpkWq1m+ayaGoYaZYM623IfBGxCEiXxURWEOAD6F0GmZyMVQIX9HEfyPJdXKkR1JFAMtZOd5kpdi85aLg6cul9DtJw9+IrcrAjhrKrETLRnNQ25ocnrTrRG5EgyAg110JQW41PFjV8IjYBpTpgG+Ogpb/kBHciICvkpBSIskqwphyqzmTvKOyW5kbRCidzQyD2VN2ehEQw69hZ2XTAF0s1DnxdGib+uTVP66yZMBvhqHtaoxim5p6L48icLGMmCmhrMpHmwyNsLuz9ssWZWbkVz830MFzYeHWshFnLJEtNkmYhBC3s0N4yEAu2FzQdZxsSIis8QXAVoVCOMuZkQWTRlDtwYbRFgMVcpdDnWjNyk8GiWcOyjxkLdrWynEJp2TUBkh1C5/RANg7lbG5p3DFycQqjwuFqUKXkhJdwM5DoIJjvrUHeEB4qcKK8OELDkUyL3lnSZUsYZII8DSibbKKBZwwTb5YqSlSNyyoSbTyE5bgq9HSv0+dkeNQMB0rxGRGyJ+kwyORoI/DpmtShbJDrExtK0RBYO7lYb4zJruSxFkU3uTrv8shWzFwMQS28FKYUNsdPWwFzDWMtI4koyccBTxO+C6tOq8heMtyjKIrcxZUJgmrN9OxwemB3rKO6e5JTcryF6yVVdL7iXCatWY8Da1VVOHdPUEMZJmkKQ90DekAKh1sNYVCIS8FkVA1jeLzb7SwexbUkTVK1yrqaPh5NIzriXdC43S9P4zRS4SbZzIxqnMFwO6Bxyregpa7r2o7iyuMySssll/rJ0Dpk2XRFiDtNik7yeAZQKKMzD9JGYlTQoZlUS/8YSKLEs4KHKGWkp4uRIkqofDpIxR5STWlw2LOxJA0UFs+SEwKSKUfkiDSwlSiTz9kGToiImMBiyGnTwQanA6RlEEqutXG0ZyJTj8x1zCmjGSduXbmeHQds4LUrak+pR0mWGT4t17KQbGikxSuipRbStHWYmeVpJrxrhgwNuZXPsLNTVZH5cGAAFzxPF6XMx1l7mgXFVDxvsGCrhPvT/jDSIRBLZEseMtmBdSYwlfg+eWDA3vMETrPxQ7ZxT7g4XqxR2KjnUsIONHSZDEUsEI7CCENVjbGgwX7daKKohScpYPw6hxWykVuszhmfa+eevCc9WscyXgC/CHZ0zvpVzTC+mx8XBzWsC0hh/BE/sS6/gzCwITnKIk+vUk5DKdXXha2ktl0QYpFrwZTLv1QmIc4dwHriSTSwD4rkXLiBICaY5EmSt8O1RIqqorPY5KShqkKKlGbVJyBhwKP7KAF7/HUARD1MSBIEUnHA5tJEgTISbNr+L//ssifIFfLGIo+WQyMSfWMclBolI7+3cEek5ikPtlbIjG2XY2tslubPzYHJt/2Pv/jpfbpTxndHh+bdKaJ0OVD46OOXLtSsxCwjcFL5mWsXRGh3r2hLWx89P//7+DMfcMtWO44FBw8f3nQ3V3f3Y93pudX/8v7MMnZvoK7xyw8fJXss1+ON69mFYhtX6e6OTz5+Zchn4NgntWMw8l7BR5X+HLXc7lbwQTHbetDr+aTfvyp3ZOe57to/Fj16IvuzMyAgMA7Oc8CI6DMLvHyWp6FffNT/HNj75WG+0/vofthX8De9W3ASJfYfZvlVKN3fhjxBZ3AvS3d89XXc/+L3Mc5NcD4X7pK0d/sr+q9s3/WKn5n6+7cjg/5GZO7vP/5wZO4ZvxWZ+/6/GZlXPRZoliXZm5badYgZJrn90ce/emlcxr1poj9mTSeXD4P6X+J+br5hXXcFfFm48yM7KYuPnkTUT3rdjvFnVH4NgZ8E5mdB7/7gGHxdQHQfQLLgnssGCgGr206S2WUcJrrV/+QJaL8S4+/BpL/KlN+/kmh+erVsRtaLEhVZ+1rU61js9fAVt3+8HP/4s0p//PjoZzTJ7TthLgfu/e7JI/37yv10LxqwB1zJ9Hof2Q96vL5UPrJ/ix88Avj8yiIvjbvXXw3lj5UfPWj7yO9vVBiCein4vBfwAtexP4PF5X7tJSkebz4+65h89PFrn7k83NPcVa/fttxAly+SfwC1uvqYoSwAAA==";

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


void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(0, OUTPUT);                                   // Status LED

  if(TerminalAttached) {  
    Serial.begin(57600);
    delay(5000);
  }
//  Serial1.begin(9600);  
//  delay(250);  

  led = 0;
  
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
}

void loop() {
  // compare the previous status to the current status
  if (status != WiFi.status()) {
    // it has changed update the variable
    status = WiFi.status();

    if (status == WL_AP_CONNECTED) {
      // a device has connected to the AP
      Serial.println("Device connected to AP");
      printWifiStatus();
    } else {
      // a device has disconnected from the AP, and we are back in listening mode
      Serial.println("Device disconnected from AP");
    }
  }

  WiFiClient client = WebServer.available();   // listen for incoming clients

  if(client)
  {
     if(TerminalAttached) {
         //Serial.println("New Client");
         Serial.print("New client: "); Serial.print(client.remoteIP()); Serial.print(":"); Serial.println(client.remotePort());
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

//        // flush the serial1 port - clear out any barcode scanner data
//        while(Serial1.available()) {
//            char ch = Serial1.read();
//        }
       if(TerminalAttached) {
//           Serial.println("Serial Port 1 Flushed");
           Serial.println("Background Init Complete");
       }
    }
    
    // Background Process 1
    // see if we have a command/request from the user 
    String data = webSocketServer.getData();
    if (data.length() > 0) 
    {
      //String cmd = data.substring(0, data.indexOf(":"));
      //String setting = data.substring(data.indexOf(":") + 1);
      // get tools to parse incoming request
      char buf[data.length()+1];
      data.toCharArray(buf, sizeof(buf));
      char *token;
      char *pter = buf;
      byte i = 0;
      String cmd;
      String subcommand;
      String usrVal; 
      while ((token = strtok_r(pter, ":", &pter)))
      {
        switch(i) {
          case 0:
          cmd = token;
        break;
          case 1:
          subcommand = token;
        break;
          case 2:
          usrVal = token;
        break;
        }
        i++;
      }
     if(TerminalAttached) {
       Serial.println("CMD: " + String(cmd));
       Serial.println("Subcommand: " + String(subcommand));
       Serial.println("UsrVal: " + String(usrVal));
     }

      // process command
      switch (cmd.toInt()) {
          // set file name
          case 1:
            filename = usrVal;
            filename += ".txt";
            if(TerminalAttached) {
                Serial.println("Filename: " + filename);
            }
            webSocketServer.sendData("R:" + cmd + " - " + filename);
            break;
          // process command/controls
          case 2:
              if(subcommand == "Ledon") {
                  digitalWrite(0, HIGH);
                  webSocketServer.sendData("R:" + cmd + " - " + subcommand);
              } else if (subcommand == "Ledoff") {
                  digitalWrite(0, LOW);
                  webSocketServer.sendData("R:" + cmd + " - " + subcommand);
              } else if (subcommand == "Rst") {
                  reset_data();
                  webSocketServer.sendData("R:Reset Sent");
              } else {
                  webSocketServer.sendData("E:" + cmd + " - " + subcommand);
              }
              break;
          default:
              break;
      }
    }
  }

//    // Background Process 2 - process barcodes
//    if(Serial1.available()) {
//         receive_byte = Serial1.read();
//         receive_buffer[receive_buffer_counter++] = receive_byte;
//
//        // see if we have a valid buffer
//        if(receive_byte == '/r') {
//             if(TerminalAttached) {
//                // Barcode
//                Serial.print("Barcode: ");
//                Serial.println(barcode);
//             }
//             webSocketServer.sendData("B:"  + receive_buffer);
//        }
//    }
  
  // Background Process 3
  // background delay and heart beat
  if(led == 0) {
    led = 1;
    digitalWrite(LED_BUILTIN, HIGH);
  } else {
    led = 0;
    digitalWrite(LED_BUILTIN, LOW);
  }
}


//Process the data for the Web page
void reset_data(void){
}
