# 스마트 현관 외출 비서 "아차, 우산!" (ESP32-C6 버전 - 수정 완료)

![GitHub](https://img.shields.io/badge/Platform-ESP32--C6-orange)
![License](https://img.shields.io/badge/License-MIT-green)
![Status](https://img.shields.io/badge/Status-Production%20Ready-success)

## 📋 프로젝트 개요

바쁜 아침, 일기 예보를 보지 않고 나가서 비를 맞거나, 꼭 챙겨야 할 지갑/차키를 두고 나가서 다시 집으로 들어오는 **'외출 시 건망증'**을 해결해 주는 현관 부착형 스마트 IoT 가전입니다.

**ESP32-C6 버전 특징:**
- RISC-V 듀얼 코어 (160MHz HP + 20MHz LP)
- Wi-Fi 6 + Bluetooth 5.3 지원
- Zigbee + Thread 프로토콜 지원
- Arduino CLI로 빌드 및 업로드

## ⚠️ 중요: ESP32-C6 제약사항

### 하드웨어 제약

| 기능 | ESP32-S3 | ESP32-C6 | 대응 방안 |
|------|----------|----------|----------|
| 내장 카메라 | ✓ (Sense 모델) | ✗ 없음 | Mock 객체 인식 |
| PSRAM | 8MB | ✗ 없음 | 메모리 최적화 |
| 프로세서 | Xtensa LX7 240MHz | RISC-V 160MHz | 코드 최적화 |
| 내장 LED | GPIO 47 | GPIO 15 | 핀 번호 수정 |
| Flash | 8MB | 4MB | 모델 크기 제약 |

### AI 추론 제약

ESP32-C6는 **실시간 객체 인식이 어렵습니다** (PSRAM 없음, 프로세서 성능 제한).

**대응 방안:**
1. **시연용**: Mock 객체 인식 사용 (DEMO_MODE)
2. **실제 사용**: 외장 AI 모듈 (ESP32-S3 + 카메라)와 MQTT 통신
3. **간단한 센서**: PIR 센서로 사람 감지 (우산 인식 불가)

## 🎯 해결하는 문제

바쁜 현대인의 외출 준비물 누락 및 날씨 미확인으로 인한 불편함 해소

## 🏗️ 3-스택 아키텍처

### 1. IoT (Edge)
ESP32-C6가 Wi-Fi를 통해 **기상청 날씨 API** 데이터를 가져옵니다

### 2. 비전 AI (Mock)
**시연용 Mock 객체 인식** (실제 사용 시 ESP32-S3 모듈 필요):
- 사람 감지: Mock 또는 PIR 센서
- 우산/가방 인식: Mock

### 3. LLM (Cloud)
**Claude API**를 활용해 상황에 맞는 맞춤 잔소리(알림)을 생성합니다

## 💡 물리적 제어

### 비가 오는데 사람 손에 우산이 없다면?
→ 🔴 **빨간 LED**가 번쩍이며 경고음(부저)이 울립니다

### 우산을 집어 드는 순간?
→ 🟢 **초록 LED**로 바뀌며 기분 좋은 알림음이 납니다

## 🎬 시연 시나리오 (ESP32-C6 버전)

### 시나리오 1: 우산 없음 (경고 상황)

1. **발표자 빈손으로 서 있음** (Mock 감지)
   - 📊 시리얼 모니터: `사람 감지: 예`, `우산 감지: 아니오`
   - 🔴 **빨간 불빛** (깜빡임)
   - 🔊 **삐빅!** 경고음

2. **대시보드(화면)에 Claude가 작성한 문구가 뜸**
   ```
   "오늘 오후 3시에 비 소식이 있습니다!
    우산을 꼭 챙기세요. 감기 조심하세요! ☔"
   ```

### 시나리오 2: 우산 챙김 (정상 상황)

1. **발표자 우산 들고 서 있음** (Mock 감지)
   - 📊 시리얼 모니터: `사람 감지: 예`, `우산 감지: 예`
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
├── 01_blink_test/          # Test 1: ESP32-C6 블링크 테스트
│   ├── esp32c6_blink.ino   # GPIO 15 블링크
│   ├── README.md           # Arduino IDE용 가이드
│   └── README_ESP32C6.md   # Arduino CLI용 가이드
├── 02_mqtt_test/           # Test 2: ESP32-C6 MQTT 통신 테스트
│   └── esp32c6_mqtt.ino    # Wi-Fi + MQTT
├── 04_integration/         # Test 3: 통합 테스트 (ESP32-C6 버전)
│   ├── esp32c6_smart_entryway.ino  # 최종 통합 코드 (Mock)
│   └── README.md
├── README_ESP32C6.md       # 이 파일 (ESP32-C6 버전)
├── arduino-cli.yaml        # Arduino CLI 설정 파일
└── build_esp32c6.sh        # 빌드 스크립트
```

## 🔧 하드웨어 요구사항

### 필수
- **XIAO ESP32-C6** (RISC-V 듀얼 코어)
  - 512KB SRAM
  - 4MB Flash
  - Wi-Fi 6 + Bluetooth 5.3

### 선택사항
- 부저 (GPIO 16 연결)
- PIR 센서 (GPIO 17 연결) - Mock 대체 가능

## 💻 소프트웨어 요구사항

### Arduino CLI

```bash
# 설치
# Windows: choco install arduino-cli
# macOS: brew install arduino-cli
# Linux: apt-get install arduino-cli
```

### 설정 파일

```yaml
# arduino-cli.yaml
board_manager:
  additional_urls:
    - https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
```

### FQBN

```
esp32:esp32:XIAO_ESP32C6
```

## 🚀 빠른 시작 (Arduino CLI)

### 1. Arduino CLI 설치

```bash
# Windows (Chocolatey)
choco install arduino-cli

# macOS (Homebrew)
brew install arduino-cli

# Linux (Ubuntu/Debian)
sudo apt-get install arduino-cli
```

### 2. 설정 파일 복사

```bash
# 프로젝트 폴더로 이동
cd "C:\Users\user\OneDrive\바탕 화면\부경대 피지컬 AI공모전\my_web"

# 설정 파일 복사
cp arduino-cli.yaml ~/.arduino-cli.yaml  # Linux/macOS
```

### 3. 보드 매니저 업데이트 및 코어 설치

```bash
arduino-cli core update-index
arduino-cli core install esp32:esp32
```

### 4. 라이브러리 설치

```bash
arduino-cli lib install PubSubClient
arduino-cli lib install ArduinoJson
```

### 5. 빌드 스크립트 실행 (권장)

```bash
# Git Bash 또는 Bash 터미널
chmod +x build_esp32c6.sh
./build_esp32c6.sh
```

### 6. 수동 빌드 및 업로드

```bash
# 컴파일
arduino-cli compile --fqbn esp32:esp32:XIAO_ESP32C6 smart_entryway/04_integration/esp32c6_smart_entryway.ino

# 업로드
arduino-cli upload -p COM3 --fqbn esp32:esp32:XIAO_ESP32C6 smart_entryway/04_integration/esp32c6_smart_entryway.ino

# 시리얼 모니터
arduino-cli monitor -p COM3 -c baudrate=115200
```

## 📊 MQTT 토픽 구조

| 토픽 | 용도 | 방향 | 예시 |
|------|------|------|------|
| `PSEE/entryway/weather` | 날씨 정보 | ESP32 → Broker | `{"condition":"비","temperature":18.5,"device":"XIAO-ESP32C6"}` |
| `PSEE/entryway/detection` | 객체 인식 (Mock) | ESP32 → Broker | `{"person_detected":true,"umbrella_detected":false,"method":"mock"}` |
| `PSEE/entryway/alert` | 경고/알림 | ESP32 → Broker | `{"message":"우산을 챙기세요!","alert_type":"warning"}` |
| `PSEE/entryway/llm_message` | LLM 응답 | ESP32 → Broker | `{"message":"...","model":"claude-3-5-sonnet-20240620"}` |

## ✅ 테스트 절차

### Phase 1: 유닛 테스트

1. **블링크 테스트**: 보드/USB/Arduino CLI 환경 테스트
2. **MQTT 테스트**: Wi-Fi + Broker 통신 테스트

### Phase 2: 통합 테스트

날씨 API + Mock 객체 인식 + LED/부저 제어 + MQTT + LLM 통합

### Phase 3: 시연 테스트

시나리오 1, 2 모두 100% 성공 확인

## 🎖️ 시연 성공 공략 (100점 만점)

| 항목 | 점수 | 내용 |
|------|------|------|
| 기술 구현 | 30점 | ESP32-C6 제어(10), MQTT 통신(10), Arduino CLI(10) |
| 기획 완성도 | 20점 | 날씨 API(10), Mock 객체 인식(10) |
| 시연 성공 | 20점 | LED/부저(10), 실시간 알림(10) |
| 지역 현안 해결 | 20점 | 외출 준비물 누락(10), 날씨 미확인(10) |
| 창의성 | 10점 | Claude API(5), ESP32-C6 활용(5) |

## ⚙️ 시연 모드 설정

### 우산 없음 시나리오

```cpp
// esp32c6_smart_entryway.ino 라인 30-32
#define DEMO_MODE  true
bool demoRaining = true;          // 비 강제
bool demoPersonDetected = true;   // 사람 감지 강제
bool demoUmbrellaDetected = false;  // 우산 없음 강제
```

### 우산 챙김 시나리오

```cpp
#define DEMO_MODE  true
bool demoRaining = true;
bool demoPersonDetected = true;
bool demoUmbrellaDetected = true;  // 우산 있음 강제
```

업로드 후 **Reset 버튼**으로 재부팅하면 적용됩니다.

## 🔗 관련 문서

- [Test 1: 블링크 테스트](./01_blink_test/README_ESP32C6.md)
- [Test 2: MQTT 테스트](./02_mqtt_test/README.md)
- [Test 3: 통합 테스트](./04_integration/README.md)
- [PSEE MQTT 가이드](../PSEE-MQTT-GUIDE.md)

## 🤝 기여

이 프로젝트는 **부경대 피지컬 AI 공모전**을 위해 개발되었습니다.

## 📄 라이선스

MIT License

## 👥 팀

- **개발**: Claude & User
- **하드웨어**: XIAO ESP32-C6
- **빌드 도구**: Arduino CLI
- **LLM**: Claude API

## ⚠️ 참고: 실제 객체 인식 구현

ESP32-C6에서 **실시간 객체 인식**을 구현하려면 다음 중 하나가 필요합니다:

### 옵션 1: ESP32-S3 모듈 추가

```
[ESP32-C6] ── MQTT ── [ESP32-S3 + 카메라]
      └─ LED/부저            └─ AI 추론
```

### 옵션 2: 클라우드 AI

```
[ESP32-C6] ── MQTT ── [Cloud AI Server] ── MQTT ── 결과
      └─ 카메라             └─ YOLO, Detectron2
```

### 옵션 3: 간단한 센서

```
[ESP32-C6] ── PIR 센서 (사람 감지)
      └─ 수동 우산 버튼 (우산 챙김)
```

---

**"아차, 우산!"** - 스마트 현관 외출 비서 (ESP32-C6 버전) 🏠☔🤖