#include <Phpoc.h>
#include <Arduino_JSON.h>
#include "grove_camera.h"

// Replace your GOOGLE_CLIENT_ID and GOOGLE_CLIENT_SECRET here
String GOOGLE_CLIENT_ID      = "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx.apps.googleusercontent.com";
String GOOGLE_CLIENT_SECRET  = "xxxxxxxxxxxxxxxxxxxxxxxx";
PhpocServer websocket_server(80);

String http_resp_hearder(PhpocClient &client){
  String hearder = "";
  while(1){
    if(client.available()){
      String line = client.readLine();

      if(line == "\r\n")
        break;
      else
        hearder += line;
    }

    if(!client.connected()){
      client.stop();
      break;
    }
  }

  return hearder;
}

String http_resp_body(PhpocClient &client){
  String body = "";
  while(1){
    if(client.available()){
       char c = client.read();
       body += c;
    }

    if(!client.connected()){
      client.stop();
      break;
    }
  }

  return body;
}

String access_token                  = "";
String refresh_token                 = "";
unsigned long access_token_expire_at = 0;

void websocket_send(String msg)
{
  char wbuf[256];
  msg.toCharArray(wbuf, msg.length() + 1);
  websocket_server.write(wbuf, msg.length());
}

void googleDeviceOAuthLogin(){
  PhpocClient client;

  // Step 1: Request device and user codes
  if(client.connectSSL("accounts.google.com", 443)){
    Serial.println(F("Connected to server"));

    String body = F("client_id=");
    body += GOOGLE_CLIENT_ID;
    body += F("&scope=https://www.googleapis.com/auth/drive.file");

    client.println(F("POST /o/oauth2/device/code HTTP/1.1"));
    client.println(F("Host: accounts.google.com"));
    client.println(F("Connection: close"));
    client.println(F("Accept: */*"));
    client.println(F("Content-Type: application/x-www-form-urlencoded"));
    client.print(F("Content-Length: ")); client.println(body.length());
    client.println();

    client.print(body);

    String response_hearder = http_resp_hearder(client);
    String response_body = http_resp_body(client);
    //Serial.println(response_hearder);
    //Serial.println(response_body);

    JSONVar body_json = JSON.parse(response_body);
    if(JSON.typeof(body_json) == "undefined"){
      Serial.println("Parsing input failed!");
      return;
    }

    // Step 2: Handle the authorization server response
    String device_code      = "";
    String user_code        = "";
    long expires_in          = 0;
    int interval            = 0;
    String verification_url = "";
    bool is_valid = true;

    if(body_json.hasOwnProperty("device_code"))
      device_code = body_json["device_code"];
    else
      is_valid = false;

    if(body_json.hasOwnProperty("user_code"))
      user_code = body_json["user_code"];
    else
      is_valid = false;

    if(body_json.hasOwnProperty("expires_in"))
      expires_in = (long) body_json["expires_in"];
    else
      is_valid = false;

    if(body_json.hasOwnProperty("interval"))
      interval = (int) body_json["interval"];
    else
      is_valid = false;

    if(body_json.hasOwnProperty("verification_url"))
      verification_url = body_json["verification_url"];
    else
      is_valid = false;

    if(is_valid){
      // Step 3: Display the user code
      Serial.print(F("Next, visit "));
      Serial.print(verification_url);
      Serial.print(F(" on your desktop or smartphone and enter this code: "));
      Serial.println(user_code);
      String msg;
      
      msg  = "{\"provider\": \"google\",";
      msg += "\"action\": \"LOGIN\",";
      msg += "\"verification_url\": \"" + verification_url + "\",";
      msg += "\"user_code\": \"" + user_code + "\"}";
      websocket_send(msg);

      // Step 5: Poll authorization server
      int poll_max = expires_in / interval;

      body = F("client_id=");
      body += GOOGLE_CLIENT_ID;
      body += F("&client_secret=");
      body += GOOGLE_CLIENT_SECRET;
      body += F("&code=");
      body += device_code;
      body += F("&grant_type=http://oauth.net/grant_type/device/1.0");

      for(int poll_count = 0; poll_count < poll_max; poll_count++){
        if(client.connectSSL("www.googleapis.com", 443)){
          client.println(F("POST /oauth2/v4/token HTTP/1.1"));
          client.println(F("Host: www.googleapis.com"));
          client.println(F("Connection: close"));
          client.println(F("Accept: */*"));
          client.println(F("Content-Type: application/x-www-form-urlencoded"));
          client.print(F("Content-Length: ")); client.println(body.length());
          client.println();

          client.print(body);

          response_hearder = http_resp_hearder(client);
          response_body = http_resp_body(client);
          //Serial.println(response_hearder);
          //Serial.println(response_body);

          body_json = JSON.parse(response_body);
          if(JSON.typeof(body_json) == "undefined"){
            Serial.println("Parsing input failed!");
            return;
          }

          long token_expires_in = 0;
          bool is_authorized = true;

          if(body_json.hasOwnProperty("access_token"))
            access_token = body_json["access_token"];
          else
            is_authorized = false;

          if(body_json.hasOwnProperty("expires_in"))
            token_expires_in = (long) body_json["expires_in"];
          else
            is_authorized = false;

          if(body_json.hasOwnProperty("refresh_token"))
            refresh_token = body_json["refresh_token"];
          else
            is_authorized = false;

          if(is_authorized){
            access_token_expire_at = millis() + token_expires_in * 1000;
            //Serial.print("access_token:");
            //Serial.println(access_token);

            // send success message to web
            msg  = "{\"provider\": \"google\",";
            msg += "\"action\": \"SUCCESS\"}";
            websocket_send(msg);
            break;
          }
        }

        delay(interval * 1000);
      }
    }
    else
      Serial.println(F("Invalid resonse from Google"));
  }
  else
    Serial.println(F("NOT Connected to server"));
}

