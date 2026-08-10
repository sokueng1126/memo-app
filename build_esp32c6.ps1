# XIAO ESP32-C6 빌드 및 업로드 스크립트 (PowerShell용)
# Arduino CLI 사용

# 색상 설정 함수
function Write-Color {
    param(
        [string]$Message,
        [string]$Color = "Green"
    )
    $colors = @{
        "Red" = "31"
        "Green" = "32"
        "Yellow" = "33"
        "Blue" = "34"
        "Magenta" = "35"
        "Cyan" = "36"
        "White" = "37"
    }
    Write-Host $Message -ForegroundColor $colors[$Color]
}

# 설정
$PROJECT_DIR = "smart_entryway"
$FQBN = "esp32:esp32:XIAO_ESP32C6"
$BAUD_RATE = 115200

Write-Color "========================================" "Green"
Write-Color "  XIAO ESP32-C6 빌드 스크립트 (PowerShell)" "Green"
Write-Color "========================================" "Green"
Write-Host ""

# 테스트 선택
Write-Host "테스트를 선택하세요:"
Write-Host "1) 블링크 테스트 (01_blink_test)"
Write-Host "2) MQTT 통신 테스트 (02_mqtt_test)"
Write-Host "3) 통합 테스트 (04_integration)"
$choice = Read-Host "선택 (1-3): "

switch ($choice) {
    "1" {
        $INO_FILE = "01_blink_test\esp32c6_blink.ino"
        $TEST_NAME = "블링크 테스트"
    }
    "2" {
        $INO_FILE = "02_mqtt_test\esp32c6_mqtt.ino"
        $TEST_NAME = "MQTT 통신 테스트"
    }
    "3" {
        $INO_FILE = "04_integration\esp32c6_smart_entryway.ino"
        $TEST_NAME = "통합 테스트"
    }
    default {
        Write-Color "잘못된 선택" "Red"
        exit 1
    }
}

$INO_PATH = "$PROJECT_DIR\$INO_FILE"
$SKETCH_DIR = Split-Path -Parent $INO_PATH

if (-not (Test-Path $INO_PATH)) {
    Write-Color "파일을 찾을 수 없습니다: $INO_PATH" "Red"
    exit 1
}

if (-not (Test-Path $SKETCH_DIR)) {
    Write-Color "스케치 폴더를 찾을 수 없습니다: $SKETCH_DIR" "Red"
    exit 1
}

Write-Color "파일: $INO_PATH" "Green"
Write-Color "폴더: $SKETCH_DIR" "Green"

# 포트 찾기
Write-Host ""
Write-Color "보드 포트를 찾는 중..." "Yellow"
$PORT = arduino-cli board list | Select-String -Pattern "xiao.*c6|xiao.*esp32.*c6" | ForEach-Object {
    if ($_.Length -gt 0) {
        $_.Split("`")[0]
    }
}

if ([string]::IsNullOrWhiteSpace($PORT)) {
    Write-Color "자동으로 포트를 찾을 수 없습니다." "Yellow"
    $PORT = Read-Host "포트를 입력하세요 (예: COM3 또는 /dev/ttyUSB0): "
}

if ([string]::IsNullOrWhiteSpace($PORT)) {
    Write-Color "포트가 설정되지 않았습니다." "Red"
    exit 1
}

Write-Color "찾은 포트: $PORT" "Green"

# FQBN 확인
Write-Host ""
Write-Color "FQBN 확인 중..." "Yellow"
$fqbnCheck = arduino-cli board details -b $FQBN 2>$null

if ($LASTEXITCODE -eq 0) {
    Write-Color "✓ FQBN 유효: $FQBN" "Green"
} else {
    Write-Color "⚠ FQBN 확인 실패, 대안 FQBN 테스트..." "Yellow"
    $ALT_FQBN = "esp32:esp32:xiao_c6"
    $altCheck = arduino-cli board details -b $ALT_FQBN 2>$null

    if ($LASTEXITCODE -eq 0) {
        Write-Color "✓ 대안 FQBN 유효: $ALT_FQBN" "Green"
        $fqbnConfirm = Read-Host "FQBN을 $ALT_FQBN로 변경하시겠습니까? (y/n): "
        if ($fqbnConfirm -eq "y" -or $fqbnConfirm -eq "Y") {
            $FQBN = $ALT_FQBN
            Write-Color "FQBN 변경됨: $FQBN" "Green"
        }
    } else {
        Write-Color "✗ FQBN 확인 실패, 계속 진행합니다..." "Red"
    }
}

