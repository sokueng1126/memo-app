#!/bin/bash
# XIAO ESP32-C6 빌드 및 업로드 스크립트 (수정 완료)
# Arduino CLI 사용

set -e  # 오류 발생 시 중지

# 설정
PROJECT_DIR="smart_entryway"
FQBN="esp32:esp32:XIAO_ESP32C6"
BAUD_RATE=115200

# 색상 출력
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${GREEN}========================================${NC}"
echo -e "${GREEN}  XIAO ESP32-C6 빌드 스크립트${NC}"
echo -e "${GREEN}========================================${NC}\n"

# 테스트 선택
echo "테스트를 선택하세요:"
echo "1) 블링크 테스트 (01_blink_test)"
echo "2) MQTT 통신 테스트 (02_mqtt_test)"
echo "3) 통합 테스트 (04_integration)"
read -p "선택 (1-3): " choice

case $choice in
  1)
    INO_FILE="01_blink_test/esp32c6_blink.ino"
    TEST_NAME="블링크 테스트"
    ;;
  2)
    INO_FILE="02_mqtt_test/esp32c6_mqtt.ino"
    TEST_NAME="MQTT 통신 테스트"
    ;;
  3)
    INO_FILE="04_integration/esp32c6_smart_entryway.ino"
    TEST_NAME="통합 테스트"
    ;;
  *)
    echo -e "${RED}잘못된 선택${NC}"
    exit 1
    ;;
esac

INO_PATH="${PROJECT_DIR}/${INO_FILE}"
SKETCH_DIR="$(dirname "$INO_PATH")"

if [ ! -f "$INO_PATH" ]; then
  echo -e "${RED}파일을 찾을 수 없습니다: $INO_PATH${NC}"
  exit 1
fi

if [ ! -d "$SKETCH_DIR" ]; then
  echo -e "${RED}스케치 폴더를 찾을 수 없습니다: $SKETCH_DIR${NC}"
  exit 1
fi

echo -e "${GREEN}파일: $INO_PATH${NC}"
echo -e "${GREEN}폴더: $SKETCH_DIR${NC}"

# 포트 찾기
echo -e "\n${YELLOW}보드 포트를 찾는 중...${NC}"
PORT=$(arduino-cli board list | grep -i "xiao.*c6\|xiao.*esp32.*c6" | head -1 | awk '{print $1}')

if [ -z "$PORT" ]; then
  echo -e "${YELLOW}자동으로 포트를 찾을 수 없습니다.${NC}"
  echo -e "${YELLOW}ESP32-C6 보드 연결 확인${NC}"
  read -p "포트를 입력하세요 (예: COM3 또는 /dev/ttyUSB0): " PORT
fi

if [ -z "$PORT" ]; then
  echo -e "${RED}포트가 설정되지 않았습니다.${NC}"
  exit 1
fi

echo -e "${GREEN}찾은 포트: $PORT${NC}"

# FQBN 확인
echo -e "\n${YELLOW}FQBN 확인 중...${NC}"
if arduino-cli board details -b "$FQBN" > /dev/null 2>&1; then
  echo -e "${GREEN}✓ FQBN 유효: $FQBN${NC}"
else
  echo -e "${YELLOW}⚠ FQBN 확인 실패, 대안 FQBN 테스트...${NC}"
  ALT_FQBN="esp32:esp32:xiao_c6"
  if arduino-cli board details -b "$ALT_FQBN" > /dev/null 2>&1; then
    echo -e "${GREEN}✓ 대안 FQBN 유효: $ALT_FQBN${NC}"
    echo -e "${YELLOW}FQBN을 $ALT_FQBN로 변경하시겠습니까? (y/n)${NC}"
    read -p "> " fqbn_confirm
    if [ "$fqbn_confirm" = "y" ] || [ "$fqbn_confirm" = "Y" ]; then
      FQBN="$ALT_FQBN"
      echo -e "${GREEN}FQBN 변경됨: $FQBN${NC}"
    fi
  else
    echo -e "${RED}✗ FQBN 확인 실패, 계속 진행합니다...${NC}"
  fi
fi

# 컴파일
echo -e "\n${GREEN}========================================${NC}"
echo -e "${GREEN}  컴파일 시작: $TEST_NAME${NC}"
echo -e "${GREEN}========================================${NC}\n"
echo -e "명령어: arduino-cli compile --fqbn $FQBN $SKETCH_DIR\n"

arduino-cli compile --fqbn "$FQBN" "$SKETCH_DIR"

if [ $? -eq 0 ]; then
  echo -e "\n${GREEN}✓ 컴파일 성공!${NC}\n"
else
  echo -e "\n${RED}✗ 컴파일 실패!${NC}\n"
  exit 1
fi

# 업로드 확인
read -p "업로드하시겠습니까? (y/n): " upload_confirm

if [ "$upload_confirm" = "y" ] || [ "$upload_confirm" = "Y" ]; then
  # 업로드
  echo -e "\n${GREEN}========================================${NC}"
  echo -e "${GREEN}  업로드 시작${NC}"
  echo -e "${GREEN}========================================${NC}\n"
  echo -e "명령어: arduino-cli upload -p $PORT --fqbn $FQBN $SKETCH_DIR\n"

  arduino-cli upload -p "$PORT" --fqbn "$FQBN" "$SKETCH_DIR"

  if [ $? -eq 0 ]; then
    echo -e "\n${GREEN}✓ 업로드 성공!${NC}\n"
  else
    echo -e "\n${RED}✗ 업로드 실패!${NC}"
    echo -e "${YELLOW}해결 방법:${NC}"
    echo -e "1. Boot 버튼을 누른 상태에서 Reset 버튼 누르기"
    echo -e "2. USB 케이블 데이터 전송 확인"
    echo -e "3. 포트 이름 확인 (COM3, /dev/ttyUSB0)"
    echo -e "4. 보드 매니저에서 XIAO ESP32C6 선택${NC}\n"
    exit 1
  fi

  # 시리얼 모니터 확인
  read -p "시리얼 모니터를 열겠습니까? (y/n): " monitor_confirm

  if [ "$monitor_confirm" = "y" ] || [ "$monitor_confirm" = "Y" ]; then
    echo -e "\n${GREEN}시리얼 모니터 열기 (Ctrl+C로 종료)${NC}\n"
    echo -e "명령어: arduino-cli monitor -p $PORT -c baudrate=$BAUD_RATE\n"
    arduino-cli monitor -p "$PORT" -c baudrate=$BAUD_RATE
  fi
fi

echo -e "\n${GREEN}========================================${NC}"
echo -e "${GREEN}  완료!${NC}"
echo -e "${GREEN}========================================${NC}\n"
echo -e "${GREEN}수동 명령어:${NC}"
echo -e "  컴파일: arduino-cli compile --fqbn $FQBN $SKETCH_DIR"
echo -e "  업로드: arduino-cli upload -p $PORT --fqbn $FQBN $SKETCH_DIR"
echo -e "  모니터: arduino-cli monitor -p $PORT -c baudrate=$BAUD_RATE"
echo -e "${GREEN}========================================${NC}\n"