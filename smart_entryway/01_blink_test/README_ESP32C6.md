# Test 1: ESP32-C6 블링크 테스트 (Arduino CLI)

## 목적

XIAO ESP32-C6 보드의 기본 기능 테스트:
- 보드 정상 작동 여부 확인
- USB 연결 및 드라이버 확인
- Arduino CLI 환경 설정 확인
- 내장 LED 제어 확인

## 하드웨어 요구사항

- **XIAO ESP32-C6** (RISC-V 듀얼 코어)
- USB Type-C 데이터 케이블 (충전용 케이블 불가)
- PC (Windows 10/11, macOS, Linux)

## 하드웨어 사양

| 항목 | 사양 |
|------|------|
| 프로세서 | RISC-V 듀얼 코어 (160MHz HP + 20MHz LP) |
| 메모리 | 512KB SRAM + 4MB Flash |
| 무선 | 2.4GHz Wi-Fi 6, Bluetooth 5.3, Zigbee, Thread |
| 내장 LED | GPIO 15 |
| GPIO | 11개 (PWM 지원) |
| ADC | 7개 (12-bit) |
| 전압 | 3.3V 레귤레이터 |

## 소프트웨어 요구사항

- **Arduino CLI 0.35.0 이상**
- Python 3.8 이상 (선택사항, Python용 Arduino CLI)

## Arduino CLI 설치

### Windows

```powershell
# Chocolatey로 설치
choco install arduino-cli

# 또는 수동 다운로드
# https://arduino.github.io/arduino-cli/latest/installation/
```

### macOS

```bash
brew install arduino-cli
```

### Linux

```bash
# Ubuntu/Debian
sudo apt-get update
sudo apt-get install arduino-cli

# 또는 다운로드
curl -fsSL https://raw.githubusercontent.com/arduino/arduino-cli/master/install.sh | sh
```

## Arduino CLI 설정

### 1. 설정 파일 복사

```bash
# 프로젝트 폴더로 이동
cd "C:\Users\user\OneDrive\바탕 화면\부경대 피지컬 AI공모전\my_web"

# 설정 파일 복사 (선택사항)
cp arduino-cli.yaml ~/.arduino-cli.yaml  # Linux/macOS
# 또는
cp arduino-cli.yaml %USERPROFILE%\.arduino-cli.yaml  # Windows
```

### 2. 보드 매니저 업데이트

```bash
arduino-cli core update-index
```

### 3. ESP32 패키지 설치

```bash
arduino-cli core install esp32:esp32
```

### 4. 라이브러리 설치

```bash
# PubSubClient (MQTT)
arduino-cli lib install PubSubClient

# ArduinoJson (JSON)
arduino-cli lib install ArduinoJson

# ESP32 Servo (부저용)
arduino-cli lib install ESP32Servo
```

## 업로드 절차 (Arduino CLI)

### 1. 보드 식별

```bash
# 연결된 보드 목록 확인
arduino-cli board list

# 출력 예시:
# Port         Protocol Type              Board Name          FQBN
# COM3         serial   Serial Port (USB) XIAO ESP32-C6      esp32:esp32:XIAO_ESP32C6
```

### 2. FQBN 확인

XIAO ESP32-C6 FQBN:
```
esp32:esp32:XIAO_ESP32C6
```

### 3. 컴파일

```bash
cd smart_entryway/01_blink_test
arduino-cli compile --fqbn esp32:esp32:XIAO_ESP32C6 esp32c6_blink.ino
```

### 4. 업로드

```bash
# COM3 (Windows) 또는 /dev/ttyUSB0 (Linux/macOS)
arduino-cli upload -p COM3 --fqbn esp32:esp32:XIAO_ESP32C6 esp32c6_blink.ino
```

### 5. 시리얼 모니터

```bash
arduino-cli monitor -p COM3 -c baudrate=115200
```

또는 Ctrl+C로 종료 후:

```bash
arduino-cli monitor -p COM3
```

## 예상 출력

```
========================================
XIAO ESP32-C6 블링크 테스트 시작!
========================================
보드: XIAO ESP32-C6 (RISC-V)
내장 LED 핀: GPIO 15
블링크 간격: 500ms
메모리: SRAM 512KB, Flash 4096KB
========================================

LED ON
LED OFF
LED ON
LED OFF
...
```

## 검증 기준

| 항목 | 기준 | 성공 여부 |
|------|------|----------|
| 보드 식별 | `arduino-cli board list`에서 XIAO ESP32-C6 확인 | ☐ |
| 컴파일 | 오류 없이 컴파일 성공 | ☐ |
| 업로드 | `Done uploading.` 메시지 확인 | ☐ |
| 시리얼 출력 | "LED ON/OFF" 메시지 출력 | ☐ |
| LED 동작 | 실제 LED 깜빡임 확인 | ☐ |

