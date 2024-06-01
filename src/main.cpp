#include <Arduino.h>
#include <ArduinoJson.h>
#include <Hash.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <WebSocketsClient.h>
#include <SocketIOclient.h>

#define WIFI_SSID "Das Residence"
#define WIFI_PASSWORD "Jayanta@1964"
#define WS_HOST "nodemcu-esp8266-server.onrender.com"
#define WS_PORT 443
#define WS_URL "/socket.io/?EIO=4"

// TLS certificate to enable secure websockets
// const char ENDPOINT_CA_CERT[] PROGMEM = R"EOF(
// -----BEGIN CERTIFICATE-----
// MIIDzTCCArWgAwIBAgIQCjeHZF5ftIwiTv0b7RQMPDANBgkqhkiG9w0BAQsFADBa
// MQswCQYDVQQGEwJJRTESMBAGA1UEChMJQmFsdGltb3JlMRMwEQYDVQQLEwpDeWJl
// clRydXN0MSIwIAYDVQQDExlCYWx0aW1vcmUgQ3liZXJUcnVzdCBSb290MB4XDTIw
// MDEyNzEyNDgwOFoXDTI0MTIzMTIzNTk1OVowSjELMAkGA1UEBhMCVVMxGTAXBgNV
// BAoTEENsb3VkZmxhcmUsIEluYy4xIDAeBgNVBAMTF0Nsb3VkZmxhcmUgSW5jIEVD
// QyBDQS0zMFkwEwYHKoZIzj0CAQYIKoZIzj0DAQcDQgAEua1NZpkUC0bsH4HRKlAe
// nQMVLzQSfS2WuIg4m4Vfj7+7Te9hRsTJc9QkT+DuHM5ss1FxL2ruTAUJd9NyYqSb
// 16OCAWgwggFkMB0GA1UdDgQWBBSlzjfq67B1DpRniLRF+tkkEIeWHzAfBgNVHSME
// GDAWgBTlnVkwgkdYzKz6CFQ2hns6tQRN8DAOBgNVHQ8BAf8EBAMCAYYwHQYDVR0l
// BBYwFAYIKwYBBQUHAwEGCCsGAQUFBwMCMBIGA1UdEwEB/wQIMAYBAf8CAQAwNAYI
// KwYBBQUHAQEEKDAmMCQGCCsGAQUFBzABhhhodHRwOi8vb2NzcC5kaWdpY2VydC5j
// b20wOgYDVR0fBDMwMTAvoC2gK4YpaHR0cDovL2NybDMuZGlnaWNlcnQuY29tL09t
// bmlyb290MjAyNS5jcmwwbQYDVR0gBGYwZDA3BglghkgBhv1sAQEwKjAoBggrBgEF
// BQcCARYcaHR0cHM6Ly93d3cuZGlnaWNlcnQuY29tL0NQUzALBglghkgBhv1sAQIw
// CAYGZ4EMAQIBMAgGBmeBDAECAjAIBgZngQwBAgMwDQYJKoZIhvcNAQELBQADggEB
// AAUkHd0bsCrrmNaF4zlNXmtXnYJX/OvoMaJXkGUFvhZEOFp3ArnPEELG4ZKk40Un
// +ABHLGioVplTVI+tnkDB0A+21w0LOEhsUCxJkAZbZB2LzEgwLt4I4ptJIsCSDBFe
// lpKU1fwg3FZs5ZKTv3ocwDfjhUkV+ivhdDkYD7fa86JXWGBPzI6UAPxGezQxPk1H
// goE6y/SJXQ7vTQ1unBuCJN0yJV0ReFEQPaA1IwQvZW+cwdFD19Ae8zFnWSfda9J1
// CZMRJCQUzym+5iPDuI9yP+kHyCREU3qzuWFloUwOxkgAyXVjBYdwRVKD05WdRerw
// 6DEdfgkfCv4+3ao8XnTSrLE=
// -----END CERTIFICATE-----
// )EOF";


ESP8266WiFiMulti wifiMulti;
SocketIOclient socketIO;

struct PinStatus
{
  int pin;
  int status;
};

