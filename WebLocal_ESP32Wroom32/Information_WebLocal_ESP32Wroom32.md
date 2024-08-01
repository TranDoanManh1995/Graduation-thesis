# Information about "WebLocal & ESP32Wroom32"
**Giao diện WebLocal** được sử dụng để giao tiếp với thiết bị công tắc cầu thang thông qua module ESP32Wroom32. Việc truy cập vào *“Giao diện WebLocal”* được thực hiện bằng địa chỉ IP sau khi ESP32 kết nối Wi-Fi thành công. Tính năng điều khiển trên giao diện Web bao gồm: 
- Tính năng mạng Zigbee: tạo, mở, đóng và xóa thiết bị khỏi mạng.
- Tính năng điều khiển đèn cầu thang.
- Tính năng cập nhật, hiển thị trạng thái mạng của thiết bị & trạng thái đèn cầu thang.
- Tính năng câp nhật, hiển thị thông số môi trường: nhiệt độ, độ ẩm, ánh sáng.

**ESP32** ***(Server)*** là nơi lập trình các tính năng cho *“Giao diện WebLocal”*, lưu trữ và xử lý các yêu cầu của giao diện đó. ESP32 sẽ tiếp nhận các request từ *“Giao diện WebLocal”* gửi response cho nó thông qua giao thức HTTP. **Ngoài ra**, ESP32 còn đóng vai trò trung gian trong giao tiếp giữa *“Giao diện WebLocal”* và thiết bị công tắc cầu thang.