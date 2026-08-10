# Test 3: ESP32-S3 AI 추론 테스트

## 목적

XIAO ESP32-S3의 AI 추론 기능 테스트:
- 카메라 이미지 캡처 테스트
- TensorFlow Lite Micro 모델 로딩 테스트
- 실시간 추론 수행 테스트
- LED로 결과 표시 테스트

## 하드웨어 요구사항

### 필수
- **XIAO ESP32-S3 Sense** (내장 카메라 OV2640 포함)
  - 듀얼 코어 240MHz
  - 8MB PSRAM (필수!)
  - 8MB Flash

또는
- 일반 XIAO ESP32-S3 + 외장 카메라 모듈 (OV2640)
  - 별도 카메라 연결 회로 필요

### 케이블
- USB Type-C 데이터 케이블

## 소프트웨어 요구사항

### Arduino IDE 라이브러리

1. **ESP32 Camera Driver**
   - Sketch → Include Library → Manage Libraries
   - "ESP32 Camera" 검색
   - "ESP32 Camera by Espressif Systems" 설치

2. **EloquentTinyML**
   - Sketch → Include Library → Manage Libraries
   - "Eloquent TinyML" 검색
   - "Eloquent TinyML by eloquentarduino" 설치

3. **TensorFlow Lite for Microcontrollers**
   - Arduino Library Manager에서 설치

### Arduino IDE 설정

1. **PSRAM 활성화** (매우 중요!):
   - Tools → PSRAM: "OPI PSRAM"
   - Tools → Flash Mode: "QIO 80MHz"
   - Tools → Partition Scheme: "Huge APP (3MB No OTA/1MB SPIFFS)"

2. **메모리 확인**:
   - Tools → Arduino IDE 2.x에서는 메모리 사용량 표시됨
   - 전역 변수는 < 200KB 권장

## 설정 절차

### 1. 카메라 핀 확인

XIAO ESP32-S3 Sense (내장 카메라):

| 핀 | 기능 |
|----|------|
| CSI | 자동 연결 (수동 설정 불필요) |

일반 XIAO ESP32-S3 (외장 카메라):

| 핀 | 기능 |
|----|------|
| GPIO 10 | XCLK |
| GPIO 40 | SDA (I2C) |
| GPIO 39 | SCL (I2C) |
| GPIO 48-18 | Y9-Y2 (데이터) |
| GPIO 38 | VSYNC |
| GPIO 47 | HREF |
| GPIO 13 | PCLK |

### 2. 보드 설정

```
Tools → Board: ESP32 Arduino → XIAO ESP32S3 (Sense)
Tools → PSRAM: OPI PSRAM
Tools → Flash Mode: QIO 80MHz
Tools → Flash Size: 8MB (MSB)
Tools → Partition Scheme: Huge APP (3MB No OTA/1MB SPIFFS)
Tools → CPU Frequency: 240MHz (WiFi/BT)
```

### 3. 업로드

1. `esp32s3_inference.ino` 파일을 Arduino IDE에서 엽니다
2. Upload 버튼 (→) 클릭
3. 업로드 완료 메시지 확인
4. 시리얼 모니터 열기 (Ctrl+Shift+M)
5. Baud rate를 115200으로 설정

## 예상 출력

### 성공 시

```
╔════════════════════════════════════════╗
║  XIAO ESP32-S3 AI 추론 테스트      ║
╚════════════════════════════════════════╝

========================================
카메라 초기화 시작
========================================
✓ 카메라 초기화 성공!
해상도: 96x96
포맷: RGB565
========================================

========================================
TFLite 모델 로딩 시작
========================================
✓ 모델 로딩 성공!
입력 크기: 9216
출력 크기: 2
텐서 아레나: 61440 bytes
========================================

========================================
테스트 준비 완료!
5초마다 이미지 캡처 및 추론 수행...
========================================

이미지 캡처: 96x96, 18432 bytes
========================================
추론 결과:
  사람 감지: 75.32%
  사람 아님: 24.68%
  추론 시간: 245 ms
  감지된 물품: 우산 (가능성 높음)
  통계: 우산 1회, 기타 0회
========================================
```

## 검증 기준

| 항목 | 기준 | 성공 여부 |
|------|------|----------|
| 카메라 캡처 | 96x96 이미지 캡처 성공 | ☐ |
| 모델 로딩 | TFLite 모델 로딩 성공 | ☐ |
| 추론 수행 | 추론 완료 및 결과 출력 | ☐ |
| LED 표시 | 결과에 따른 LED 색상 변화 | ☐ |
| 추론 속도 | < 500ms/프레임 | ☐ |
| 메모리 사용 | 전역 변수 < 200KB | ☐ |

