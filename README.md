# 🔘 Smart Button Mode Switcher (ESP-IDF)

> Một dự án Embedded C tối giản nhưng mạnh mẽ, minh họa cách xử lý đa nhiệm (Non-blocking), quản lý Ngắt (Interrupt) và Con trỏ (Pointer) trên nền tảng ESP-IDF.

![Language](https://img.shields.io/badge/Language-C-blue.svg)
![Platform](https://img.shields.io/badge/Platform-ESP--IDF-red.svg)
![Hardware](https://img.shields.io/badge/Hardware-ESP32%2FESP8266-green.svg)

## 📖 Giới thiệu (Overview)
Project này giải quyết bài toán kinh điển trong lập trình nhúng: **Làm sao để một nút nhấn đơn giản có thể điều khiển hệ thống phức tạp mà không làm treo vi xử lý?**

Thay vì sử dụng vòng lặp chặn (`delay`), dự án áp dụng tư duy **Non-blocking I/O** kết hợp với **State Machine** (Máy trạng thái) để điều khiển nhiều đèn LED hoạt động song song với tần số khác nhau.

## 🗝️ 4 Từ khóa cốt lõi (Core Concepts)
Dự án được xây dựng xoay quanh 4 kỹ thuật nền tảng:

1.  **GPIO**: Cấu hình Input (Pull-up) cho nút nhấn và Output cho LED.
2.  **Interrupt (Ngắt)**: Bắt sự kiện nhấn nút tức thời mà không cần kiểm tra liên tục (polling), giúp tiết kiệm tài nguyên CPU.
3.  **Debounce (Khử rung)**: Thuật toán lọc nhiễu tín hiệu cơ học của nút nhấn dựa trên so sánh thời gian thực (Hardware Timer).
4.  **Pointer (Con trỏ)**: Sử dụng con trỏ để thao tác trực tiếp và an toàn lên biến trạng thái (`struct`) giữa các tầng xử lý (ISR -> Main Loop).

## ⚙️ Tính năng (Features)
* **Chuyển chế độ bằng một nút nhấn**:
    * 🟢 **Mode FAST**: LED nháy nhanh (100ms).
    * 🟡 **Mode SLOW**: LED nháy chậm (1000ms).
    * ⚫ **Mode OFF**: Tắt LED.
* **Status LED độc lập**: Một đèn LED phụ nháy nền (Heartbeat) để báo hiệu hệ thống đang chạy, hoàn toàn không bị ảnh hưởng bởi đèn chính.
* **Zero-Delay**: Không sử dụng `vTaskDelay` hay `delay` để làm trễ logic, đảm bảo nút nhấn luôn phản hồi tức thì (Real-time responsiveness).

## 🛠️ Phần cứng (Hardware)
* Kit phát triển: ESP32 hoặc ESP8266 (NodeMCU).
* 1x Button (Nối GPIO 19).
* 1x LED chính (Nối GPIO 21).
* 1x LED trạng thái (Nối GPIO 18).

### Sơ đồ đấu nối (Pinout)
| Component | GPIO Pin (ESP32) | Mode | Note |
|-----------|------------------|------|------|
| **Button**| GPIO 19 | Input | Pull-up (Nối đất khi nhấn) |
| **Main LED**| GPIO 21 | Output | Active High/Low tùy mạch |
| **Status LED**| GPIO 18 | Output | Nháy nền 200ms |

## 🧠 Luồng xử lý (Logic Flow)

```mermaid
graph TD;
    Start([Khởi động]) --> Init[Cấu hình GPIO & Ngắt];
    Init --> Loop{Vòng lặp vô tận};
    
    subgraph "Interrupt Service Routine (ISR)"
        BtnClick[Nhấn nút] --> Debounce{Check > 200ms?};
        Debounce -- Yes --> SetFlag[Bật cờ g_button_pressed];
        Debounce -- No --> Ignore[Bỏ qua];
    end

    subgraph "Main Loop (Non-blocking)"
        Loop -- Check Flag --> FlagOn{Cờ == True?};
        FlagOn -- Yes --> PtrChange[Dùng Pointer đổi State] --> ResetFlag[Hạ cờ];
        FlagOn -- No --> CheckTimer;
        
        CheckTimer -- Đủ thời gian --> ToggleLED[Đảo trạng thái LED];
        CheckTimer -- Chưa đủ --> TaskDelay[Nhường CPU 10ms];
    end
