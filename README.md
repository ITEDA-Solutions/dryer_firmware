# ITEDA Dryer Firmware

Firmware for the **ITEDA Solutions Dryer** running on an ESP32.\
Monitors environmental conditions, controls heating, tracks energy usage, and sends sensor data to a remote API.

---

## Features

- Dual DHT22 sensors for chamber & ambient monitoring
- Relay-based temperature control with hysteresis
- Current sensing with power & energy tracking
- Local web dashboard for real-time status
- Secure HTTPS data transmission
- WiFi-enabled ESP32 operation

---

## Hardware Requirements

- ESP32 development board
- 2 × DHT22 temperature & humidity sensors
- Relay module
- Current sensor module
- 12V heating system

---

## Pin Configuration

| Component      | GPIO |
| -------------- | ---- |
| DHT22 #1       | 4    |
| DHT22 #2       | 5    |
| Relay          | 15   |
| Current Sensor | 33   |

---

## Configuration

Update WiFi credentials and API endpoint:

```cpp
const char* ssid = "YOUR_WIFI";
const char* password = "YOUR_PASSWORD";
```

```cpp
String url = "https://your-api-endpoint/api/sensor-data";
```

Adjust control parameters if needed:

```cpp
const float setpoint = 45.0;   // target temperature (°C)
const float band = 5.0;        // hysteresis band
```

---

## Local Monitoring

After connecting to WiFi, open the ESP32 IP address in a browser to view:

- Temperature & humidity
- Relay status
- Current & power usage
- Energy consumption
- Input voltage

---

## Data Transmission

Sensor readings are sent via HTTPS POST to the configured API every 2 seconds.

---

## License

Proprietary — ITEDA Solutions

