#include <RF24.h>

RF24 radio(D1, D2); // CE, CSN
const byte address[6] = "00001";

void setup() {
  Serial.begin(115200);
  radio.begin();
  radio.setPALevel(RF24_PA_MAX);
  radio.openReadingPipe(0, address);
  //radio.setDataRate(RF24_1MBPS);
  radio.setDataRate(RF24_250KBPS);
  radio.startListening();
  Serial.println("Ready");
}

void loop() {
  if (radio.available()) {
    char buffer[32] = "";
    radio.read(buffer, sizeof(buffer));

    char *p = buffer;
    Serial.print("[ ");
    for (int i = 0; i < 23; ++i) {
      Serial.print(buffer[i], HEX);
      Serial.print(" ");
    }
    Serial.println("]");
    
    char type = *(p++);
    
    char serial[6];    
    for(int i = 0; i < 6; ++i)
      serial[i] = *(p++);

    float bat  = *((float*)p); p += 4;
    float temp = *((float*)p); p += 4;
    float hum  = *((float*)p); p += 4;
    float pres = *((float*)p); p += 4;

    Serial.print("<");
    Serial.print(type, HEX);
    Serial.print(">");
    Serial.print("S: ");
    for (int i = 0; i < 6; ++i) {
      if(serial[i] < 0x10) {
        Serial.print("0");
      }
      Serial.print(serial[i], HEX);
    }
    Serial.print(" B: "); Serial.print(bat);
    Serial.print(" T: "); Serial.print(temp);
    Serial.print(" H: "); Serial.print(hum);
    Serial.print(" P: "); Serial.print(pres);

    Serial.println("");
  }
}
