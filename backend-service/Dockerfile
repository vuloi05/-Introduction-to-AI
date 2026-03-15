# Môi trường FastAPI cho Backend
FROM python:3.10-slim

WORKDIR /app

# Vô hiệu hóa byte-code sinh ra từ các thư viện Python, tăng cường performance
ENV PYTHONDONTWRITEBYTECODE=1 \
    PYTHONUNBUFFERED=1

# Vì project structure backend chưa cụ thể, giả định đọc trước file requirements
# Điều kiện fallback để không vỡ Docker build nếu chưa có file
COPY requirements.txt* ./
RUN if [ -f requirements.txt ]; then pip install --no-cache-dir -r requirements.txt; fi

# Đảm bảo cài đặt tối thiểu framework FastAPI
RUN pip install --no-cache-dir fastapi uvicorn httpx pydantic

# Chép toàn bộ file sau để cache docker được duy trì
COPY . .

# Expose cổng cho API Service
EXPOSE 8080

# Chạy Uvicorn server (giả định entrypoint ở đây là module chính sẽ được thiết lập sau)
# `--reload` thường bị loại bỏ trên production, nhưng ở đây dùng cho dev loop qua Volumes
CMD ["uvicorn", "src.main:app", "--host", "0.0.0.0", "--port", "8080", "--reload"]
