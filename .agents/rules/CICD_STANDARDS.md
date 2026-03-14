# CI/CD Standards & Rules

Tài liệu này định nghĩa các quy tắc cốt lõi về Continuous Integration và Continuous Deployment (CI/CD) cho monorepo của hệ thống Smart Parking ALPR.

## 1. PR Merge Gates

Tất cả các Pull Request (PR) hướng vào nhánh `main` hoặc `develop` bắt buộc phải vượt qua các kiểm tra tự động trước khi được phép merge. Hệ thống áp dụng nguyên tắc "Zero Exception".

- **Linting & Formatting**: Mỗi service phải tuân thủ nghiêm ngặt style guide tương ứng. Pipeline sẽ fail ngay lập tức nếu code vi phạm (ví dụ: Ruff/Black cho Python, ESLint/Prettier cho frontend).
- **Type Checking**: Bắt buộc phát hiện lỗi tĩnh trước khi runtime. Sử dụng TypeScript strict mode cho frontend và `mypy` cho Python backend/AI.
- **Unit Tests**: Phải vượt qua 100% các unit tests. Khuyến nghị thiết lập ngưỡng coverage tối thiểu (ví dụ: 80%).

Bất kỳ branch bảo vệ nào cũng sẽ block hành động merge nếu các checks trên rơi vào trạng thái failed.

## 2. Monorepo Path Filtering Rule

Hệ thống monorepo chứa nhiều service độc lập (`/frontend-dashboard`, `/backend-service`, `/ai-engine`). Để tối ưu thời gian CI và tiết kiệm compute resources (GitHub Actions minutes), áp dụng **Path Filtering**:

- Thay đổi code ở một service chỉ kích hoạt (trigger) các job kiểm tra của riêng service đó.
- Ví dụ: Một dev update file UI trong `/frontend-dashboard` sẽ chỉ trigger chuỗi pipeline `frontend-checks`. Phần `backend` và `ai` sẽ được hệ thống bỏ qua một cách an toàn.
- Những thay đổi diễn ra tại root hoặc config chung (như `.github/workflows/`) mới trigger toàn bộ pipeline.

## 3. Versioning & Tagging

Hệ thống tuân thủ **Semantic Versioning (SemVer)** với cấu trúc `MAJOR.MINOR.PATCH`:

- **MAJOR**: Các thay đổi lớn, kiến trúc core bị điều chỉnh, hoặc không còn tương thích ngược.
- **MINOR**: Giao tiếp API thêm endpoint, tính năng mới được đưa vào (tương thích ngược).
- **PATCH**: Các bản vá lỗi tĩnh, tối ưu hiệu năng hoặc bảo mật không làm đổi logic hiện tại.

**Tagging Release**: 
Git tags (ví dụ: `v1.2.0`) được sử dụng làm mốc đánh dấu bản release. Khi tag mới được tạo trên `main`, luồng CD/Deployment mới bắt đầu chạy để thực hiện rollout lên Production. Không deploy thủ công.