# 컴파일
Write-Host ""
Write-Color "========================================" "Green"
Write-Color "  컴파일 시작: $TEST_NAME" "Green"
Write-Color "========================================" "Green"
Write-Host "명령어: arduino-cli compile --fqbn $FQBN $SKETCH_DIR`"
Write-Host ""

arduino-cli compile --fqbn $FQBN $SKETCH_DIR

if ($LASTEXITCODE -eq 0) {
    Write-Host ""
    Write-Color "✓ 컴파일 성공!" "Green"
    Write-Host ""
} else {
    Write-Host ""
    Write-Color "✗ 컴파일 실패!" "Red"
    Write-Host ""
    Write-Color "해결 방법:" "Yellow"
    Write-Host "1. Arduino CLI 설치 확인"
    Write-Host "2. ESP32 코어 설치 확인"
    Write-Host "3. FQBN 정확성 확인"
    Write-Host "4. 보드 매니저 업데이트 확인"
    Write-Host ""
    exit 1
}

# 업로드 확인
$uploadConfirm = Read-Host "업로드하시겠습니까? (y/n): "

if ($uploadConfirm -eq "y" -or $uploadConfirm -eq "Y") {
    # 업로드
    Write-Host ""
    Write-Color "========================================" "Green"
    Write-Color "  업로드 시작" "Green"
    Write-Color "========================================" "Green"
    Write-Host "명령어: arduino-cli upload -p $PORT --fqbn $FQBN $SKETCH_DIR"
    Write-Host ""

    arduino-cli upload -p $PORT --fqbn $FQBN $SKETCH_DIR

    if ($LASTEXITCODE -eq 0) {
        Write-Host ""
        Write-Color "✓ 업로드 성공!" "Green"
        Write-Host ""
    } else {
        Write-Host ""
        Write-Color "✗ 업로드 실패!" "Red"
        Write-Host ""
        Write-Color "해결 방법:" "Yellow"
        Write-Host "1. Boot 버튼을 누른 상태에서 Reset 버튼 누르기"
        Write-Host "2. USB 케이블 데이터 전송 확인"
        Write-Host "3. 포트 이름 확인 (COM3, /dev/ttyUSB0)"
        Write-Host "4. 보드 매니저에서 XIAO ESP32C6 선택"
        Write-Host ""
        exit 1
    }

    # 시리얼 모니터 확인
    $monitorConfirm = Read-Host "시리얼 모니터를 열겠습니까? (y/n): "

    if ($monitorConfirm -eq "y" -or $monitorConfirm -eq "Y") {
        Write-Host ""
        Write-Color "시리얼 모니터 열기 (Ctrl+C로 종료)" "Green"
        Write-Host "명령어: arduino-cli monitor -p $PORT -c baudrate=$BAUD_RATE"
        Write-Host ""

        arduino-cli monitor -p $PORT -c baudrate=$BAUD_RATE
    }
}

Write-Host ""
Write-Color "========================================" "Green"
Write-Color "  완료!" "Green"
Write-Color "========================================" "Green"
Write-Host ""
Write-Color "수동 명령어:" "Green"
Write-Color "  컴파일: arduino-cli compile --fqbn $FQBN $SKETCH_DIR" "Cyan"
Write-Color "  업로드: arduino-cli upload -p $PORT --fqbn $FQBN $SKETCH_DIR" "Cyan"
Write-Color "  모니터: arduino-cli monitor -p $PORT -c baudrate=$BAUD_RATE" "Cyan"
Write-Color "========================================" "Green"
Write-Host ""