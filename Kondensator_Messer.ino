#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h> n
#include <LiquidCrystal_I2C.h> 

LiquidCrystal_I2C lcd(0x27, 16, 2); 

WebServer server(80);  // Object of WebServer(HTTP port, 80 is defult)

#define PIN_ANALOG    34
#define PIN_CHARGE    14
#define PIN_TASTER 12 


int i = 500;
int k = 0;
int zeit1[1024];
int spannung1[1024];
int zeit2[1024];
int spannung2[1024];
int run = 0;
int tasterStatus;

const char* ssid = "SSID"; 
const char* password = "password"; 

const int resistorValue = 10000;
unsigned long startTime1, startTime2, elapsedTime, endTime1, endTime2;
float microFarads, nanoFarads;

String v1 = "";
String v2 = "";
String ms1 = "";
String ms2 = "";
int b1 = 0;
int b2 = 0;

void setup() {
    Serial.begin(115200);
    
    pinMode(PIN_TASTER, INPUT);
    pinMode(PIN_CHARGE, OUTPUT);
    digitalWrite(PIN_CHARGE, LOW);
  
    //LCD an
    lcd.init();
    lcd.backlight();
    
    lcd.setCursor(0, 0);
    lcd.print("suche:");
    lcd.print(ssid);
    Serial.println("Versuche zu verbinden");
    Serial.println(ssid);

    // Verbindet sich mit WLAN
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
    }
    
    lcd.clear();
    lcd.print("Connected"); 
    
    Serial.println("");
    Serial.println("WiFi verbunden");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());

    lcd.clear();
    lcd.print(WiFi.localIP()); 
    
    server.on("/", handle_root);

    server.begin();
    Serial.println("HTTP server  ist gestartet");
    delay(100); 
}


void loop()
{   
   
   if(run < 2){ 
    
    if(run == 0){
    lcd.setCursor(0, 1);
    lcd.print("Warten...");
    Serial.println("Messung");
    }
    Serial.print(".");
    run++;
    digitalWrite(PIN_CHARGE, HIGH); // Beginnt den Kondensator zu Laden
    startTime1 = millis();
    bool query = false;
    b1 = 0;
    
    while(analogRead(PIN_ANALOG) < (4096*0.9933)) {
      if (analogRead(PIN_ANALOG) > (4096*0.632) && query == false){
          elapsedTime = millis() - startTime1;
          query = true;
          }  
      if(analogRead(PIN_ANALOG)%40 == 0){
          zeit1[b1] = (millis() - startTime1);
          spannung1[b1] = analogRead(PIN_ANALOG)*3330/4096;
          b1++;
          delay(5);
      }
    }
     
    endTime1 = millis()- startTime1;
    zeit1[b1] = endTime1;
    spannung1[b1] = analogRead(PIN_ANALOG)*3330/4096;

    
    microFarads = ((float)elapsedTime / resistorValue) * 1000;//Rechnung
    digitalWrite(PIN_CHARGE, LOW); // Stoppt Laden und Startet Entladen
    startTime2 = millis();  
    b2 = 0;
    while(analogRead(PIN_ANALOG) > 0) {
        Serial.println(analogRead(PIN_ANALOG));
        if(analogRead(PIN_ANALOG)%40 == 0){
        zeit2[b2] = (millis() - startTime2);
        spannung2[b2] = analogRead(PIN_ANALOG)*3330/4096;
        b2++;
        delay(5);
      }
    }
    
    endTime2 = millis()- startTime2;
    zeit2[b2] = endTime2;
    spannung2[b2] = analogRead(PIN_ANALOG)*3330/4096;
    pinMode(PIN_CHARGE, OUTPUT);
    
    //Datenaufbereitung
    ms1 = millisecond(zeit1, b1);
    v1 = voltage(spannung1, "0", b1);
    ms2 = millisecond(zeit2, b2);
    v2 = voltage(spannung2, (String)spannung1[b1], b2);

    if(run == 1){
     lcd.clear();
     lcd.print(WiFi.localIP()); 
     lcd.setCursor(0, 1);
     lcd.print("Fertig!");  
     Serial.println("Fertig!");
      }
 
    }else{
     server.handleClient();
     
     tasterStatus=digitalRead(PIN_TASTER);
     if (tasterStatus == HIGH){run = 0;}
      }
}