void cameraToGoogleDrive()
{
  if(access_token == ""){
    Serial.println(F("access_token is invalid"));
    return;
  }

  long picture_len = cameraGetPicture();
  if(picture_len)
  {
    PhpocDateTime datetime;
    PhpocClient client;
    String file_name;
    String metadata;
    String jpeg_boundary;
    String end_boundary;
    

    datetime.date(F("YmdHis"));
    file_name = datetime.date();

    metadata  = F("--foo_bar_baz\r\n");
    metadata += F("Content-Type: application/json; charset=UTF-8\r\n\r\n");
    metadata += "{\"title\": \"ARDUINO_" + file_name + "\"}\r\n\r\n";
    jpeg_boundary  = F("--foo_bar_baz\r\n");
    jpeg_boundary += F("Content-Type: image/jpeg\r\n\r\n");
    end_boundary = F("\r\n--foo_bar_baz--");

    unsigned long body_len =metadata.length() + jpeg_boundary.length() + picture_len + end_boundary.length();

    int total = 0;
    if(client.connectSSL("www.googleapis.com", 443)){
      Serial.println(F("Connected to server"));

      String body = F("client_id=");
      body += GOOGLE_CLIENT_ID;
      body += F("&scope=https://www.googleapis.com/auth/drive.file");

      client.println(F("POST /upload/drive/v2/files?uploadType=multipart HTTP/1.1"));
      client.println(F("Host: www.googleapis.com"));
      client.println(F("Connection: close"));
      client.println(F("Accept: */*"));
      client.println(F("Content-Type: multipart/related; boundary=foo_bar_baz"));
      client.print(F("Content-Length: ")); client.println(body_len);
      client.print(F("Authorization: Bearer ")); client.println(access_token);
      client.println();

      client.print(metadata);
      client.print(jpeg_boundary);

      int i;
      int packet_num = cameraPacketNum();
      char packet[PIC_PKT_LEN] = {0};

      for(i = 0; i < packet_num; i++)
      {
        long packet_len = cameraGetPacket(i, packet);
        client.write((const uint8_t *)&packet[4], packet_len - 6);
        total += packet_len - 6;
      }
      
      cameraGetPacket(i, packet);
      client.print(end_boundary);

      String response_hearder = http_resp_hearder(client);
      String response_body = http_resp_body(client);
      //Serial.println(response_hearder);
      Serial.println(response_body);
    }
  }
  else
  {
    Serial.print("picture_len:");
    Serial.println(picture_len);
  }
}

int buttonState;
int lastButtonState = LOW;
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50;
bool isButtonPressed(int pin)
{
  int reading = digitalRead(pin);

  if (reading != lastButtonState)
    lastDebounceTime = millis();

  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != buttonState) {
      buttonState = reading;
      if (buttonState == HIGH) {
        return true;
      }
    }
  }

  lastButtonState = reading;
  return false;
}



void setup(){
  Serial.begin(115200);
  while(!Serial)
    ;

  Phpoc.begin(PF_LOG_SPI | PF_LOG_NET);
  websocket_server.beginWebSocket("login");
  Serial.print("WebSocket server address : ");
  Serial.println(Phpoc.localIP());

  pinMode(2, INPUT);
  cameraInit(CT_JPEG, PR_160x120, JR_640x480);
}

void loop(){
   PhpocClient client = websocket_server.available();

  if (client) {
    String ws_str = client.readLine();

    if(ws_str == "google\r\n")
    {
      googleDeviceOAuthLogin();
    }
  }

  if(isButtonPressed(2))
  {
    if(access_token != "" && access_token_expire_at > millis())
      cameraToGoogleDrive();
    else
      Serial.println("access_token is invalid, please login again");
  }
}
