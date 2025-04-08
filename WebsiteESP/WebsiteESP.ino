#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

const char* ssid = "";
const char* password = "";

ESP8266WebServer server(80);

float Kp = 0, Ki = 0, Kd = 0;

void handleRoot() {
  server.send(200, "text/html",
    "<!DOCTYPE html>"
    "<html lang='ru'>"
    "<head>"
    "<meta charset='UTF-8'>"
    "<meta name='viewport' content='width=device-width, initial-scale=1.0'>"
    "<title>Controller</title>"
    "<style>"
    "body {font-family: Arial, sans-serif; background-color: #121212; color: #ffffff; text-align: center;"
    "display: flex; justify-content: center; align-items: center; height: 100vh; margin: 0;}"
    ".container {background: #1e1e1e; padding: 20px; border-radius: 8px; width: 270px; box-sizing: border-box;}"
    "input, button {width: calc(100% - 10px); padding: 10px; margin: 5px 5px; border: none; border-radius: 5px; font-size: 16px; box-sizing: border-box;}"
    "input {background: #333; color: #ffffff; text-align: center;}"
    "button {background: #00cc99; color: #ffffff; cursor: pointer;}"
    "button:hover {background: #00997a;}"
    "</style>"
    "</head>"
    "<body>"
    "<div class='container'>"
    "<h2>Настройки PID</h2>"
    "<input type='number' id='kp' step='any' placeholder='Kp'>"
    "<input type='number' id='ki' step='any' placeholder='Ki'>"
    "<input type='number' id='kd' step='any' placeholder='Kd'>"
    "<button onclick='sendPID()'>Отправить PID</button>"
    "<button onclick='stopCar()'>Стоп</button>"
    "</div>"
    "<script>"
    "function sendPID() {"
    "  const kp = document.getElementById('kp').value;"
    "  const ki = document.getElementById('ki').value;"
    "  const kd = document.getElementById('kd').value;"
    "  fetch(`/setPID?kp=${kp}&ki=${ki}&kd=${kd}`);"
    "}"
    "function stopCar() {"
    "  fetch('/stop');"
    "}"
    "</script>"
    "</body>"
    "</html>");
}

void handleSetPID() {
  if (server.hasArg("kp") && server.hasArg("ki") && server.hasArg("kd")) {
    Kp = server.arg("kp").toFloat();
    Ki = server.arg("ki").toFloat();
    Kd = server.arg("kd").toFloat();

    // ФОРМАТ: "PID 0.06 0.01 0.2"
    Serial.print("PID ");
    Serial.print(Kp, 3); Serial.print(" ");
    Serial.print(Ki, 3); Serial.print(" ");
    Serial.print(Kd, 3); Serial.print("\n");

    server.send(200, "text/plain", "Параметры отправлены");
  }
  else {
    server.send(400, "text/plain", "Ошибка: отсутствуют параметры!");
  }
}

void handleStop() {
  Serial.println("STOP");
  server.send(200, "text/plain", "Стоп отправлен");
}

void setup() {
  Serial.begin(9600);  // <-- Скорость для связи с Arduino (SoftwareSerial на Arduino тоже 9600)
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWi-Fi подключен!");
  Serial.println(WiFi.localIP());

  server.on("/", handleRoot);
  server.on("/setPID", handleSetPID);
  server.on("/stop", handleStop);
  server.begin();
}

void loop() {
  server.handleClient();
}
