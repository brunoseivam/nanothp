// Change it in the library header itself if the following is not picked up.
// Library's default is 128.
#define MQTT_MAX_PACKET_SIZE 1024

#include <RF24.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// Connect to the WiFi
const char* ssid = "martins-win7";
const char* password = "872CvMG0";
const char* mqtt_server = "10.42.0.1";

RF24 radio(D1, D2); // CE, CSN
const byte address[6] = "00001";

WiFiClient wifiClient;
PubSubClient client(mqtt_server, 1883, wifiClient);

void setup() {
  Serial.begin(115200);
  radio.begin();
  radio.setPALevel(RF24_PA_MAX);
  radio.openReadingPipe(0, address);
  //radio.setDataRate(RF24_1MBPS);
  radio.setDataRate(RF24_250KBPS);
  radio.startListening();

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  client.connect("Gateway");
  Serial.println("Ready");
}

const char *device_name = "nanothp";

void loop() {
  if (radio.available()) {
    char buffer[32] = "";
    radio.read(buffer, sizeof(buffer));

    char *p = buffer;
    Serial.print("[ ");
    for (int i = 0; i < 21; ++i) {
      Serial.print(buffer[i], HEX);
      Serial.print(" ");
    }
    Serial.println("]");

    char type = *(p++);

    char serial[6];    
    for(int i = 0; i < 6; ++i)
      serial[i] = *(p++);

    uint16_t bat = *((uint16_t*)p); p += 2;
    float temp   = *((float*)p);    p += 4;
    float hum    = *((float*)p);    p += 4;
    float pres   = *((float*)p);    p += 4;

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

    char serial_str[32];
    sprintf(serial_str, "%02X%02X%02X%02X%02X%02X",
      serial[0], serial[1], serial[2],
      serial[3], serial[4], serial[5]);

    // Publish to MQTT
    if (!client.connected()) {
      client.connect("gateway");
    }

    if (client.connected()) {
      char topic[128];
      char value[1024];

      sprintf(topic, "%s/%s/battery", device_name, serial_str);
      sprintf(value, "%d", bat);
      client.publish(topic, value);

      sprintf(topic, "%s/%s/batteryUnit", device_name, serial_str);
      client.publish(topic, "cts");

      sprintf(topic, "%s/%s/temperature", device_name, serial_str);
      sprintf(value, "%.2f", temp);
      client.publish(topic, value);

      sprintf(topic, "%s/%s/temperatureUnit", device_name, serial_str);
      client.publish(topic, "C");

      sprintf(topic, "%s/%s/humidity", device_name, serial_str);
      sprintf(value, "%.2f", hum);
      client.publish(topic, value);

      sprintf(topic, "%s/%s/humidityUnit", device_name, serial_str);
      client.publish(topic, "%");

      sprintf(topic, "%s/%s/pressure", device_name, serial_str);
      sprintf(value, "%.2f", pres);
      client.publish(topic, value);

      sprintf(topic, "%s/%s/pressureUnit", device_name, serial_str);
      client.publish(topic, "hPa");

      const char *location;
      if (!strcmp(serial_str, "44BD0422CF0F"))
        location = "\"location\":\"desk\",";
      else if (!strcmp(serial_str, "AF02F3B3CC79"))
        location = "\"location\":\"shelf\",";
      else if (!strcmp(serial_str, "03F3F9628046"))
        location = "\"location\":\"locker\",";
      else
        location = "";

      sprintf(topic, "%s/%s/all", device_name, serial_str);
      sprintf(value, "{"
        "\"device\": \"%s\","
        "\"id\": \"%s\","
        "%s"
        "\"battery\":%d,"
        "\"temperature\":%.2f,"
        "\"humidity\":%.2f,"
        "\"pressure\":%.2f,"
        "\"batteryUnit\":\"cts\","
        "\"temperatureUnit\":\"C\","
        "\"humidityUnit\":\"%%\","
        "\"pressureUnit\":\"hPa\""
      "}", device_name, serial_str, location, bat, temp, hum, pres);
      client.publish(topic, value);

    } else {
      Serial.println("MQTT disconnected");
    }
  }
}
