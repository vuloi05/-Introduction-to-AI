# Tài liệu Điều phối Hệ thống Docker (Global Orchestration)

Hệ thống Smart Parking ALPR của chúng ta là một kiến trúc Microservices phức tạp với thành phần cốt lõi là AI Engine tích hợp tài nguyên tính toán cao. Bằng việc ứng dụng Docker Compose, quy trình thiết lập môi trường cho toàn bộ kỹ sư phát triển (Developer Experience - DX) trên máy Local đã được tiêu chuẩn hóa chỉ với **MỘT LỆNH DUY NHẤT**. 

Tài liệu này hướng dẫn chi tiết cách vận hành toàn bộ vòng đời ứng dụng.

## 1. Hướng dẫn Khởi chạy Toàn Hệ Thống

Tại thư mục gốc dự án (nơi chứa file `docker-compose.yml`), khởi chạy hệ thống phát triển bằng lệnh:

```bash
docker compose up --build
```

> [!TIP]
> Bạn có thể thêm cờ `-d` (`docker compose up --build -d`) để hệ thống chạy ngầm trong chế độ Detached. Để dừng hệ thống, sử dụng lệnh `docker compose stop` hoặc `docker compose down`.

**Hoạt động ngầm của Docker Compose:**
1. Trình điều phối sẽ ưu tiên build container `ai-engine` trước do cấu hình cấu trúc phụ thuộc `depends_on`.
2. Lần lượt build độc lập lớp Image của `backend-service`, `frontend-dashboard`, và `simulation-tools` trong các môi trường cô lập tuyệt đối.
3. Liên kết chung 4 Container vào dưới một Virtual Network (`alpr-net`).

## 2. Giao thức Truy cập Dịch Vụ Môi Trường Trực Quan (Dev)

Sau khi log thông báo trên Terminal cho thấy các Container đã "Started/Alive", bạn sử dụng máy Host mở trình duyệt và truy cập các ánh xạ cổng thông tin (Ports exposure) tương ứng:

- **🔗 Frontend Dashboard (React/Vite)**: `http://localhost:5173`
- **⚙️ Backend API Layer (FastAPI)**: `http://localhost:8080` *(Xem giao diện tài liệu Swagger Labs tại `/docs`)*
- **🧠 AI Core Inference Engine**: `http://localhost:8000` 
- **🎥 Stream Simulation Tool**: `http://localhost:8554`

## 3. Kiến trúc Liên Kết Kín & Bức Tường Bảo Mật

Việc giao tiếp bên trong mạng được định tuyến gắt gao.

### Tuyến Giao Tiếp Web (Public -> Private)
Client từ bên ngoài (Trình duyệt) điều tương tác truy xuất hệ thống Frontend (`:5173`). Frontend này sau đó sẽ giao tiếp lấy dữ liệu công khai từ API Host của Backend (`http://localhost:8080`).

### Tuyến Giao Tiếp Trục (Private -> Deep Private)
Để đảm bảo luồng hình ảnh chứa biển số xe không bị gián đoạn hay rò rỉ, Backend Service gọi trực tiếp qua AI Engine hoàn toàn dựa trên Mạng Nội Bộ (Docker Internal DNS). Trong cấu hình Container Backend, biến `AI_ENGINE_URL` lưu trữ giá trị `http://ai-engine:8000`. 
Docker Engine trên Host sẽ tự động phân giải hostname `ai-engine` thành private IP động, cô lập triệt để với thế giới bên ngoài. **Tuyệt đối KHÔNG thay thế bằng `localhost` khi muốn 2 Container nói chuyện với nhau.**

## 4. Trải Nghiệm Lập Trình & Tích Hợp Hot-Reloading

Điểm cốt lõi trong DX của Docker Compose là khả năng đồng bộ file nóng thông qua ánh xạ `Volume`:

```yaml
volumes:
  - ./ai-engine:/app
```

**Bất kỳ lập trình viên nào trong đội ngũ DevOps** tiến hành viết thêm hay sửa lỗi ở file `ai-engine/src/...` hoặc cập nhật Component React tại `frontend-dashboard/...` thông qua IDE (Visual Studio) trên máy mình, bản cập nhật sẽ ngay lập tức được truyền nóng (hot-reload) vào thư mục làm việc `/app` trên Container.
Các công cụ chạy tiến trình như `uvicorn --reload` và Vite HMR sẽ tự động làm mới ứng dụng mà nhà lập trình không bao giờ phải chạy lại dòng lệnh `docker compose build` quá một lần trừ khi có sự thay đổi tại file requirements/packages.