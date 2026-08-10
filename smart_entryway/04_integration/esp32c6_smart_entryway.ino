/**
 * XIAO ESP32-C6 스마트 현관 외출 비서 (통합 테스트 - 수정 완료)
 * "아차, 우산!" AI 기반 외출 알림 시스템 (ESP32-C6 버전)
 *
 * 기능:
 * 1. Wi-Fi 연결 및 날씨 API (KMA) 확인
 * 2. PIR 센서로 사람 감지 (GPIO 1)
 * 3. Mock 객체 인식 (ESP32-C6는 카메라/PSRAM 없음)
 * 4. 로직: 비가 오는데 우산 없음 → 빨간 LED + 부저 경고
 * 5. 우산 챙김 → 초록 LED + 기분 좋은 알림음
 * 6. MQTT로 상태 전송
 * 7. Claude API로 알림 문구 생성 (Mock)
 *
 * 하드웨어 (수정 완료):
 * - XIAO ESP32-C6 (RISC-V 듀얼 코어, 512KB SRAM, 4MB Flash)
 * - 내장 LED (GPIO 15): 빨간색/초록색 표시
 * - 부저 (GPIO 21, D3): 경고음/알림음 (PWM)
 * - PIR 센서 (GPIO 1, D1): 사람 감지
 *
 * 핀 맵 (수정 완료):
 * - GPIO 15: 내장 LED ✓
 * - GPIO 16: UART0 TX (시리얼 통신용) ⚠ 사용 금지
 * - GPIO 17: UART0 RX (시리얼 통신용) ⚠ 사용 금지
 * - GPIO 1: PIR 센서 ✓
 * - GPIO 21: 부저 ✓
 *
 * 참고: GPIO 16, 17은 UART0 기본 핀으로 사용됩니다
 * 부저/PIR 센서에는 GPIO 1, 21 등 일반 GPIO를 사용해야 합니다
 */

#include <WiFi.h>
#include <PubSubClient.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// ========== WiFi 설정 ==========
const char* ssid = "ICEE";
const char* password = "icee2026";

// ========== MQTT 설정 ==========
const char* mqtt_server = "192.168.0.43";
const int mqtt_port = 1883;
const char* mqtt_topic_weather = "PSEE/entryway/weather";
const char* mqtt_topic_detection = "PSEE/entryway/detection";
const char* mqtt_topic_alert = "PSEE/entryway/alert";
const char* mqtt_topic_llm = "PSEE/entryway/llm_message";

// ========== 하드웨어 핀 (ESP32-C6 수정 완료) ==========
#define USER_LED 15      // 내장 LED (GPIO 15)
#define BUZZER_PIN 21    // 부저 (GPIO 21, D3) - UART 핀 회피
#define PIR_SENSOR 1     // PIR 센서 (GPIO 1, D1) - UART 핀 회피

// UART0 기본 핀 (시리얼 통신용):
// GPIO 16: TX (송신) - 사용 금지!
// GPIO 17: RX (수신) - 사용 금지!
// 부저/PIR 센서에는 GPIO 1, 21 등 일반 GPIO 사용

// ========== 시연 모드 설정 ==========
#define DEMO_MODE  true  // true: 시연용 Mock 사용, false: PIR 센서 사용
#define USE_PIR_SENSOR false  // true: PIR 센서 사용, false: Mock

// ========== 글로벌 변수 ==========
WiFiClient espClient;
PubSubClient client(espClient);

// 시스템 상태
bool wifiConnected = false;
bool mqttConnected = false;
bool pirSensorReady = false;

// 날씨 상태
bool isRaining = false;
String weatherCondition = "맑음";
float temperature = 20.0;

// 객체 인식 상태 (Mock)
bool personDetected = false;
bool umbrellaDetected = false;

// 시연용 강제 날씨 설정 (DEMO_MODE가 true일 때만 사용)
bool demoRaining = true;           // 시연용: 비 오는 상태 강제
bool demoPersonDetected = true;    // 시연용: 사람 감지 강제
bool demoUmbrellaDetected = false; // 시연용: 우산 없음 강제 (시나리오 1)

// LED/부저 상태
unsigned long lastAlertTime = 0;
const unsigned long alertInterval = 2000;  // 2초마다 경고

