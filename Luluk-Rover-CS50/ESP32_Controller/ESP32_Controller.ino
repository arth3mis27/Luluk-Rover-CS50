#include <WiFi.h>
#include <WebServer.h>
#include <ESP32Servo.h>

const char* ssid = "Tasindisukur2.4";
const char* password = "ALP2004eren";

WebServer server(80);

// Motor Pinleri
#define SAG_ON_IN1 32
#define SAG_ON_IN2 33
#define SAG_ARKA_IN3 25
#define SAG_ARKA_IN4 26

#define SOL_ON_IN1 27
#define SOL_ON_IN2 14
#define SOL_ARKA_IN3 13
#define SOL_ARKA_IN4 4

// Servo ve Röle Pinleri
#define SERVO1_PIN 16
#define SERVO2_PIN 17
#define SERVO3_PIN 18
#define SERVO4_PIN 19
#define RELAY_PIN 21

Servo servo1, servo2, servo3, servo4;
bool relayState = false;

// Global Hız Değişkeni (0-255 arası PWM)
int robotSpeed = 255; 

// Makro Durum Tanımlamaları
enum MacroType { MACRO_NONE, MACRO_ANGRY, MACRO_ACCEPT, MACRO_REJECT, MACRO_SCAN, MACRO_PICKUP, MACRO_NORMAL };
MacroType currentMacro = MACRO_NONE;
unsigned long lastMacroUpdate = 0;
int macroDir = 1; 
int currentMacroAngle = 90;

