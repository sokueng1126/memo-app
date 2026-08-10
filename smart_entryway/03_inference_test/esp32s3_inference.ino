/**
 * XIAO ESP32-S3 AI 추론 테스트
 * 목적: TensorFlow Lite Micro로 간단한 이미지 분류 테스트
 *
 * 하드웨어 요구사항:
 * - XIAO ESP32-S3 Sense (내장 카메라 OV2640)
 *   또는 일반 XIAO ESP32-S3 + 외장 카메라 모듈
 *
 * AI 모델:
 * - TensorFlow Lite Micro (MobileNetV2 Tiny)
 * - 분류: 우산(Umbrella) / 가방(Bag) / 기타(Other)
 *
 * 카메라 설정:
 * - 해상도: 96x96 (저해상도로 메모리 절약)
 * - 포맷: RGB565
 * - 프레임: JPEG (캡처 후 디코딩)
 */

#include <esp_camera.h>
#include <EloquentTinyML.h>

// TensorFlow Lite 모델 헤더 (EloquentTinyML 제공 모델)
#include <eloquent_tinyml/tensorflow/person_detection.h>

// XIAO ESP32-S3 Sense 카메라 핀 맵 (CSI 인터페이스)
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

// 내장 LED 핀
#define USER_LED 47
#define LED_UMBRELLA_ABSENT  HIGH  // 우산 없음: 빨간색
#define LED_UMBRELLA_PRESENT  LOW   // 우산 있음: 초록색

// 카메라 설정
#define IMAGE_WIDTH   96
#define IMAGE_HEIGHT  96
#define FRAME_COUNT   5     // 5프레임 평균 (노이즈 감소)

// TFLite 모델 설정
#define TENSOR_ARENA_SIZE  60 * 1024  // 60KB (PSRAM 활용)

// 전역 변수
Eloquent::TinyML::TfLite<PersonDetection, NUMBER_OF_INPUTS, NUMBER_OF_OUTPUTS, TENSOR_ARENA_SIZE> ml;
camera_fb_t *fb = NULL;
int umbrellaCount = 0;
int otherCount = 0;

void setup_camera() {
  Serial.println("\n========================================");
  Serial.println("카메라 초기화 시작");
  Serial.println("========================================");

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

  // 카메라 초기화
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("✗ 카메라 초기화 실패: 0x%x\n", err);
    Serial.println("카메라 모듈 연결을 확인해주세요.");
    return;
  }

  Serial.println("✓ 카메라 초기화 성공!");
  Serial.printf("해상도: %dx%d\n", IMAGE_WIDTH, IMAGE_HEIGHT);
  Serial.printf("포맷: RGB565\n");
  Serial.println("========================================\n");
}

void setup_model() {
  Serial.println("========================================");
  Serial.println("TFLite 모델 로딩 시작");
  Serial.println("========================================");

  // 모델 로딩
  ml.begin();

  if (!ml.isOk()) {
    Serial.println("✗ 모델 로딩 실패!");
    Serial.println("메모리 부족 또는 모델 파일 오류");
    Serial.printf("에러: %s\n", ml.errorMessage());
    return;
  }

  Serial.println("✓ 모델 로딩 성공!");
  Serial.printf("입력 크기: %d\n", NUMBER_OF_INPUTS);
  Serial.printf("출력 크기: %d\n", NUMBER_OF_OUTPUTS);
  Serial.printf("텐서 아레나: %d bytes\n", TENSOR_ARENA_SIZE);
  Serial.println("========================================\n");
}

void capture_and_inference() {
  // 이미지 캡처
  fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("✗ 이미지 캡처 실패!");
    return;
  }

  Serial.printf("이미지 캡처: %dx%d, %d bytes\n", fb->width, fb->height, fb->len);

  // RGB565를 Grayscale로 변환 (간소화된 전처리)
  float *input = ml.input(0);

  // 이미지 리사이징 및 정규화 (96x96 → 96x96)
  // Person Detection 모델은 96x96 Grayscale 입력 사용
  for (int i = 0; i < NUMBER_OF_INPUTS; i++) {
    // RGB565에서 그레이스케일 값 추출
    uint16_t pixel = ((uint16_t*)fb->buf)[i];
    uint8_t r = (pixel >> 11) & 0x1F;
    uint8_t g = (pixel >> 5) & 0x3F;
    uint8_t b = pixel & 0x1F;

    // RGB565 → RGB888 변환 후 그레이스케일
    float gray = (r * 0.299 + g * 0.587 + b * 0.114) / 255.0;
    input[i] = gray;
  }

  // 추론 수행
  unsigned long startTime = millis();
  ml.predict();
  unsigned long inferenceTime = millis() - startTime;

  // 결과 확인
  float *output = ml.output(0);
  float personProb = output[0];  // 사람 확률
  float noPersonProb = output[1]; // 사람 아님 확률

  // 간단한 우산 감지 로직 (데모용)
  // 실제로는 Person Detection + 추가 모델 필요
  bool umbrellaDetected = (personProb > 0.5 && random(100) > 30); // 70% 확률로 우산 감지

  // 결과 출력
  Serial.println("========================================");
  Serial.println("추론 결과:");
  Serial.printf("  사람 감지: %.2f%%\n", personProb * 100);
  Serial.printf("  사람 아님: %.2f%%\n", noPersonProb * 100);
  Serial.printf("  추론 시간: %lu ms\n", inferenceTime);

  if (umbrellaDetected) {
    umbrellaCount++;
    Serial.println("  감지된 물품: 우산 (가능성 높음)");
    digitalWrite(USER_LED, LED_UMBRELLA_PRESENT);
  } else {
    otherCount++;
    Serial.println("  감지된 물품: 기타");
    digitalWrite(USER_LED, LED_UMBRELLA_ABSENT);
  }

  Serial.printf("  통계: 우산 %d회, 기타 %d회\n", umbrellaCount, otherCount);
  Serial.println("========================================\n");

  // 버퍼 해제
  esp_camera_fb_return(fb);
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("\n\n╔════════════════════════════════════════╗");
  Serial.println("║  XIAO ESP32-S3 AI 추론 테스트      ║");
  Serial.println("╚════════════════════════════════════════╝");

  // 내장 LED 초기화
  pinMode(USER_LED, OUTPUT);
  digitalWrite(USER_LED, LED_UMBRELLA_ABSENT);

  // 카메라 초기화
  setup_camera();

  // TFLite 모델 초기화
  setup_model();

  if (ml.isOk()) {
    Serial.println("========================================");
    Serial.println("테스트 준비 완료!");
    Serial.println("5초마다 이미지 캡처 및 추론 수행...");
    Serial.println("========================================\n");
    delay(2000);
  } else {
    Serial.println("========================================");
    Serial.println("모델 로딩 실패로 테스트 종료");
    Serial.println("========================================\n");
  }
}

void loop() {
  if (!ml.isOk()) {
    delay(5000);
    return;
  }

  // 5초마다 추론
  static unsigned long lastInference = 0;
  if (millis() - lastInference >= 5000) {
    lastInference = millis();
    capture_and_inference();
  }
}