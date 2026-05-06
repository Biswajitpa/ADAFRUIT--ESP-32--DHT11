/***************************************************
   ESP32 + DHT11 + Adafruit IO Cloud (MQTT)
 ***************************************************/

#include <WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include "DHT.h"

// ----------- WiFi Settings ------------
#define WIFI_SSID       "BABU"
#define WIFI_PASS       "12345678"

// ----------- Adafruit IO Settings ----
#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883
#define AIO_USERNAME    "satyabrata"
#define AIO_KEY         "aio_Cbey13ys8xxOmkYUkUzPFimHCVfN"

// ----------- DHT11 Setup -------------
#define DHTPIN  4       // GPIO pin where DHT11 data pin is connected
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// ----------- WiFi Client -------------
WiFiClient client;

// ----------- MQTT Client -------------
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

// ----------- Feeds (match Adafruit IO feed keys) -------------------
Adafruit_MQTT_Publish temperatureFeed = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/Temprature1");
Adafruit_MQTT_Publish humidityFeed    = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/Humudity1");

// ----------- Functions ---------------
void MQTT_connect();

void setup() {
  Serial.begin(115200);
  delay(10);

  // Connect to WiFi
  Serial.println(); Serial.print("Connecting to WiFi: "); Serial.println(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected!");

  // Initialize DHT11
  dht.begin();
}

void loop() {
  // Connect/reconnect to MQTT
  MQTT_connect();

  // Read data from DHT11
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  // Check if read failed
  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    delay(2000);
    return;
  }

  Serial.print("Temperature: ");
  Serial.print(t);
  Serial.print(" °C  | Humidity: ");
  Serial.print(h);
  Serial.println(" %");

  // Publish to Adafruit IO
  if (!temperatureFeed.publish(t)) {
    Serial.println("Failed to publish temperature");
  } else {
    Serial.println("Temperature published!");
  }

  if (!humidityFeed.publish(h)) {
    Serial.println("Failed to publish humidity");
  } else {
    Serial.println("Humidity published!");
  }

  delay(5000); // 5 sec delay before next reading
}

// ----------- MQTT Connection ---------
void MQTT_connect() {
  int8_t ret;

  if (mqtt.connected()) {
    return;
  }

  Serial.print("Connecting to MQTT... ");

  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) {
    Serial.println(mqtt.connectErrorString(ret));
    Serial.println("Retrying MQTT connection in 5 seconds...");
    mqtt.disconnect();
    delay(5000);
    retries--;
    if (retries == 0) {
      ESP.restart();
    }
  }
  Serial.println("MQTT Connected!");
}
