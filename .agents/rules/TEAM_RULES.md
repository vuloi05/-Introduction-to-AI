# Tiêu chuẩn Kỹ thuật và Quy tắc Làm việc Nhóm

## 1. Tư duy Senior Engineering (Senior Mindset)
- **Modular & Loosely Coupled:** Mọi module phải hoạt động độc lập, giao tiếp qua Interface rõ ràng. Đảm bảo tính mở rộng cao nhất.
- **Zero Silent Failures:** Bắt buộc phải try-catch và log toàn bộ exception một cách tường minh. Không bỏ qua bất kì lỗi nào (swallow exceptions).
- **Fault-Tolerant:** Hệ thống phải có khả năng tự phục hồi hoặc graceful degradation (Ví dụ: AI service bị lỗi thì Backend vẫn có thể hỗ trợ ghi nhận xe vào/ra bằng phương pháp thủ công).
- **Graceful Edge-case Handling:** Xử lý triệt để các góc chết có thể xảy ra ở hệ thống phân tán (Camera bị ngắt kết nối, mất mạng, dữ liệu trả về null, biển số bị mờ).

## 2. Git Workflow & Branching Strategy
- **Nhánh chính:** `main` (Production) và `develop` (Staging). Tuyệt đối không push trực tiếp lên các nhánh này.
- **Nhánh tính năng:** Tuân thủ tên nhánh chuẩn `feature/ticket-name`, `bugfix/ticket-name`, `hotfix/...`.
- **Commit Messages:** Tuân thủ Conventional Commits bằng Tiếng Anh (vd: `feat(ai): add YOLOv8 model for plate detection`, `fix(backend): resolve race condition in DB`).
- **PR Review Checklist (Bắt buộc):**
  - Đã chạy linter / formatter trên code mới chưa?
  - Có viết đủ unit test / integration test (nếu cần) không?
  - Có log các hành vi lỗi không mong muốn/xử lý trường hợp timeout rớt mạng không?
  - Bắt buộc phải có ít nhất 1 approval từ Senior Engineer / Tech Lead trước khi merge.

## 3. Tiêu chuẩn Mã nguồn (Code Standards)
- **Python (AI / Simulation):**
  - Mọi hàm phải có Type hinting đầy đủ (mypy check).
  - Sử dụng Formatter: `Black` hoặc `Ruff`, Linter: `Ruff`.
- **JavaScript/TypeScript (Backend / Frontend):**
  - Bật Strict Mode trong TypeScript. Không sử dụng kiểu dữ liệu `any`.
  - Khuyến nghị sử dụng ESLint, Prettier trong dự án.
- **Quản lý Cấu hình (Environment Variables):**
  - Tuyệt đối không commit file nội dung `.env` nào. Sử dụng `.env.example` làm template.
  - Phải có cơ chế config validation ở mọi service kể từ lúc chạy app (Fail-fast/Stop Process ngay lập tức nếu thiếu biến môi trường cần thiết).

## 4. API Contract First
- **Thiết kế trước - Code sau:** Backend, Frontend, và AI Engineer phải đối thoại để thống nhất Payload JSON hoặc dữ liệu WebSocket. Nên có Swagger / OpenAPI chuẩn, hoặc Type Definitions dùng chung (Monorepo shared types) trước khi hiện thực logic rẽ nhánh.
- Nếu có sự thay đổi về giao thức, toàn bộ các bên liên quan phải được thông báo và tài liệu phải được cập nhật ngay theo tinh thần Single Source of Truth.
