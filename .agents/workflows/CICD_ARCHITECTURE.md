---
description: Kiến trúc CI/CD và Kế hoạch Dockerization cho hệ thống Smart Parking ALPR
---

# CI/CD Architecture & Docker Plan

Bản thiết kế này mô tả chiến lược container hóa và các giai đoạn pipeline CI/CD, chuẩn bị sẵn sàng cho việc đưa hệ thống Smart Parking ALPR từ môi trường simulation lên môi trường Production.

## 1. Dockerization Strategy

Mỗi service trong monorepo được xem như một microservice riêng biệt và sẽ có `Dockerfile` độc lập.

- **AI Engine (`/ai-engine`)**
  - Đóng gói bằng Python slim hoặc base image hỗ trợ tính toán GPU của NVIDIA (`nvidia/cuda:*-cudnn*-runtime-ubuntu*`) nếu chạy inference thực tế.
  - Tối ưu hóa dung lượng image bằng multi-stage build cho các model ML/CV cồng kềnh.

- **Backend FastAPI (`/backend-service`)**
  - Chạy dựa trên `python:3.11-slim`.
  - Kết nối giữa AI Engine và database, xuất API. Sử dụng `uvicorn` làm ASGI gateway.

- **Frontend React (`/frontend-dashboard`)**
  - Multi-stage build: 
    - Stage Build: Dùng `node:20-alpine` để cài npm modules và build ra static files.
    - Stage Serve: Nginx image nhẹ (`nginx:alpine`) để host file build tĩnh, hỗ trợ routing cho SPA (Single Page Application).

- **Simulation Tools (`/simulation-tools`)**
  - Container hóa các công cụ mô phỏng giả lập camera stream và event sinh data ảo, hỗ trợ việc chạy integration test mà không phụ thuộc thiết bị vật lý thật.

## 2. Pipeline Stages

Pipeline CI/CD chạy xuyên suốt qua các giai đoạn (stages) chuẩn hóa với độ tin cậy và tốc độ tối đa:

1. **Code Checkout**: Fetch mã nguồn theo version/commit hash hiện tại từ Git repository để đảm bảo pipeline đang chạy bản code đúng.
2. **Setup Env/Cache**: Cài đặt platform SDK (Node, Python). Kéo dependencies từ Cache based trên file hash lock (như `package-lock.json` hoặc `requirements.txt`). Bước này quyết định tốc độ build.
3. **Lint**: Định dạng và quét lỗi static code. Đảm bảo clean code standards.
4. **Test**: Chạy suites kiểm thử (Unit test/Integration). 
5. **Build**: Tổng hợp code ra artifact hoặc build thử Docker Image để chắc chắn quá trình biên dịch/container hóa không sinh lỗi.
6. **Mock Deploy**: Triển khai image lên môi trường container giả lập (ví dụ: staging sandbox hoặc docker-compose) nhằm thực hiện smoke test nội bộ cho toàn bộ flow của hệ thống từ frontend -> backend -> AI. Cổng gác cuối cùng trước Production.
