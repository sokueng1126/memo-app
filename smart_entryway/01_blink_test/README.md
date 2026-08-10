# Test 1: ESP32-S3 블링크 테스트

## 목적

XIAO ESP32-S3 보드의 기본 기능 테스트:
- 보드 정상 작동 여부 확인
- USB 연결 및 드라이버 확인
- Arduino IDE 환경 설정 확인
- 내장 LED 제어 확인

## 하드웨어 요구사항

- XIAO ESP32-S3 또는 XIAO ESP32-S3 Sense
- USB Type-C 데이터 케이블 (충전용 케이블 불가)
- PC (Windows 10/11, macOS, Linux)

## 소프트웨어 요구사항

- Arduino IDE 2.0 이상
- ESP32 보드 패키지 3.0.0 이상

## 설정 절차

### 1. Arduino IDE 보드 매니저 URL 추가

Arduino IDE에서:
1. File → Preferences
2. "Additional Boards Manager URLs"에 다음 추가:
   ```
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   ```
3. OK 클릭

### 2. ESP32 보드 패키지 설치

1. Tools → Board → Boards Manager
2. "esp32" 검색
3. "esp32 by Espressif Systems" 설치 (버전 3.0.0 이상)

### 3. 보드 선택

Tools → Board → ESP32 Arduino → XIAO ESP32S3

### 4. 시리얼 포트 선택

Tools → Port에서 XIAO ESP32S3가 연결된 포트 선택 (COM3, COM4 등)

## 업로드 절차

1. `esp32s3_blink.ino` 파일을 Arduino IDE에서 엽니다
2. Upload 버튼 (→) 클릭
3. 업로드 완료 메시지 확인:
   ```
   "Done uploading."
   ```
4. 시리얼 모니터 열기 (돋보기 아이콘 또는 Ctrl+Shift+M)
5. Baud rate를 115200으로 설정

## 예상 출력

시리얼 모니터(115200 baud)에 다음 메시지가 출력되어야 합니다:

```
========================================
XIAO ESP32-S3 블링크 테스트 시작!
========================================
내장 LED 핀: GPIO 47
블링크 간격: 500ms
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
| 업로드 | Arduino IDE에서 정상 업로드 | ☐ |
| 시리얼 출력 | "LED ON/OFF" 메시지 출력 | ☐ |
| LED 동작 | 실제 LED 깜빡임 확인 | ☐ |

## 실패 시 진단

### 업로드 실패

**증상**: `A fatal error occurred: Failed to connect to ESP32`

**원인 및 해결**:
1. USB 케이블 확인: 데이터 전송 가능한 케이블인지 확인
2. 드라이버 확인: [CP210x USB to UART Bridge VCP Drivers](https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers) 설치
3. 부팅 모드: 보드의 Boot 버튼을 누른 상태에서 Reset 버튼 누르기
4. 포트 확인: 다른 포트 선택

### LED 안 켜짐

**증상**: 메시지는 출력되지만 LED가 깜빡이지 않음

**원인 및 해결**:
1. 핀 번호 확인: XIAO ESP32-S3는 GPIO 47 (코드에 이미 적용됨)
2. 보드 모델 확인: 일반 ESP32-S3와 XIAO ESP32-S3는 핀 번호가 다름
3. 보드 고장 가능성: 다른 보드로 테스트

### 시리얼 출력 없음

**증상**: 시리얼 모니터에 아무것도 출력되지 않음

**원인 및 해결**:
1. Baud rate 확인: 115200으로 설정
2. 시리얼 포트 확인: 올바른 포트 선택
3. 업로드 확인: 업로드가 완전히 되었는지 확인

## 다음 단계

이 테스트가 성공하면 다음 테스트로 진행:
- **Test 2: ESP32-S3 MQTT 통신 테스트**

## 참고

- XIAO ESP32-S3 사양: https://wiki.seeedstudio.com/XIAO_ESP32S3/
- Arduino ESP32 문서: https://docs.espressif.com/projects/arduino-esp32/en/latest/