## 실패 시 진단

### 보드 식별 실패

**증상**: `arduino-cli board list`에 보드 없음

**원인 및 해결**:
1. USB 케이블 확인: 데이터 전송 가능한 케이블인지 확인
2. 드라이버 확인: CP210x 또는 CH340 드라이버 설치
3. Boot 버튼: 보드의 Boot 버튼을 누른 상태에서 Reset 버튼 누르기

```bash
# 드라이버 다운로드
# CP210x: https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers
# CH340: https://www.wch.cn/download/CH341SER_EXE.html
```

### 컴파일 실패

**증상**: 컴파일 오류

**원인 및 해결**:
1. ESP32 패키지 설치 확인:
   ```bash
   arduino-cli core list
   ```

2. FQBN 확인:
   ```
   esp32:esp32:XIAO_ESP32C6
   ```

3. 보드 매니저 업데이트:
   ```bash
   arduino-cli core update-index
   ```

### 업로드 실패

**증상**: `Failed uploading`

**원인 및 해결**:
1. 포트 확인: 올바른 포트 선택 (COM3, /dev/ttyUSB0)
2. Boot 모드: 보드의 Boot 버튼을 누른 상태에서 업로드
3. 보드 전원: 충분한 전원 공급 확인

```bash
# Boot 모드 진입
1. Boot 버튼 누르고 유지
2. Reset 버튼 눌렀다 떼기
3. Boot 버튼 떼기
4. 업로드 시도
```

### LED 안 켜짐

**증상**: 메시지는 출력되지만 LED가 깜빡이지 않음

**원인 및 해결**:
1. 핀 번호 확인: XIAO ESP32-C6는 GPIO 15 (코드에 적용됨)
2. 보드 모델 확인: XIAO ESP32-C6인지 확인
3. 보드 고장 가능성: 다른 보드로 테스트

### 시리얼 출력 없음

**증상**: 시리얼 모니터에 아무것도 출력되지 않음

**원인 및 해결**:
1. Baud rate 확인: 115200으로 설정
2. 시리얼 포트 확인: 올바른 포트 선택
3. 업로드 확인: 업로드가 완전히 되었는지 확인

## Arduino CLI 유용한 명령어

```bash
# 보드 정보 확인
arduino-cli board details -b esp32:esp32:XIAO_ESP32C6

# 코어 버전 확인
arduino-cli core list

# 라이브러리 검색
arduino-cli lib search PubSubClient

# 라이브러리 설치 목록
arduino-cli lib list

# 컴파일 시 자동 업로드
arduino-cli compile --upload -p COM3 --fqbn esp32:esp32:XIAO_ESP32C6 esp32c6_blink.ino

# 스케치 생성
arduino-cli sketch new my_sketch
```

## VS Code 통합 (선택사항)

### Arduino Extension 설치

1. VS Code에서 Arduino 확장 프로그램 설치
2. `Ctrl+Shift+P` → "Arduino: Board Config"
3. FQBN 입력: `esp32:esp32:XIAO_ESP32C6`
4. 포트 선택: COM3

### Arduino CLI 사용

VS Code의 Arduino 확장은 Arduino CLI를 백엔드로 사용합니다.

## 다음 단계

이 테스트가 성공하면 다음 테스트로 진행:
- **Test 2: ESP32-C6 MQTT 통신 테스트**

## 참고

- XIAO ESP32-C6 문서: https://wiki.seeedstudio.com/XIAO_ESP32C6/
- Arduino CLI 문서: https://arduino.github.io/arduino-cli/
- ESP32 Arduino Core: https://github.com/espressif/arduino-esp32

## ESP32-C6 vs ESP32-S3 차이점

| 항목 | ESP32-C6 | ESP32-S3 |
|------|----------|----------|
| 프로세서 | RISC-V 듀얼 코어 | Xtensa LX7 듀얼 코어 |
| 주파수 | 160MHz HP + 20MHz LP | 240MHz 듀얼 코어 |
| SRAM | 512KB | 512KB |
| PSRAM | 없음 | 8MB (Sense 모델) |
| Flash | 4MB | 8MB |
| 내장 LED | GPIO 15 | GPIO 47 |
| Wi-Fi | Wi-Fi 6 | Wi-Fi 4/5 |
| Bluetooth | 5.3 | 5.0 |
| 카메라 | 내장 없음 | 내장 (Sense) |
| 무선 | Wi-Fi + BT + Zigbee + Thread | Wi-Fi + BT |