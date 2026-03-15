# Môi trường xử lý video và giả lập luồng camera an ninh
FROM python:3.10-slim

WORKDIR /app

# Cài đặt thư viện xử lý ảnh nguồn mở (tránh thiết lập giao diện GUI - headless)
RUN apt-get update && apt-get install -y \
    libgl1-mesa-glx \
    libglib2.0-0 \
    && rm -rf /var/lib/apt/lists/*

# Cài đặt dependencies (fallback an toàn nếu chưa có thư viện)
COPY requirements.txt* ./
RUN if [ -f requirements.txt ]; then pip install --no-cache-dir -r requirements.txt; fi

# Cài đặt lõi OpenCV phục vụ tool mô phỏng
RUN pip install --no-cache-dir opencv-python-headless numpy requests

COPY . .

# Giả định xuất luồng giả lập RTSP/HTTP trên cổng này
EXPOSE 8554

# Default state loop giữ container ở trạng thái Alive
CMD ["python", "-m", "http.server", "8554"]
