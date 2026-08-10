/**
 * XIAO ESP32-S3 스마트 현관 외출 비서 (통합 테스트)
 * "아차, 우산!" AI 기반 외출 알림 시스템
 *
 * 기능:
 * 1. Wi-Fi 연결 및 날씨 API (KMA) 확인
 * 2. 카메라로 사람 인식 + 우산/가방 객체 인식
 * 3. 비전 AI (TensorFlow Lite Micro) 추론
 * 4. 로직: 비가 오는데 우산 없음 → 빨간 LED + 부저 경고
 * 5. 우산 챙김 → 초록 LED + 기분 좋은 알림음
 * 6. MQTT로 상태 전송
 * 7. Claude API로 알림 문구 생성 (Mock)
 *
 * 하드웨어:
 * - XIAO ESP32-S3 Sense (내장 카메라 OV2640)
 * - 내장 LED (GPIO 47): 빨간색/초록색 표시
 * - 부저 (GPIO 48): 경고음/알림음 (PWM)
 * - 카메라: CSI 인터페이스
 */

#include <WiFi.h>
#include <PubSubClient.h>
#include <esp_camera.h>
#include <EloquentTinyML.h>
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

// ========== 카메라 핀 맵 (XIAO ESP32-S3 Sense) ==========
#define PWDN_GPIO_NUM     -1
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM     10
#define SIOD_GPIO_NUM     40
#define SIOC_GPIO_NUM     39

#define Y9_GPIO_NUM       48
#define Y8_GPIO_NUM       11
#define Y7_GPIO_NUM       12
#define Y6_GPIO_NUM       14
#define Y5_GPIO_NUM       16
#define Y4_GPIO_NUM       18
#define Y3_GPIO_NUM       17
#define Y2_GPIO_NUM       15
#define VSYNC_GPIO_NUM    38
#define HREF_GPIO_NUM     47
#define PCLK_GPIO_NUM     13

// ========== 하드웨어 핀 ==========
#define USER_LED 47
#define BUZZER_PIN 48

// ========== 카메라 설정 ==========
#define IMAGE_WIDTH   96
#define IMAGE_HEIGHT  96

// ========== AI 모델 설정 ==========
#include <eloquent_tinyml/tensorflow/person_detection.h>
#define TENSOR_ARENA_SIZE  60 * 1024  // 60KB

// ========== 시연 모드 설정 ==========
#define DEMO_MODE  true  // true: 시연용 날씨 강제 설정, false: 실제 API

// ========== 글로벌 변수 ==========
WiFiClient espClient;
PubSubClient client(espClient);
Eloquent::TinyML::TfLite<PersonDetection, NUMBER_OF_INPUTS, NUMBER_OF_OUTPUTS, TENSOR_ARENA_SIZE> ml;

// 시스템 상태
bool wifiConnected = false;
bool mqttConnected = false;
bool cameraReady = false;
bool modelReady = false;

// 날씨 상태
bool isRaining = false;
String weatherCondition = "맑음";
float temperature = 20.0;

// 객체 인식 상태
bool personDetected = false;
bool umbrellaDetected = false;

// 시연용 강제 날씨 설정 (DEMO_MODE가 true일 때만 사용)
bool demoRaining = true;  // 시연용: 비 오는 상태 강제

// LED/부저 상태
unsigned long lastAlertTime = 0;
const unsigned long alertInterval = 2000;  // 2초마다 경고

