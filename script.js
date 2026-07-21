// 이미지 미리보기 함수
function previewImage() {
    const imageInput = document.getElementById('imageInput');
    const previewSection = document.getElementById('previewSection');
    const imagePreview = document.getElementById('imagePreview');

    if (imageInput.files && imageInput.files[0]) {
        const reader = new FileReader();

        reader.onload = function(e) {
            imagePreview.src = e.target.result;
            previewSection.style.display = 'block';
        };

        reader.readAsDataURL(imageInput.files[0]);
    } else {
        previewSection.style.display = 'none';
    }
}

// 물체 분석 함수
function analyzeObject() {
    const btn = document.getElementById('objectBtn');
    const imageInput = document.getElementById('imageInput');
    const resultSection = document.getElementById('resultSection');
    const resultContent = document.getElementById('resultContent');

    // 이미지가 선택되었는지 확인
    if (!imageInput.files || !imageInput.files[0]) {
        alert('사진을 먼저 선택해주세요!');
        return;
    }

    // 버튼 텍스트 변경
    btn.textContent = '잠깐만요, 분석할께요';
    btn.disabled = true;

    // 결과 섹션 표시
    resultSection.style.display = 'block';
    resultContent.textContent = '분석 중...';

    // TODO: 여기에 실제 객체 인식 기능 연결 필요
    // 예: API 호출, TensorFlow.js, YOLO 모델 등
    // const objectDetectionResult = await detectObjects(imageInput.files[0]);

    // 테스트용으로 2초 후 결과 표시 (실제 구현 시 제거)
    setTimeout(function() {
        btn.textContent = '물체 찾기';
        btn.disabled = false;
        resultContent.innerHTML = `
            <p>분석이 완료되었습니다.</p>
            <p><strong>TODO:</strong> 실제 객체 인식 결과가 이곳에 표시됩니다.</p>
        `;
    }, 2000);
}