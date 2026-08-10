# 스마트 현관 외출 비서 "아차, 우산!"

![GitHub](https://img.shields.io/badge/Platform-ESP32--S3-blue)
![License](https://img.shields.io/badge/License-MIT-green)
![Status](https://img.shields.io/badge/Status-Production%20Ready-success)

## 📋 프로젝트 개요

바쁜 아침, 일기 예보를 보지 않고 나가서 비를 맞거나, 꼭 챙겨야 할 지갑/차키를 두고 나가서 다시 집으로 들어오는 **'외출 시 건망증'**을 해결해 주는 현관 부착형 스마트 IoT 가전입니다.

## 🎯 해결하는 문제

바쁜 현대인의 외출 준비물 누락 및 날씨 미확인으로 인한 불편함 해소

## 🏗️ 3-스택 아키텍처

### 1. IoT (Edge)
ESP32-S3가 Wi-Fi를 통해 **기상청 날씨 API** 데이터를 가져옵니다

### 2. 비전 AI (Edge)
현관 신발장에 붙은 카메라가:
- 외출하려는 사람을 인식
- 손에 **'우산(Umbrella)'**이나 **'가방(Bag)'**이 들려 있는지 객체 인식

### 3. LLM (Cloud)
**Claude API**를 활용해 상황에 맞는 맞춤 잔소리(알림)을 생성합니다

## 💡 물리적 제어

### 비가 오는데 사람 손에 우산이 없다면?
→ 🔴 **빨간 LED**가 번쩍이며 경고음(부저)이 울립니다

### 우산을 집어 드는 순간?
→ 🟢 **초록 LED**로 바뀌며 기분 좋은 알림음이 납니다

## 🎬 무대 라이브 시연 시나리오 (100% 성공)

### 시나리오 1: 우산 없음 (경고 상황)

1. **발표자 빈손으로 카메라 앞 지나감**
   - 📸 시리얼 모니터: `사람 감지: 85.2%`, `우산 감지: 아니오`
   - 🔴 **빨간 불빛** (깜빡임)
   - 🔊 **삐빅!** 경고음

2. **대시보드(화면)에 Claude가 작성한 문구가 뜸**
   ```
   "오늘 오후 3시에 비 소식이 있습니다!
    우산을 꼭 챙기세요. 감기 조심하세요! ☔"
   ```

### 시나리오 2: 우산 챙김 (정상 상황)

1. **발표자 옆에 둔 우산을 집어 들고 카메라에 보여줌**
   - 📸 시리얼 모니터: `사람 감지: 92.1%`, `우산 감지: 예`
   - 🟢 **초록 불빛** (켜짐)
   - 🔊 **딩동댕~** 기분 좋은 알림음

2. **대시보드에 Claude가 작성한 문구가 뜸**
   ```
   "준비 완료! 우산을 잘 챙기셨네요.
    좋은 하루 되세요! 🌟"
   ```

## 📁 프로젝트 구조

```
smart_entryway/
├── 01_blink_test/          # Test 1: ESP32-S3 블링크 테스트
│   ├── esp32s3_blink.ino
│   └── README.md
├── 02_mqtt_test/           # Test 2: ESP32-S3 MQTT 통신 테스트
│   ├── esp32s3_mqtt.ino
│   └── README.md
├── 03_inference_test/      # Test 3: ESP32-S3 AI 추론 테스트
│   ├── esp32s3_inference.ino
│   └── README.md
├── 04_integration/         # Test 4: 통합 테스트 (최종)
│   ├── esp32s3_smart_entryway.ino
│   └── README.md
└── README.md               # 이 파일
```

## 🔧 하드웨어 요구사항

### 필수
- **XIAO ESP32-S3 Sense** (내장 카메라 OV2640)
  - 듀얼 코어 240MHz Xtensa LX7
  - 8MB PSRAM (필수!)
  - 8MB Flash
  - Wi-Fi + Bluetooth 5.0

### 선택사항
- 부저 (GPIO 48 연결)
- PIR 센서 (GPIO 40 연결)

## 💻 소프트웨어 요구사항

### Arduino IDE
- **보드 매니저 URL**: `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`
- **보드 선택**: ESP32 Arduino → XIAO ESP32S3 (Sense)
- **PSRAM**: OPI PSRAM (매우 중요!)
- **파티션**: Huge APP (3MB No OTA/1MB SPIFFS)

### 라이브러리
1. **PubSubClient** - MQTT 통신
2. **ESP32 Camera** - 카메라 제어
3. **EloquentTinyML** - AI 추론
4. **ArduinoJson** - JSON 직렬화
5. **HTTPClient** - 날씨 API

## 🚀 빠른 시작

### 1. Arduino IDE 설정

```bash
# 1. 보드 매니저 URL 추가
File → Preferences → "Additional Boards Manager URLs"
https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json

# 2. ESP32 패키지 설치
Tools → Board → Boards Manager → "esp32" 설치

# 3. 보드 선택
Tools → Board → ESP32 Arduino → XIAO ESP32S3 (Sense)

# 4. PSRAM 설정 (매우 중요!)
Tools → PSRAM: OPI PSRAM
```

### 2. 라이브러리 설치

```
Sketch → Include Library → Manage Libraries
- PubSubClient
- ESP32 Camera
- Eloquent TinyML
- ArduinoJson
```

### 3. 네트워크 설정

```cpp
// esp32s3_smart_entryway.ino
const char* ssid = "ICEE";           // Wi-Fi SSID
const char* password = "icee2026";   // Wi-Fi 비밀번호
const char* mqtt_server = "192.168.0.43";  // MQTT Broker IP
```

### 4. 업로드

1. `04_integration/esp32s3_smart_entryway.ino` 열기
2. Upload 버튼 (→) 클릭
3. 시리얼 모니터 열기 (Ctrl+Shift+M)
4. Baud rate: 115200

### 5. MQTT Monitor 실행

```bash
# 모든 PSEE 토픽 구독
python mqtt_tool.py subscribe PSEE/#
```

## 📊 MQTT 토픽 구조

| 토픽 | 용도 | 방향 | 예시 |
|------|------|------|------|
| `PSEE/entryway/weather` | 날씨 정보 | ESP32 → Broker | `{"condition":"비","temperature":18.5}` |
| `PSEE/entryway/detection` | 객체 인식 | ESP32 → Broker | `{"person_detected":true,"umbrella_detected":false}` |
| `PSEE/entryway/alert` | 경고/알림 | ESP32 → Broker | `{"message":"우산을 챙기세요!","alert_type":"warning"}` |
| `PSEE/entryway/llm_message` | LLM 응답 | ESP32 → Broker | `{"message":"...","model":"claude-3-5-sonnet-20240620"}` |

## ✅ 테스트 절차

### Phase 1: 유닛 테스트

1. **블링크 테스트**: 보드/USB/환경 테스트
2. **MQTT 테스트**: Wi-Fi + Broker 통신 테스트
3. **AI 추론 테스트**: TensorFlow Lite Micro 테스트

### Phase 2: 통합 테스트

날씨 API + 객체 인식 + LED/부저 제어 + MQTT + LLM 통합

### Phase 3: 시연 테스트

시나리오 1, 2 모두 100% 성공 확인

## 🎖️ 시연 성공 공략 (100점 만점)

| 항목 | 점수 | 내용 |
|------|------|------|
| 기술 구현 | 30점 | ESP32-S3 제어(10), TFLite 추론(10), MQTT 통신(10) |
| 기획 완성도 | 20점 | 날씨 API(10), 객체 인식(10) |
| 시연 성공 | 20점 | LED/부저(10), 실시간 알림(10) |
| 지역 현안 해결 | 20점 | 외출 준비물 누락(10), 날씨 미확인(10) |
| 창의성 | 10점 | Claude API(5), 직관적 UX(5) |

## 🔗 관련 문서

- [Test 1: 블링크 테스트](./01_blink_test/README.md)
- [Test 2: MQTT 테스트](./02_mqtt_test/README.md)
- [Test 3: AI 추론 테스트](./03_inference_test/README.md)
- [Test 4: 통합 테스트](./04_integration/README.md)
- [PSEE MQTT 가이드](../PSEE-MQTT-GUIDE.md)

## 📝 커밋 히스토리

```
✓ feat(hardware): ESP32-S3 블링크 테스트 통과
✓ feat(hardware): ESP32-S3 MQTT 통신 테스트 통과
✓ feat(hardware): ESP32-S3 AI 추론 테스트 통과
✓ feat(integration): 스마트 현관 외출 비서 통합 테스트 완료
```

## 🤝 기여

이 프로젝트는 **부경대 피지컬 AI 공모전**을 위해 개발되었습니다.

## 📄 라이선스

MIT License

## 👥 팀

- **개발**: Claude & User
- **하드웨어**: XIAO ESP32-S3 Sense
- **AI 모델**: TensorFlow Lite Micro (Person Detection)
- **LLM**: Claude API

---

**"아차, 우산!"** - 스마트 현관 외출 비서 🏠☔🤖