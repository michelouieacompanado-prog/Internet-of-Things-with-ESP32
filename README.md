# Internet of Things with ESP32 - Chapter 9 (BLE Mesh Smart Home)

Complete PlatformIO & ESP-IDF implementation for a BLE Mesh smart home network featuring three distinct nodes: Light Sensor, Switch, and Gateway.

---

## 🏗 System Architecture

```text
               +----------------------+
               |   Browser / Client   |
               +----------+-----------+
                          | HTTP (Wi-Fi)
                          v
               +----------------------+
               |     Gateway Node     |
               |  (Gen OnOff Client)  |
               +----------+-----------+
                          |
              BLE Mesh    |  Generic OnOff Set
                          v
+--------------------+         +--------------------+
| Light Sensor Node  |         |    Switch Node     |
| (Gen OnOff Server) +-------->| (Gen OnOff Server  |
+--------------------+ Publish |  & OnOff Client)   |
        TSL2561        Status  +---------+----------+
       (<30 lux)                         |
                                         v
                                    Relay Module
```

---

## 📁 Repository Structure

* **`common/`**:
  * **`ble/`** (`app_blecommon.c/h`): Device UUID generation, Bluetooth controller initialization, provisioning, and health attention event dispatchers.
  * **`led/`** (`app_ledattn.c/h`): GPIO19 indicator for visual attention during BLE Mesh provisioning.
  * **`esp-idf-lib/components/`**: Standard `i2cdev` bus driver and `tsl2561` ambient light sensor library.
* **`ch9/light_sensor/`**:
  * Reads ambient light from the TSL2561 sensor every 1 second over I2C (`SDA: GPIO21`, `SCL: GPIO22`).
  * Triggers OnOff state changes when illuminance crosses the `30 lux` threshold.
  * Publishes status updates to all mesh nodes.
* **`ch9/switch/`**:
  * Controls the relay on `GPIO4`.
  * Receives commands from the Gateway (`Generic OnOff Server`) and sensor updates (`Generic OnOff Client`) to toggle the relay.
* **`ch9/gateway/`**:
  * Connects to local Wi-Fi station.
  * Serves an HTTP web UI allowing users to submit `ON`/`OFF` commands.
  * Translates web requests into BLE Mesh `Generic OnOff Set` packets directed to the switch.

---

## 🛠 Prerequisites

* [PlatformIO Core or VS Code PlatformIO IDE](https://platformio.org/)
* ESP-IDF framework
* ESP32 Development Boards (x3)
* TSL2561 Sensor & Relay Module
* [nRF Mesh Mobile App](https://www.nordicsemi.com/Products/Development-tools/nRF-Mesh) for iOS / Android

---

## 🚀 Getting Started

### 1. Light Sensor Node
```bash
cd ch9/light_sensor
pio run -t upload
```

### 2. Switch Node
```bash
cd ch9/switch
pio run -t upload
```

### 3. Gateway Node
```bash
cd ch9/gateway
# Set environment variables for your Wi-Fi credentials
export WIFI_SSID="your_wifi_ssid"
export WIFI_PASS="your_wifi_password"
pio run -t upload
```
