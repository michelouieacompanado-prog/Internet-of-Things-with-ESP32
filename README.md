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

---

## 🧪 Testing

The application logic for each node is verified with the PlatformIO **Unity** test framework on the `native` environment, which builds and runs the tests on the host machine. Static analysis is performed with **cppcheck**, which PlatformIO installs automatically. Neither check requires ESP32 hardware.

### Running the Tests

Run the following commands from the directory of the node under test (`gateway`, `light_sensor`, or `switch`):

```bash
cd ch9/<node>
pio test -e native     # Unit tests
pio check -e native    # Static analysis
```

### Results Summary

| Node | Unit Tests | Static Analysis (cppcheck) |
|------|:----------:|:--------------------------:|
| Gateway (`ch9/gateway`) | ✅ Passed (2/2) | ✅ Passed (6 low) |
| Light Sensor (`ch9/light_sensor`) | ✅ Passed | ✅ Passed (5 low) |
| Switch (`ch9/switch`) | ✅ Passed (3/3) | ✅ Passed (5 low) |

No HIGH or MEDIUM severity findings were reported.

**Overall unit test**

<img width="312" height="173" alt="unit-test-overall" src="https://github.com/user-attachments/assets/ad19f0a1-9e46-4927-ad51-b9dee1c5716f" />

### Unit Test Details

#### Switch Node

Test file: `ch9/switch/test/test_switch_state.cpp`

| Test Case | Description | Result |
|-----------|-------------|:------:|
| `test_switch_off` | Verifies that the switch state can be set to OFF | ✅ Passed |
| `test_switch_on` | Verifies that the switch state can be set to ON | ✅ Passed |
| `test_switch_can_turn_off_after_on` | Verifies that the switch returns to OFF after being turned ON | ✅ Passed |

**Switch unit test**

<img width="709" height="380" alt="unit-test-switch" src="https://github.com/user-attachments/assets/f7c32f67-a98c-42b4-a1df-f4da643bbaa0" />

**Summary:** 3 test cases, 3 succeeded, completed in 3.37 s.

#### Gateway Node

Test file: `ch9/gateway/test/test_switch_command.cpp`

| Test Case | Description | Result |
|-----------|-------------|:------:|
| `test_switch_on_returns_one` | Verifies that an ON command returns `1` | ✅ Passed |
| `test_switch_off_returns_zero` | Verifies that an OFF command returns `0` | ✅ Passed |

**Gateway unit test**
<img width="675" height="368" alt="unit-test-gateway" src="https://github.com/user-attachments/assets/0d6b846a-8d09-4954-88f8-393a08c022b7" />


**Summary:** 2 test cases, 2 succeeded, completed in 4.21 s.

### Static Analysis

Tool: cppcheck (v1.21100.230717), executed through `pio check`.

| Node | HIGH | MEDIUM | LOW | Status |
|------|:----:|:------:|:---:|:------:|
| Gateway | 0 | 0 | 6 | ✅ Passed |
| Light Sensor | 0 | 0 | 5 | ✅ Passed |
| Switch | 0 | 0 | 5 | ✅ Passed |

**Light sensor cppcheck**

<img width="676" height="344" alt="static-analysis-light-sensor" src="https://github.com/user-attachments/assets/5703e87a-aa05-4f87-b590-38e1516043f8" />

**Switch cppcheck**
<img width="312" height="173" alt="unit-test-overall" src="https://github.com/user-attachments/assets/40ce77b0-517e-4c35-8457-ecdac7183fbb" />


All LOW findings are of type `unusedFunction` (style). The complete list is shown below.

<details>
<summary>View LOW-severity findings</summary>

| Node | File | Line | Function |
|------|------|-----:|----------|
| Gateway | `src/app_ble.c` | 101 | `appble_set_switch` |
| Gateway | `src/app_ble.c` | 125 | `init_ble` |
| Gateway | `src/app_ip.c` | 54 | `appip_connect_wifi` |
| Gateway | `src/app_web.c` | 48 | `appweb_init` |
| Gateway | `src/app_web.c` | 53 | `appweb_start_server` |
| Gateway | `src/main.c` | 32 | `app_main` |
| Light Sensor | `src/app_ble.c` | 132 | `appble_update_state` |
| Light Sensor | `src/app_ble.c` | 145 | `init_ble` |
| Light Sensor | `src/app_sensor.c` | 73 | `init_hw` |
| Light Sensor | `src/app_sensor.c` | 98 | `is_light_low` |
| Light Sensor | `src/main.c` | 20 | `app_main` |
| Switch | `src/app_ble.c` | 174 | `init_ble` |
| Switch | `src/app_sw.c` | 8 | `init_hw` |
| Switch | `src/app_sw.c` | 23 | `switch_set` |
| Switch | `src/app_sw.c` | 30 | `switch_get` |
| Switch | `src/main.c` | 20 | `app_main` |

</details>

**Note:** The flagged functions are module API functions, functions passed between modules as callbacks, or the `app_main` entry point. cppcheck does not resolve these call sites across source files in this configuration, so they are reported as unused. These are style-level notices and do not indicate functional defects.
