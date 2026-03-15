import torch
import torch.nn as nn

class CustomCRNN(nn.Module):
    """
    Kiến trúc mạng CRNN (CNN + RNN + CTC) cho nhận diện ký tự biển số.
    """
    def __init__(self, img_channels=3, num_classes=37): # 36 ký tự (A-Z, 0-9) + 1 blank cho CTC
        super(CustomCRNN, self).__init__()
        
        # 1. Feature Extractor (CNN)
        # Thiết kế các lớp pooling ưu tiên giữ nguyên chiều rộng mảng, giảm chiều cao
        self.cnn = nn.Sequential(
            nn.Conv2d(img_channels, 64, kernel_size=3, stride=1, padding=1),
            nn.ReLU(True),
            nn.MaxPool2d((2, 2), (2, 2)), # Giảm chiều cao & rộng / 2
            
            nn.Conv2d(64, 128, kernel_size=3, stride=1, padding=1),
            nn.ReLU(True),
            nn.MaxPool2d((2, 2), (2, 2)), # Giảm chiều cao & rộng / 4
            
            nn.Conv2d(128, 256, kernel_size=3, stride=1, padding=1),
            nn.BatchNorm2d(256),
            nn.ReLU(True),
            
            nn.Conv2d(256, 256, kernel_size=3, stride=1, padding=1),
            nn.ReLU(True),
            # Chỉ giảm chiều cao (kích thước frame), giữ nguyên chiều rộng timestep
            nn.MaxPool2d((2, 1), (2, 1)), 
            
            nn.Conv2d(256, 512, kernel_size=3, stride=1, padding=1),
            nn.BatchNorm2d(512),
            nn.ReLU(True),
            
            # Đưa chiều cao về 1
            nn.AdaptiveAvgPool2d((1, None))
        )
        
        # 2. Sequence Learning (RNN)
        # Sử dụng Bidirectional LSTM để nắm bắt đặc trưng trước/sau
        self.rnn = nn.LSTM(input_size=512, hidden_size=256, bidirectional=True, num_layers=2, batch_first=True)
        
        # 3. Transcription (Linear Head)
        # Dự đoán phân phối xác suất trên tổng số ký tự
        self.fc = nn.Linear(256 * 2, num_classes) # * 2 vì bidirectional

    def forward(self, x):
        # Đặc trưng từ CNN: có shape [batch_size, channels, H, W]
        conv_feats = self.cnn(x)
        
        # Trích xuất chiều: [batch_size, 512, 1, W] -> loại bỏ chiều H
        batch, channels, _, width = conv_feats.size()
        conv_feats = conv_feats.squeeze(2) # [batch_size, 512, width]
        
        # Để đưa vào RNN (batch_first), cần đổi chiều: [batch_size, width (timesteps), channels]
        conv_feats = conv_feats.permute(0, 2, 1)
        
        # RNN trả về chuỗi dự đoán
        rnn_out, _ = self.rnn(conv_feats)
        
        # Phân loại cho từng timestep
        outputs = self.fc(rnn_out)
        
        # Trả về shape [timesteps, batch, num_classes] chuẩn cho CTCLoss trong PyTorch 
        # (Nếu loss require batch_first=False)
        return outputs.permute(1, 0, 2)
