
#define BLYNK_TEMPLATE_ID "TMPL432NTVGlL"
#define BLYNK_TEMPLATE_NAME "PATIENT MONITORING"
#define BLYNK_AUTH_TOKEN "kaYAm-58zMKOqO4ex8GZTwutt9ToaG4S"
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
// Wi-Fi credentials
char ssid[] = "Savonia-guest";  // Enter your Wi-Fi SSID
char pass[] = "";      // Enter your Wi-Fi Password
#include "DFRobot_BloodOxygen_S.h"
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "DHTesp.h" 

#ifdef ESP32
#pragma message(THIS EXAMPLE IS FOR ESP8266 ONLY!)
#error Select ESP8266 board.
#endif

DHTesp dht;



#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define I2C_COMMUNICATION  //use I2C for communication, but use the serial port for communication if the line of codes were masked

#ifdef  I2C_COMMUNICATION
#define I2C_ADDRESS    0x57
  DFRobot_BloodOxygen_S_I2C MAX30102(&Wire ,I2C_ADDRESS);
#else
#if defined(ARDUINO_AVR_UNO) || defined(ESP8266)
SoftwareSerial mySerial(4, 5);
DFRobot_BloodOxygen_S_SoftWareUart MAX30102(&mySerial, 9600);
#else
DFRobot_BloodOxygen_S_HardWareUart MAX30102(&Serial1, 9600); 
#endif
#endif

#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< OLED I2C address for 128x64 displays
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

float Low_body_temp = 18.0;
float High_body_temp = 35.0;

int low_bpm = 50;
int high_bpm = 190;

void setup() {
  Serial.begin(115200);

  dht.setup(2, DHTesp::DHT22); 

  // Initialize the MAX30102 sensor
  while (false == MAX30102.begin()) {
    Serial.println("MAX30102 init fail!");
    delay(1000);
  }
  Serial.println("MAX30102 init success!");
  MAX30102.sensorStartCollect();

  // Initialize OLED display
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;); // Stop the program if OLED fails to initialize
  }

  // Display initialization
  display.display();
  delay(2000);
  display.clearDisplay(); // Clear initial Adafruit splash screen
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass, "blynk.cloud", 80);
}
void loop() {
  Blynk.run();
  HRV();
  DHT();
  }

void HRV() {
  // Get the heartbeat and SPO2 readings from the sensor
  MAX30102.getHeartbeatSPO2();
  int spo2 = MAX30102._sHeartbeatSPO2.SPO2;
  int Heartbeat = MAX30102._sHeartbeatSPO2.Heartbeat;
  float temperature = MAX30102.getTemperature_C();



  // Check HeartRate thresholds
  if (Heartbeat < high_bpm || Heartbeat< 0 ) {
    Serial.println("Checking");
    Blynk.logEvent("high_bpm"); 
  } else if (Heartbeat > low_bpm) {
    Serial.print("High Heart Rate");
    Blynk.logEvent("low_bpm"); 
    }

  // Check temperature thresholds
  if (temperature < Low_body_temp ) {
    Serial.print("High Body Temperature");
    
  } else if (temperature > High_body_temp) {
    Serial.print("Low Body Temperature");
    Blynk.logEvent("low_temperature");
    
  }
  // Print values to Serial Monitor
  Serial.print("SPO2: ");
  if (spo2 < 0){
    Serial.println("Checking");
  }else{
  Serial.print(spo2);
  Serial.println("%");
  Blynk.virtualWrite(V0, spo2);
  }

  Serial.print("Heart rate: ");
  if (Heartbeat< 0){
    Serial.println("Checking");
  }else{
  Serial.print(Heartbeat);
  Serial.println("Times/min");
  Blynk.virtualWrite(V1, Heartbeat);
  }
  
  Serial.print("Body Temperature: ");
  Serial.print(temperature);
  Serial.println(" ℃");
  Blynk.virtualWrite(V2, temperature);

  // Update OLED display
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  // Display Heart Rate
  display.setCursor(0, 0); 
  display.setTextSize(2);
  display.print("HR:");
  if (Heartbeat < 0){
    display.print("wait");
  }else{
  display.print(Heartbeat);
  display.println("T/mn");}

  // Display SPO2 level
  display.setCursor(0, 24);
  display.setTextSize(2);
  display.print("SPO2:");
  if (spo2 < 0){
    display.print("wait");
  }else{
  display.print(spo2);
  display.println("%");}

  // Display board temperature
  display.setCursor(0, 48);
  display.setTextSize(1);
  display.print("Temp: ");
  display.print(temperature);
  display.println(" C");

  // Refresh the display to show the updated values
  display.display();

  // Sensor updates every 4 seconds
  delay(4000);
}

void DHT()
{
  delay(dht.getMinimumSamplingPeriod());

  float humidity = dht.getHumidity();
  float temperature = dht.getTemperature();
  Blynk.virtualWrite(V3, humidity);
  Blynk.virtualWrite(V4, temperature);
 

  Serial.print(dht.getStatusString());
  Serial.print("\t");
  Serial.print(humidity, 1);
  Serial.print("\t\t");
  Serial.print(temperature, 1);
  Serial.print("\t\t");
  delay(2000);
}

