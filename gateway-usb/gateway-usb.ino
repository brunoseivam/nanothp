#include "RF24.h"

RF24 radio(D1, D2); // CE, CSN
const byte address[6] = "00001";

void setup() {
  Serial.begin(115200);  
  radio.begin();
  radio.setPALevel(RF24_PA_MAX);
  radio.openReadingPipe(0, address);
  //radio.setDataRate(RF24_1MBPS);
  radio.setDataRate(RF24_250KBPS);
  radio.setAutoAck(true);
  radio.startListening();
  Serial.println("");
  Serial.println("Receiver: Ready");
  Serial.println("Address, Battery (cts), Temperature (C), Humidity(%), Pressure(hPa)");
}

const char *device_name = "nanothp";

void loop() {
  if (radio.available()) {
    char buffer[32] = "";
    radio.read(buffer, sizeof(buffer));

    char *p = buffer;
    /*
    Serial.print("[ ");
    for (int i = 0; i < 21; ++i) {
      Serial.print(buffer[i], HEX);
      Serial.print(" ");
    }
    Serial.println("]");
    */
    char type = *(p++);

    char serial[6];    
    for(int i = 0; i < 6; ++i)
      serial[i] = *(p++);

    uint16_t bat = *((uint16_t*)p); p += 2;
    float temp   = *((float*)p);    p += 4;
    float hum    = *((float*)p);    p += 4;
    float pres   = *((float*)p);    p += 4;

    //Serial.print("<");
    //Serial.print(type, HEX);
    //Serial.print(">");
    //Serial.print("S: ");
    for (int i = 0; i < 6; ++i) {
      if(serial[i] < 0x10) {
        Serial.print("0");
      }
      Serial.print(serial[i], HEX);
    }
    Serial.print(","); Serial.print(bat);
    Serial.print(","); Serial.print(temp);
    Serial.print(","); Serial.print(hum);
    Serial.print(","); Serial.print(pres);
    Serial.println("");

  }
}