// Yavaş Animasyonlar İçin Değişkenler
int pickupState = 0;
unsigned long pickupTimer = 0;

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta name="viewport" content="width=device-width, initial-scale=1, maximum-scale=1, user-scalable=no">
<meta charset="utf-8">
<title>Mecanum FPV Robot</title>
<style>
body{ background:#0f172a; color:white; text-align:center; font-family:system-ui, Arial; margin:0; padding:15px; user-select:none; -webkit-user-select:none; }
h1{ margin:0 0 15px 0; color:#38bdf8; font-size: 24px; }

.main-container { display: flex; flex-wrap: wrap; justify-content: center; gap: 20px; align-items: flex-start; max-width: 1200px; margin: 0 auto; }
.panel { background: #1e293b; padding: 15px; border-radius: 12px; border: 1px solid #334155; box-shadow: 0 4px 6px -1px rgb(0 0 0 / 0.1); width: 100%; max-width: 340px; }

.cam-setup { margin-bottom: 10px; display: flex; gap: 5px; }
.cam-setup input { flex: 1; padding: 8px; border-radius: 4px; border: 1px solid #38bdf8; background: #334155; color: white; }
.cam-setup button { padding: 8px 12px; border: none; border-radius: 4px; background: #0ea5e9; color: white; font-weight: bold; cursor: pointer; }
.cam-box { height: 400px; width: 300px; background: #000; border: 2px solid #38bdf8; border-radius: 8px; overflow: hidden; position: relative; display: none; margin: 0 auto; }
.cam-box img { width: 400px; height: auto; display: block; transform: rotate(-90deg); position: absolute; left: -50px; top: 50px; }

.grid { display: grid; grid-template-columns: repeat(3, 80px); gap: 10px; justify-content: center; margin-bottom: 10px; }
.grid-rot { display: grid; grid-template-columns: repeat(2, 125px); gap: 10px; justify-content: center; margin-bottom: 15px; }
.btn { width: 100%; height: 80px; border: none; border-radius: 12px; background: #334155; color: white; font-size: 32px; font-weight: bold; touch-action: manipulation; transition: 0.1s; cursor: pointer; }
.btn:active { background: #0ea5e9; transform: scale(0.95); }
.stop { background: #ef4444; }
.stop:active { background: #b91c1c; }

.slider-group { margin-bottom: 15px; text-align: left; }
.slider-group label { display: flex; justify-content: space-between; font-weight: 600; color: #94a3b8; font-size: 14px; }
input[type=range] { width: 100%; height: 8px; border-radius: 5px; background: #0f172a; outline: none; margin-top: 8px; accent-color: #38bdf8; }

.relayBtn { width: 100%; height: 50px; border: none; border-radius: 8px; background: #059669; color: white; font-size: 18px; font-weight: bold; cursor: pointer; transition: 0.2s; margin-top: 15px; }
.relayBtn.off { background: #4b5563; }
.relayBtn:active { transform: scale(0.95); }

.speed-box { background: #0f172a; padding: 10px; border-radius: 8px; border: 1px solid #0ea5e9; }
.speed-box input[type=range] { accent-color: #ef4444; }

.macro-grid { display: grid; grid-template-columns: repeat(3, 1fr); gap: 8px; margin: 15px 0; }
.btn-macro { padding: 12px 5px; border: none; border-radius: 8px; font-weight: bold; color: white; cursor: pointer; font-size: 12px; transition: 0.1s; touch-action: manipulation; }
.btn-macro:active { transform: scale(0.95); }
.angry { background: #ef4444; }
.angry:active { background: #b91c1c; }
.accept { background: #10b981; }
.accept:active { background: #047857; }
.reject { background: #6366f1; }
.reject:active { background: #4338ca; }
.normal { background: #64748b; }
.normal:active { background: #475569; }
.pickup { background: #f59e0b; }
.pickup:active { background: #d97706; }
.scan { background: #06b6d4; }
.scan:active { background: #0891b2; }
</style>
</head>
<body>
<h1>Mecanum FPV Robot Kontrol</h1>

<div class="main-container">
    
    <div class="panel">
        <div class="cam-setup" id="camSetupDiv">
            <input type="text" id="camIpInput" placeholder="Kamera IP (Örn: 192.168.x.x)">
            <button onclick="loadCamera()">Aç</button>
        </div>
        <div class="cam-box" id="camBox"></div>
    </div>

    <div class="panel">
        <div class="grid">
            <button class="btn" ontouchstart="go('TL')" ontouchend="go('S')" onmousedown="go('TL')" onmouseup="go('S')">↖️</button>
            <button class="btn" ontouchstart="go('F')" ontouchend="go('S')" onmousedown="go('F')" onmouseup="go('S')">▲</button>
            <button class="btn" ontouchstart="go('TR')" ontouchend="go('S')" onmousedown="go('TR')" onmouseup="go('S')">↗️</button>
            
            <button class="btn" ontouchstart="go('L')" ontouchend="go('S')" onmousedown="go('L')" onmouseup="go('S')">◀️</button>
            <button class="btn stop" onclick="go('S')">■</button>
            <button class="btn" ontouchstart="go('R')" ontouchend="go('S')" onmousedown="go('R')" onmouseup="go('S')">▶️</button>
            
            <button class="btn" ontouchstart="go('BL')" ontouchend="go('S')" onmousedown="go('BL')" onmouseup="go('S')">↙️</button>
            <button class="btn" ontouchstart="go('B')" ontouchend="go('S')" onmousedown="go('B')" onmouseup="go('S')">▼</button>
            <button class="btn" ontouchstart="go('BR')" ontouchend="go('S')" onmousedown="go('BR')" onmouseup="go('S')">↘️</button>
        </div>
        <div class="grid-rot">
            <button class="btn" ontouchstart="go('RL')" ontouchend="go('S')" onmousedown="go('RL')" onmouseup="go('S')">⟲ Çevir</button>
            <button class="btn" ontouchstart="go('RR')" ontouchend="go('S')" onmousedown="go('RR')" onmouseup="go('S')">Çevir ⟳</button>
        </div>
        
        <div class="speed-box">
            <div class="slider-group" style="margin-bottom:0;">
                <label><span style="color:#ef4444;">Motor Hızı (PWM)</span><span id="spdTxt">100%</span></label>
                <input type="range" min="50" max="255" value="255" oninput="updateSpeed(this.value)">
            </div>
        </div>
    </div>

    <div class="panel">
        <h3 style="margin-top:0; color:#38bdf8;">Robot Kol</h3>
        <div class="slider-group"><label><span>Taban (Servo 1)</span><span id="s1">90°</span></label><input type="range" min="0" max="180" value="90" oninput="updateServo(1,this.value)"></div>
        <div class="slider-group"><label><span>Omuz (Servo 2)</span><span id="s2">90°</span></label><input type="range" min="0" max="180" value="90" oninput="updateServo(2,this.value)"></div>
        <div class="slider-group"><label><span>Dirsek (Servo 3)</span><span id="s3">90°</span></label><input type="range" min="0" max="180" value="90" oninput="updateServo(3,this.value)"></div>
        <div class="slider-group"><label><span>Kıskaç (Servo 4)</span><span id="s4">90°</span></label><input type="range" min="0" max="180" value="90" oninput="updateServo(4,this.value)"></div>
        
        <div class="macro-grid">
            <button class="btn-macro angry" ontouchstart="macro('ANGRY')" ontouchend="macro('STOP')" onmousedown="macro('ANGRY')" onmouseup="macro('STOP')">🤬 Sinirlen</button>
            <button class="btn-macro accept" ontouchstart="macro('ACCEPT')" ontouchend="macro('STOP')" onmousedown="macro('ACCEPT')" onmouseup="macro('STOP')">✅ Kabul Et</button>
            <button class="btn-macro reject" ontouchstart="macro('REJECT')" ontouchend="macro('STOP')" onmousedown="macro('REJECT')" onmouseup="macro('STOP')">❌ Reddet</button>
            <button class="btn-macro scan" ontouchstart="macro('SCAN')" ontouchend="macro('STOP')" onmousedown="macro('SCAN')" onmouseup="macro('STOP')">🔍 Tarama</button>
            
            <button class="btn-macro pickup" onclick="macro('PICKUP')">📦 Eşya Al</button>
            <button class="btn-macro normal" onclick="macro('NORMAL')">🏠 Normal</button>
        </div>

        <button id="relayBtn" class="relayBtn off" onclick="toggleRelay()">RÖLE: KAPALI</button>
    </div>

</div>

<script>
function loadCamera() {
    let ip = document.getElementById("camIpInput").value.trim();
    if(ip === "") return;
    if(!ip.startsWith("http://")) ip = "http://" + ip;
    if(!ip.endsWith("/")) ip = ip + "/";
    document.getElementById("camSetupDiv").style.display = "none";
    let camBox = document.getElementById("camBox");
    camBox.style.display = "block";
    camBox.innerHTML = '<img src="' + ip + '" onerror="alert(\'Kamera bağlanamadı!\'); location.reload();">';
}

let lastCmd = "";
function go(cmd){
    if(cmd === lastCmd && cmd !== 'S') return; 
    lastCmd = cmd;
    fetch('/move?dir=' + cmd).catch(err => console.log(err));
}

function macro(type) {
    fetch('/macro?type=' + type).catch(err => console.log(err));
}

let speedTimer;
function updateSpeed(val){
    let percentage = Math.round((val / 255) * 100);
    document.getElementById('spdTxt').innerText = percentage + "%";
    clearTimeout(speedTimer);
    speedTimer = setTimeout(() => { fetch('/speed?val=' + val); }, 50);
}

let servoTimers = {};
function updateServo(id,val){
    document.getElementById('s'+id).innerText = val + "°";
    clearTimeout(servoTimers[id]);
    servoTimers[id] = setTimeout(() => { fetch('/servo?id=' + id + '&val=' + val); }, 40); 
}

function toggleRelay(){
    fetch('/relay').then(r => r.text()).then(state => {
        let btn = document.getElementById('relayBtn');
        if(state == "1"){ btn.className = "relayBtn"; btn.innerText = "RÖLE: AÇIK"; }
        else{ btn.className = "relayBtn off"; btn.innerText = "RÖLE: KAPALI"; }
    });
}
document.addEventListener('dblclick', function(e) { e.preventDefault(); }, { passive: false });
</script>
</body>
</html>
)rawliteral";

void sendFastResponse(String msg) {
  server.sendHeader("Connection", "close"); 
  server.send(200, "text/plain", msg);
}

void setMotors(int lf, int lr, int rf, int rr) {
    analogWrite(SOL_ON_IN1, lf > 0 ? robotSpeed : 0);
    analogWrite(SOL_ON_IN2, lf < 0 ? robotSpeed : 0);
    
    analogWrite(SOL_ARKA_IN3, lr > 0 ? robotSpeed : 0);
    analogWrite(SOL_ARKA_IN4, lr < 0 ? robotSpeed : 0);
    
    analogWrite(SAG_ON_IN1, rf > 0 ? robotSpeed : 0);
    analogWrite(SAG_ON_IN2, rf < 0 ? robotSpeed : 0);
    
    analogWrite(SAG_ARKA_IN3, rr > 0 ? robotSpeed : 0);
    analogWrite(SAG_ARKA_IN4, rr < 0 ? robotSpeed : 0);
}

void setup() {
    Serial.begin(115200);

    ESP32PWM::allocateTimer(0);
    ESP32PWM::allocateTimer(1);
    ESP32PWM::allocateTimer(2);
    ESP32PWM::allocateTimer(3);

    servo1.setPeriodHertz(50);
    servo2.setPeriodHertz(50);
    servo3.setPeriodHertz(50);
    servo4.setPeriodHertz(50);

    servo1.attach(SERVO1_PIN, 500, 2400); 
    servo2.attach(SERVO2_PIN, 500, 2400);
    servo3.attach(SERVO3_PIN, 500, 2400); 
    servo4.attach(SERVO4_PIN, 500, 2400);

    servo1.write(90); servo2.write(90); servo3.write(90); servo4.write(90);

    pinMode(SAG_ON_IN1, OUTPUT); pinMode(SAG_ON_IN2, OUTPUT);
    pinMode(SAG_ARKA_IN3, OUTPUT); pinMode(SAG_ARKA_IN4, OUTPUT);
    pinMode(SOL_ON_IN1, OUTPUT); pinMode(SOL_ON_IN2, OUTPUT);
    pinMode(SOL_ARKA_IN3, OUTPUT); pinMode(SOL_ARKA_IN4, OUTPUT);
    setMotors(0,0,0,0);

    pinMode(RELAY_PIN, OUTPUT);
    digitalWrite(RELAY_PIN, LOW);
    
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
    
    Serial.println("\n=== MOTOR SISTEMI AKTIF ===");
    Serial.print("ROBOT ARAYUZ IP: "); Serial.println(WiFi.localIP());

    server.on("/", [](){ server.send_P(200, "text/html", index_html); });

    server.on("/move", [](){
        String dir = server.arg("dir");
        if(dir == "F")       setMotors( 1,  1,  1,  1);
        else if(dir == "B")  setMotors(-1, -1, -1, -1);
        else if(dir == "L")  setMotors(-1,  1,  1, -1);
        else if(dir == "R")  setMotors( 1, -1, -1,  1);
        else if(dir == "TL") setMotors( 0,  1,  1,  0);
        else if(dir == "TR") setMotors( 1,  0,  0,  1);
        else if(dir == "BL") setMotors(-1,  0,  0, -1);
        else if(dir == "BR") setMotors( 0, -1, -1,  0);
        else if(dir == "RL") setMotors(-1, -1,  1,  1);
        else if(dir == "RR") setMotors( 1,  1, -1, -1);
        else                 setMotors( 0,  0,  0,  0);
        sendFastResponse("OK");
    });

    server.on("/macro", [](){
        String type = server.arg("type");
        if(type == "ANGRY") {
            currentMacro = MACRO_ANGRY;
            servo1.write(90); servo2.write(90); servo4.write(90);
            currentMacroAngle = 50; macroDir = 1; servo3.write(currentMacroAngle);
        }
        else if(type == "ACCEPT") {
            currentMacro = MACRO_ACCEPT;
            servo2.write(36); servo4.write(90);
            currentMacroAngle = 60; macroDir = 1; servo3.write(currentMacroAngle);
        }
        else if(type == "REJECT") {
            currentMacro = MACRO_REJECT;
            servo2.write(36); servo3.write(110); servo4.write(90);
            currentMacroAngle = 70; macroDir = 1; servo1.write(currentMacroAngle);
        }
        else if(type == "NORMAL") {
            currentMacro = MACRO_NORMAL; // 🔥 YUMUŞAK NORMAL MOD
            lastMacroUpdate = millis();
        }
        else if(type == "PICKUP") {
            currentMacro = MACRO_PICKUP; // 🔥 YUMUŞAK EŞYA ALMA MODU
            pickupState = 1;
            lastMacroUpdate = millis();
        }
        else if(type == "SCAN") {
            currentMacro = MACRO_SCAN;
            servo2.write(180);
            servo3.write(0);
            servo4.write(80);
            currentMacroAngle = 0;
            macroDir = 1;
            servo1.write(currentMacroAngle);
        }
        else {
            currentMacro = MACRO_NONE; 
        }
        sendFastResponse("OK");
    });

    server.on("/speed", [](){
        robotSpeed = constrain(server.arg("val").toInt(), 0, 255);
        sendFastResponse("OK");
    });

    server.on("/servo", [](){
        int id = server.arg("id").toInt();
        int val = constrain(server.arg("val").toInt(), 0, 180);
        if(id == 1) servo1.write(val); else if(id == 2) servo2.write(val);
        else if(id == 3) servo3.write(val); else if(id == 4) servo4.write(val);
        sendFastResponse("OK");
    });

    server.on("/relay", [](){
        relayState = !relayState;
        digitalWrite(RELAY_PIN, relayState ? HIGH : LOW);
        sendFastResponse(String(relayState ? "1" : "0"));
    });

    server.begin();
}

void loop() {
    server.handleClient();
    
    // 🔥 YAVAŞ (SOFT SWEEP) EŞYA ALMA ANİMASYONU
    if (currentMacro == MACRO_PICKUP) {
        if (millis() - lastMacroUpdate >= 20) { // Her 20 ms'de 1 derece hareket eder (Güvenli hız)
            lastMacroUpdate = millis();
            bool reached = true;
            
            int s1 = servo1.read(), s2 = servo2.read(), s3 = servo3.read(), s4 = servo4.read();

            if (pickupState == 1) { // 1. Aşama: Yere eğil
                if(s1 != 90)  { servo1.write(s1 < 90 ? s1 + 1 : s1 - 1); reached = false; }
                if(s2 != 105) { servo2.write(s2 < 105 ? s2 + 1 : s2 - 1); reached = false; }
                if(s3 != 143) { servo3.write(s3 < 143 ? s3 + 1 : s3 - 1); reached = false; }
                if(s4 != 130) { servo4.write(s4 < 130 ? s4 + 1 : s4 - 1); reached = false; }
                
                if (reached) { pickupState = 2; pickupTimer = millis(); }
            }
            else if (pickupState == 2) { // 2. Aşama: 0.5 saniye bekle ve kavra
                if (millis() - pickupTimer > 500) {
                    if(s4 != 80) { servo4.write(s4 > 80 ? s4 - 1 : s4 + 1); reached = false; }
                    if (reached) { pickupState = 3; pickupTimer = millis(); }
                }
            }
            else if (pickupState == 3) { // 3. Aşama: 0.5 saniye bekle ve havaya kaldır
                if (millis() - pickupTimer > 500) {
                    if(s2 != 180) { servo2.write(s2 < 180 ? s2 + 1 : s2 - 1); reached = false; }
                    if(s3 != 0)   { servo3.write(s3 > 0 ? s3 - 1 : s3 + 1); reached = false; }
                    if (reached) { currentMacro = MACRO_NONE; } // Tamamlandı
                }
            }
        }
    }
    // 🔥 YAVAŞ (SOFT SWEEP) NORMALE DÖNME
    else if (currentMacro == MACRO_NORMAL) {
        if (millis() - lastMacroUpdate >= 20) { 
            lastMacroUpdate = millis();
            bool reached = true;
            int s1 = servo1.read(), s2 = servo2.read(), s3 = servo3.read(), s4 = servo4.read();
            
            if(s1 != 90) { servo1.write(s1 < 90 ? s1 + 1 : s1 - 1); reached = false; }
            if(s2 != 90) { servo2.write(s2 < 90 ? s2 + 1 : s2 - 1); reached = false; }
            if(s3 != 90) { servo3.write(s3 < 90 ? s3 + 1 : s3 - 1); reached = false; }
            if(s4 != 90) { servo4.write(s4 < 90 ? s4 + 1 : s4 - 1); reached = false; }
            
            if (reached) { currentMacro = MACRO_NONE; }
        }
    }
    // DİĞER SÜREKLİ DÖNGÜLER (Sinirlen, Kabul Et, vb.)
    else if (currentMacro != MACRO_NONE) {
        unsigned long now = millis();
        unsigned long interval = 5;
        if (currentMacro == MACRO_ACCEPT) interval = 8;
        if (currentMacro == MACRO_SCAN) interval = 25; 
        
        if (now - lastMacroUpdate >= interval) {
            lastMacroUpdate = now;
            
            if (currentMacro == MACRO_ANGRY) {
                currentMacroAngle += macroDir * 12; 
                if (currentMacroAngle >= 180) { currentMacroAngle = 180; macroDir = -1; }
                if (currentMacroAngle <= 50)  { currentMacroAngle = 50;  macroDir = 1; }
                servo3.write(currentMacroAngle);
            }
            else if (currentMacro == MACRO_ACCEPT) {
                currentMacroAngle += macroDir * 5; 
                if (currentMacroAngle >= 110) { currentMacroAngle = 110; macroDir = -1; }
                if (currentMacroAngle <= 60)  { currentMacroAngle = 60;  macroDir = 1; }
                servo3.write(currentMacroAngle);
            }
            else if (currentMacro == MACRO_REJECT) {
                currentMacroAngle += macroDir * 10; 
                if (currentMacroAngle >= 120) { currentMacroAngle = 120; macroDir = -1; } 
                if (currentMacroAngle <= 70)  { currentMacroAngle = 70;  macroDir = 1;  } 
                servo1.write(currentMacroAngle);
            }
            else if (currentMacro == MACRO_SCAN) {
                currentMacroAngle += macroDir * 1; 
                if (currentMacroAngle >= 180) { currentMacroAngle = 180; macroDir = -1; } 
                if (currentMacroAngle <= 0)   { currentMacroAngle = 0;   macroDir = 1;  } 
                servo1.write(currentMacroAngle);
            }
        }
    }
    delay(2);
}