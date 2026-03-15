import torch
import torch.nn as nn
import torch.nn.functional as F

class CustomPlateDetector(nn.Module):
    """
    Mạng CNN phát hiện vùng chứa kích thước biển số.
    Sử dụng kiến trúc thu gọn để đạt tốc độ real-time.
    """
    def __init__(self, in_channels=3):
        super(CustomPlateDetector, self).__init__()
        
        # Backbone (Feature Extraction)
        # Sử dụng các khối Conv mỏng để tiết kiệm chi phí tính toán
        self.backbone = nn.Sequential(
            self._conv_block(in_channels, 32),
            nn.MaxPool2d(2, 2), # Giảm kích thước / 2
            
            self._conv_block(32, 64),
            nn.MaxPool2d(2, 2), # Giảm kích thước / 4
            
            self._conv_block(64, 128),
            nn.MaxPool2d(2, 2), # Giảm kích thước / 8
            
            self._conv_block(128, 256),
            nn.MaxPool2d(2, 2), # Giảm kích thước / 16
        )
        
        # Detection Head
        # Dự đoán Bounding Box [x, y, w, h] (4 giá trị)
        self.bbox_head = nn.Sequential(
            nn.Conv2d(256, 128, kernel_size=3, padding=1),
            nn.ReLU(inplace=True),
            nn.AdaptiveAvgPool2d(1), # Ép về 1x1 map
            nn.Flatten(),
            nn.Linear(128, 4)
        )
        
        # Dự đoán Objectness (Xác suất có biển số) (1 giá trị logit)
        self.conf_head = nn.Sequential(
            nn.Conv2d(256, 128, kernel_size=3, padding=1),
            nn.ReLU(inplace=True),
            nn.AdaptiveAvgPool2d(1),
            nn.Flatten(),
            nn.Linear(128, 1)
        )

    def _conv_block(self, in_channels, out_channels):
        return nn.Sequential(
            nn.Conv2d(in_channels, out_channels, kernel_size=3, stride=1, padding=1, bias=False),
            nn.BatchNorm2d(out_channels),
            nn.ReLU(inplace=True),
            # Tăng cường bằng lớp thứ 2 để map đặc trưng tốt hơn
            nn.Conv2d(out_channels, out_channels, kernel_size=3, stride=1, padding=1, bias=False),
            nn.BatchNorm2d(out_channels),
            nn.ReLU(inplace=True)
        )

    def forward(self, x):
        # Trích xuất đặc trưng
        features = self.backbone(x)
        
        # Dự đoán phân nhánh
        bbox_preds = self.bbox_head(features)
        conf_preds = self.conf_head(features)
        
        return bbox_preds, conf_preds
