#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include "DHT.h"

/* ---------------- Hardware Configuration ---------------- */

// DHT11 sensor setup
#define DHTPIN 4
#define DHTTYPE DHT11

// Relay pins (Active LOW)
#define RELAY1 16
#define RELAY2 17

// WiFi Access Point name
const char* apSSID = "Home LAN";

/* ---------------- Object Initialization ---------------- */

DHT dht(DHTPIN, DHTTYPE);
WebServer server(80);

// DNS server for captive portal
const byte DNS_PORT = 53;
DNSServer dnsServer;

// Device states
bool light1State = false;
bool light2State = false;

/* ---------------- Web Page (Stored in Flash) ---------------- */
/*
  PROGMEM is used to store large HTML content in flash memory
  instead of RAM to improve performance and stability.
*/
const char htmlPage[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>Room Automation System</title>
<style>
*{margin:0;padding:0;box-sizing:border-box;font-family:"Segoe UI",Roboto,system-ui;}
body{min-height:100vh;background:#000;display:flex;justify-content:center;align-items:center;color:#fff;}
.container{width:90%;max-width:420px;}
h1{text-align:center;margin-bottom:6px;font-weight:600;}
.datetime{text-align:center;font-size:20px;margin-bottom:30px;font-weight:500;letter-spacing:0.8px;color:#ddd;}
.row{display:flex;gap:15px;margin-bottom:25px;}
.glass{background:rgba(255,255,255,0.08);backdrop-filter:blur(14px);-webkit-backdrop-filter:blur(14px);border-radius:18px;border:1px solid rgba(255,255,255,0.25);box-shadow:0 10px 30px rgba(0,0,0,0.5);transition:0.3s;}
.card{flex:1;padding:22px;text-align:center;cursor:pointer;}
.label{font-size:14px;color:#ccc;}
.card h2{margin-top:10px;font-size:26px;color:#fff;}
.light-card.on{border:1px solid rgba(255,255,255,0.9);box-shadow:0 0 16px rgba(255,255,255,0.7),0 0 40px rgba(255,255,255,0.5);animation:glowPulse 1.6s infinite;}
@keyframes glowPulse{0%{box-shadow:0 0 12px rgba(255,255,255,0.4);}50%{box-shadow:0 0 38px rgba(255,255,255,0.9);}100%{box-shadow:0 0 12px rgba(255,255,255,0.4);}}
@media(max-width:400px){h1{font-size:22px;}.datetime{font-size:17px;}}
</style>
</head>
<body>
<div class="container">
<h1>Room Automation System</h1>
<br>
<p id="datetime" class="datetime">--</p>
<div class="row">
  <div class="glass card">
    <p class="label">🌡 Temperature</p>
    <h2><span id="temp">--</span> °C</h2>
  </div>
  <div class="glass card">
    <p class="label">💧 Humidity</p>
    <h2><span id="hum">--</span> %</h2>
  </div>
</div>
<div class="row">
  <div class="glass card light-card" onclick="toggleLight(1,this)">
    <p class="label">💡 Light</p>
    <h2 class="status-text">OFF</h2>
  </div>
  <div class="glass card light-card" onclick="toggleLight(2,this)">
    <p class="label">🌀 Fan</p>
    <h2 class="status-text">OFF</h2>
  </div>
</div>
</div>
<script>
function updateDateTime(){const now=new Date();const options={weekday:'short',month:'short',day:'numeric',hour:'2-digit',minute:'2-digit'};document.getElementById("datetime").innerText=now.toLocaleDateString('en-US',options);}setInterval(updateDateTime,1000);updateDateTime();
function updateSensorData(){fetch("/data").then(res=>res.json()).then(data=>{document.getElementById("temp").innerText=data.temp;document.getElementById("hum").innerText=data.hum;}).catch(()=>console.log("ESP32 offline"));}setInterval(updateSensorData,5000);updateSensorData();
function toggleLight(id,card){const isOn=card.classList.contains("on");const statusText=card.querySelector(".status-text");fetch(`/light${id}?state=${isOn?0:1}`);card.classList.toggle("on");statusText.innerText=isOn?"OFF":"ON";}
</script>
</body>
</html>
)rawliteral";

/* ---------------- Setup Function ---------------- */

void setup() {
  Serial.begin(115200);

  // Initialize sensor
  dht.begin();

  // Configure relay pins
  pinMode(RELAY1, OUTPUT);
  pinMode(RELAY2, OUTPUT);

  // Set relays OFF (Active LOW logic)
  digitalWrite(RELAY1, HIGH);
  digitalWrite(RELAY2, HIGH);

  /* -------- WiFi Access Point -------- */
  WiFi.mode(WIFI_AP);
  WiFi.softAP(apSSID, "12345678");

  IPAddress IP = WiFi.softAPIP();
  Serial.print("ESP32 AP IP: ");
  Serial.println(IP);

  /* -------- Captive Portal DNS -------- */
  dnsServer.start(DNS_PORT, "*", IP);

  /* -------- Routes -------- */

  // Serve main page
  server.on("/", HTTP_GET, [](){
    server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
    server.send_P(200, "text/html", htmlPage);
  });

  // Sensor data endpoint (JSON)
  server.on("/data", HTTP_GET, [](){
    float t = dht.readTemperature();
    float h = dht.readHumidity();

    // Handle sensor error
    if (isnan(t) || isnan(h)) {
      server.send(500, "application/json", "{\"error\":\"sensor\"}");
      return;
    }

    String json = "{\"temp\":" + String(t) + ",\"hum\":" + String(h) + "}";
    server.send(200, "application/json", json);
  });

  // Light 1 control
  server.on("/light1", HTTP_GET, [](){
    if (!server.hasArg("state")) {
      server.send(400, "text/plain", "Missing state");
      return;
    }

    String state = server.arg("state");
    light1State = state == "1";

    // Active LOW relay control
    digitalWrite(RELAY1, light1State ? LOW : HIGH);

    server.send(200, "text/plain", light1State ? "ON" : "OFF");
  });

  // Light 2 control
  server.on("/light2", HTTP_GET, [](){
    if (!server.hasArg("state")) {
      server.send(400, "text/plain", "Missing state");
      return;
    }

    String state = server.arg("state");
    light2State = state == "1";

    digitalWrite(RELAY2, light2State ? LOW : HIGH);

    server.send(200, "text/plain", light2State ? "ON" : "OFF");
  });

  // Captive portal fallback (redirect all unknown requests)
  server.onNotFound([](){
    server.send_P(200, "text/html", htmlPage);
  });

  // Start web server
  server.begin();
}

/* ---------------- Main Loop ---------------- */

void loop() {
  // Handle DNS requests (for captive portal)
  dnsServer.processNextRequest();

  // Handle HTTP client requests
  server.handleClient();

  // Small delay for stability
  delay(1);
}
