# 🏠 ESP32 Room Automation System

A simple and efficient **ESP32-based home automation system** with a modern web interface.
Control lights and fan, and monitor temperature & humidity in real-time — all through a captive WiFi portal.

---

## 🚀 Features

* 📡 ESP32 runs as a WiFi Access Point (no internet required)
* 🌐 Captive portal (auto-redirect to control page)
* 💡 Control Light and Fan (Relay-based)
* 🌡 Real-time Temperature monitoring (DHT11)
* 💧 Real-time Humidity monitoring
* 📱 Mobile-friendly responsive UI
* ⚡ Smooth UI with live updates (AJAX / Fetch API)

---

## 🧰 Hardware Requirements

* ESP32 Development Board
* DHT11 Temperature & Humidity Sensor
* 2-Channel Relay Module
* Jumper wires
* Power supply

---

## 🔌 Pin Configuration

| Component       | ESP32 Pin |
| --------------- | --------- |
| DHT11 Data      | GPIO 4    |
| Relay 1 (Light) | GPIO 16   |
| Relay 2 (Fan)   | GPIO 17   |

---

## ⚙️ How It Works

1. ESP32 creates a WiFi network:

   * **SSID:** `Home LAN`
   * **Password:** `12345678`

2. When a user connects:

   * Captive portal automatically opens
   * Displays control dashboard

3. Web interface allows:

   * Toggle Light & Fan
   * View temperature & humidity

4. Sensor data updates every 5 seconds.

---

## 📁 Project Structure

```
ESP32-Room-Automation/
│── main.ino        # Main Arduino code
│── README.md       # Project documentation
```

---

## 🧑‍💻 Installation

1. Install Arduino IDE

2. Add ESP32 board support

3. Install required libraries:

   * WiFi
   * WebServer
   * DNSServer
   * DHT sensor library

4. Upload code to ESP32

---

## 📡 Usage

1. Power on ESP32
2. Connect to WiFi:

   ```
   SSID: Home LAN
   Password: 12345678
   ```
3. Open browser
4. You will be redirected to the control panel automatically

---

## ⚡ API Endpoints

| Endpoint          | Description                         |
| ----------------- | ----------------------------------- |
| `/`               | Main UI                             |
| `/data`           | Returns temperature & humidity JSON |
| `/light1?state=1` | Turn ON Light                       |
| `/light1?state=0` | Turn OFF Light                      |
| `/light2?state=1` | Turn ON Fan                         |
| `/light2?state=0` | Turn OFF Fan                        |

---

## 🧠 Technical Details

* Uses **PROGMEM** to store HTML (saves RAM)
* Uses `server.send_P()` for efficient memory usage
* DNS server redirects all requests to ESP32 (captive portal)
* Non-blocking loop with minimal delay for stability

---

## ⚠️ Notes

* Relay module is **active LOW**
* DHT11 may occasionally return invalid values (handled in code)
* Works completely offline

---

## 📜 License

This project is open-source and free to use.

---

## 🙌 Author

**Gavindu2006**
