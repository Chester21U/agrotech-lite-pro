#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <DHT.h>
#include "secrets.h"

#define PIN_DHT D2
#define PIN_RELE D1
#define TIPO_DHT DHT11
#define RELE_ACTIVO HIGH

#define SUELO_SECO 784
#define SUELO_MOJADO 298

#define HUMEDAD_ENCENDER 30
#define HUMEDAD_APAGAR 60
#define MAX_RIEGO_MS 10000UL
#define REPOSO_MS 60000UL
#define PULSOS_MAX 3

#define INTERVALO_TELEMETRIA 2000UL
#define INTERVALO_SENSORES 2000UL
#define INTERVALO_MQTT 5000UL
#define INTERVALO_WIFI 30000UL

const char* TOPICO_TELEMETRIA = "agrosmart/nodo1/telemetria";
const char* TOPICO_MODO = "agrosmart/nodo1/modo";
const char* TOPICO_BOMBA = "agrosmart/nodo1/bomba";
const char* TOPICO_ALARMA = "agrosmart/nodo1/alarma";

DHT dht(PIN_DHT, TIPO_DHT);
WiFiClient espCliente;
PubSubClient mqtt(espCliente);

float temperatura = 0;
float humedadAire = 0;
int humedadSuelo = 0;

bool modoAuto = true;
bool bombaEncendida = false;
bool bloqueo = false;
uint8_t pulsos = 0;

unsigned long inicioRiego = 0;
unsigned long finRiego = 0;
unsigned long ultimaTelemetria = 0;
unsigned long ultimaLectura = 0;
unsigned long ultimoIntentoMqtt = 0;
unsigned long ultimoIntentoWifi = 0;

void aplicarBomba(bool encender) {
  if (encender == bombaEncendida) return;
  bombaEncendida = encender;
  digitalWrite(PIN_RELE, encender ? RELE_ACTIVO : !RELE_ACTIVO);
  if (encender) inicioRiego = millis();
  else finRiego = millis();
}

void publicarAlarma(const char* motivo) {
  if (mqtt.connected()) mqtt.publish(TOPICO_ALARMA, motivo);
  Serial.print(F("ALARMA: "));
  Serial.println(motivo);
}

void leerSensores() {
  int crudo = analogRead(A0);
  long pct = map(crudo, SUELO_SECO, SUELO_MOJADO, 0, 100);
  humedadSuelo = constrain(pct, 0, 100);
  float t = dht.readTemperature();
  float h = dht.readHumidity();
  if (!isnan(t)) temperatura = t;
  if (!isnan(h)) humedadAire = h;
}

void controlRiego() {
  unsigned long ahora = millis();

  if (bombaEncendida && ahora - inicioRiego >= MAX_RIEGO_MS) {
    aplicarBomba(false);
    pulsos++;
    publicarAlarma("anti_inundacion");
    if (pulsos >= PULSOS_MAX) {
      bloqueo = true;
      publicarAlarma("deposito_vacio");
    }
    return;
  }

  if (!modoAuto) return;

  if (bombaEncendida) {
    if (humedadSuelo >= HUMEDAD_APAGAR) {
      aplicarBomba(false);
      pulsos = 0;
    }
    return;
  }

  if (bloqueo) return;
  if (finRiego != 0 && ahora - finRiego < REPOSO_MS) return;
  if (humedadSuelo < HUMEDAD_ENCENDER) aplicarBomba(true);
}

void alRecibirMensaje(char* topico, byte* carga, unsigned int largo) {
  String mensaje;
  for (unsigned int i = 0; i < largo; i++) mensaje += (char)carga[i];
  mensaje.trim();

  if (strcmp(topico, TOPICO_MODO) == 0) {
    modoAuto = (mensaje == "auto");
    if (bombaEncendida) aplicarBomba(false);
    return;
  }

  if (strcmp(topico, TOPICO_BOMBA) == 0) {
    if (mensaje == "reset") {
      bloqueo = false;
      pulsos = 0;
      finRiego = 0;
      return;
    }
    if (modoAuto || bloqueo) return;
    if (mensaje == "on") aplicarBomba(true);
    else if (mensaje == "off") aplicarBomba(false);
  }
}

void publicarTelemetria() {
  if (!mqtt.connected()) return;
  JsonDocument doc;
  doc["t"] = temperatura;
  doc["ha"] = humedadAire;
  doc["hs"] = humedadSuelo;
  doc["bomba"] = bombaEncendida;
  doc["modo"] = modoAuto ? "auto" : "manual";
  doc["bloqueo"] = bloqueo;
  char carga[160];
  serializeJson(doc, carga);
  mqtt.publish(TOPICO_TELEMETRIA, carga);
}

void conectarWifi() {
  if (WiFi.status() == WL_CONNECTED) return;
  unsigned long ahora = millis();
  if (ahora - ultimoIntentoWifi < INTERVALO_WIFI) return;
  ultimoIntentoWifi = ahora;
  WiFi.begin(WIFI_SSID, WIFI_PASS);
}

void conectarMqtt() {
  if (WiFi.status() != WL_CONNECTED || mqtt.connected()) return;
  unsigned long ahora = millis();
  if (ahora - ultimoIntentoMqtt < INTERVALO_MQTT) return;
  ultimoIntentoMqtt = ahora;
  if (mqtt.connect("agrosmart-nodo1")) {
    mqtt.subscribe(TOPICO_MODO);
    mqtt.subscribe(TOPICO_BOMBA);
    Serial.println(F("MQTT conectado"));
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  pinMode(PIN_RELE, OUTPUT);
  digitalWrite(PIN_RELE, !RELE_ACTIVO);
  dht.begin();
  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  mqtt.setServer(MQTT_HOST, MQTT_PORT);
  mqtt.setCallback(alRecibirMensaje);
  leerSensores();
  Serial.println();
  Serial.println(F("AgroSmart - nodo de cultivo 1"));
}

void loop() {
  unsigned long ahora = millis();

  conectarWifi();
  conectarMqtt();
  mqtt.loop();

  if (ahora - ultimaLectura >= INTERVALO_SENSORES) {
    ultimaLectura = ahora;
    leerSensores();
  }

  controlRiego();

  if (ahora - ultimaTelemetria >= INTERVALO_TELEMETRIA) {
    ultimaTelemetria = ahora;
    publicarTelemetria();
    Serial.print(F("hs="));
    Serial.print(humedadSuelo);
    Serial.print(F("%  t="));
    Serial.print(temperatura, 1);
    Serial.print(F("  ha="));
    Serial.print(humedadAire, 0);
    Serial.print(F("%  bomba="));
    Serial.print(bombaEncendida ? F("ON") : F("off"));
    Serial.print(F("  modo="));
    Serial.print(modoAuto ? F("auto") : F("manual"));
    Serial.print(F("  bloqueo="));
    Serial.print(bloqueo ? F("SI") : F("no"));
    Serial.print(F("  wifi="));
    Serial.print(WiFi.status() == WL_CONNECTED ? F("ok") : F("--"));
    Serial.print(F("  mqtt="));
    Serial.println(mqtt.connected() ? F("ok") : F("--"));
  }
}
