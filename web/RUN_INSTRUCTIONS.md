# Hướng Dẫn Khởi Chạy Gomoku Web GUI

Tài liệu này hướng dẫn chi tiết từng bước để biên dịch (build) Engine C++ và khởi chạy giao diện Web cho trò chơi Gomoku.

---

## 1. Yêu cầu hệ thống (Prerequisites)

Để chạy được toàn bộ dự án, máy tính của bạn cần được cài đặt sẵn các phần mềm sau:
- **C++ Compiler & CMake:** Dùng để biên dịch mã nguồn C++ (khuyến nghị dùng MinGW trên Windows).
- **Python:** Phiên bản 3.9 trở lên để chạy Web Server.
- **Trình duyệt Web:** Chrome, Edge, Firefox, Safari,...

---

## 2. Bước 1: Biên dịch (Build) Gomoku Engine C++

Giao diện Web hoạt động bằng cách giao tiếp với file thực thi `.exe` của Engine C++. Do đó, bạn **bắt buộc phải build mã C++** trước khi chạy giao diện Web.

Mở Terminal (Command Prompt hoặc PowerShell) tại **thư mục gốc của dự án** (`C:\DU_AN\-Introduction-to-AI`) và chạy lần lượt các lệnh sau:

```powershell
# 1. Tạo thư mục build và di chuyển vào đó
mkdir build
cd build

# 2. Cấu hình CMake (Sử dụng MinGW cho Windows, build mode Release để chạy nhanh nhất)
cmake -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release ..

# 3. Tiến hành biên dịch mã nguồn
cmake --build .
```

*✅ Kết quả: Sau khi chạy xong, bạn sẽ thấy file thực thi `Gomoku.exe` xuất hiện bên trong thư mục `build/`.*

---

## 3. Bước 2: Khởi chạy Web Server (Python)

Sau khi đã có file `Gomoku.exe`, bạn tiến hành chạy máy chủ Backend FastAPI để làm cầu nối lên trình duyệt.

Vẫn trong Terminal, bạn chuyển sang thư mục `web/` và thực hiện cài đặt thư viện:

```powershell
# 1. Trở về thư mục gốc và di chuyển vào thư mục web
cd ..
cd web

# 2. Cài đặt các thư viện Python cần thiết (fastapi, uvicorn)
pip install -r requirements.txt

# 3. Khởi chạy máy chủ Server
python server.py
```

*✅ Kết quả: Bạn sẽ thấy dòng thông báo hiển thị máy chủ đã khởi động, ví dụ:*
> `INFO: Uvicorn running on http://127.0.0.1:8080 (Press CTRL+C to quit)`

---

## 4. Bước 3: Mở trình duyệt và chơi game

1. Mở bất kỳ trình duyệt web nào (Chrome, Edge, Firefox...).
2. Truy cập vào địa chỉ: [http://localhost:8080](http://localhost:8080)
3. Tại giao diện trò chơi, bạn có thể thiết lập:
   - **Game Mode:** Chọn chế độ chơi (Human vs AI, AI vs AI, hoặc Human vs Human).
   - **Level & Depth:** Độ khó và độ sâu tư duy của AI.
   - Nhấn nút **New Game** để bắt đầu ván đấu.

---

## 5. Các lỗi thường gặp (Troubleshooting)

- **Lỗi `FileNotFoundError: Engine not found`:**
  Máy chủ Python không tìm thấy file `Gomoku.exe`. 
  *Cách sửa:* Đảm bảo bạn đã thực hiện thành công **Bước 1** và file `Gomoku.exe` nằm đúng trong thư mục `build/`.
  
- **Lỗi `ModuleNotFoundError` khi chạy `server.py`:**
  Máy tính chưa cài đặt các thư viện cần thiết.
  *Cách sửa:* Chạy lệnh `pip install -r requirements.txt` ở Bước 2.

- **Muốn dừng Server thì làm thế nào?**
  Tại cửa sổ Terminal đang chạy `server.py`, bạn nhấn tổ hợp phím `Ctrl + C` để tắt máy chủ một cách an toàn.
