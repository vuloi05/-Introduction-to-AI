# Cấu trúc Thiết kế Giả lập Hệ Thống (Simulation Protocol Blueprint)

Do dự án là bản trình diễn Demo (Software-Only) chưa tích hợp mạch điện và camera vật lý thật, chúng ta cần một protocol giả lập hệ thống hoàn chỉnh. Tinh thần cốt lõi: **Adapter/Interface Pattern** để trong tương lai, khi có API kết nối phần cứng, sẽ chỉ thay đổi ở tầng Connector, hoàn toàn miễn nhiễm tác động làm vỡ logic cốt lõi.

## 1. Giả lập Nguồn Video (IP Camera Simulator)
Mục tiêu là mô phỏng quá trình truyền phát streaming trực tiếp cho tầng AI Engine như khi camera đẩy dữ liệu.

**Giao thức Mô phỏng Đề xuất (RTSP Mocking):**
Sử dụng bộ công cụ Mediamtx / FFmpeg loop server để tạo luồng RTSP từ các chuỗi phát Video / Image Frame cố định.
Tầng AI nhận dữ liệu vào bằng tham số `CAMERA_URL_STREAM`. Không được Hard-code, biến số này cần được cấu hình env nhằm dễ dàng thay thế sang địa chỉ Camera Local Network của bãi đỗ thực tế.

Cấu trúc Interface cho AI CV Module (Python):
```python
from abc import ABC, abstractmethod

class IVideoStreamReader(ABC):
    @abstractmethod
    def get_frame(self):
        # Trả về numpy array mô phỏng frame ảnh gửi về model
        pass

class SimulatedRTSPReader(IVideoStreamReader):
    def get_frame(self):
        # Mock logic kết nối vào localhost:8554 (MediaMTX giả lập)
        pass

class PhysicalIPCameraReader(IVideoStreamReader):
    def get_frame(self):
        # Thiết kế sẵn logic authenticate TCP IP khi có Camera Thật
        pass
```

## 2. Giả lập Các Thiết Bị Khác (Barrier & Cảm Biến)
Mục tiêu là mô phỏng việc xuất lệnh điều khiển Barrier đóng/mở và lấy trạng thái thông xe. 

**Thiết kế tầng Hardware Adapter (Tích hợp trong Backend):**

Backend sẽ định nghĩa rõ Interface Điều khiển Barrier với những quy chuẩn chung:
```typescript
interface IBarrierController {
    openBarrier(gateId: string): Promise<boolean>;
    closeBarrier(gateId: string): Promise<boolean>;
    // Bắt buộc xử lý Edge case trạng thái lỗi phần cứng
    getBarrierStatus(gateId: string): Promise<'OPEN' | 'CLOSED' | 'ERROR'>;
}
```

**Workflow Thực thi thông qua Giao thức API/Webhook:**
- **Thực thể Giả Lập:** Team Data/Simulation tạo ra 1 endpoint độc lập, Node.js đơn giản `/mock-hardware-unit/barrier`. Khi nhận được HTTP Request báo mở cổng, module Delay một thời gian `1500ms` mô phỏng độ trễ cơ cấu động cơ tĩnh mở cần gạt lên, trả về HTTP Status `200`. Backend tiếp nhận và broadcast WebSocket cập nhật trạng thái Frontend xanh.
- Nếu muốn Test tính ổn định của hệ thống: có thể cấu hình Server giả lập ngẫu nhiên sinh ra lỗi `Error 500: Động cơ bị kẹt` ở xác suất 5%. Backend tiếp nhận lỗi, thực hiện tính năng Retry 3 lần => Chuyển Log sang Danger cho người trực bãi đỗ xử lý tay.

- **Tương lai (Phần Cứng Thật):** Tự tạo mới class `RealBarrierAdapter` implementing `IBarrierController`. Nó sẽ gọi lệnh đến Raspberry Pi hoặc mạch ESP32 thông qua giao thức TCP Socket/MQTT.

## 3. Kiến trúc Đồng Bộ Thời Gian Thực
- Sử dụng **Message Broker (Redis PubSub)** hoặc **WebSockets** làm cầu nối giao tiếp chính giữa Backend và Simulation Module / Frontend.
- Trấn áp các điểm nghẽn bằng cơ chế Queue nếu số lượng xe đẩy tín hiệu vào hệ thống ồ ạt, tránh sập Server đột biến tài nguyên.