// ========== 함수 프로토타입 ==========
void setup_wifi();
void setup_mqtt();
void setup_hardware();
void setup_pir_sensor();
void reconnect();
bool fetch_weather_kma();
bool fetch_weather_mock();
void check_alert_logic();
void set_alert(bool alertActive);
void play_alert_sound(bool warning);
void publish_weather();
void publish_detection();
void publish_alert(String message);
void publish_llm_message(String message);
String generate_llm_message_mock();
bool read_pir_sensor();

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("\n\n╔════════════════════════════════════════════════╗");
  Serial.println("║   스마트 현관 외출 비서 (ESP32-C6 버전)    ║");
  Serial.println("║           \"아차, 우산!\" AI 시스템             ║");
  Serial.println("╚════════════════════════════════════════════════╝\n");

  // 하드웨어 초기화
  setup_hardware();

  // PIR 센서 초기화 (선택사항)
  if (USE_PIR_SENSOR) {
    setup_pir_sensor();
  }

  // Wi-Fi 연결
  setup_wifi();

  if (wifiConnected) {
    // MQTT 연결
    setup_mqtt();
  }

  // 시스템 준비 상태 출력
  Serial.println("\n════════════════════════════════════════════════");
  Serial.println("시스템 상태 요약 (수정 완료):");
  Serial.printf("  보드: XIAO ESP32-C6 (RISC-V)\n");
  Serial.printf("  메모리: SRAM %dKB, Flash %dMB\n",
                ESP.getHeapSize() / 1024, ESP.getFlashChipSize() / (1024 * 1024));
  Serial.printf("  Wi-Fi: %s\n", wifiConnected ? "✓ 연결됨" : "✗ 연결 실패");
  Serial.printf("  MQTT: %s\n", mqttConnected ? "✓ 연결됨" : "✗ 연결 실패");
  Serial.printf("  PIR 센서: %s (GPIO %d)\n", pirSensorReady ? "✓ 준비됨" : "✗ 사용 안 함", PIR_SENSOR);
  Serial.printf("  부저: GPIO %d (D3)\n", BUZZER_PIN);
  Serial.printf("  시연 모드: %s\n", DEMO_MODE ? "ON (Mock)" : "OFF (실제 센서)");
  Serial.println("════════════════════════════════════════════════\n");

  Serial.println("⚠ 핀 할당 (수정 완료):");
  Serial.println("  - GPIO 15: 내장 LED ✓");
  Serial.println("  - GPIO 16: UART0 TX (시리얼 통신용) - 사용 금지!");
  Serial.println("  - GPIO 17: UART0 RX (시리얼 통신용) - 사용 금지!");
  Serial.println("  - GPIO 1: PIR 센서 (D1) ✓");
  Serial.println("  - GPIO 21: 부저 (D3) ✓\n");

  if (wifiConnected && mqttConnected) {
    Serial.println("✓ 모든 시스템 준비 완료!\n");
    Serial.println("시연 시나리오:");
    Serial.println("1. 날씨: 비 (시연 모드 강제)");
    Serial.println("2. 사람: Mock으로 감지");
    Serial.println("3. 우산: Mock으로 인식");
    Serial.println("   → 우산 없음: 빨간 LED + 부저 경고");
    Serial.println("   → 우산 있음: 초록 LED + 기분 좋은 알림\n");
    Serial.println("참고: ESP32-C6는 내장 카메라/PSRAM이 없어서");
    Serial.println("실제 객체 인식은 외장 카메라 + 별도 AI 모듈 필요\n");
  } else {
    Serial.println("✗ 일부 시스템 준비 실패!\n");
  }

  delay(2000);
}

void loop() {
  // MQTT 연결 유지
  if (wifiConnected) {
    if (!client.connected()) {
      mqttConnected = false;
      reconnect();
    }
    client.loop();
  }

  // 모든 시스템 준비된 경우에만 실행
  if (wifiConnected && mqttConnected) {
    // 날씨 확인 (5분마다)
    static unsigned long lastWeatherCheck = 0;
    if (millis() - lastWeatherCheck >= 300000 || lastWeatherCheck == 0) {
      lastWeatherCheck = millis();

      if (DEMO_MODE) {
        fetch_weather_mock();
      } else {
        fetch_weather_kma();
      }

      if (mqttConnected) {
        publish_weather();
      }
    }

    // 객체 감지 (시연용 Mock, 2초마다)
    static unsigned long lastDetection = 0;
    if (millis() - lastDetection >= 2000) {
      lastDetection = millis();

      if (DEMO_MODE) {
        // Mock 감지
        personDetected = demoPersonDetected;
        umbrellaDetected = demoUmbrellaDetected;

        Serial.println("────────────────────────────────────────────────");
        Serial.println("객체 감지 (Mock):");
        Serial.printf("  사람 감지: %s\n", personDetected ? "예" : "아니오");
        Serial.printf("  우산 감지: %s\n", umbrellaDetected ? "예" : "아니오");
        Serial.println("────────────────────────────────────────────────\n");
      } else if (USE_PIR_SENSOR && pirSensorReady) {
        // PIR 센서 사용
        personDetected = read_pir_sensor();
        umbrellaDetected = false;  // PIR은 사람만 감지

        Serial.println("────────────────────────────────────────────────");
        Serial.println("객체 감지 (PIR 센서 - GPIO 1):");
        Serial.printf("  사람 감지: %s\n", personDetected ? "예" : "아니오");
        Serial.println("────────────────────────────────────────────────\n");
      }

      if (mqttConnected) {
        publish_detection();
      }

      // 경고 로직 체크
      check_alert_logic();
    }

    // 경고 상태 유지 (2초 간격)
    if (isRaining && personDetected && !umbrellaDetected) {
      set_alert(true);
    } else {
      set_alert(false);
    }
  }

  delay(100);
}

