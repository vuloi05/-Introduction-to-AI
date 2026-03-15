# Sử dụng base image Python slim để giữ cho kích thước image gọn nhẹ và an toàn
FROM python:3.10-slim

# Thiết lập thư mục làm việc trong container
WORKDIR /app

# Cài đặt các thư viện hệ thống cần thiết cho OpenCV và môi trường build
RUN apt-get update && apt-get install -y \
    libgl1-mesa-glx \
    libglib2.0-0 \
    gcc \
    && rm -rf /var/lib/apt/lists/*

# Copy requirements và cài đặt dependencies Python
# Lưu ý: Lớp cache này giúp tăng tốc độ build nếu requirements không đổi
COPY requirements.txt .

# Tại vì yêu cầu KHÔNG dùng pre-trained models cấp cao, cài đặt các lib ML mức thấp
# Cài đặt qua pip với no-cache-dir giúp giảm dung lượng image
RUN pip install --no-cache-dir -r requirements.txt

# Copy toàn bộ source code của AI engine vào container
COPY src/ ./src/

# Mặc định expose cổng 8000 (để phục vụ inference API trong tương lai qua FastAPI/Flask)
EXPOSE 8000

# Lệnh khởi chạy mặc định chạy workflow inference
# (Trong docker-compose dev, lệnh này có thể bị override để chạy training/testing)
CMD ["python", "src/inference.py"]
