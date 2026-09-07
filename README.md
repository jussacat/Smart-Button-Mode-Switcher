# Smart Button Mode Switcher (ESP-IDF)
## Overview
Đây là một Project tối giản minh họa hệ thống chuyển chế độ nháy LED bằng nút bấm và một LED được nháy độc lập để kiểm tra hệ thống hoạt động.

Dự án được xây dựng xoay quanh 4 kỹ thuật nền tảng:

1.  **GPIO**: Cấu hình Input (Pull-up) cho nút nhấn và Output cho LED.
2.  **Interrupt (Ngắt)**: Bắt sự kiện nhấn nút tức thời mà không cần kiểm tra liên tục (polling), giúp tiết kiệm tài nguyên CPU.
3.  **Debounce (Chống dội phím)**: Thuật toán lọc nhiễu tín hiệu cơ học của nút nhấn dựa trên so sánh thời gian thực.
4.  **Pointer (Con trỏ)**: Sử dụng con trỏ để thao tác trực tiếp và an toàn lên biến trạng thái (`struct`) giữa các tầng xử lý (ISR -> Main Loop).

## Tính năng (Features)
* **Chuyển chế độ bằng một nút nhấn**:
    * **Mode FAST**: LED nháy nhanh (500ms).
    * **Mode SLOW**: LED nháy chậm (2000ms).
    * **Mode OFF**: Dừng nháy LED (LED đỏ sẽ sáng, LED xanh sẽ tắt).
* **LED xanh độc lập**: Một đèn LED phụ nháy nền để báo hiệu hệ thống đang chạy, hoàn toàn không bị ảnh hưởng bởi đèn chính.
* **Zero-Delay**: Không sử dụng `vTaskDelay` hay `delay` để làm trễ logic, đảm bảo nút nhấn luôn phản hồi tức thì.

## Hardware
* Kit phát triển: ESP32-VROOM-32D.
* 1x Button (Nối GPIO 19).
* 1x LED đỏ (Nối GPIO 21).
* 1x LED xanh (Nối GPIO 18).

### Sơ đồ đấu nối (Pinout)
| Component | GPIO Pin (ESP32) | Mode | Note |
|-----------|------------------|------|------|
| **Button**| GPIO 19 | Input | Pull-up (Nối đất khi nhấn) |
| **Red LED**| GPIO 21 | Output | Active High/Low tùy mạch |
| **Green LED**| GPIO 18 | Output | Nháy nền 200ms |

## Logic Flow

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
