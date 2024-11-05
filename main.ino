#include <WiFi.h>
#include <Arduino.h>
#include <Wire.h>
#include <MicroNMEA.h>

//I2C communication parameters
#define DEFAULT_DEVICE_ADDRESS 0x3A
#define DEFAULT_DEVICE_PORT 0xFF

#define RESET_PIN 18

TwoWire& liv3f = Wire;

char c;
int idx = 0;

char nmeaBuffer[100];
MicroNMEA nmea(nmeaBuffer, sizeof(nmeaBuffer));

unsigned long previousMillis = 0;   
const long interval = 1000;        

void liv3fHardwareReset();

void sendCommand(char *cmd);

char *ssid = "DESKTOP-1CTQC9F-4593"; 
char *password = "3M45$64p";  

WiFiServer server(48569);


WiFiClient serverclient;
float readFloatFromWire(byte rawData[4]);

//////////////////////////////////////
char latBuffer[20];
char lonBuffer[20];
char angBuffer[20];

float readFloatFromWire(byte rawData[4]);

void liv3fHardwareReset()
{
 
   digitalWrite(RESET_PIN, LOW);
   delay(50);
   digitalWrite(RESET_PIN, HIGH);
   delay(2000);

}

float readFloatFromWire(byte rawData[4]) {
  float floatValue;
  byte* floatBytes = reinterpret_cast<byte*>(&floatValue);

  
  for (int i = 0; i < 4; i++) {
    floatBytes[i] = rawData[i];
  }
  return floatValue*57.2958;
}

void sendCommand(char *cmd)
{
   liv3f.beginTransmission(DEFAULT_DEVICE_ADDRESS);
   liv3f.write((uint8_t) DEFAULT_DEVICE_PORT);
   MicroNMEA::sendSentence(liv3f, cmd);
   liv3f.endTransmission(true);
}
void setup() {
  liv3f.begin(0,2); 
  pinMode(RESET_PIN, OUTPUT);
  digitalWrite(RESET_PIN, HIGH);

  liv3fHardwareReset();

  sendCommand("$PSTMCFGMSGL,3,1,0,0");
  sendCommand("$PSTMSETPAR,1227,1,2");

  delay(4000);
 
  liv3f.begin(0,2);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

   server.begin();
  while(!((serverclient)&&(serverclient.connected()))){
    serverclient = server.available();
  }
  Wire.begin(0,2);
  
}

void loop() {
  unsigned long currentMillis = millis(); 

  float lat, lon;
  byte rawData[4];
  int x=0;
  if(serverclient && serverclient.connected()){
    if(serverclient.available()){
    char d =serverclient.read();
    if(d=='G'||d=='g'||d=='Z'||d=='z'||d=='R'||d=='r'||d=='T'||d=='t'||d=='S'||d=='s'){      
      
      do{
        
        switch(d) {
          case 'G':
          case 'g':
              {
                Wire.beginTransmission(0x51);
                Wire.write(0x07);
                Wire.write(0x01);
                Wire.endTransmission();
                serverclient.print("gyroscope calibration is started...");
                delay(5000);
                serverclient.print("Done!!!\n");
                serverclient.print("You can leave by pressing X");
              }
              break;

          case 'Z':
          case 'z':
              {
                Wire.beginTransmission(0x51);
                Wire.write(0x07);
                Wire.write(0x02);
                serverclient.print("Zero is adjusted!\n");
                Wire.endTransmission();
                serverclient.print("You can leave by pressing X");
              }
              break;
          case 'R':
          case 'r':
              {
                Wire.beginTransmission(0x51);
                Wire.write(0x07);
                Wire.write(0x04);
                serverclient.print("Rotate right 90 degrees\n");
                Wire.endTransmission();
                serverclient.print("You can leave by pressing X");
              }
              break;
          case 'T':
          case 't':
              {
                Wire.beginTransmission(0x51);
                Wire.write(0x07);
                Wire.write(0x08);
                serverclient.print("Results are stored !!!\n");
                Wire.endTransmission();
                serverclient.print("You can leave by pressing X");
              }
              break;
          case 'S':
          case 's':
              {
                Wire.beginTransmission(0x51);
                Wire.write(0x07);
                Wire.write(0x10);
                serverclient.print("Calibration is saved!\n");
                Wire.endTransmission();
                serverclient.print("You can leave by pressing X");
              }
              break;
          case 'X':
          case 'x':
              x=1;
              break;
          default:
              break;
      }
      d='a';  
        delay(10);
        if(serverclient.available()){
          d =serverclient.read();
        }
         
      }while(x==0);
      x=0;
      
    }
    

  }
    liv3f.beginTransmission(DEFAULT_DEVICE_ADDRESS);
    liv3f.write((uint8_t) DEFAULT_DEVICE_PORT);
    liv3f.endTransmission(false);
    liv3f.requestFrom((uint8_t)DEFAULT_DEVICE_ADDRESS, (uint8_t) 32);
      
    while(liv3f.available()){
      c=liv3f.read();
      nmea.process(c);
       if (nmea.isValid()){
        long latitude_mdeg, longitude_mdeg;
        latitude_mdeg = nmea.getLatitude(); 
        longitude_mdeg = nmea.getLongitude();
        // Convert to degrees
        lat = static_cast<float>(latitude_mdeg) / 1000000.0f;
        lon = static_cast<float>(longitude_mdeg) / 1000000.0f;  
        nmea.clear();
        break;
     }
    }
    

    if (currentMillis - previousMillis >= interval){

      previousMillis = currentMillis;  
      Wire.beginTransmission(0x51);
      Wire.write(0x20);
      Wire.endTransmission();
      Wire.requestFrom(0x51, 4);
      delay(100);

      if (Wire.available() >= 4) {
        
        for (int i = 0; i < 4; i++) {
          rawData[i] = Wire.read();
          }
        }    

    float ang = readFloatFromWire(rawData);
        
    dtostrf(lat, 17, 10, latBuffer); 
    dtostrf(lon, 17, 10, lonBuffer); 
    dtostrf(ang, 17, 10, angBuffer); 

    String data = String(latBuffer) + ", " + String(lonBuffer) + ", " + String(angBuffer);
    
    serverclient.println(data);
   
    }
    
  }
  else{
    while(!((serverclient)&&(serverclient.connected()))){
      serverclient = server.available();
    }  
  } 
  
}







