/*
  GS 2025 - SmartBreak IoT
  Estação de Bem-Estar e Produtividade para o Futuro do Trabalho

  Funcionalidades principais:
  - Leitura de temperatura e umidade (DHT22)
  - Leitura de luminosidade (LDR)
  - Botão para registrar pausa do colaborador
  - LED e buzzer avisam quando condições não estão ideais ou há muito tempo sem pausa
  - Envio periódico dos dados via MQTT em formato JSON

  Autor(es): (preencha com os nomes do grupo)
*/

#include <WiFi.h>
#include <PubSubClient.h>
#include "DHT.h"

// === Pinos ===
#define DHTPIN 15
#define DHTTYPE DHT22
#define LDRPIN 34
#define LEDPIN 2
#define BUZZERPIN 4
#define BREAK_BUTTON_PIN 5

// === Configuração Wi-Fi (Wokwi) ===
const char* ssid = "Wokwi-GUEST";
const char* password = "";

// === Broker MQTT ===
// Para demonstração está sendo usado o broker público test.mosquitto.org.
// Em um ambiente real, o ideal é utilizar um broker próprio/autenticado.
const char* mqtt_server = "test.mosquitto.org";
const int   mqtt_port   = 1883;

// Tópicos MQTT
const char* topic_sensores = "gs2025/smartbreak/estacao1/sensores";
const char* topic_alertas  = "gs2025/smartbreak/estacao1/alertas";

WiFiClient espClient;
PubSubClient client(espClient);
DHT dht(DHTPIN, DHTTYPE);

unsigned long lastMsg = 0;
unsigned long lastBreakMillis = 0;

// Intervalo entre envios (ms)
const unsigned long sendInterval = 5000;

// Para a apresentação em vídeo, usamos 1 minuto sem pausa para disparar alerta.
// Em produção, ajustar para 50 * 60 * 1000 (50 minutos).
const unsigned long maxNoBreakMillisDemo = 60000;

bool lastBreakButtonState = HIGH;

void setupWifi();
void reconnectMQTT();
void publishReadings(float t, float h, int lightPct,
                     unsigned long minsSemPausa,
                     bool alerta, const String& motivo);
void beepAlert();

void setup() {
  Serial.begin(115200);
  delay(1000);

  pinMode(LEDPIN, OUTPUT);
  pinMode(BUZZERPIN, OUTPUT);
  pinMode(BREAK_BUTTON_PIN, INPUT_PULLUP); // Botão ligado ao GND

  digitalWrite(LEDPIN, LOW);
  digitalWrite(BUZZERPIN, LOW);

  dht.begin();
  setupWifi();

  client.setServer(mqtt_server, mqtt_port);

  randomSeed(analogRead(0));
  lastBreakMillis = millis();

  Serial.println("Sistema SmartBreak IoT iniciado.");
}

void loop() {
  if (!client.connected()) {
    reconnectMQTT();
  }
  client.loop();

  unsigned long now = millis();

  // Verifica se o colaborador apertou o botão para registrar pausa
  bool currentBtn = digitalRead(BREAK_BUTTON_PIN);
  if (currentBtn == LOW && lastBreakButtonState == HIGH) {
    lastBreakMillis = now;
    Serial.println("Pausa registrada pelo usuario.");
  }
  lastBreakButtonState = currentBtn;

  // Envia leituras periodicamente
  if (now - lastMsg >= sendInterval) {
    lastMsg = now;

    float h = dht.readHumidity();
    float t = dht.readTemperature();

    if (isnan(h) || isnan(t)) {
      Serial.println("Falha ao ler o sensor DHT22.");
      return;
    }

    int lightRaw = analogRead(LDRPIN);
    int lightPct = map(lightRaw, 0, 4095, 0, 100);

    unsigned long msSemPausa = now - lastBreakMillis;
    unsigned long minsSemPausa = msSemPausa / 60000UL;

    bool alerta = false;
    String motivo = "";

    // Faixas recomendadas aproximadas
    if (t > 26.0) { alerta = true; motivo += "Temperatura acima do ideal; "; }
    else if (t < 20.0) { alerta = true; motivo += "Temperatura abaixo do ideal; "; }

    if (h < 30.0) { alerta = true; motivo += "Umidade baixa; "; }
    else if (h > 70.0) { alerta = true; motivo += "Umidade alta; "; }

    if (lightPct < 35) { alerta = true; motivo += "Iluminacao insuficiente; "; }

    if (msSemPausa >= maxNoBreakMillisDemo) {
      alerta = true;
      motivo += "Muito tempo sem pausa; ";
    }

    if (alerta) {
      digitalWrite(LEDPIN, HIGH);
      beepAlert();
    } else {
      digitalWrite(LEDPIN, LOW);
      digitalWrite(BUZZERPIN, LOW);
    }

    publishReadings(t, h, lightPct, minsSemPausa, alerta, motivo);
  }
}

void setupWifi() {
  delay(10);
  Serial.println();
  Serial.print("Conectando-se ao Wi-Fi: ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("Wi-Fi conectado com sucesso.");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
}

void reconnectMQTT() {
  while (!client.connected()) {
    Serial.print("Conectando ao broker MQTT... ");
    String clientId = "SmartBreakGS-";
    clientId += String(random(0xffff), HEX);

    if (client.connect(clientId.c_str())) {
      Serial.println("conectado.");
    } else {
      Serial.print("falhou, rc=");
      Serial.print(client.state());
      Serial.println(" tentando novamente em 5 segundos.");
      delay(5000);
    }
  }
}

void publishReadings(float t, float h, int lightPct,
                     unsigned long minsSemPausa,
                     bool alerta, const String& motivo) {

  // JSON simples com leituras principais
  String payload = "{";
  payload += "\"temp\":" + String(t, 1) + ",";
  payload += "\"hum\":" + String(h, 1) + ",";
  payload += "\"light\":" + String(lightPct) + ",";
  payload += "\"minsSinceBreak\":" + String(minsSemPausa) + ",";
  payload += "\"alert\":" + String(alerta ? "true" : "false");
  payload += "}";

  client.publish(topic_sensores, payload.c_str());
  Serial.print("Publicado em ");
  Serial.print(topic_sensores);
  Serial.print(": ");
  Serial.println(payload);

  if (alerta) {
    String alertaMsg = "{";
    alertaMsg += "\"motivo\":\"" + motivo + "\"";
    alertaMsg += "}";

    client.publish(topic_alertas, alertaMsg.c_str());
    Serial.print("Publicado alerta em ");
    Serial.print(topic_alertas);
    Serial.print(": ");
    Serial.println(alertaMsg);
  }
}

void beepAlert() {
  // Dois beeps curtos para indicar alerta de pausa/condição inadequada
  for (int i = 0; i < 2; i++) {
    digitalWrite(BUZZERPIN, HIGH);
    delay(120);
    digitalWrite(BUZZERPIN, LOW);
    delay(120);
  }
}