String HTML(){  //Website also HTML code
String head = "<!DOCTYPE html>\
<html lang=\"\">\
<style>\
    body {\
        font-family: Menlo, 'Roboto Mono', 'Andale Mono', \"Lucida Console\", monospace;\
        background-color: lightgray;\
    }\
\
    button {\
        width: 180px;\
        padding: 5px;\
        margin: 6px;\
        background-color: gray;\
        border: thin solid darkgrey;\
        color: white;\
        font-size: 14pt;\
        text-align: center;\
        text-decoration: none;\
        font-weight: bold;\
        display: inline-block;\
        cursor: pointer;\
    }\
</style>\
\
<head>\
    <meta charset=\"utf-8\">\
    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\
    <title>Kondensator</title>\
</head>\
\
<body>\
<center>\
        <h1>Kondensator</h1>\
    </center>\
    <center>\
        <table>\
            <tr>\
                <th>Laden</th>\
                <th>Entladen</th>\
            </tr>\
            <tr>\
                <td><canvas id=\"ladenChart\" width=\"500 \" height=\"600 \"></canvas></td>\
                <td><canvas id=\"entladenChart\" width=\"500 \" height=\"600 \"></canvas></td>\
            </tr>\
            <tr>\
                <td style=\"padding-left:30px;\">\
\
                    y-Achse &rarr; t in ms <br> x-Achse &rarr; U in mV<br><br>\
                    t<sub>max</sub> = "
+ (String)endTime1 +
" ms\
                </td>\
                <td style=\"padding-left:30px;\">\
\
                    y-Achse &rarr; t in ms <br>x-Achse &rarr; U in mV<br><br>\
                    t<sub>max</sub> = "
 + (String)endTime2 +
"ms\
                </td>\
            </tr>\
        </table>\
        <div>\
            <center><br><br>\
                <h2>C = "
+ (String)microFarads +
" &microF</h2>\
            </center>\
            <div>\
                <button onClick=\"window.location.href=window.location.href\">Aktualisieren</button>\
                <script src=\"https://cdnjs.cloudflare.com/ajax/libs/Chart.js/2.9.4/Chart.min.js\">\
                </script>\
                <script>\
                    var myChartObject = document.getElementById('ladenChart');\
                    var chart = new Chart(myChartObject, {\
                        type: 'line',\
                        data: {\
                            labels: ["
+ ms1 +
"],\
                            datasets: [{\
                                label: \"U in mV\",\
                                data: ["
+v1+
"],\
                                backgroundColor: 'rgba(2,224,5,0.4)',\
                                borderColr: 'rgba(65,105,225,1)'\
                            }]\
                        }\
                    });\
                    var myChartObject = document.getElementById('entladenChart');\
                    var chart = new Chart(myChartObject, {\
                        type: 'line',\
                        data: {\
                        labels: ["
+ms2+
"],\
                        datasets: [{\
                            label: \"U in mV\",\
                            data: ["
+v2+
"],\
                            backgroundColor: 'rgba(249,7,35,0.4)',\
                            borderColr: 'rgba(249,7,35,1)'\
                        }]\
                    }\
                    });\
\
                </script>\
</body>\
\
</html>\
"; // Ende html code
Serial.println(head);
return head;
  }

void handle_root() {
  server.send(200, "text/html", HTML());
  Serial.println("HTML wurde gestartet");
  }

String millisecond(int zeit[], int b){
  String milli;
  milli = "\"0\"";
    for(k = 1; k<b ;k++){ 
      milli = milli + ", \"" +zeit[k]+"\"";
     //\"1\", \"2\", \"3\"   
    }
    return milli;
  }

String voltage(int spannung[], String s, int b){
    String volt = s;
    for(k = 0; k < b ;k++){
    volt = volt+", " +spannung[k];
     // 2, 3, 5, 6, 7, 8, 9, 2
    }
    return volt;
  }
