# Test 4: ESP32-S3 통합 테스트

## 목적

모든 하드웨어 및 소프트웨어 기능 통합 테스트:
- Wi-Fi 연결 및 날씨 API 확인
- 카메라로 사람 인식
- AI 추론으로 우산/가방 객체 인식
- LED/부저로 경고 및 알림
- MQTT로 상태 전송
- Claude API로 맞춤 알림 생성 (Mock)

## 하드웨어 요구사항

### 필수
- **XIAO ESP32-S3 Sense** (내장 카메라 + 8MB PSRAM)
- USB Type-C 데이터 케이블
- 부저 (GPIO 48 연결, 선택사항)

### 네트워크
- Wi-Fi: ICEE
- MQTT Broker: 192.168.0.43:1883

## 소프트웨어 요구사항

### Arduino IDE 라이브러리

1. **PubSubClient** (MQTT 통신)
2. **ESP32 Camera** (카메라 제어)
3. **EloquentTinyML** (AI 추론)
4. **ArduinoJson** (JSON 직렬화)
5. **HTTPClient** (날씨 API)

### Arduino IDE 설정

```
Tools → Board: ESP32 Arduino → XIAO ESP32S3 (Sense)
Tools → PSRAM: OPI PSRAM
Tools → Flash Mode: QIO 80MHz
Tools → Flash Size: 8MB (MSB)
Tools → Partition Scheme: Huge APP (3MB No OTA/1MB SPIFFS)
Tools → CPU Frequency: 240MHz (WiFi/BT)
```

## 시연 시나리오 (100% 성공)

### 설정

시연 환경의 날씨 데이터를 강제로 '비(Rain)'로 설정:

```cpp
// esp32s3_smart_entryway.ino 라인 67
#define DEMO_MODE  true  // 시연 모드 ON
bool demoRaining = true;  // 비 오는 상태 강제
```

### 시나리오 1: 우산 없음 (경고 상황)

1. **발표자 빈손으로 카메라 앞 지나감**
   - 시리얼 모니터: `사람 감지: 85.2%`, `우산 감지: 아니오`
   - LED: 🔴 **빨간 불빛** (깜빡임)
   - 부저: **삐빅!** 경고음

2. **대시보드에 Claude 문구 표시**
   ```
   "오늘 오후 3시에 비 소식이 있습니다!
    우산을 꼭 챙기세요. 감기 조심하세요! ☔"
   ```

3. **MQTT 메시지 확인**
   ```
   PSEE/entryway/weather: {"condition":"비","temperature":18.5,"is_raining":true}
   PSEE/entryway/detection: {"person_detected":true,"umbrella_detected":false}
   PSEE/entryway/alert: {"message":"오늘 오후 3시에 비 소식이 있습니다!...","alert_type":"warning"}
   PSEE/entryway/llm_message: {"message":"오늘 오후 3시에 비 소식이 있습니다!...","model":"claude-3-5-sonnet-20240620"}
   ```

### 시나리오 2: 우산 챙김 (정상 상황)

1. **발표자 우산 집어 들고 카메라에 보여줌**
   - 시리얼 모니터: `사람 감지: 92.1%`, `우산 감지: 예`
   - LED: 🟢 **초록 불빛** (켜짐)
   - 부저: **딩동댕~** 기분 좋은 알림음

2. **대시보드에 Claude 문구 표시**
   ```
   "준비 완료! 우산을 잘 챙기셨네요.
    좋은 하루 되세요! 🌟"
   ```

3. **MQTT 메시지 확인**
   ```
   PSEE/entryway/detection: {"person_detected":true,"umbrella_detected":true}
   PSEE/entryway/alert: {"message":"준비 완료!...","alert_type":"info"}
   PSEE/entryway/llm_message: {"message":"준비 완료! 우산을 잘 챙기셨네요...","model":"claude-3-5-sonnet-20240620"}
   ```

## 업로드 및 실행

### 1. 업로드

1. `esp32s3_smart_entryway.ino` 파일을 Arduino IDE에서 엽니다
2. Upload 버튼 (→) 클릭
3. 업로드 완료 메시지 확인
4. 시리얼 모니터 열기 (Ctrl+Shift+M)
5. Baud rate를 115200으로 설정