void controlDigitalPins(uint8_t *message)
{
  // Allocate a temporary JsonDocument
  StaticJsonDocument<200> doc;

  // Deserialize the JSON document
  DeserializationError error = deserializeJson(doc, message);

  // Test if parsing succeeds
  if (error)
  {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    return;
  }

  // Extract values
  const char *event = doc[0];
  const char *payload = doc[1];

  // Handle the event
  if (strcmp(event, "message") == 0)
  {
    Serial.println(payload);
  }
  else if (strcmp(event, "digital") == 0)
  {
    Serial.println(payload);
    StaticJsonDocument<200> pinDoc;

    DeserializationError pinDeserializeError = deserializeJson(pinDoc, payload);

    if (pinDeserializeError)
    {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(pinDeserializeError.f_str());
      return;
    }

    PinStatus pinstatus;
    pinstatus.pin = pinDoc["pin"];
    pinstatus.status = pinDoc["status"];
    Serial.printf("Pin: %d\n", pinstatus.pin);
    Serial.printf("Status: %d\n", pinstatus.status);
    digitalWrite(pinstatus.pin, pinstatus.status);
  }
}

void socketIOEvent(socketIOmessageType_t type, uint8_t *payload, size_t length)
{
  switch (type)
  {
  case sIOtype_DISCONNECT:
    Serial.printf("[IOc] Disconnected!\n");
    digitalWrite(LED_BUILTIN, HIGH); // My board LED_BUILTIN is off at HIGH
    break;
  case sIOtype_CONNECT:
    Serial.printf("[IOc] Connected to url: %s\n", payload);
    digitalWrite(LED_BUILTIN, LOW); // My board LED_BUILTIN is on at LOW
    // join default namespace (no auto join in Socket.IO V3)
    socketIO.send(sIOtype_CONNECT, "/");
    break;
  case sIOtype_EVENT:
    // Serial.printf("[IOc] get event: %s\n", payload);
    controlDigitalPins(payload);
    break;
  case sIOtype_ACK:
    Serial.printf("[IOc] get ack: %u\n", length);
    hexdump(payload, length);
    break;
  case sIOtype_ERROR:
    Serial.printf("[IOc] get error: %u\n", length);
    hexdump(payload, length);
    break;
  case sIOtype_BINARY_EVENT:
    Serial.printf("[IOc] get binary: %u\n", length);
    hexdump(payload, length);
    break;
  case sIOtype_BINARY_ACK:
    Serial.printf("[IOc] get binary ack: %u\n", length);
    hexdump(payload, length);
    break;
  }
}

void setup()
{
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(D0, OUTPUT);
  pinMode(D1, OUTPUT);
  pinMode(D2, OUTPUT);
  pinMode(D3, OUTPUT);
  pinMode(D4, OUTPUT);
  pinMode(D5, OUTPUT);
  pinMode(D6, OUTPUT);
  pinMode(D7, OUTPUT);
  pinMode(D8, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH); // My board LED_BUILTIN is off at HIGH
  digitalWrite(D0, HIGH);

  // Serial.setDebugOutput(true);
  Serial.setDebugOutput(true);

  Serial.println();
  Serial.println();
  Serial.println();

  for (uint8_t t = 4; t > 0; t--)
  {
    Serial.printf("[SETUP] BOOT WAIT %d...\n", t);
    Serial.flush();
    delay(1000);
  }

  // disable AP
  if (WiFi.getMode() & WIFI_AP)
  {
    WiFi.softAPdisconnect(true);
  }

  wifiMulti.addAP(WIFI_SSID, WIFI_PASSWORD);

  // WiFi.disconnect();
  while (wifiMulti.run() != WL_CONNECTED)
  {
    delay(100);
  }

  String ip = WiFi.localIP().toString();
  Serial.println("");
  Serial.println("Connected to Wifi");

  // socketIO.begin(WS_HOST, WS_PORT, WS_URL); // USE THIS WHEN SERVER IS RUNNING IN LOCALHOST
  socketIO.beginSSL(WS_HOST, WS_PORT, WS_URL); 
  socketIO.onEvent(socketIOEvent);
}

unsigned long messageTimestamp = 0;

void loop()
{
  socketIO.loop();

  uint64_t now = millis();

  if (now - messageTimestamp > 2000)
  {
    messageTimestamp = now;

    // creat JSON message for Socket.IO (event)
    DynamicJsonDocument doc(1024);
    JsonArray array = doc.to<JsonArray>();

    // Hint: socket.on('event_name', ....
    array.add("ping");

    // add payload (parameters) for the event
    JsonObject param1 = array.createNestedObject();
    param1["now"] = (uint32_t)now;

    // JSON to String (serialization)
    String output;
    serializeJson(doc, output);

    // Send event
    socketIO.sendEVENT(output);

    // Print JSON for debugging
    // Serial.println(output);
  }
}