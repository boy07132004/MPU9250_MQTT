#include <SPI.h>
#include <WiFi.h>
#include <AsyncMqttClient.h>
uint8_t MAG_ADR = 0x0c;
#define MOSI_PIN 23
#define SCK_PIN  18
#define SS_PIN   5  //cs
#define MISO_PIN 19
#define MQTT_HOST IPAddress(192, 168, 3, 161)
#define MQTT_PORT 1883

AsyncMqttClient mqttClient;
uint8_t buffer[14];
int16_t ax,ay,az;
const char* ssid = "RobotAccelerometer01";
const char* passwd = "Robot1234";

void setup() {
  //WiFi.mode(WIFI_AP);
  //WiFi.softAP(ssid,passwd);
  WiFi.begin(ssid,passwd);
  while (!WiFi.isConnected()){
    Serial.println("Failed to Connect WiFi.");
    delay(100);
  }
  Serial.println("WiFi Connected.");
  Serial.begin(115200);
  SPI.begin();
  SPI.setDataMode(SPI_MODE3);
  SPI.setBitOrder(MSBFIRST);
  SPI.setClockDivider(SPI_CLOCK_DIV8);
  
  pinMode(SS_PIN, OUTPUT);
  uint8_t who_am_i;

  digitalWrite(SS_PIN,LOW);
  who_am_i = SPI.transfer(0);
  digitalWrite(SS_PIN, HIGH);
  
  if(who_am_i == 0x71){
      Serial.println("Successfully connected to MPU9250");
      digitalWrite(SS_PIN,LOW);
      SPI.transfer(0x1b); //  Write(MSB=0)
      SPI.transfer(0x00);
      digitalWrite(SS_PIN,HIGH);
      delay(1);

      digitalWrite(SS_PIN,LOW);
      SPI.transfer(0x1c);
      SPI.transfer(0x00);
      digitalWrite(SS_PIN,HIGH);
      delay(1);
      
      digitalWrite(SS_PIN,LOW);
      SPI.transfer(0x6b);
      SPI.transfer(0x00);
      digitalWrite(SS_PIN,HIGH);
      delay(1);

 }
  else{
      Serial.println("Failed to Connect to MPU9250");
  }
  mqttClient.setServer(MQTT_HOST, MQTT_PORT);
  mqttClient.connect();
}



#include <string>
String msgStr="";
unsigned long time_ = 0;
uint16_t interval;
uint8_t data_per_pkg = 100;
void loop() {
  uint8_t count;
  if (WiFi.status()==WL_CONNECTED){
    for(count=0;count<data_per_pkg;count++){
        while (micros()-time_<1000 && count!=0 );
        interval = micros()-time_;
        time_ = micros();
        uint8_t i;
        digitalWrite(SS_PIN,LOW);
        SPI.transfer(0xbb);
        for(i=0;i<6;i++){
          buffer[i] = SPI.transfer(0x00);
        }

        digitalWrite(SS_PIN, HIGH);

        ax = ((buffer[0]) << 8) | buffer[1];
        ay = ((buffer[2]) << 8) | buffer[3];
        az = ((buffer[4]) << 8) | buffer[5];
        char ax_[15] = "";
        char ay_[15] = "";
        char az_[15] = "";
        dtostrf((ax/16384.0),1,5,ax_);
        dtostrf((ay/16384.0),1,5,ay_);
        dtostrf((az/16384.0),1,5,az_);
        msgStr = msgStr + String(interval) + ',' + ax_+ ',' +ay_ + ',' + az_;
        if (count+1 != data_per_pkg) msgStr+='\n';
    }
    
    mqttClient.publish("rand", 2, true,(char*)msgStr.c_str());
    msgStr="";
  };
  

}