# Gomoku Web GUI — Development Log

Tài liệu này ghi lại toàn bộ quá trình và chi tiết các thao tác đã thực hiện để xây dựng giao diện Web (Web GUI) cho dự án Gomoku AI Engine bên trong thư mục `web/`.

## 1. Tổng quan kiến trúc
- **Mục tiêu:** Tạo một giao diện đồ họa trên trình duyệt (Web GUI) để thay thế cho giao diện dòng lệnh (CLI), cho phép người chơi tương tác trực quan với Gomoku C++ Engine.
- **Kiến trúc:** `Trình duyệt (HTML/CSS/JS)  <--HTTP API-->  Python FastAPI Server  <--stdin/stdout pipes-->  Gomoku.exe (C++ Subprocess)`
- **Điểm nổi bật:** Không can thiệp hay chỉnh sửa bất kỳ dòng code C++ nào của engine gốc. Giao tiếp hoàn toàn thông qua giao thức I/O chuẩn trong chế độ `quiet`.

## 2. Chi tiết các bước thực hiện

### 2.1. Khởi tạo Backend (Python FastAPI)
- **Tạo `requirements.txt`:** Cài đặt các thư viện cần thiết là `fastapi` và `uvicorn`.
- **Tạo `server.py`:** 
  - Khởi tạo FastAPI application phục vụ tĩnh (static files) và cung cấp các REST API (`/api/new-game`, `/api/move`, `/api/ai-step`, `/api/resign`).
  - Xây dựng lớp `GameSession` để quản lý vòng đời của trò chơi:
    - Quản lý quá trình khởi chạy (launch), dừng (stop) và dọn dẹp (cleanup) tiến trình `Gomoku.exe` dưới dạng subprocess.
    - Duy trì trạng thái bàn cờ hiện tại (mảng 2D 16x16) và lịch sử các nước đi.
    - Xử lý việc kiểm tra thắng/thua (thuật toán kiểm tra 5 quân liên tiếp) ngay trên Python để tối ưu hóa và tránh phải parse các dòng output phức tạp từ C++.

### 2.2. Xây dựng Giao diện Frontend (HTML/CSS/JS)
- **Tạo `static/index.html`:** Xây dựng bố cục trang web dạng Single-Page Application (SPA). Bao gồm khu vực bàn cờ chính, bảng điều khiển (Cài đặt, Lịch sử nước đi, Thông tin Engine), và một lớp phủ (overlay) hiển thị kết quả khi kết thúc ván đấu.
- **Tạo `static/style.css`:** Thiết kế Design System với:
  - **Dark theme & Glassmorphism:** Các bảng điều khiển mờ ảo trên nền tối, tạo cảm giác hiện đại và cao cấp.
  - **Bàn cờ gỗ & Quân cờ 3D:** Sử dụng CSS gradients và box-shadow để tạo ra bàn cờ vân gỗ và quân cờ (đen/trắng) có hiệu ứng nổi 3D chân thực. Cung cấp hiệu ứng hover (xem trước nước đi) và viền đỏ cho nước đi mới nhất.
- **Tạo `static/app.js`:** 
  - Quản lý trạng thái giao diện (DOM manipulation).
  - Giao tiếp với backend qua hàm `fetch` (hàm `apiCall`).
  - Hiển thị hoạt ảnh đặt quân cờ, theo dõi trạng thái trò chơi (lượt đi, kết thúc) và cập nhật thanh trạng thái theo thời gian thực.

### 2.3. Cải tiến và bổ sung đa chế độ chơi
Ban đầu GUI chỉ hỗ trợ **Human vs AI**. Sau đó đã được mở rộng để hỗ trợ tổng cộng 3 chế độ:
- **👤 Human vs AI:** Người chơi tương tác trực tiếp với Engine.
- **🤖 AI vs AI:** Trận đấu tự động giữa 2 Engine.
  - Thêm tính năng cấu hình độc lập cho Black AI và White AI (Level và Depth riêng biệt) bằng cách sử dụng các tham số `--black-level`, `--white-level` của CLI engine.
  - Bổ sung thanh điều khiển Playback: Nút **Play** (Auto-play), **Pause** và **Step** (chạy từng bước).
- **👥 Human vs Human:** 2 người chơi đấu với nhau trên cùng một máy, không cần gọi đến Engine C++.

### 2.4. Tối ưu hóa và Xử lý lỗi (Bug Fixes)
- **Vấn đề Race Condition trong AI vs AI:** Ban đầu, chế độ Auto-play sử dụng `setInterval` trong JavaScript. Khi Engine đang tính toán lâu (ví dụ ở depth 4 VCF mất nhiều thời gian), việc gọi liên tục API `/api/ai-step` dẫn đến hàng loạt request bị kẹt, gây ra race condition trên đường ống đọc/ghi (pipe) của subprocess, khiến game bị lỗi kết thúc sớm.
- **Giải pháp Frontend:** Thay thế `setInterval` bằng một vòng lặp tuần tự bất đồng bộ (`sequential async loop`). Javascript sẽ `await` cho đến khi nhận được phản hồi từ Engine rồi mới delay và yêu cầu nước đi tiếp theo.
- **Giải pháp Backend:** Thêm `threading.Lock()` vào lớp `GameSession` để khóa luồng (thread-safe) mỗi khi đọc/ghi với tiến trình C++, đảm bảo không bao giờ có 2 request thao tác với engine cùng một thời điểm.
- **UI Indicator:** Thêm trạng thái nhấp nháy `"⏳ AI is thinking..."` để người dùng không cảm thấy ứng dụng bị treo khi engine tốn thời gian suy nghĩ dài.

## 3. Cấu trúc thư mục `web/` hiện tại

```text
c:\DU_AN\-Introduction-to-AI\web\
├── requirements.txt      # Chứa danh sách các package Python cần cài đặt
├── server.py             # Máy chủ FastAPI Backend & Quản lý Gomoku subprocess
├── DEVELOPMENT_LOG.md    # Tài liệu ghi chú lại quá trình phát triển (File này)
└── static/               # Chứa các tài nguyên giao diện
    ├── index.html        # Khung giao diện chính
    ├── style.css         # Định dạng và giao diện trực quan (UI)
    └── app.js            # Logic xử lý giao diện và giao tiếp Backend
```

## 4. Tóm tắt luồng hoạt động (Workflow)
1. User nhấn **New Game** trên trình duyệt.
2. `app.js` gửi HTTP POST đến `/api/new-game`.
3. `server.py` khởi chạy `Gomoku.exe` thông qua `subprocess.Popen` với các tham số tương ứng (level, depth, chế độ chơi, v.v.).
4. Khi có một nước đi (từ Human hoặc qua Auto-play Step), tọa độ được gửi đến `/api/move` hoặc `/api/ai-step`.
5. `server.py` đẩy chuỗi `row col\n` vào luồng `stdin` của Engine C++.
6. Engine C++ tính toán, in kết quả ra luồng `stdout`.
7. `server.py` đọc chuỗi từ `stdout`, phân tích cú pháp thành `{row, col}` và trả về dạng JSON cho Frontend.
8. `app.js` nhận tọa độ, cập nhật DOM để hiển thị nước đi và thời gian thực thi.
