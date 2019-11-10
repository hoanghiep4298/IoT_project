#include <ESP8266WiFi.h>
#include <SocketIOClient.h>
#include <ArduinoJson.h>


StaticJsonBuffer<200> jsonBuffer;


SocketIOClient client;
//const char* ssid     = "hiephoang";
//const char* password = "12345abcd";
const char* ssid     = "hangxomtot";
const char* password = "0hoipass";

//char host[] = "104.215.198.148";
char host[] = "192.168.0.103";

int port = 80;
//int port = 443;

extern String RID;
extern String Rname;
extern String Rcontent;

int led = 13;
unsigned long previousMillis = 0;
long interval = 3000;
long pumpTiming = 7000;
unsigned long lastreply = 0;
unsigned long lastsend = 0;
unsigned long pumpStart = 0;
String JSON;
// Trang thai on/off cua may bom
String stateOfPump = "off";
// Trang thai on/off cua che do bom tu dong
String autoModeState = "on";
bool lockState = false;
unsigned censorValue;

JsonObject& root = jsonBuffer.createObject();
void setup() {
    pinMode(led, OUTPUT);
    digitalWrite(led, LOW);
//  root["time"] = 1351824120;level
//  JsonArray& data = root.createNestedArray("data");
//  data.add(double_with_n_digits(48.756080, 6));
//  data.add(double_with_n_digits(2.302038, 6));
  
  Serial.begin(115200);
  delay(10);

  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  if (!client.connect(host, port)) {
    Serial.println("connection failed");
    return;
  }
}
void loop() {
  if(!client.connected()){
    client.disconnect();
    client.connect(host, port);
    if (client.connected())
    {
      client.send("connection", "message", "I'm ESP8266");
    }
    Serial.println("Ket noi lai ...");
    delay(5000);
  }

  //Neu dang bom ma khong nhan turn off thi cu tiep tuc bom 7s
  
  //censorValue = analogRead(A0);
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis > interval)
  {
    //Handle
    if(stateOfPump == "on"){
      if(lockState == false) {
        pumpStart = millis();
        lockState = true;
        //turn pump on
        digitalWrite(led, HIGH);
      }
      // Turn off Pump after 7s
      if(currentMillis - pumpStart > pumpTiming){
        stateOfPump = "off";
      }
      
    } else {
      
      if(lockState == true){
        lockState = false;
        //turn pump off
        digitalWrite(led, LOW);
      }
    }
    //==============================
    //nếu bật chế độ tự động thì khi hết nước sẽ tự bơm.
    if(autoModeState == "on"){
      if(stateOfPump == "off"){
        censorValue = analogRead(A0);
        Serial.println(censorValue);
        if(censorValue > 300){
          stateOfPump = "on";
        }
      }
    }
    // update state
    previousMillis = currentMillis;
    JSON = "";
    root["stateOfPump"] = stateOfPump;
    root["autoModeState"] = autoModeState;
    root.printTo(JSON);
  
    Serial.println(JSON);
    //client.send("updateStatus", "status", String(statusOfPump));
    client.sendJSON("updateStatus", JSON);

  }
  //=============================

  if (client.monitor())
  {
    lastreply = millis(); 
    Serial.println(String(RID));
    Serial.println(String(Rname));
    // event switch Auto Mode
    if(RID == "switchAutoModeState" && Rname == "state") {
      Serial.println("------AUTO MODE STATE-----");

        //parse received mess to object and get it
        StaticJsonBuffer<200> autoModeBuf;
        JsonObject& x = autoModeBuf.parseObject(Rcontent);
        const char* autoStateTemp = x["state"] ;
        String autoModeCurrentState(autoStateTemp); //convert string

        //update state for autoModeState
        autoModeState = autoModeCurrentState;
        Serial.println(autoModeState);  
    }
    //------------------------------------------------------
    // event manually turn on or off the pump
    if(RID == "switchPumpState" && Rname == "state"){
      Serial.println("------MANUAL MODE STATE-----");

      StaticJsonBuffer<200> manualModeBuf;
      JsonObject& y = manualModeBuf.parseObject(Rcontent);
      const char* manualStateTemp = y["state"] ;
      String manualCurrentState(manualStateTemp);

       //update state for pump
      stateOfPump = manualCurrentState;
      Serial.println(stateOfPump);
    }

  }


  
}
