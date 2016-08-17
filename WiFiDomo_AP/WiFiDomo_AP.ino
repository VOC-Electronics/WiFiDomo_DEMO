#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>

const char *ssid = "WiFiDomo-Demo";
const char *password = "123456789";
const int REDPIN = 4;
const int GREENPIN = 5;
const int BLUEPIN = 12;
const byte DNS_PORT = 53;
IPAddress apIP(192, 168, 1, 1);
DNSServer dnsServer;
ESP8266WebServer webServer(80);

String webpage = "<!DOCTYPE html><html><head><title>RGB control</title><meta name='mobile-web-app-capable' content='yes' />"
                 "<meta name='viewport' content='width=device-width' /></head><body style='margin: 0px; padding: 0px;'>"
                 "<canvas id='colorspace'></canvas></body>"
                 "<script type='text/javascript'>"
                 "(function () {"
                 " var canvas = document.getElementById('colorspace');"
                 " var ctx = canvas.getContext('2d');"
                 " function drawCanvas() {"
                 " var colours = ctx.createLinearGradient(0, 0, window.innerWidth, 0);"
                 " for(var i=0; i <= 360; i+=10) {"
                 " colours.addColorStop(i/360, 'hsl(' + i + ', 100%, 50%)');"
                 " }"
                 " ctx.fillStyle = colours;"
                 " ctx.fillRect(0, 0, window.innerWidth, window.innerHeight);"
                 " var luminance = ctx.createLinearGradient(0, 0, 0, ctx.canvas.height);"
                 " luminance.addColorStop(0, '#ffffff');"
                 " luminance.addColorStop(0.10, '#ffffff');"
                 " luminance.addColorStop(0.5, 'rgba(0,0,0,0)');"
                 " luminance.addColorStop(0.90, '#000000');"
                 " luminance.addColorStop(1, '#000000');"
                 " ctx.fillStyle = luminance;"
                 " ctx.fillRect(0, 0, ctx.canvas.width, ctx.canvas.height);"
                 " }"
                 " var eventLocked = false;"
                 " function handleEvent(clientX, clientY) {"
                 " if(eventLocked) {"
                 " return;"
                 " }"
                 " function colourCorrect(v) {"
                 " return Math.round(1023-(v*v)/64);"
                 " }"
                 " var data = ctx.getImageData(clientX, clientY, 1, 1).data;"
                 " var params = ["
                 " 'r=' + colourCorrect(data[0]),"
                 " 'g=' + colourCorrect(data[1]),"
                 " 'b=' + colourCorrect(data[2])"
                 " ].join('&');"
                 " var req = new XMLHttpRequest();"
                 " req.open('POST', '?' + params, true);"
                 " req.send();"
                 " eventLocked = true;"
                 " req.onreadystatechange = function() {"
                 " if(req.readyState == 4) {"
                 " eventLocked = false;"
                 " }"
                 " }"
                 " }"
                 " canvas.addEventListener('click', function(event) {"
                 " handleEvent(event.clientX, event.clientY, true);"
                 " }, false);"
                 " canvas.addEventListener('touchmove', function(event){"
                 " handleEvent(event.touches[0].clientX, event.touches[0].clientY);"
                 "}, false);"
                 " function resizeCanvas() {"
                 " canvas.width = window.innerWidth;"
                 " canvas.height = window.innerHeight;"
                 " drawCanvas();"
                 " }"
                 " window.addEventListener('resize', resizeCanvas, false);"
                 " resizeCanvas();"
                 " drawCanvas();"
                 " document.ontouchmove = function(e) {e.preventDefault()};"
                 " })();"
                 "</script></html>";

void handleRoot() {
  // Serial.println("handle root..");
  String red = webServer.arg(0); // read RGB arguments
  String green = webServer.arg(1);
  String blue = webServer.arg(2);

  if ((red != "") && (green != "") && (blue != "")) {
    analogWrite(REDPIN, 1023 - red.toInt());
    analogWrite(GREENPIN, 1023 - green.toInt());
    analogWrite(BLUEPIN, 1023 - blue.toInt());
  }
  webServer.send(200, "text/html", webpage);
}

void fade(int pin) {

  for (int u = 0; u < 1024; u++) {
    analogWrite(pin, u);
    delay(1);
  }
  for (int u = 0; u < 1024; u++) {
    analogWrite(pin, 1023 - u);
    delay(1);
  }
}

void testRGB() { // fade in and out of Red, Green, Blue

  analogWrite(REDPIN, 0); // R off
  analogWrite(GREENPIN, 0); // G off
  analogWrite(BLUEPIN, 0); // B off
  fade(REDPIN); // R
  fade(GREENPIN); // G
  fade(BLUEPIN); // B
}

void setup() {

  pinMode(REDPIN, OUTPUT);
  pinMode(GREENPIN, OUTPUT);
  pinMode(BLUEPIN, OUTPUT);
  analogWrite(REDPIN, 0);
  analogWrite(GREENPIN, 0);
  analogWrite(BLUEPIN, 0);

  delay(500);
  // Serial.begin(9600);
  // Serial.println();

  WiFi.mode(WIFI_AP);
  delay(500);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP(ssid, password);
  delay(1000);

  // if DNSServer is started with "*" for domain name, it will reply with provided IP to all DNS request
  dnsServer.start(DNS_PORT, "wifiDomo1", apIP);

  webServer.on("/", handleRoot);

  webServer.begin();

  testRGB();
}



void loop() {

  dnsServer.processNextRequest();
  webServer.handleClient();

}
