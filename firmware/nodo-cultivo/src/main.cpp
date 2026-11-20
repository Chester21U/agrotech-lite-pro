#include <Arduino.h>
#include <DHT.h>

#define PIN_DHT D2
#define PIN_RELE D1
#define TIPO_DHT DHT11

DHT dht(PIN_DHT, TIPO_DHT);

void setup() {
  Serial.begin(115200);
  delay(1500);
  pinMode(PIN_RELE, OUTPUT);
  digitalWrite(PIN_RELE, HIGH);
  dht.begin();
  Serial.println();
  Serial.println(F("AgroSmart - prueba de sensores"));
  Serial.println(F("Seco al aire = valor alto | Dentro del agua = valor bajo"));
  Serial.println();
}

void loop() {
  int crudo = analogRead(A0);
  float t = dht.readTemperature();
  float h = dht.readHumidity();

  Serial.print(F("suelo_crudo="));
  Serial.print(crudo);
  Serial.print(F("   temp="));
  if (isnan(t)) Serial.print(F("ERROR")); else Serial.print(t, 1);
  Serial.print(F("   hum_aire="));
  if (isnan(h)) Serial.print(F("ERROR")); else Serial.print(h, 1);
  Serial.println();

  delay(1500);
}