// ========== Wi-Fi 설정 ==========
void setup_wifi() {
  Serial.println("════════════════════════════════════════════════");
  Serial.println("Wi-Fi 연결 시작");
  Serial.println("════════════════════════════════════════════════");

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
    Serial.printf("  IP: %s\n", WiFi.localIP().toString().c_str());
    Serial.printf("  신호: %d dBm\n", WiFi.RSSI());
  } else {
    Serial.println("\n✗ Wi-Fi 연결 실패!");
    wifiConnected = false;
  }

  Serial.println("════════════════════════════════════════════════\n");
}

// ========== MQTT 설정 ==========
void setup_mqtt() {
  Serial.println("════════════════════════════════════════════════");
  Serial.println("MQTT Broker 연결");
  Serial.println("════════════════════════════════════════════════");

  client.setServer(mqtt_server, mqtt_port);
  reconnect();

  Serial.println("════════════════════════════════════════════════\n");
}

void reconnect() {
  if (!wifiConnected) return;

  String clientId = "PSEE-SmartEntryway-C6-" + String(random(0xffff), HEX);

  if (client.connect(clientId.c_str())) {
    mqttConnected = true;
    Serial.println("✓ MQTT 접속 성공!");
  } else {
    mqttConnected = false;
    Serial.printf("✗ MQTT 접속 실패 (rc=%d)\n", client.state());
  }
}

// ========== 하드웨어 설정 ==========
void setup_hardware() {
  // 내장 LED (GPIO 15)
  pinMode(USER_LED, OUTPUT);
  digitalWrite(USER_LED, LOW);  // 초기: 초록색

  // 부저 (GPIO 21, PWM)
  ledcSetup(0, 2000, 8);  // 채널 0, 2000Hz, 8비트 해상도
  ledcAttachPin(BUZZER_PIN, 0);
  ledcWrite(0, 0);  // 초기: 꺼짐

  Serial.println("════════════════════════════════════════════════");
  Serial.println("하드웨어 초기화 (수정 완료)");
  Serial.println("════════════════════════════════════════════════");
  Serial.printf("  내장 LED: GPIO %d\n", USER_LED);
  Serial.printf("  부저: GPIO %d (D3)\n", BUZZER_PIN);
  Serial.printf("  PIR 센서: GPIO %d (D1) %s\n", PIR_SENSOR, pirSensorReady ? "✓ 준비됨" : "사용 안 함");
  Serial.printf("  UART0: GPIO 16(TX), 17(RX) - 시리얼 통신용 (사용 금지)\n");
  Serial.println("════════════════════════════════════════════════\n");
}

// ========== PIR 센서 설정 ==========
void setup_pir_sensor() {
  Serial.println("════════════════════════════════════════════════");
  Serial.println("PIR 센서 초기화 (수정 완료)");
  Serial.println("════════════════════════════════════════════════");

  pinMode(PIR_SENSOR, INPUT);
  pirSensorReady = true;

  Serial.println("✓ PIR 센서 준비 완료!");
  Serial.printf("  PIR 센서: GPIO %d (D1)\n", PIR_SENSOR);
  Serial.println("════════════════════════════════════════════════\n");
}

// ========== PIR 센서 읽기 ==========
bool read_pir_sensor() {
  return digitalRead(PIR_SENSOR) == HIGH;
}

// ========== 날씨 API (KMA) ==========
bool fetch_weather_kma() {
  // TODO: 기상청 API 구현
  // 현재는 Mock으로 대체
  Serial.println("⚠ 실제 KMA API 구현 필요 (현재 Mock 사용)");
  return fetch_weather_mock();
}

