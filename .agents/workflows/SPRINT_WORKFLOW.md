# Quy trình Sprint & Các Cột mốc Dự án (4 Phases)

## Phase 1: Kiến trúc Nền tảng & Contract (Bootstrapping & Mocking)
- **AI/CV:** Phân tích quy cách format kết quả OCR, thiết kế JSON contract trả về để Backend tiêu thụ (tiêu thụ ảnh raw hoặc qua API).
- **Backend:** Xây dựng API và Database Schema đầu tiên (chưa gắn kết model thật). Định nghĩa các Socket Events báo tín hiệu (VD: `PLATE_DETECTED`, `BARRIER_STATUS`).
- **Frontend:** Cấu trúc dự án Dashboard, lập trình các Component cơ sở bằng Dummy Data/Mock API từ Backend. 
- **Simulation/Data:** Viết công cụ giả lập stream đẩy ảnh/RTSP fake từ dữ liệu có sẵn để cung cấp source thử nghiệm cho AI Engineer làm quen.
- **PM/QA:** Review các API Contract, kiểm thử tính tương thích hợp đồng, chốt quy tắc code base chung (TEAM_RULES) và khởi tạo CI Pipeline cho Linter/Formatter.

*Dependency (Nghẽn cổ chai Phase 1):* Backend và AI không thể gửi đi tín hiệu cho Frontend code nếu Mock API và Socket Event Rules chưa qua vòng review chốt. Do đó Contract Definition phải hoàn thành sớm.

## Phase 2: Core Logic & Luồng Dữ liệu Thời gian thực
- **AI/CV:** Tích hợp logic YOLO và nền tảng OCR vào hệ thống, xuất các bounding box, crop biển số kèm text nhận dạng. Kết nối thẳng vào Backend.
- **Backend:** Xử lý nghiệp vụ chính: Kiểm tra CSDL, validate logic xem biển số xe đã được cho phép hay chưa. Gọi (hook) lệnh báo mở/đóng cổng, phát Emit WebSocket event tổng đài.
- **Frontend:** Nâng cấp từ Dummy Data lên đọc Real-time thông qua tích hợp WebSockets từ Backend. Hoàn thiện layout camera viewer giả lập trên UI.
- **Simulation/Data:** Khởi tạo tool Data Generator để push các bài test góc khuất, tối độ phủ ánh sáng nhằm challenge hệ thống AI, tạo giả lập thiết bị Barrier với độ trễ (delay) từ hardware.
- **PM/QA:** Bắt đầu lên bộ Unit Test cho Backend/AI logic, rà soát memory leak (cực kì quan trọng cho AI node).

*Dependency:* AI cần Mock Video/Image set chuẩn từ Simulation; Frontend lúc này sẽ cần WebSocket Connection Interface chạy ổn định từ Backend chứ không dùng mock API tĩnh nữa.

## Phase 3: Tích hợp Giả lập Phần cứng & Xử lý Ngoại lệ
- **AI/CV:** Benchmark và tối ưu hóa xử lý vòng lặp (FPS), thử nghiệm với các trường hợp input khó lỗi qua giả lập để fail-gracefully thay vì sập app. 
- **Backend:** Kết nối Adapter với hệ thống giả lập Cảm biến/Barrier. Test hệ thống tự động retry/timeout nếu Simulation-barrier giả lập mất kết nối mạng.
- **Frontend:** Thiết kế UX/UI cảnh báo lỗi phần cứng tới người quản trị (barrier không hoạt động, camera mất nguồn...). 
- **Simulation/Data:** Chạy service "Hardware Emulator Websocket/Webhook" có khả năng lắng nghe và giả lập tỉ lệ lỗi phần cứng (bắn signal báo cửa hỏng, động cơ kẹt) đễ Backend bắt.
- **PM/QA:** Mở hệ thống Chaos testing, đánh giá độ an toàn toàn vẹn dữ liệu cho các Race Condition.

*Dependency:* Cần Simulation phần cứng hoạt động ổn định cung cấp mock endpoints cho Backend thực thi các API điều khiển vật lý.

## Phase 4: Đánh giá & Tinh chỉnh (UAT & Polish)
- Thực hiện chạy thử nghiệm xuyên suốt E2E toàn bộ kiến trúc từ Camera Stream Giả lập 👉 AI Engine 👉 Backend 👉 Giao diện Dashboard 👉 Trả tín hiệu điều khiển Barrier phần cứng.
- Rà soát Code. Refactor các đoạn code phình to. Optimize truy vấn dữ liệu theo đúng "Senior Standard".
- Đống gói Container (Dockerization) để tạo thành giải pháp triển khai One-click.
- PM/QA hoàn thiện mọi mô tả hệ thống cần thiết. Tiến tới release phiên bản Demo trình diễn nội bộ.
