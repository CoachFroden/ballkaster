#include <WiFi.h>
#include <WebServer.h>
#include <ESP32Servo.h>

const char* AP_SSID = "Ballkaster-Test";
const char* AP_PASSWORD = "balltest"; // minst 8 tegn

const int SERVO_PIN = 18;
const int SERVO_MIN_US = 450;
const int SERVO_MAX_US = 2500;

Servo pitchServo;
WebServer server(80);

float currentAngle = 90.0;
float targetAngle = 90.0;
int speedPercent = 35;
bool stopped = false;
unsigned long lastStepMicros = 0;

const char PAGE[] PROGMEM = R"HTML(
<!doctype html><html lang="no"><head>
<meta charset="UTF-8">\n<meta name="viewport" content="width=device-width,initial-scale=1,viewport-fit=cover">
<meta name="theme-color" content="#111827">
<title>Ballkaster – Servo test</title>
<style>
*{box-sizing:border-box}body{margin:0;background:#0b1020;color:#f8fafc;font-family:system-ui,-apple-system,Segoe UI,sans-serif}
main{max-width:620px;margin:auto;padding:22px 18px 40px}.top{display:flex;justify-content:space-between;align-items:center;margin-bottom:24px}
h1{font-size:25px;margin:0}.badge{font-size:13px;background:#16351f;color:#86efac;padding:7px 10px;border-radius:99px}
.card{background:#151c2f;border:1px solid #27324b;border-radius:22px;padding:20px;margin:14px 0;box-shadow:0 12px 35px #0004}
.label{display:flex;justify-content:space-between;align-items:end;margin-bottom:12px;color:#cbd5e1}
.value{font-size:42px;font-weight:800;color:white}.unit{font-size:18px;color:#94a3b8}
input[type=range]{width:100%;accent-color:#38bdf8;height:34px}.ticks{display:flex;justify-content:space-between;color:#64748b;font-size:12px}
.quick{display:grid;grid-template-columns:repeat(3,1fr);gap:10px;margin-top:14px}
button{border:0;border-radius:14px;padding:15px 10px;font-size:16px;font-weight:700;background:#26334d;color:white}
button:active{transform:scale(.98)}.primary{background:#0284c7}.stop{width:100%;background:#b91c1c;font-size:18px;margin-top:4px}
.note{color:#94a3b8;font-size:13px;line-height:1.45;margin-top:16px}
.track{height:8px;background:#26334d;border-radius:9px;overflow:hidden;margin-top:12px}.track>div{height:100%;background:#38bdf8;width:50%;transition:width .15s}
</style></head><body><main>
<div class="top"><h1>⚽ Ballkaster</h1><span class="badge">SERVO TEST</span></div>
<div class="card">
 <div class="label"><span>Målvinkel</span><span><b class="value" id="angleVal">90</b><span class="unit">&deg;</span></span></div>
 <input id="angle" type="range" min="0" max="180" value="90" step="1">
 <div class="ticks"><span>0&deg;</span><span>90&deg;</span><span>180&deg;</span></div>
 <div class="quick"><button onclick="setAngle(0)">0&deg;</button><button class="primary" onclick="setAngle(90)">90&deg;</button><button onclick="setAngle(180)">180&deg;</button></div>
</div>
<div class="card">
 <div class="label"><span>Bevegelseshastighet</span><span><b class="value" id="speedVal">35</b><span class="unit">%</span></span></div>
 <input id="speed" type="range" min="1" max="100" value="35" step="1">
 <div class="ticks"><span>Sakte</span><span></span><span>Rask</span></div>
</div>
<div class="card">
 <div class="label"><span>Beregnet servoposisjon</span><span id="actual">90&deg;</span></div>
 <div class="track"><div id="bar"></div></div>
 <p class="note">Posisjonen er beregnet fra kommandoene til servoen; SG90 har ikke posisjonsfeedback til ESP32.</p>
</div>
<button class="stop" onclick="stopServo()">STOPP BEVEGELSE</button>
<p class="note">ESP32 D18 → servo signal. Servo drives fra separat 5 V, med felles GND til ESP32.</p>
<script>
const a=document.getElementById('angle'),s=document.getElementById('speed');
const av=document.getElementById('angleVal'),sv=document.getElementById('speedVal');
let timer;
function send(){fetch('/set?angle='+a.value+'&speed='+s.value).catch(()=>{});}
function delayedSend(){clearTimeout(timer);timer=setTimeout(send,80)}
a.oninput=()=>{av.textContent=a.value};
a.onchange=()=>{send()};
s.oninput=()=>{sv.textContent=s.value;delayedSend()};
function setAngle(v){a.value=v;av.textContent=v;send()}
function stopServo(){fetch('/stop').catch(()=>{})}
setInterval(()=>fetch('/status').then(r=>r.json()).then(x=>{
 document.getElementById('actual').textContent=x.current.toFixed(0)+'&deg;';
 document.getElementById('bar').style.width=(x.current/180*100)+'%';
}).catch(()=>{}),300);
</script></main></body></html>
)HTML";

void handleRoot(){ server.send_P(200, "text/html; charset=utf-8", PAGE); }

void handleSet(){
  if(server.hasArg("angle")) targetAngle = constrain(server.arg("angle").toFloat(), 0.0, 180.0);
  if(server.hasArg("speed")) speedPercent = constrain(server.arg("speed").toInt(), 1, 100);
  stopped = false;
  server.send(200, "text/plain", "OK");
}

void handleStop(){
  stopped = true;
  targetAngle = currentAngle;
  server.send(200, "text/plain", "STOPPED");
}

void handleStatus(){
  String json = "{\"current\":" + String(currentAngle,1) + ",\"target\":" + String(targetAngle,1) + ",\"speed\":" + String(speedPercent) + "}";
  server.send(200, "application/json", json);
}

void updateServo(){
  if(stopped || abs(targetAngle-currentAngle)<0.5) return;
  // 1–100 % tilsvarer omtrent 9–180 grader/sekund.
  float degreesPerSecond = 5.0 + speedPercent * 5.95;
  unsigned long intervalUs = (unsigned long)(1000000.0 / degreesPerSecond);
  unsigned long now = micros();
  if(now-lastStepMicros < intervalUs) return;
  lastStepMicros = now;
  currentAngle += (targetAngle > currentAngle) ? 1.0 : -1.0;
  currentAngle = constrain(currentAngle,0.0,180.0);
  pitchServo.write((int)round(currentAngle));
}

void setup(){
  Serial.begin(115200);
  pitchServo.setPeriodHertz(50);
  pitchServo.attach(SERVO_PIN, SERVO_MIN_US, SERVO_MAX_US);
  pitchServo.write((int)currentAngle);

  WiFi.mode(WIFI_AP);
  WiFi.softAP(AP_SSID, AP_PASSWORD);
  Serial.print("Åpne: http://");
  Serial.println(WiFi.softAPIP());

  server.on("/", handleRoot);
  server.on("/set", handleSet);
  server.on("/stop", handleStop);
  server.on("/status", handleStatus);
  server.begin();
}

void loop(){
  server.handleClient();
  updateServo();
}
