#include <Arduino.h>
#include <ArduinoJson.h>
#include <Hash.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <WebSocketsClient.h>
#include <SocketIOclient.h>

#define WIFI_SSID "Das Residence"
#define WIFI_PASSWORD "Jayanta@1964"
#define WS_HOST "192.168.0.101"
#define WS_PORT 3000
#define WS_URL "/socket.io/?EIO=4"

ESP8266WiFiMulti wifiMulti;
SocketIOclient socketIO;

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
  digitalWrite(LED_BUILTIN, HIGH); // My board LED_BUILTIN is off at HIGH

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

  socketIO.begin(WS_HOST, WS_PORT, WS_URL);
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