bool fetch_weather_mock() {
  // 시연용 Mock 데이터
  isRaining = demoRaining;
  weatherCondition = isRaining ? "비" : "맑음";
  temperature = 18.0 + (random(-50, 50) / 10.0);

  Serial.println("════════════════════════════════════════════════");
  Serial.println("날씨 정보 (Mock)");
  Serial.println("════════════════════════════════════════════════");
  Serial.printf("  날씨: %s\n", weatherCondition.c_str());
  Serial.printf("  온도: %.1f°C\n", temperature);
  Serial.printf("  비 여부: %s\n", isRaining ? "예" : "아니오");
  Serial.println("════════════════════════════════════════════════\n");

  return true;
}

// ========== 경고 로직 ==========
void check_alert_logic() {
  bool needAlert = isRaining && personDetected && !umbrellaDetected;

  if (needAlert && mqttConnected) {
    // 경고 메시지 생성
    String message = "오늘 오후 3시에 비 소식이 있습니다! 우산을 꼭 챙기세요.";
    publish_alert(message);

    // LLM 메시지 생성 (Mock)
    String llmMessage = generate_llm_message_mock();
    publish_llm_message(llmMessage);
  } else if (!needAlert && umbrellaDetected && mqttConnected) {
    // 준비 완료 메시지
    String message = "준비 완료! 좋은 하루 되세요.";
    publish_alert(message);
  }
}

void set_alert(bool alertActive) {
  if (alertActive) {
    digitalWrite(USER_LED, HIGH);  // 빨간색
    play_alert_sound(true);
  } else {
    digitalWrite(USER_LED, LOW);   // 초록색
    play_alert_sound(false);
  }
}

void play_alert_sound(bool warning) {
  if (warning) {
    // 경고음: 빠른 비프 (삐빅!)
    ledcWriteTone(0, 2000);
    delay(100);
    ledcWriteTone(0, 0);
    delay(100);
  } else {
    // 알림음: 기분 좋은 멜로디 (딩동댕~)
    ledcWriteTone(0, 523);  // C5
    delay(200);
    ledcWriteTone(0, 659);  // E5
    delay(200);
    ledcWriteTone(0, 784);  // G5
    delay(200);
    ledcWriteTone(0, 0);
  }
}

// ========== MQTT 메시지 전송 ==========
void publish_weather() {
  StaticJsonDocument<200> doc;
  doc["condition"] = weatherCondition;
  doc["temperature"] = temperature;
  doc["is_raining"] = isRaining;
  doc["device"] = "XIAO-ESP32C6";
  doc["timestamp"] = millis();

  String jsonStr;
  serializeJson(doc, jsonStr);

  client.publish(mqtt_topic_weather, jsonStr.c_str());
  Serial.printf("MQTT [날씨]: %s\n", jsonStr.c_str());
}

void publish_detection() {
  StaticJsonDocument<200> doc;
  doc["person_detected"] = personDetected;
  doc["umbrella_detected"] = umbrellaDetected;
  doc["method"] = DEMO_MODE ? "mock" : (USE_PIR_SENSOR ? "pir_sensor_gpio1" : "none");
  doc["device"] = "XIAO-ESP32C6";
  doc["timestamp"] = millis();

  String jsonStr;
  serializeJson(doc, jsonStr);

  client.publish(mqtt_topic_detection, jsonStr.c_str());
  Serial.printf("MQTT [감지]: %s\n", jsonStr.c_str());
}

void publish_alert(String message) {
  StaticJsonDocument<200> doc;
  doc["message"] = message;
  doc["alert_type"] = (isRaining && personDetected && !umbrellaDetected) ? "warning" : "info";
  doc["device"] = "XIAO-ESP32C6";
  doc["timestamp"] = millis();

  String jsonStr;
  serializeJson(doc, jsonStr);

  client.publish(mqtt_topic_alert, jsonStr.c_str());
  Serial.printf("MQTT [알림]: %s\n", jsonStr.c_str());
}

void publish_llm_message(String message) {
  StaticJsonDocument<500> doc;
  doc["message"] = message;
  doc["model"] = "claude-3-5-sonnet-20240620";
  doc["device"] = "XIAO-ESP32C6";
  doc["timestamp"] = millis();

  String jsonStr;
  serializeJson(doc, jsonStr);

  client.publish(mqtt_topic_llm, jsonStr.c_str());
  Serial.printf("MQTT [LLM]: %s\n", jsonStr.c_str());
}

// ========== LLM 메시지 생성 (Mock) ==========
String generate_llm_message_mock() {
  if (isRaining && !umbrellaDetected) {
    return "오늘 오후 3시에 비 소식이 있습니다! 우산을 꼭 챙기세요. 감기 조심하세요! ☔";
  } else if (umbrellaDetected) {
    return "준비 완료! 우산을 잘 챙기셨네요. 좋은 하루 되세요! 🌟";
  } else {
    return "오늘은 날씨가 좋습니다! 안전하게 다녀오세요! ☀️";
  }
}