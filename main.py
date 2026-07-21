from fastapi import FastAPI
from pydantic import BaseModel
from dotenv import load_dotenv
import os

# .env 로드
load_dotenv()

# 환경 변수 출력
MY_SECRET = os.getenv("MY_SECRET")
print(f"MY_SECRET 값: {MY_SECRET}")

app = FastAPI()

# 메모 목록
memos = []

# 메모 양식 (Pydantic)
class Memo(BaseModel):
    id: int
    content: str

# GET 엔드포인트: 메모 목록 조회
@app.get("/memos")
def get_memos():
    return {"count": len(memos), "memos": memos}

# POST 엔드포인트: 메모 저장
@app.post("/memos")
def add_memo(memo: Memo):
    memos.append(memo)
    return {"message": "메모가 저장되었습니다", "memo": memo, "count": len(memos)}