### 2. MQTT Monitor 실행

```bash
# 모든 PSEE 토픽 구독
python mqtt_tool.py subscribe PSEE/#
```

### 3. 시연 준비

1. 시리얼 모니터에서 "모든 시스템 준비 완료!" 메시지 확인
2. 날씨 설정이 "비"로 되어 있는지 확인
3. MQTT Monitor에서 메시지 수신 확인

## 예상 출력

### 시작 메시지

```
╔════════════════════════════════════════════════╗
║     스마트 현관 외출 비서 (통합 테스트)      ║
║           "아차, 우산!" AI 시스템             ║
╚════════════════════════════════════════════════╝

════════════════════════════════════════════════
Wi-Fi 연결 시작
════════════════════════════════════════════════
...
✓ Wi-Fi 연결 완료!
  IP: 192.168.0.45
  신호: -45 dBm
════════════════════════════════════════════════

════════════════════════════════════════════════
MQTT Broker 연결
════════════════════════════════════════════════
✓ MQTT 접속 성공!
════════════════════════════════════════════════

════════════════════════════════════════════════
카메라 초기화
════════════════════════════════════════════════
✓ 카메라 초기화 성공!
  해상도: 96x96
════════════════════════════════════════════════

════════════════════════════════════════════════
AI 모델 로딩
════════════════════════════════════════════════
✓ 모델 로딩 성공!
  입력: 9216 (96x96 Grayscale)
  출력: 2 (Person/NoPerson)
  텐서 아레나: 61440 bytes
════════════════════════════════════════════════

════════════════════════════════════════════════
시스템 상태 요약:
  Wi-Fi: ✓ 연결됨
  MQTT: ✓ 연결됨
  카메라: ✓ 준비됨
  AI 모델: ✓ 준비됨
  시연 모드: ON (비 강제)
════════════════════════════════════════════════

✓ 모든 시스템 준비 완료!

시연 시나리오:
1. 날씨: 비 (시연 모드 강제)
2. 사람: 카메라로 감지
3. 우산: AI로 인식
   → 우산 없음: 빨간 LED + 부저 경고
   → 우산 있음: 초록 LED + 기분 좋은 알림
```

### 실행 중 메시지

```
════════════════════════════════════════════════
날씨 정보 (Mock)
════════════════════════════════════════════════
  날씨: 비
  온도: 18.5°C
  비 여부: 예
════════════════════════════════════════════════

MQTT [날씨]: {"condition":"비","temperature":18.5,"is_raining":true,"timestamp":1234567890}

────────────────────────────────────────────────
추론 결과 (245 ms)
  사람 감지: 85.2%
  우산 감지: 아니오
────────────────────────────────────────────────

MQTT [감지]: {"person_detected":true,"umbrella_detected":false,"timestamp":1234567950}
MQTT [알림]: {"message":"오늘 오후 3시에 비 소식이 있습니다! 우산을 꼭 챙기세요.","alert_type":"warning","timestamp":1234567950}
MQTT [LLM]: {"message":"오늘 오후 3시에 비 소식이 있습니다! 우산을 꼭 챙기세요. 감기 조심하세요! ☔","model":"claude-3-5-sonnet-20240620","timestamp":1234567950}
```

## 검증 기준

| 항목 | 기준 | 성공 여부 |
|------|------|----------|
| Wi-Fi 연결 | IP 주소 획득, 신호 강도 확인 | ☐ |
| MQTT 연결 | Broker 접속 성공 | ☐ |
| 날씨 확인 | Mock 데이터 수신 (비) | ☐ |
| 카메라 캡처 | 이미지 캡처 성공 | ☐ |
| AI 추론 | 사람/우산 감지 성공 | ☐ |
| LED 제어 | 빨간색(경고) / 초록색(정상) | ☐ |
| 부저 작동 | 경고음/알림음 출력 | ☐ |
| MQTT 전송 | 4개 토픽 모두 전송 | ☐ |
| LLM 메시지 | Mock 문구 생성 성공 | ☐ |
| 시연 성공 | 시나리오 1, 2 모두 성공 | ☐ |

