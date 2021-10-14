#include <ArduinoJson.h>
#include <HTTP_Method.h>
#include <WebServer.h>
#include <Uri.h>
#include <WiFiSTA.h>
#include <WiFiMulti.h>
#include <WiFiGeneric.h>
#include <WiFiClient.h>
#include <WiFiUdp.h>
#include <ETH.h>
#include <WiFiType.h>
#include <WiFiServer.h>
#include <WiFiScan.h>
#include <WiFiAP.h>
#include <Arduino.h>
#include "epd_driver.h"
#include <WiFi.h>
#include "background.h"

const char* ssid = "DarkWeb";
const char* password = "vizibicikli";

WebServer server(80);
uint8_t *framebuffer;

void update(uint32_t delay_ms)
{
  epd_poweron();
  epd_clear();
  volatile uint32_t t1 = millis();
  epd_draw_grayscale_image(epd_full_screen(), framebuffer);
  volatile uint32_t t2 = millis();
  Serial.printf("EPD draw took %dms.\n", t2 - t1);
  epd_poweroff();
  delay(delay_ms);
}

void setup() {
  Serial.begin(115200);
  // Display setup
  epd_init();
  framebuffer = (uint8_t *)heap_caps_malloc(EPD_WIDTH * EPD_HEIGHT / 2, MALLOC_CAP_SPIRAM);
  memset(framebuffer, 0xFF, EPD_WIDTH * EPD_HEIGHT / 2);
  epd_clear();

  // Server setup
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }

  Serial.println("WiFi connected!");
  Serial.print("IP: ");  Serial.println(WiFi.localIP());

  server.on("/image", handleIncomingRequest);

  server.begin();
  Serial.println("Server listening");
}

void loop() {
  server.handleClient();
}

void handleIncomingRequest() {
  Serial.println("Request received");
  epd_clear();
  if (server.hasArg("plain") == false) {
    server.send(500, "text/plain", "Body not received");
    return;
  }


  DynamicJsonDocument doc(4096);
  deserializeJson(doc, server.arg("plain"));
  
  const char* imgData = doc["image"];
  Serial.println(imgData);

  const uint32_t height = (uint32_t) doc["height"];
  const uint32_t width = (uint32_t) doc["width"];
  
  Rect_t area = {
    .x = 0,
    .y = 0,
    .width = width,
    .height =  height
  };
  epd_poweron();
  epd_clear();
  epd_draw_grayscale_image(area, (uint8_t *) background_data);

  epd_poweroff();
  server.send(200, "text/plain", "OK");
}
