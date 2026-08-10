/**
 * XIAO ESP32-C6 MQTT 통신 테스트
 * 목적: Wi-Fi 연결, MQTT Broker 통신 테스트
 *
 * 네트워크 설정:
 * - WiFi: ICEE
 * - MQTT Broker: 192.168.0.43:1883
 *
 * 토픽 구조:
 * - PSEE/entryway/test: 테스트 메시지 전송
 * - PSEE/entryway/status: 장치 상태 전송
 *
 * 하드웨어: XIAO ESP32-C6 (RISC-V 듀얼 코어)
 */

#include <WiFi.h>
#include <PubSubClient.h>

// WiFi 설정 (ICEE)
const char* ssid = "ICEE";
const char* password = "icee2026";

// MQTT Broker 설정
const char* mqtt_server = "192.168.0.43";
const int mqtt_port = 1883;
const char* mqtt_topic_test = "PSEE/entryway/test";
const char* mqtt_topic_status = "PSEE/entryway/status";

// 클라이언트 ID 생성 (PSEE 접두사)
String clientId_base = "PSEE-XIAO-C6-";

// WiFi와 MQTT 클라이언트 객체
WiFiClient espClient;
PubSubClient client(espClient);

// 타이머 변수
unsigned long lastMsgTime = 0;
const long interval = 3000;  // 3초마다 메시지 전송
int messageCount = 0;

// 연결 상태 추적
bool wifiConnected = false;
bool mqttConnected = false;

void setup_wifi() {
  delay(10);
  Serial.println("\n========================================");
  Serial.println("Wi-Fi 연결 시작");
  Serial.println("========================================");
  Serial.printf("SSID: %s\n", ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    wifiConnected = true;
    Serial.println("\n✓ Wi-Fi 연결 완료!");
    Serial.printf("IP 주소: %s\n", WiFi.localIP().toString().c_str());
    Serial.printf("신호 강도: %d dBm\n", WiFi.RSSI());
    Serial.printf("MAC 주소: %s\n", WiFi.macAddress().c_str());
    Serial.printf("Wi-Fi 6 지원: %s\n", WiFi.getProtocol() == WIFI_PROTOCOL_11AX ? "예" : "아니오");
  } else {
    Serial.println("\n✗ Wi-Fi 연결 실패!");
    Serial.println("SSID 또는 비밀번호를 확인해주세요.");
    wifiConnected = false;
  }
  Serial.println("========================================\n");
}

void reconnect() {
  if (!wifiConnected) {
    Serial.println("Wi-Fi에 연결되지 않음 - 재시도 불가");
    return;
  }

  Serial.print("MQTT Broker 접속 시도: ");
  Serial.print(mqtt_server);
  Serial.print(":");
  Serial.println(mqtt_port);

  // 클라이언트 ID 생성
  String clientId = clientId_base + String(random(0xffff), HEX);
  Serial.printf("클라이언트 ID: %s\n", clientId.c_str());

  int attempts = 0;
  while (!client.connected() && attempts < 5) {
    if (client.connect(clientId.c_str())) {
      mqttConnected = true;
      Serial.println("✓ MQTT 접속 성공!");

      // 접속 상태 전송
      String statusMsg = "{\"device\":\"XIAO-ESP32C6\",\"status\":\"online\",\"ip\":\"" +
                         WiFi.localIP().toString() + "\"}";
      client.publish(mqtt_topic_status, statusMsg.c_str());

      Serial.printf("테스트 토픽: %s\n", mqtt_topic_test);
      Serial.printf("상태 토픽: %s\n", mqtt_topic_status);
    } else {
      Serial.print("✗ 접속 실패, rc=");
      Serial.print(client.state());
      Serial.println(" - 5초 후 재시도...");
      delay(5000);
      attempts++;
    }
  }

  if (!client.connected()) {
    mqttConnected = false;
    Serial.println("✗ MQTT 접속 최종 실패!");
    Serial.println("Broker IP 또는 방화벽을 확인해주세요.");
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("\n\n╔════════════════════════════════════════╗");
  Serial.println("║   XIAO ESP32-C6 MQTT 통신 테스트    ║");
  Serial.println("╚════════════════════════════════════════╝");

  // WiFi 연결
  setup_wifi();

  if (wifiConnected) {
    // MQTT 클라이언트 설정
    client.setServer(mqtt_server, mqtt_port);

    // MQTT 재접속
    reconnect();

    if (mqttConnected) {
      Serial.println("========================================");
      Serial.println("테스트 준비 완료!");
      Serial.println("3초마다 테스트 메시지 전송...");
      Serial.println("========================================\n");
    }
  } else {
    Serial.println("========================================");
    Serial.println("Wi-Fi 연결 실패로 테스트 종료");
    Serial.println("========================================\n");
  }
}

void loop() {
  // Wi-Fi 연결 상태 확인
  if (wifiConnected && WiFi.status() != WL_CONNECTED) {
    wifiConnected = false;
    Serial.println("\n⚠ Wi-Fi 연결 끊김 - 재연결 시도...");
    setup_wifi();
  }

  // MQTT 연결 상태 확인 및 재접속
  if (wifiConnected && !client.connected()) {
    mqttConnected = false;
    Serial.println("\n⚠ MQTT 연결 끊김 - 재접속 시도...");
    reconnect();
  }

  client.loop();

  // 주기적으로 테스트 메시지 전송
  if (wifiConnected && mqttConnected) {
    unsigned long now = millis();
    if (now - lastMsgTime >= interval) {
      lastMsgTime = now;
      messageCount++;

      // 테스트 메시지 생성
      String message = "MQTT Test Message #" + String(messageCount) + " - " + String(now);

      Serial.print("Publish: ");
      Serial.print(mqtt_topic_test);
      Serial.print(" -> ");
      Serial.println(message);

      // 메시지 전송
      if (client.publish(mqtt_topic_test, message.c_str())) {
        Serial.printf("  ✓ 메시지 #%d 전송 성공\n", messageCount);
      } else {
        Serial.printf("  ✗ 메시지 #%d 전송 실패\n", messageCount);
      }
      Serial.println();
    }
  } else {
    // 연결 실패 시 10초마다 재시도
    delay(10000);
  }
}