## MQTT 토픽 구조

| 토픽 | 용도 | 방향 | 예시 |
|------|------|------|------|
| `PSEE/entryway/weather` | 날씨 정보 | ESP32 → Broker | `{"condition":"비","temperature":18.5,"is_raining":true}` |
| `PSEE/entryway/detection` | 객체 인식 | ESP32 → Broker | `{"person_detected":true,"umbrella_detected":false}` |
| `PSEE/entryway/alert` | 경고/알림 | ESP32 → Broker | `{"message":"우산을 챙기세요!","alert_type":"warning"}` |
| `PSEE/entryway/llm_message` | LLM 응답 | ESP32 → Broker | `{"message":"...","model":"claude-3-5-sonnet-20240620"}` |

## 실패 시 진단

### Wi-Fi 연결 실패

**증상**: `✗ Wi-Fi 연결 실패!`

**해결**:
1. SSID/비밀번호 확인
2. Wi-Fi 신호 확인
3. 다른 Wi-Fi로 테스트

### MQTT 연결 실패

**증상**: `✗ MQTT 접속 실패`

**해결**:
1. Broker IP 확인: `ping 192.168.0.43`
2. 방화벽 확인: 포트 1883 개방
3. Mosquitto 서비스 확인

### 카메라 초기화 실패

**증상**: `✗ 카메라 초기화 실패`

**해결**:
1. 보드 모델 확인: XIAO ESP32-S3 Sense
2. PSRAM 설정 확인: OPI PSRAM
3. 전원 확인

### AI 모델 로딩 실패

**증상**: `✗ 모델 로딩 실패`

**해결**:
1. PSRAM 설정 확인 (가장 중요!)
2. 라이브러리 설치 확인
3. 파티션 확인: Huge APP

### LED/부저 작동 안 함

**증상**: LED가 켜지지 않거나 부저 소리 안 남

**해결**:
1. 핀 번호 확인: GPIO 47 (LED), GPIO 48 (부저)
2. 연결 확인
3. 코드 로직 확인

## 시연 팁

### 1. 시연 환경 설정

```cpp
// 시연 모드 ON으로 설정
#define DEMO_MODE  true
bool demoRaining = true;  // 비 강제
```

### 2. 카메라 조명

- 밝은 조명에서 카메라 작동 확인
- 실내 조명 최소 300 lux 이상 권장

### 3. 객체 인식

- 사람은 카메라에서 1~2m 거리에서 인식
- 우산은 크고 명확하게 보여주기
- 천천히 움직이기 (빠르면 인식 실패)

### 4. MQTT Monitor 실시간 확인

```bash
# 터미널에서 실시간 메시지 확인
python mqtt_tool.py subscribe PSEE/#
```

## 다음 단계

이 테스트가 성공하면:
- **Git master 브랜치 병합**
- **최종 테스트 및 시연 준비**
- **프로젝트 완료**

## 참고

- 이전 테스트: `../01_blink_test/`, `../02_mqtt_test/`, `../03_inference_test/`
- MQTT 가이드: `../../PSEE-MQTT-GUIDE.md`
- 하드웨어 문서: `../../XIAOESP32C6.md` (ESP32-S3 유사)

## 시연 성공 공략 (100점 만점)

### 기술 구현 (30점)
- ✓ ESP32-S3 하드웨어 제어 (10점)
- ✓ TensorFlow Lite Micro 추론 (10점)
- ✓ MQTT 통신 (10점)

### 기획 완성도 (20점)
- ✓ 날씨 API 연동 (Mock) (10점)
- ✓ 객체 인식 로직 (10점)

### 시연 성공 (20점)
- ✓ LED/부저 제어 (10점)
- ✓ 실시간 알림 (10점)

### 지역 현안 해결 (20점)
- ✓ 외출 준비물 누락 해결 (10점)
- ✓ 날씨 미확인 불편 해소 (10점)

### 창의성 (10점)
- ✓ Claude API 맞춤 알림 (5점)
- ✓ 직관적인 UX (5점)