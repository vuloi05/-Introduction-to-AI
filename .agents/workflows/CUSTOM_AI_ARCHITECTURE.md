---
description: Kiến trúc AI tùy chỉnh cho hệ thống Smart Parking ALPR
---
# Kiến trúc AI Tùy chỉnh (Smart Parking ALPR)

Tài liệu này mô tả chi tiết kiến trúc mạng nơ-ron được thiết kế độc quyền từ đầu cho dự án, không phụ thuộc vào các mô hình pre-trained cấp cao như YOLO hay EasyOCR. Việc tự xây dựng kiến trúc giúp đội ngũ hoàn toàn làm chủ mức độ tối ưu hóa, tốc độ suy luận và kích thước mô hình để dễ dàng triển khai (deploy) hiệu quả trong hệ sinh thái Docker.

## 1. Custom License Plate Detector (Mạng CNN Phát hiện Biển số)

Thay vì dùng YOLO, chúng ta sử dụng một mạng Convolutional Neural Network (CNN) thuần túy với kiến trúc gọn nhẹ, được thiết kế theo dạng Single Shot Detector (SSD) thu nhỏ để suy luận theo thời gian thực (Real-time inference).

**Kiến trúc cốt lõi:**
- **Backbone Feature Extractor**: Gồm 4-5 khối Convolution (`Conv2d -> BatchNorm2d -> ReLU -> MaxPool2d`). Backbone này đóng vai trò trích xuất các đặc trưng biểu diễn mức độ phức tạp từ thấp đến cao (cạnh, góc chữ, định dạng hình học của biển số).
- **Detection Head**: Một vài lớp chập (Convolutional layers) cuối cùng để dự đoán đồng thời:
  1.  **Bounding Box (Hộp bao)**: `[x, y, w, h]` - tọa độ trung tâm và kích thước vùng biển số.
  2.  **Objectness Score**: Xác suất vùng đó có chứa biển số hay không.

**Chiều hướng & Triết lý thiết kế:**
Tối thiểu hóa tham số bằng việc sử dụng Depthwise Separable Convolutions ở các layer cuối để giữ kiến trúc nhẹ mà vẫn bao quát được Receptive Field.

**Chi tiết Huấn luyện (Training):**
- **Loss Function**: `Smooth L1 Loss` cho hồi quy tọa độ bounding box nhằm chống outliers, và `BCEWithLogitsLoss` cho đánh giá có/không có đối tượng.
- Cần áp dụng Data Augmentation (Albumentations) mạnh mẽ để kháng nhiễu thời tiết thực tế.

## 2. Custom OCR Model (Mạng CNN-RNN - CRNN Nhận diện Ký tự)

Sau khi crop được ảnh biển số từ mạng Detector, ảnh sẽ được đưa vào mô hình OCR. Kiến trúc CRNN (Convolutional Recurrent Neural Network) kết hợp sức mạnh trích xuất hình ảnh của CNN, khả năng đọc chuỗi của RNN và hàm phân tích chuỗi CTC (Connectionist Temporal Classification).

**Kiến trúc cốt lõi:**
- **Feature Extraction (CNN)**: Gồm mạng CNN cơ bản (với các Pooling layer không đối xứng, ưu tiên nén chiều cao thay vì chiều rộng) để biến đổi ảnh crop thành một dãy feature mang tuần tự về không gian (spatial sequence).
- **Sequence Learning (RNN)**: Bản đồ đặc trưng được xem như một chuỗi các khung (frames). Chuỗi này được đưa qua ngăn xếp mạng Bi-directional LSTM hoặc Bi-GRU để tìm ra học ngữ cảnh ký tự trước & sau, nhằm phán đoán các nét bị mòn/mờ trên biển số.
- **Transcription (CTC Layer)**: Ở mỗi bước (time-step), một lớp `Linear` dự đoán xác suất của tập ký tự (`A-Z, 0-9` và ký tự `blank`). Lớp CTC được thiết kế riêng giúp mô hình tự học sự căn chỉnh chuỗi mã hóa mà không cần phải gán nhãn từng khung bounding box cho từng ký tự rời rạc (character-level bounding box).

**Chi tiết Huấn luyện (Training):**
- **Loss Function**: `CTCLoss` trong PyTorch. Cần lưu ý đặc tính kích thước chuỗi đầu vào của RNN luôn phải lớn hơn độ dài Ground-Truth.