## 실패 시 진단

### 카메라 초기화 실패

**증상**: `✗ 카메라 초기화 실패: 0x...`

**원인 및 해결**:
1. 보드 모델 확인: XIAO ESP32-S3 Sense인지 확인
2. 핀 연결 확인: 외장 카메라일 경우 핀 매핑 확인
3. 전원 확인: 카메라에 충분한 전원 공급
4. I2C 통신 확인: 카메라와 I2C 통신 가능한지

### 모델 로딩 실패

**증상**: `✗ 모델 로딩 실패!`

**원인 및 해결**:
1. **PSRAM 확인** (가장 중요!):
   - Tools → PSRAM: "OPI PSRAM"로 설정
   - 설정하지 않으면 메모리 부족으로 실패

2. 라이브러리 확인:
   - EloquentTinyML이 설치되어 있는지 확인
   - TensorFlow Lite for Microcontrollers 설치

3. 파티션 확인:
   - Tools → Partition Scheme: "Huge APP (3MB No OTA/1MB SPIFFS)"

4. 메모리 확인:
   - 전역 변수 크기 줄이기
   - `TENSOR_ARENA_SIZE` 줄이기 (최소 40KB)

### 추론 실패

**증상**: 추론 결과가 항상 같거나 이상한 값

**원인 및 해결**:
1. 이미지 데이터 확인:
   - 캡처된 이미지가 올바른지 확인
   - RGB565 → Grayscale 변환 확인

2. 모델 입력 확인:
   - 입력 크기가 모델과 일치하는지 확인
   - 정규화가 올바르게 수행되는지 확인

3. 메모리 부족:
   - `TENSOR_ARENA_SIZE` 늘리기 (최대 100KB)
   - 다른 작업 중단

### LED 동작 안 함

**증상**: LED가 켜지지 않거나 색상이 바뀌지 않음

**원인 및 해결**:
1. 핀 번호 확인: XIAO ESP32-S3는 GPIO 47
2. 연결 확인: 내장 LED가 정상적으로 연결되어 있는지
3. PWM 확인: LED 제어가 올바른지

## 테스트 팁

### 1. 메모리 최적화

```cpp
// 텐서 아레나 크기 조정 (40KB ~ 100KB)
#define TENSOR_ARENA_SIZE 60 * 1024  // 60KB

// 이미지 해상도 낮추기 (속도 향상)
#define IMAGE_WIDTH   96   // 96x96
#define IMAGE_HEIGHT  96
```

### 2. 추론 속도 최적화

```cpp
// CPU 주파수 최대로 설정
Tools → CPU Frequency: 240MHz (WiFi/BT)

// 이미지 해상도 낮추기
#define IMAGE_WIDTH   64   // 64x64
#define IMAGE_HEIGHT  64
```

### 3. 디버깅

```cpp
// 메모리 사용량 출력
Serial.printf("Free heap: %d bytes\n", ESP.getFreeHeap());
Serial.printf("Free PSRAM: %d bytes\n", ESP.getFreePsram());
```

## 다음 단계

이 테스트가 성공하면 다음 테스트로 진행:
- **Test 4: 통합 테스트 (날씨 + 객체 인식 + LED/부저 제어)**

## 참고

- EloquentTinyML 문서: https://github.com/eloquentarduino/EloquentTinyML
- TensorFlow Lite Micro: https://www.tensorflow.org/lite/microcontrollers
- ESP32 Camera: https://github.com/espressif/esp32-camera
- XIAO ESP32-S3 Sense: https://wiki.seeedstudio.com/XIAO_ESP32S3_Sense/

## AI 모델 정보

### 현재 모델: Person Detection
- **입력**: 96x96 Grayscale 이미지 (9216 픽셀)
- **출력**: 2개 클래스 (사람 / 사람 아님)
- **크기**: 약 150KB
- **텐서 아레나**: 60KB

### 향후 개선
- 우산/가방 객체 인식 모델 학습 (Edge Impulse 활용)
- 양자화 모델 (INT8) 적용으로 메모리 절약
- CNN 기반 맞춤 모델 개발

## 핀 맵 (XIAO ESP32-S3 Sense)

| 기능 | 핀 | 설명 |
|------|-----|------|
| 내장 LED | GPIO 47 | 결과 표시 (빨간색=우산X, 초록색=우산O) |
| 카메라 | CSI | 내장 OV2640 (자동 연결) |
| UART0 | GPIO 44/43 | 시리얼 디버깅 (USB) |
| PSRAM | 내장 | 8MB (AI 추론용) |