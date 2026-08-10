/*
 * XIAO ESP32-S3 블링크 테스트
 * 목적: 보드 정상 작동, USB 연결, Arduino 환경 테스트
 *
 * XIAO ESP32-S3 내장 LED: GPIO 47
 * 테스트 기준:
 * - Arduino IDE에서 정상 업로드
 * - 시리얼 모니터(115200 baud)에 "LED ON/OFF" 메시지 출력
 * - 실제 LED 깜빡임 확인
 */

// XIAO ESP32-S3 내장 LED 핀 정의
#define USER_LED 47
#define BLINK_INTERVAL 500  // 0.5초 (500ms) 간격

void setup() {
  // 시리얼 통신 시작 (디버깅용)
  Serial.begin(115200);

  // 사용자 LED 핀을 출력 모드로 설정
  pinMode(USER_LED, OUTPUT);

  // LED 초기 상태: 꺼짐
  digitalWrite(USER_LED, LOW);

  // 시작 메시지
  Serial.println("\n========================================");
  Serial.println("XIAO ESP32-S3 블링크 테스트 시작!");
  Serial.println("========================================");
  Serial.printf("내장 LED 핀: GPIO %d\n", USER_LED);
  Serial.printf("블링크 간격: %dms\n", BLINK_INTERVAL);
  Serial.println("========================================\n");
  delay(1000);
}

void loop() {
  // LED 켜기
  digitalWrite(USER_LED, HIGH);
  Serial.println("LED ON");
  delay(BLINK_INTERVAL);

  // LED 끄기
  digitalWrite(USER_LED, LOW);
  Serial.println("LED OFF");
  delay(BLINK_INTERVAL);
}