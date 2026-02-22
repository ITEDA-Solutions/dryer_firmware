#include <WiFi.h>
#include <WebServer.h>
#include <DHT.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>

// -------------------- PIN DEFINITIONS --------------------
#define DHTPIN1 4
#define DHTPIN2 5
#define DHTTYPE DHT22
#define RELAY 15
#define currentpin 33

// -------------------- SENSOR SETUP --------------------
DHT dht1(DHTPIN1, DHTTYPE);
DHT dht2(DHTPIN2, DHTTYPE);

// -------------------- CURRENT SENSOR --------------------
float R1 = 6800.0;
float R2 = 12000.0;
float midpoint = 2.20;
float sensitivity = 2.2 / 30.0;

// -------------------- CONTROL SETTINGS --------------------
const float setpoint = 45.0;
const float band = 5.0;
const float voltage = 12.0;

// -------------------- ENERGY TRACKING --------------------
float energy_Wh = 0;
unsigned long lastMillis = 0;

// -------------------- WIFI CREDENTIALS --------------------
const char* ssid = "dono-call";
const char* password = "@ubiquitoU5";

// -------------------- SERVER --------------------
WebServer server(80);

// -------------------- GLOBAL VARIABLES --------------------
float t1, h1, t2, h2;
float current, power, vin;
int relayState;

// ==========================================================
// WEB PAGE DISPLAY
// ==========================================================
void handleRoot() {
  String html = "<html><head><meta http-equiv='refresh' content='2'>";
  html += "<title>ESP32 Energy Monitor</title></head><body>";
  html += "<h2>ESP32 Energy Monitor</h2>";
  html += "<p>DHT1: " + String(t1,2) + "°C " + String(h1,2) + "%</p>";
  html += "<p>DHT2: " + String(t2,2) + "°C " + String(h2,2) + "%</p>";
  html += "<p>Relay: " + String(relayState) + "</p>";
  html += "<p>Current: " + String(current,3) + " A</p>";
  html += "<p>Power: " + String(power,2) + " W</p>";
  html += "<p>Energy: " + String(energy_Wh,3) + " Wh</p>";
  html += "<p>Vin: " + String(vin,3) + " V</p>";
  html += "</body></html>";
  server.send(200, "text/html", html);
}

// ==========================================================
// SEND DATA TO API
// ==========================================================
void sendSensorData() {

  if (WiFi.status() == WL_CONNECTED) {

    WiFiClientSecure client;
    client.setInsecure();  // Skip SSL verification

    HTTPClient https;

    String url = "https://iteda-solutions-dryers-platform.vercel.app/api/sensor-data";

    if (https.begin(client, url)) {

      https.addHeader("Content-Type", "application/json");

      String jsonPayload = "{";
      jsonPayload += "\"dryer_id\":\"REAL_DRYER\",";
      jsonPayload += "\"chamber_temp\":" + String(t1, 2) + ",";
      jsonPayload += "\"ambient_temp\":" + String(t2, 2) + ",";
      jsonPayload += "\"internal_humidity\":" + String(h1, 2) + ",";
      jsonPayload += "\"external_humidity\":" + String(h2, 2);
      jsonPayload += "}";

      int httpResponseCode = https.POST(jsonPayload);

      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);

      if (httpResponseCode > 0) {
        String response = https.getString();
        Serial.println("Server response: " + response);
      } else {
        Serial.println("Error sending POST");
      }

      https.end();
    }
  }
}

// ==========================================================
// SETUP
// ==========================================================
void setup() {

  Serial.begin(115200);

  pinMode(RELAY, OUTPUT);
  digitalWrite(RELAY, HIGH);

  pinMode(currentpin, INPUT);

  dht1.begin();
  dht2.begin();

  lastMillis = millis();

  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi connected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  server.on("/", handleRoot);
  server.begin();
}

// ==========================================================
// LOOP
// ==========================================================
void loop() {

  server.handleClient();

  // Read sensors
  t1 = dht1.readTemperature();
  h1 = dht1.readHumidity();
  t2 = dht2.readTemperature();
  h2 = dht2.readHumidity();

  // Read current sensor
  int adc = analogRead(currentpin);
  float vout = adc * (3.3 / 4095.0);
  vin = vout * ((R1 + R2) / R1);

  current = (midpoint - vin) / sensitivity;
  if (current < 0) current = 0;

  power = voltage * current;

  // Energy calculation
  unsigned long now = millis();
  float dt = (now - lastMillis) / 3600000.0;
  lastMillis = now;
  energy_Wh += power * dt;

  // Relay temperature control
  if (t1 < setpoint - band) {
    digitalWrite(RELAY, HIGH);
    relayState = 1;
  } 
  else if (t1 > setpoint + band) {
    digitalWrite(RELAY, LOW);
    relayState = 0;
  }

  Serial.printf(
    "T1: %.2f°C H1: %.2f%% | T2: %.2f°C H2: %.2f%% | Relay:%d\n",
    t1, h1, t2, h2, relayState
  );

  // Send to API every 2 seconds
  sendSensorData();

  delay(2000);   // ✅ 2 seconds
}
