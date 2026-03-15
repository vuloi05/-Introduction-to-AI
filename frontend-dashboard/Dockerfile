# Môi trường Node.js nhỏ gọn, thiết kế linh hoạt cho dev environment (Vite/React)
FROM node:18-alpine

# Set working directory
WORKDIR /app

# Copy package cơ bản nếu có
COPY package.json package-lock.json* ./

# Cài đặt môi trường. Lệnh fallback tránh lỗi Docker nếu folder trống lúc init project
RUN if [ -f package.json ]; then npm install; fi

# Cài Vite global để có thể chạy dev server trực tiếp nếu chưa npm install từ đầu
RUN npm install -g vite

# Export cổng dev server mặc định của Vite
EXPOSE 5173

# Chạy server với host 0.0.0.0 để expose cổng ra khỏi container, phục vụ hot-realoading
CMD ["vite", "--host", "0.0.0.0"]
