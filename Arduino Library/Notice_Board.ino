#include <Wire.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>

#include <LiquidCrystal_I2C.h>
const int  en = 2, rw = 1, rs = 0, d4 = 4, d5 = 5, d6 = 6, d7 = 7, bl = 3;
const int i2c_addr = 0x3F;
LiquidCrystal_I2C lcd(i2c_addr, en, rw, rs, d4, d5, d6, d7, bl, POSITIVE);

AsyncWebServer server(80);

int buzzer = 2;
int screenWidth = 16;
int screenHeight = 2;
int stringStart, stringStop, scrollCursor = 0;
String message = "";
String line1 = ".:NOTICE:.";
String line2 = message;

const char* ssid = "Abid";
const char* password = "12345678";

const char* PARAM_INPUT_1 = "input1";

const char index_html[] PROGMEM = R"rawliteral(
  <!DOCTYPE HTML><html><head>
  <title>Smart Notice Board</title>
  <meta name="viewport" content="width=device-width, initial-scale=0">
  <style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}
  body{margin-top: 50px;}
  </style></head><body bgcolor="#ffffff"><center>
  <p> <font size="9" face="sans-serif"> Notice Board </font></p>
  <form action="/get">
  <p> <font size="5" face="sans-serif"> Enter Text to Display: 
  <br/>
  <input type="text" name="input1">
  <br/>
  <input type="submit" value="Broadcast">
  </font></p></form><br>
  </center></body></html>)rawliteral";

void notFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "Not found");
}

void setup() {
  Serial.begin(115200);
  pinMode(buzzer, OUTPUT);
  lcd.begin(screenWidth, screenHeight);
  lcd.clear();
  lcd.setCursor(2, 0);
  lcd.print("Notice Board");
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("WiFi Failed!");
    return;
  }
  Serial.println();
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  lcd.setCursor(0, 1);
  lcd.print("(");
  lcd.print(WiFi.localIP());
  lcd.print(")");
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html);
  });

  server.on("/get", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String inputParam;
    if (request->hasParam(PARAM_INPUT_1)) {
      message = "";
      message = request->getParam(PARAM_INPUT_1)->value();
      inputParam = PARAM_INPUT_1;
      line2 = message;
      lcd.clear();
      lcd.setCursor(3,0);
      lcd.print(line1);
      lcd.setCursor(0,1);
      lcd.print(line2);      
    }
    else {
      message = "No message sent";
      inputParam = "none";
    }
    Serial.println(message);
    digitalWrite(buzzer, HIGH);
   
  request->send_P(200, "text/html", index_html);
  });
  server.onNotFound(notFound);
  server.begin();
}

void loop() {
  if(line2.length()>0){
    if(line2.length()>16){
      lcd.setCursor(3, 0);
      lcd.print(line1);
      lcd.setCursor(scrollCursor, 1);
      lcd.print(line2.substring(stringStart,stringStop));

      delay(400);
      lcd.clear();
      if(stringStart == 0 && scrollCursor > 0){
        scrollCursor--;
        stringStop++;
      } else if (stringStart == stringStop){
        stringStart = stringStop = 0;
        scrollCursor = screenWidth;
      } else if (stringStop == line2.length() && scrollCursor == 0) {
        stringStart++;
      } else {
        stringStart++;
        stringStop++;
      }
    }else{
    lcd.clear();
    lcd.setCursor(3, 0);
    lcd.print(line1);
    lcd.setCursor(0, 1);
    lcd.print(line2);
    delay(400);
    }
  }else{
    lcd.clear();
    lcd.setCursor(2, 0);
    lcd.print("Notice Board");
    lcd.setCursor(0, 1);
    lcd.print("(");
    lcd.print(WiFi.localIP());
    lcd.print(")");
    delay(400);
  }
  digitalWrite(buzzer, LOW);
}
