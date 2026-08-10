# Test 2: ESP32-S3 MQTT 통신 테스트

## 목적

XIAO ESP32-S3의 네트워크 통신 기능 테스트:
- Wi-Fi 연결 테스트
- MQTT Broker 접속 테스트
- 메시지 송수신 테스트
- 연결 안정성 확인

## 하드웨어 요구사항

- XIAO ESP32-S3 또는 XIAO ESP32-S3 Sense
- USB Type-C 데이터 케이블
- Wi-Fi 네트워크 (ICEE)

## 네트워크 요구사항

### WiFi
- **SSID**: ICEE
- **비밀번호**: icee2026
- **대역**: 2.4GHz

### MQTT Broker
- **Broker IP**: 192.168.0.43
- **Port**: 1883 (TCP)
- **Protocol**: MQTT 3.1.1

## 설정 절차

### 1. Arduino IDE 라이브러리 설치

1. Sketch → Include Library → Manage Libraries
2. "PubSubClient" 검색
3. "PubSubClient by Nick O'Leary" 설치 (최신 버전)

### 2. 네트워크 확인

PC에서 MQTT Broker가 실행 중인지 확인:

```bash
ping 192.168.0.43
```

```powershell
# Windows에서 MQTT 포트 확인
netstat -an | findstr "1883"
```

### 3. MQTT Monitor 준비 (선택사항)

메시지 수신 확인을 위해 MQTT Monitor 실행:

```bash
# 프로젝트 폴더에서
cd mqtt-monitor
npm install
npm start PSEE/# -v
```

또는 mosquitto_sub 사용:

```bash
mosquitto_sub -h 192.168.0.43 -p 1883 -t "PSEE/#" -v
```

## 업로드 절차

1. `esp32s3_mqtt.ino` 파일을 Arduino IDE에서 엽니다
2. Upload 버튼 (→) 클릭
3. 업로드 완료 메시지 확인
4. 시리얼 모니터 열기 (돋보기 아이콘 또는 Ctrl+Shift+M)
5. Baud rate를 115200으로 설정

## 예상 출력

### 성공 시

```
╔════════════════════════════════════════╗
║   XIAO ESP32-S3 MQTT 통신 테스트    ║
╚════════════════════════════════════════╝

========================================
Wi-Fi 연결 시작
========================================
SSID: ICEE
...
✓ Wi-Fi 연결 완료!
IP 주소: 192.168.0.45
신호 강도: -45 dBm
MAC 주소: XX:XX:XX:XX:XX:XX
========================================

MQTT Broker 접속 시도: 192.168.0.43:1883
클라이언트 ID: PSEE-XIAO-S3-1234
✓ MQTT 접속 성공!
테스트 토픽: PSEE/entryway/test
상태 토픽: PSEE/entryway/status
========================================
테스트 준비 완료!
3초마다 테스트 메시지 전송...
========================================

Publish: PSEE/entryway/test -> MQTT Test Message #1 - 1234567890
  ✓ 메시지 #1 전송 성공

Publish: PSEE/entryway/test -> MQTT Test Message #2 - 1234597890
  ✓ 메시지 #2 전송 성공
...
```

### MQTT Monitor에서 수신

```
PSEE/entryway/status {"device":"XIAO-ESP32S3","status":"online","ip":"192.168.0.45"}
PSEE/entryway/test MQTT Test Message #1 - 1234567890
PSEE/entryway/test MQTT Test Message #2 - 1234597890
...
```

## 검증 기준

| 항목 | 기준 | 성공 여부 |
|------|------|----------|
| Wi-Fi 연결 | IP 주소 획득, 신호 강도 -30~-70 dBm | ☐ |
| MQTT 접속 | Broker 접속 성공, 상태 메시지 전송 | ☐ |
| 메시지 전송 | 3초 간격 메시지 전송 성공 | ☐ |
| 수신 확인 | MQTT Monitor에서 메시지 수신 | ☐ |
| 재연결 | 연결 끊김 시 자동 재접속 | ☐ |

## 실패 시 진단

### Wi-Fi 연결 실패

**증상**: `✗ Wi-Fi 연결 실패!`

**원인 및 해결**:
1. SSID 확인: "ICEE" 정확히 입력
2. 비밀번호 확인: "icee2026" 정확히 입력
3. 신호 강도 확인: Wi-Fi 신호가 약한 경우 다른 위치로 이동
4. 대역 확인: 2.4GHz 대역 사용 (5GHz 미지원)

```cpp
// 코드에서 확인
const char* ssid = "ICEE";
const char* password = "icee2026";
```

### MQTT 접속 실패

**증상**: `✗ MQTT 접속 최종 실패!`

**원인 및 해결**:
1. Broker IP 확인: `ping 192.168.0.43` 테스트
2. 포트 확인: `netstat -an | findstr "1883"` (Broker 실행 중인지)
3. 방화벽 확인:
   ```powershell
   # 방화벽 규칙 추가 (관리자 권한)
   New-NetFirewallRule -DisplayName "MQTT-1883" -Direction Inbound -LocalPort 1883 -Protocol TCP -Action Allow
   ```
4. Mosquitto 서비스 확인:
   ```powershell
   # Mosquitto 실행 중인지 확인
   tasklist | findstr mosquitto
   ```

### 메시지 전송 실패

**증상**: `✗ 메시지 #N 전송 실패`

**원인 및 해결**:
1. 연결 상태 확인: Wi-Fi/MQTT 연결이 유지되고 있는지
2. Broker 과부하 확인: 다른 장치에서도 동시에 접속하는지
3. 토픽 확인: `PSEE/entryway/test` 토픽이 올바른지

## 테스트 팁

### 1. MQTT Monitor 사용

`mqtt_tool.py` 사용:

```bash
# 모든 PSEE 토픽 구독
python mqtt_tool.py subscribe PSEE/#

# 특정 토픽만 구독
python mqtt_tool.py subscribe PSEE/entryway/#
```

### 2. Wi-Fi 신호 최적화

- 라우터 근처에서 테스트
- 금속 물체 근처 피하기
- 안테나 외부 사용 시 (GPIO14 HIGH)

### 3. 연결 안정성 테스트

- 10분 이상 연결 유지 테스트
- Wi-Fi 끊김 시 자동 재연결 확인
- MQTT 연결 끊김 시 자동 재접속 확인

## 다음 단계

이 테스트가 성공하면 다음 테스트로 진행:
- **Test 3: ESP32-S3 AI 추론 테스트**

## 참고

- PubSubClient 라이브러리: https://github.com/knolleary/pubsubclient
- MQTT 프로토콜: https://mqtt.org/
- Mosquitto Broker: https://mosquitto.org/
- 프로젝트 MQTT 가이드: `../PSEE-MQTT-GUIDE.md`

## 토픽 구조

| 토픽 | 용도 | 방향 |
|------|------|------|
| `PSEE/entryway/test` | 테스트 메시지 | ESP32 → Broker |
| `PSEE/entryway/status` | 장치 상태 | ESP32 → Broker |
| `PSEE/entryway/weather` | 날씨 정보 | Broker → ESP32 |
| `PSEE/entryway/command` | 제어 명령 | Broker → ESP32 |