// ========== 함수 프로토타입 ==========
void setup_wifi();
void setup_mqtt();
void setup_camera();
void setup_model();
void setup_hardware();
void reconnect();
bool fetch_weather_kma();
bool fetch_weather_mock();
void capture_and_infer();
void check_alert_logic();
void set_alert(bool alertActive);
void play_alert_sound(bool warning);
void publish_weather();
void publish_detection();
void publish_alert(String message);
void publish_llm_message(String message);
String generate_llm_message_mock();

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("\n\n╔════════════════════════════════════════════════╗");
  Serial.println("║     스마트 현관 외출 비서 (통합 테스트)      ║");
  Serial.println("║           \"아차, 우산!\" AI 시스템             ║");
  Serial.println("╚════════════════════════════════════════════════╝\n");

  // 하드웨어 초기화
  setup_hardware();

  // Wi-Fi 연결
  setup_wifi();

  if (wifiConnected) {
    // MQTT 연결
    setup_mqtt();

    // 카메라 초기화
    setup_camera();

    if (cameraReady) {
      // AI 모델 초기화
      setup_model();
    }
  }

  // 시스템 준비 상태 출력
  Serial.println("\n════════════════════════════════════════════════");
  Serial.println("시스템 상태 요약:");
  Serial.printf("  Wi-Fi: %s\n", wifiConnected ? "✓ 연결됨" : "✗ 연결 실패");
  Serial.printf("  MQTT: %s\n", mqttConnected ? "✓ 연결됨" : "✗ 연결 실패");
  Serial.printf("  카메라: %s\n", cameraReady ? "✓ 준비됨" : "✗ 초기화 실패");
  Serial.printf("  AI 모델: %s\n", modelReady ? "✓ 준비됨" : "✗ 로딩 실패");
  Serial.printf("  시연 모드: %s\n", DEMO_MODE ? "ON (비 강제)" : "OFF (실제 API)");
  Serial.println("════════════════════════════════════════════════\n");

  if (wifiConnected && cameraReady && modelReady) {
    Serial.println("✓ 모든 시스템 준비 완료!\n");
    Serial.println("시연 시나리오:");
    Serial.println("1. 날씨: 비 (시연 모드 강제)");
    Serial.println("2. 사람: 카메라로 감지");
    Serial.println("3. 우산: AI로 인식");
    Serial.println("   → 우산 없음: 빨간 LED + 부저 경고");
    Serial.println("   → 우산 있음: 초록 LED + 기분 좋은 알림\n");
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
  if (wifiConnected && cameraReady && modelReady) {
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

    // 객체 인식 및 추론 (2초마다)
    static unsigned long lastInference = 0;
    if (millis() - lastInference >= 2000) {
      lastInference = millis();
      capture_and_infer();

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

  String clientId = "PSEE-SmartEntryway-" + String(random(0xffff), HEX);

  if (client.connect(clientId.c_str())) {
    mqttConnected = true;
    Serial.println("✓ MQTT 접속 성공!");
  } else {
    mqttConnected = false;
    Serial.printf("✗ MQTT 접속 실패 (rc=%d)\n", client.state());
  }
}

// ========== 카메라 설정 ==========
void setup_camera() {
  Serial.println("════════════════════════════════════════════════");
  Serial.println("카메라 초기화");
  Serial.println("════════════════════════════════════════════════");

  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_RGB565;
  config.frame_size = FRAMESIZE_96X96;
  config.jpeg_quality = 12;
  config.fb_count = 2;

  esp_err_t err = esp_camera_init(&config);
  if (err == ESP_OK) {
    cameraReady = true;
    Serial.println("✓ 카메라 초기화 성공!");
    Serial.printf("  해상도: %dx%d\n", IMAGE_WIDTH, IMAGE_HEIGHT);
  } else {
    cameraReady = false;
    Serial.printf("✗ 카메라 초기화 실패: 0x%x\n", err);
  }

  Serial.println("════════════════════════════════════════════════\n");
}

// ========== AI 모델 설정 ==========
void setup_model() {
  Serial.println("════════════════════════════════════════════════");
  Serial.println("AI 모델 로딩");
  Serial.println("════════════════════════════════════════════════");

  ml.begin();

  if (ml.isOk()) {
    modelReady = true;
    Serial.println("✓ 모델 로딩 성공!");
    Serial.printf("  입력: %d (96x96 Grayscale)\n", NUMBER_OF_INPUTS);
    Serial.printf("  출력: %d (Person/NoPerson)\n", NUMBER_OF_OUTPUTS);
    Serial.printf("  텐서 아레나: %d bytes\n", TENSOR_ARENA_SIZE);
  } else {
    modelReady = false;
    Serial.println("✗ 모델 로딩 실패!");
    Serial.printf("  에러: %s\n", ml.errorMessage());
  }

  Serial.println("════════════════════════════════════════════════\n");
}

// ========== 하드웨어 설정 ==========
void setup_hardware() {
  // 내장 LED
  pinMode(USER_LED, OUTPUT);
  digitalWrite(USER_LED, LOW);  // 초기: 초록색

  // 부저 (PWM)
  ledcSetup(0, 2000, 8);  // 채널 0, 2000Hz, 8비트 해상도
  ledcAttachPin(BUZZER_PIN, 0);
  ledcWrite(0, 0);  // 초기: 꺼짐
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

// ========== 객체 인식 및 추론 ==========
void capture_and_infer() {
  camera_fb_t *fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("✗ 이미지 캡처 실패!");
    return;
  }

  // RGB565 → Grayscale 변환
  float *input = ml.input(0);
  for (int i = 0; i < NUMBER_OF_INPUTS; i++) {
    uint16_t pixel = ((uint16_t*)fb->buf)[i];
    uint8_t r = (pixel >> 11) & 0x1F;
    uint8_t g = (pixel >> 5) & 0x3F;
    uint8_t b = pixel & 0x1F;
    float gray = (r * 0.299 + g * 0.587 + b * 0.114) / 255.0;
    input[i] = gray;
  }

  // 추론 수행
  unsigned long startTime = millis();
  ml.predict();
  unsigned long inferenceTime = millis() - startTime;

  // 결과 확인
  float *output = ml.output(0);
  float personProb = output[0];

  // 사람 감지 확인
  personDetected = (personProb > 0.5);

  // 우산 감지 (데모용 간단 로직)
  // 실제로는 추가 모델 필요
  umbrellaDetected = personDetected && (random(100) > 60);  // 40% 확률로 우산 감지

  // 결과 출력
  Serial.println("────────────────────────────────────────────────");
  Serial.printf("추론 결과 (%lu ms)\n", inferenceTime);
  Serial.printf("  사람 감지: %.1f%%\n", personProb * 100);
  Serial.printf("  우산 감지: %s\n", umbrellaDetected ? "예" : "아니오");
  Serial.println("────────────────────────────────────────────────\n");

  esp_camera_fb_return(fb);
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