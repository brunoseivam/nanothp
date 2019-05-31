// BME280: https://github.com/finitespace/BME280

#define DEVICE_TYPE 0x01
#define DEBUG
//#define RGB_LED
//#define USE_INTR

#include <RF24.h>
#include <BME280I2C.h>
#include <Wire.h>

#include <avr/sleep.h>
#include <avr/wdt.h>
#include <avr/power.h>

#ifdef RGB_LED
  #include <FastLED.h>
  #define RGB_LED_NUM 1
  #define RGB_LED_PIN 6
  #define RGB_MOSFET_PIN 4

  CRGB leds[RGB_LED_NUM];
#endif

#include <sha204_library.h>
#define SHA204_PIN A3

atsha204Class sha204(SHA204_PIN);

// Expected: "1 23 x x x x x x x EE"
uint8_t serialNumber[9] = {0};

const byte ADDR[6] = "00001";

#ifdef USE_INTR
  #define INTR_PIN 2
#endif

#define LED_PIN LED_BUILTIN // pin 13, same as SCK...
#define RF_CE_PIN 7
#define RF_CSN_PIN 8
#define BATT_PIN A0
#define MOSFET_PIN 5

// CE, CSN pins
RF24 radio(RF_CE_PIN, RF_CSN_PIN);
BME280I2C bme;

int duration = 0;

float readBattery ()
{
  /* 1M, 470K divider across battery and using internal ADC ref of 1.1V
   *  Sense point is bypassed with 0.1 uF cap to reduce noise at that point
   *  ((1e6+470e3)/470e3)*1.1 = Vmax = 3.44 Volts
   *  3.44/1023 = Volts per bit = 0.003363075
   */
  // Read twice
  int val = analogRead(BATT_PIN);
  val = analogRead(BATT_PIN);
  return val * 0.003363075;
}

bool interrupt = 0;

#ifdef USE_INTR
  void pin3Interrupt(void)
  {
    /* This will bring us back from sleep. */
    
    /* We detach the interrupt to stop it from 
     * continuously firing while the interrupt pin
     * is low.
     */
    detachInterrupt(digitalPinToInterrupt(INTR_PIN));
    interrupt = 1;
  }
#endif

// Sleeping code from Sketch H, https://www.gammon.com.au/power
ISR (WDT_vect) {
  wdt_disable();
}

void _wdt_sleep(void) {
  ADCSRA = 0;                                     // Disable ADC
  MCUSR = 0;                                      // Clear various reset flags  
  WDTCSR = bit (WDCE) | bit (WDE);                // Allow changes, disable reset
  WDTCSR = bit (WDIE) | bit (WDP3) | bit (WDP0);  // Set WDIE, and 8 seconds delay
  wdt_reset();                                    // Pat the dog

  set_sleep_mode (SLEEP_MODE_PWR_DOWN);  
  noInterrupts();                                 // Timed sequence follows
  sleep_enable();
 
  MCUCR = bit (BODS) | bit (BODSE);               // Turn off brown-out enable in software
  MCUCR = bit (BODS); 
  interrupts();                                   // Guarantees next instruction executed
  sleep_cpu();      
  sleep_disable();                                // Cancel sleep as a precaution
}

void wdt_sleep(int t) {
  while(t > 0) {
    _wdt_sleep();
    t -= 8;
  }
}

void enablePeriph(bool enable) {
  digitalWrite(MOSFET_PIN, enable ? LOW : HIGH);
}

void setup() {
  int start = millis();

#ifdef RGB_LED
  pinMode(RGB_MOSFET_PIN, OUTPUT);
  FastLED.addLeds<WS2812B, RGB_LED_PIN, RGB>(leds, RGB_LED_NUM);
#endif
  
  analogReference(INTERNAL);
  pinMode(MOSFET_PIN, OUTPUT);

#ifdef DEBUG
  Serial.begin(115200);
  Serial.println("Starting up!");
#endif
  
  Wire.begin();
  if (!bme.begin()) {
#ifdef DEBUG
    Serial.println("Failed to Initialize temp sensor");
#endif
  }

  sha204.getSerialNumber(serialNumber);

#ifdef USE_INTR
  pinMode(INTR_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(INTR_PIN), pin3Interrupt, HIGH);
#endif

#ifdef DEBUG
  Serial.print("ID: ");
  for (int i=0; i<9; i++)
  {
    Serial.print(serialNumber[i], HEX);
    Serial.print(" ");
  }
  Serial.println();
  
  int startup_t = millis() - start;
  Serial.print("Startup time: ");
  Serial.println(startup_t);
#endif
}

void loop() {
#ifdef RGB_LED
  digitalWrite(RGB_MOSFET_PIN, HIGH);
  leds[0] = CRGB::Blue;
  FastLED.show();
#endif

  enablePeriph(true);

#ifdef USE_INTR
  if (interrupt) {
#ifdef DEBUG
    Serial.println("====INTERRUPT!!!!====");
    Serial.flush();
#endif
    sleep_disable();
  }
#endif
  
  int start = millis();
  char buffer[32] = "";
  
  float pres = 0.0, temp = 0.0, hum = 0.0;

  bme.read(pres, temp, hum);

  float battery = readBattery();

#ifdef DEBUG
  Serial.println("\nReadings:");
  Serial.print("T: "); Serial.println(temp); Serial.flush();
  Serial.print("H: "); Serial.println(hum); Serial.flush();
  Serial.print("P: "); Serial.println(pres); Serial.flush();
  Serial.print("B: "); Serial.println(battery); Serial.flush();
  Serial.print("t: "); Serial.println(duration); Serial.flush();
#endif

  char *p = buffer;

  *(p++) = DEVICE_TYPE;
  
  for (int i = 2; i < 8; ++i) {
    *(p++) = serialNumber[i];
  }

  *((float*)p) = battery; p += 4;
  *((float*)p) = temp;    p += 4;
  *((float*)p) = hum;     p += 4;
  *((float*)p) = pres;    p += 4;

#ifdef DEBUG
  Serial.print("[ ");
  for (int i = 0; i < 23; ++i) {
    Serial.print((unsigned char)buffer[i], HEX);
    Serial.print(" ");
  }
  Serial.println("]");
#endif

  radio.begin();

  bool ok = false;

  if (radio.isChipConnected()) {
    radio.openWritingPipe(ADDR);
    radio.setPALevel(RF24_PA_MAX);
    radio.setDataRate(RF24_250KBPS);
    radio.stopListening();
    ok = radio.write(&buffer, 23, 1);
    radio.powerDown();
  }

  duration = millis() - start;

#ifdef USE_INTR
  if(interrupt) {
    interrupt = 0;
    attachInterrupt(digitalPinToInterrupt(INTR_PIN), pin3Interrupt, HIGH);
  }
#endif

  enablePeriph(false);
  wdt_sleep(60); // seconds
}
