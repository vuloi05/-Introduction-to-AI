import torch
import torch.nn as nn
import torch.optim as optim
from models.plate_detector import CustomPlateDetector
from models.character_recognizer import CustomCRNN

def train_plate_detector(epochs=50, lr=1e-3, device='cpu'):
    """
    Quy trình huấn luyện Custom License Plate Detector.
    Áp dụng SmoothL1 cho độ lệch Box, BCE cho tỷ lệ Objectness.
    """
    model = CustomPlateDetector().to(device)
    optimizer = optim.Adam(model.parameters(), lr=lr)
    
    criterion_bbox = nn.SmoothL1Loss()
    criterion_conf = nn.BCEWithLogitsLoss()
    
    print(f"Bắt đầu huấn luyện Detector trên thiết bị {device}")
    model.train()
    
    for epoch in range(epochs):
        # Mock training step do không có dữ liệu thực tế
        # inputs, targets_bbox, targets_conf = next(dataloader)
        
        optimizer.zero_grad()
        
        # Dummy forward pass example
        dummy_inputs = torch.randn((4, 3, 224, 224)).to(device)
        dummy_bbox_gt = torch.randn((4, 4)).to(device)
        dummy_conf_gt = torch.ones((4, 1)).to(device)
        
        pred_bbox, pred_conf = model(dummy_inputs)
        
        # Tính Loss
        loss_bbox = criterion_bbox(pred_bbox, dummy_bbox_gt)
        loss_conf = criterion_conf(pred_conf, dummy_conf_gt)
        loss = loss_bbox + loss_conf
        
        # Backprop
        loss.backward()
        optimizer.step()
        
        if (epoch + 1) % 10 == 0:
            print(f"Epoch [{epoch+1}/{epochs}], Loss: {loss.item():.4f}")
            
    # Lưu trọng số
    torch.save(model.state_dict(), "detector_weights.pth")
    print("Huấn luyện Detector hoàn tất.")

def train_crnn(epochs=50, lr=1e-3, device='cpu'):
    """
    Quy trình huấn luyện Mạng nhận diện ký tự (CRNN).
    Sử dụng CTCLoss để ánh xạ xác suất thời gian tuyến tính với chuỗi nhãn.
    """
    model = CustomCRNN(num_classes=37).to(device)
    optimizer = optim.Adam(model.parameters(), lr=lr)
    criterion_ctc = nn.CTCLoss(blank=36, zero_infinity=True)
    
    print(f"Bắt đầu huấn luyện CRNN trên thiết bị {device}")
    model.train()
    
    for epoch in range(epochs):
        optimizer.zero_grad()
        
        # Mẫu giả lập đầu vào của CRNN
        dummy_inputs = torch.randn((4, 3, 32, 128)).to(device) # Height 32, Width 128
        preds = model(dummy_inputs) # Shape: [timesteps, batch, num_classes]
        
        # Giả lập mục tiêu chuỗi
        timesteps, batch_size, _ = preds.size()
        preds_lengths = torch.full(size=(batch_size,), fill_value=timesteps, dtype=torch.long)
        targets = torch.randint(low=1, high=36, size=(batch_size, 10)) # VD: 10 ký tự mỗi biển số
        target_lengths = torch.full(size=(batch_size,), fill_value=10, dtype=torch.long)
        
        # Tính loss
        preds_log_softmax = nn.functional.log_softmax(preds, dim=2)
        loss = criterion_ctc(preds_log_softmax, targets, preds_lengths, target_lengths)
        
        # Backprop
        loss.backward()
        optimizer.step()
        
        if (epoch + 1) % 10 == 0:
            print(f"Epoch [{epoch+1}/{epochs}], Loss: {loss.item():.4f}")
            
    # Lưu trọng số
    torch.save(model.state_dict(), "crnn_weights.pth")
    print("Huấn luyện CRNN hoàn tất.")

if __name__ == "__main__":
    device = torch.device('cuda' if torch.cuda.is_available() else 'cpu')
    train_plate_detector(epochs=10, device=device)
    train_crnn(epochs=10, device=device)
