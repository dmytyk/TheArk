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
int chipSelect = 4;   //MEM Shield chip select for an MRK 1010
File ARK;             // general purpose file class
File ARKSEARCH;         // search file class
String fileName;      // fileName, must be 8.3 name format (8 = name, 3 = extension)
char sdName[20];      // SD Open friendly name, can include directory, must end with a 0
String searchfileName;      // fileName, must be 8.3 name format (8 = name, 3 = extension)
char searchsdName[20];      // SD Open friendly name, can include directory, must end with a 0


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
char webpage_base64[] = "H4sICLKPj2MEAHdlYnBhZ2UuaHRtbADtXXmTqkYQ/z9V+Q7GVCWbMlm8j7x9qeISETwAESVJpZBL5JRDxFS+ewaPdV1do64vN5W3K8NMT/eve7p7pnHz9AXWQwfjPp6Zhrb1w+efPaW/M5bk6B+zqpMFLRlwPU1VSdl+Xt+HRmipPwymagb2zUzfkhLddyNHeYLWT152tdVQyshTyQ/U8GM2CrXv6tmXzy3DMTO+an3MBlPXD+UozBiy62QzU1/VPma/POgchMmW+v6auEqS+fW5bd8uyeaGq+9k13L97zNf4hW80mx+OOz72+Gtd4qW5jrhd5pkG1byPRDZkKxvM4HkBN8Fqm9oxwQPGxRj8RgbmhGEUhgFp+h7bmCEhut8n5EmgWtFofrhuFPoet9nChVveeKZpWrhyYe/HbMSAaZlIJDvWu/kJZ9/NzOyr0qh6vmzd7JyDpZS+SJWFCPwgCX/LXiZqKo3icLQdd7HS7FRe7eKLFVxnT+BmXr+Um407R7slMr597PjB+GfwMtlegIORjH8e7Bz1oQrl5lwyo5mWOo9+Knm38+Pby/uhU7+DvAAdu6FTqFyB3hky78bP4074BN5lisp92KpWL4DRKGr61Yaq+7BUekWjg4bHmXXtiVHeZudiesrKsh8ACuG8uGSHKnng9zvFNPb5xMLDDnx2JMUxXB0gLS3TP+d6BIbSjjdZQ1v5FiBsVJBl9ME5MgPUh4813BC1T+FrLoMv5MsQwcKkNW003mtvoizIKjchOD6+Xcv0bkIZ8SKzqAcT41QPYcysA7w7xzKlb8Rypa6tdEbrfRGjAlfVZ3/IMh/tiWDpv8MyiDf+ytMmVWV/wzE2zz2r4D5k3vl4t8M5zS/+h/oTww0SPX/Inv+1G7jb4byX2XN/yWYwT7xf5g/Pcyb7e9fhfSnz5z/Tlg/7+sNR3P/TJjXm+1/dRzcNzxBu+LN5u5FSekpLeG8LPOAw5aMbElB8DG7r5vsCkH7IpQ0sdTMmurH7Ilqj7a+Puy0B8TcqGarxh29Q5r+tvX4iZJqJfAk52O2mM2kcoNPG7mzOy62UxVeTbXVRik9+QDzPk1+4JIgVO0Mt5bsCZr8ACpnyompQbN/LZ8/CEbTyKDgbChyDFlKj6MydFpl+349SSqJoXzMcp1fDvpk78oDIoUAmCQzdK1Q0tVXcyPwYICz41+4ATzguWtmBk2p2kHrvgWYC7g/bT8vil3/LgNSw8jLQEDNjmbokb9W4b0Nqe+7M1UO17o70+3JcDxQtg0TD0iQeoXsWslp8HIkWwXCGSvwpFDKZmxpCRr1cPoxC8rAXzmTwPvwtIlw2/Gbm2wGjAR36DT1kNmdNrcHnrs+riNbhmyC0rHqKFstb7s8pOO/yf6wIfAEbYbcE523tQlm3TABMpbAc51ADT7VvD88pXhLoIi6Bczfzpiq4PluD+C+wXdjcF/NpuTAh1IdNAGX7DpWAlDaUb0CsItXZsrYc9l3x9m+5Z+/TAu1ym6ZomupMtuFdOf1+drqYDk0FmqmCdbdobvdPPglfXCNr/1jBs4sAU6WHEdVMojko66ifqpZjxZAsJl3IvkymDZI18FR49bojtp3q6KwWxXFwntXxXXSHGK4tZ6/wIukrnvrKjYQHjQ9A3jYuoev/AK/22C7ypnsX9zY8vXc9G9zJ9hGrD/Jn2zCczrBXWc5Y/CsKv35HgN8OnAXr1r2oVP62zoKgNvtycZ7eTkJ6IH3OGh6Belf6T22HY4cyr4qvGX1oOkEDydT2GdSz0QOqs3P6eum/QEkqwM1CDMIuNnlq0dynWF695rWnutt2y1sg4GHZDYP94k5rSoXZuCgZ6bXvVUiTTsQadd4q0yadixU2riXCtxdLpam3SLXpoa4F2rdcotErAq2ga+J7LW07XCROMB5qOGFwlyS128IPS/14wdvgXNQ/dsOf269BaTt0NekjgwaNGOGfxFY3NSNdzE4uBC0IyHT1OlAyn3zLWIqke2lY4+InRI03QlcLektgj6XvvZirhtv1OVm5CtCR0ZvLy5VJKvaLtgxge5APtdPbhTxSJXr1rtpckfrWM6L9bgR9D2a3Nd99mKuW28Vczv0NKkXJ0CWf7GUqKVK/nuE3FRdjuTcPbhVVN5TUhleUTuWdt9zL+2+705YICjmxk7a/B5Zn6seh6Lu26+VdD9+X03ZC7JvTwXY8p3pgGz6QuYv2gAehp3nJxfEnbvDsVFo/q/AJZB9wwtfjbPUMAMOwxeqn/mYycbB9xCUzeTAHtNR3PjRcjdVgMepG4Qp6+BR9vt6vp6Hsh9OEHJlE/z6mHEiyzr1XFqou/PfdLquG2bSVORVV8WVIxvsYR51NcQtNf2IJKTy8PVzuvD1N4/rTfLu2xmA2NcTC8z+9dWkthbwforbgW9wB1azegOpLXfnKB7S1CJHXtd2gC90gM1wa5U8fLMrYF4y/3ER6JtHA1DzW4MOnU79lBYKN+XIdLumAB+7mQ2UH5+g9OEPX3+4Yr7Dws8fz5Z/zN8yzYsDzz+eY2ubL+Y5nmlv72qcEdTJFuzNevrmzJBH4N091QEjd/raa+hica7Skp7W7p/1pCpn8PvtLN+2GgSgdPeSdfuA96NFDxjGpFACI+xHBXx4BCZshA/Z77PffDg9zNAyD7thP+Z/znwEvgLJPs/y1kzbc5i0MAz4u2Dtv15WW3beYumQ/rODOODrKg2+Omh+pcBnBAo/p273Jyd7hr/fMqoVqDdzAhC5Dxuff3ZF8yk9o3s9nx6yH1HYjOhl/1ADEJSR11lgCL6cmkagzO6Aaf2+wsRd3ojbi5PuI+Cy2Q93ogoit2tZA9fLfLx2UEs19Gn4bsM5ADA1lnsBmNK6P4Ap1asBTAfdAuC7TR7bGPC7pdxjl7ty2V48xacC8gq0mmfROswxD2C4Rf43U4aDed4vFH2jCVybOX2dyb2yjK+PMoHja3MS/X4pu59eynWl7BY5rxCjd6MYqbl84vX6Z0WOK9Bib0Rrx9GnAWpH/VKMdv3vDo94Hh4IAiHXCDLgv3XoTQ/kn2PvDXJvXVYqx0ICX7fYhdmLHepu034T6nffgZ3KT7b7gDTRUy0l87DNsnfN39zA95lE/Y8BvHjdXZz7XEzxItM+t/xvBh3kAO9CHIy/N9w3p5oXU7wI7vtkR2c36UAu3/UvOl3YDpEtN1Afvrn6NGA97qKJ7nOM4frrNz/TV17kC44y0gs4zIFhq24UPhyciX2bfuc8/4bI5xB4fbT22hp3h6fgdXx8AcRMS2sqEOgBVPQ111ej9SF99tsD0K7E+Dcw6VWq/O2bt48KT5ROZFvZMnV8zJK+PD2UrNfHvPsr9JM3zS02Qnn6Fvk9whKQePea7/fnN4F7dh4ujnrfPIa+YT+DdOawYUt9Har/+Jzh0pTjIKHJ4r6f+S7TdXd1m+wf5zXXTHahV7omv3l5+eCdc3/3taArN8m7awKcovnhD+xhV4zfGcSlCcuuJP8YgnfQbz1RuDRz2e4/Xs55wQbrEgwUVZMiK/z+diq/nQ0BgbpZ98D4QI0H2OBuZZ10kUAlYCFnHtTnNXE+GD2ot3jaZxd1XO/czHuckcwi8LaU7kuTdVbybAea79rrlvpjKQO8sC2FJ73bevxu1Ktt/u7g+nF9cH1qdlsywUukka9mYjUzBUMz0jMLoP+xe3k522ZHsE+w1xL+Yd51xWnmxRngRZnRxdQu8j83JaEAiHQ7ZIBvmbgZB8Cmpe/svpPdA+GbKbqpQpop5b8FFNcd4IK4DtbD7m3Jl9POI/BNLE611u+4AKN+Ne2Hc/RA4WOyLXsh4OPDj7sJNuH1528zv6Zl7+8z2fQBBGorhpP97TzNdH7eT+M4z9LbvwDYm6ReFNw/pDOeH7/+k5F7+bYUtsg+ZKXsN+fsaNM7XVEbQun3dKQImNW6Hg9M7OTIdd9HZfveBZj91Xr+cGZU+pcswYit2Od6rll4eJf/3L9O8GZad9eyWXrScXGdDMC/dpy74ra0jrXXLbUtd1dU3y+neEER/irC2/HXvMdwBc1LXmi4IK5cv5+/eS9/NbH7xapPsYM/ycKpTVYWC7w++C7KN1d79d2K2ZjoJ1gwtxnhJSvmNsrXv1xzBckL1vf1C+ayU8f3nDheRe+fmuKdWjaoH6bL5paACDS2Vm2wSxSPY+W62vV2mJxI4S+SJfn2Nv/JwJFiuA/Z9M2W76X0MxRLiw8TsGOtlr+FoIhhYXAJHY6OeXIMw8HYHOkuE8O4NKYFzRR0mMzHQgCRIehHYPo4hHU46eYYQYJhYxp3BLtM1BmGxYyOjCJyq4TKORRbQLUlXB4spg12BSV1BsYQ0FVnYJGASLoLwyxTJuiBjKkkSU6XRA+BI6w27WkIUoIaZV1ul+KcW4EqKgmTYOZhnA4eMsUeK5FMjHfgtoHpjQpRdhi6ZjNDgawSMStrgQjLAWW2F0t9ic9WTV7XWi7HNJh5jerG81YcIYVSBy5LhTmE2Yjf0vuyPTMIdwpBeIxWHKKP21wZUDKHVG/a1nJCT3eXbdlqrkqm7kCQgsC5RCoQ08jvOvXVBIIa3W4UdqPpqlTQIHA1FiH4ucqRCowD3qkyzZAw5U5aZIiTY5SA8SUSq/WZPHCbjRVJ+biCjcmoX+zoPaHJ4aVyPDbYijFn+rN8m1TdoWIR5SFWHsHVmg3LXamgTVdwYRa3ogG7xHgGgowxIq9mLWPQlgGlhLJajLmA6BbDj82IMyq1hBkAfeu6JnalGVMa2qucCNjM2XZJsEtMpVZdpLznSkXwswLhPR2wjg31KQOnV2orIszDSEEg830x0tetwFLAT9wsOaxSnK+bUKAXeHMh6BzHGZZG9dhDiI7VNslVfUZNicWsbFZnK9aZS0GXAgJOKzpZQ9oSWoALsNtFZm19ksDekCuTmCXqGF8ReHiJU1jXtmG6tRK9fNNcFZqdgU00LfAbT1yFlBYyCvMtTPApmAQqloouqpM0jkSeqLcW8BRHZLE6cxlkydWqsC7LftOSV8iCotE8QxZlxEWbC22AwW5NdXsM6cyxPhIufAwXke64W1nG3XjJsFpcF5lKzAzpRa/eG/M5u7Vo0hY2zVc6il5DBtWxz/Y9FFk5HaToQ9CSas+SpDfNyaTtF6Gy4gZyjFUdVZxFIluGdYDVeI1YUljKChMux8h4LCqGZvKYvlqNpJHTqIt512iQNF+xkVkT7tpsbA3bJag6G2OwvZQMjAA3yxnDoChlshOoaupQLORMrl2BIJ92JybNGZQ970N5w2xHmMRJnAZV2wY9pvGxI0A5sPYLAjts01ADt5nCWCSDBVRlVnFSmZs1KBfifN4xraAErUi+06e6I6iaiLSWM5s6rOlEQjdhPh4/24qsVHij2x+OUtm8tU3g+aHIUHGD5FJb2VjKkCETncStod80ES7Xnho5HI6inEXB5pjHycDGYdztqJiN4oPOQuNaA8JsVlihVTNXEM61hD48h0wZZadzJOwortlXOrWFYdVYolGHi4FcW/Wq4mIAWj27x8PiwGkLue5oObeVckGZt6s2PRgt/b7nDOvSQiK1Crega56j9Bd98LPNNqgoqmtqfxEZEfib/7ow82odDeqzs6XcRPuYKZQKgW+4KL40FouGJccULrUCDaq5cRFY6ABqsABdfDrGLdhF6WbYgf1lbCywdjs3YQutAdVWLNwd4DRKznU8HhIck5uSdr47oerCSB7gZJ7vu1KxOyHKSZHK59u4MBGlRXtkjqW520DZFlepNGVxMKPAguxSLjrmHY7J+8LSFydNakKR0x7VF9WC3KZWIicxPpnwdAsT7WGFKi8TLsBYSuj5keQQ4Yxuj5q4HUvkBKNH+SnZR3imP2wOTcbEXL6rcS6mMwTVV013qRIruIzxC8cpT5YrWfTDdmJ4lNbvJpU417CcUazPAyNpdVVu6Kl2XJ4Ik4YXjNX52lYQF0aYsJx0hsvx2t2bjAt+thZjj67YVThdIOgIS20FqzRmE2tg+IzaiLxmYIsTrxMpSY1eFUYFbBwsez1O8LT+vDsqCnMzpPJYZxERsTXpzsrlgaXSfSZHBNokR/SEHtV2SV+YlGSyo6q9btVaFSN7MO+MhvXOrJRPhL43czmpEc2oOpeo7X4lqdg9oYBVpwta9dSFCkwTc/O2jrKo1cN1uLsoIQ2pqhmmznMRiDFwBx74xLLmOHrFqxiG3e+KgcbQY6QYYDRprlhK7LUIWOITTPaaCEcTjQ7KwDQIvToTFMeR0wtWNAcnzLg/RItjhSDUFlZUBkrbLHgwGCpgM8eUaaqZ2FyJZcQB6awSzgUhl8gzkYqW9E6A4uMlP7d8eUlVzQLNMHJgJ2OV6SuApBYRfZhDEVMedhhm7g3bwTQyCqE5YtrMMN8bEbDYGXE9HEdYwx9Nhiwj1CJ9XiM7Nb8vcGKCWZ0SGXVxc7lCe+28kxQH066ZsJJRmFRVEFpKyqpLrLogXJYjeNEaVAKelUW0KUcUPe01g7pNwyBchmVegIENIJsIUp5PWm4+JGdDk+c3fiWGEY0DwVicbyKTo69jUKE163i7GCRi+tpWeHOAF3hdxHl8OR8nTrcwFxdzssfieNNG5DZjxsrSaIZxfo73TGKsgTRGQyKj0WH0MgO34LkQwkLLRZdRYayVh5pA98lO0W5hzWDC4EMiTuaMy8ixo7VU0uyGjbFjGhJXcI1VIs/b/rJM9ZiZNBWXwkQW9VU4K8rlhkPZJqmUy1ytMMvLdXXYSDCqGvXZ1bIyKtRVbDFbMNWEchsix1UtCKqDk7A+BdEgvSBJx+81oeYCc2FsDBNIh5jUmiHu25XOqGPZfRhoTBdCqSYui7qbqy/bed5AJ3W61VK0UEjoNg6CHUwGZI6fjKJqTRE7M1vHCJSwCq3cwmuWuxwmmxxpmrWhkguW+BRhOixfn0GFIk+y0xUn8QqT8xm+YhJqc2nnppOCO616gmAvStHCZlXMyvPOIpS8oUSu3MJMnGiQh1Vxi55zo5zP2z2m67WbEuTnAqmsQh5iAW27a+2VUF7nprRCkMtgXEvjSJ/1dRRGFCxiKgRaw+Z9BtUnkzYPuyzHz3JdEjf0/FhLBH7qFoYIgsAbv1KGEXFZWWkdsB9NSdtx+rNZaZb6+bhBhKmtaOtJY3raAWt1LHT0IU4U+HyTJFBEHyEjONQ6rUIeXPj6QnlkbDn50mAwULoIJrqe640XfnonlYoVm7S7OnjYHzbmJc8e55durzpZlPx8PsmRGGEoUEOSJHFAdKJZw/aiRscz3IIy8v1KC2cHImfP+VIVoBDZqy4IG+5kAY2W5UXDmJkcy3VpZgoLkS159aLAcgg/H3HhEBrLXG40pB2Oywk05rtzGATTDh7AXE4VbJ9f+b28i9t4Oc9aHbGS62GkCScUTPCxWLKHlC1Lttx2OkKx2mH7Oh9XZznanCYtYkiMZxzPU10vavOM2yPhBFvgCVHnKolGVZWpKOcHywJFQvrCoOjqfLFoBoXcBJjgqlKP+K7jJ/OqGVbYVQ81+4I1nA9HRQkCvoRX+ALlj2itnCdcw2iOKJB7zsGfb8NGfk+fY2yp3kbyk7jGuXRORBpugUVNZsYUUWQsUgqUG7aXIsuK1pwzKSMO8kJzySdoHFBGJ6lN+WFzSkqo32XAjmBABrSxiCnK0INZfTS24hASKKPYLSvrfRAKEpIZz3bJltcREWJsSEyfGyepE6Lh7YUK618e34Q6ei4vuVOOBPpB4KBj9SCIMsuB6wZJoc012bEg0Wh5LiJjockSohLPAYdmFxnaLsjx2yZgdVRuNoEjZXN0wJUFyG+CpO3/6+99DfFmC77w4ouNwsRmjUnJjESCh997dWZw3CWmeaUFV+mkUVJKciS/NQfKfzx1CLE/InhMzxqOX8V4+S295y/lbW63f/PwCVr/n7d+B7Vid6CJawAA";




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
       String sbc = String(receive_buffer);
       if(socketClient.connected()) {
           webSocketServer.sendData("B:" + sbc);
       }

       // see what mode we are in, create == true OR display == false
       if(ProjectMode == true) {
           // create project
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
                   Serial.println("Error File Not Available: " + String(sdName));
               }
           }
       } else {
            // clear the read responses box
            webSocketServer.sendData("C:R");

            String tempName;
            boolean notFound = true;

            // open the ARK directory
            ARK = SD.open("/ARK");

            // make sure we are at the first file in the directory
            ARK.rewindDirectory();

            // keep displaying files until we are done
            while (notFound) {
                // get next directory entry
                File entry =  ARK.openNextFile();
                if (! entry) {
                  break;
                }

                // get the file name, and convert to a string
                tempName = entry.name();
                if(TerminalAttached) {
                    Serial.println("Searching File Name: " + tempName);
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
      } else if (cmd == "CrtPrj") {
            ProjectMode = true;
            webSocketServer.sendData("R:Create Project Mode");
      } else if (cmd == "DspPrj") {
            ProjectMode = false;
            webSocketServer.sendData("R:Display Project Mode");
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
                Serial.println("Filename: " + fileName);
            }

            // CREATE a file if it does not exists
            fileName.toCharArray(sdName, 20);
            if(!SD.exists(sdName)) {
              ARK = SD.open(sdName, FILE_WRITE);
              if(ARK) {
                if(TerminalAttached) {
                    Serial.println("File Created: " + fileName);
                }
                // send the 8.3 part of the file name to the user as Active Filename
                webSocketServer.sendData("F:" + fileName.substring(5));
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
              // send the 8.3 part of the file name to the user as Active Filename
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
            webSocketServer.sendData("O:File Name Not Set!");
          }
       } else if (cmd == "RmvDir") {   
            // clear the file responses box
            webSocketServer.sendData("C:O");
                                 
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
            webSocketServer.sendData("C:O");
                                 
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
            webSocketServer.sendData("C:O");
                                 
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

           // send the battery status every 10 seconds
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
