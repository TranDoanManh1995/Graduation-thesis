# Giải pháp IoT cho thiết bị "CÔNG TẮC CẦU THANG"
## Tổng quan về giải pháp
Trong đồ án này, tôi tạo ra thiết bị ***"Công tắc cầu thang"*** để bật/tắt đèn sử dụng đồng bộ 03 phương thức: 
- Điều khiển đèn sử dụng nút bấm cơ trên công tắc
- Điều khiển đèn từ xa sử dụng trình duyệt Web
- Điều khiển đèn tự động sử dụng cảm biến chuyển động PIR
## Mục đích của giải pháp
 Việc sử dụng “Giải pháp IoT cho thiết bị công tắc cầu thang” sẽ mang đến trải nghiệm tiện lợi, linh hoạt cho người sử dụng so với công tắc cầu thang truyền thống như tính năng tự động hóa việc bật/tắt, tính năng điều khiển từ xa bằng điện thoại di động và các thiết bị thông minh khác. Đây là một xu hướng không thể phủ nhận trong thế giới ngày nay, khi mà sự kết nối và tiện ích là những yếu tố quan trọng trong cuộc sống hiện đại.
 ## Mô hình thiết bị "Công tắc cầu thang"
![Mô hình thiết bị "Công tắc cầu thang"]([image.jpg](https://drive.google.com/file/d/1Es6vH57kfZSiTpoEwLuS1_35K2RROFHF/view?usp=sharing))
## Thành phần thiết bị "Công tắc cầu thang"
- **WebLocal** để thực hiện các tính năng của mạng Zigbee, điều khiển đèn cầu thang và cập nhật thông số của cảm biến.
- 01 thiết bị **EFR32** có nhiệm vụ tạo mạng và quản lý toàn bộ các thiết bị gia nhập mạng được gọi là **Zigbee Coordinator** *(ZC)*. 
- 02 thiết bị **EFR32** ***(Zigbee Router – ZR)*** đóng vai trò là 02 công tắc cầu thang.
- 01 thiết bị **ESP32** đóng vai trò là **Server** trong giao tiếp giữa trình duyệt *(Client)* với ESP32 *(Server)* và được sử dụng để giao tiếp với ZC. Để truy cập vào WebLocal, chúng ta sẽ sử dụng **địa chỉ IP** mà ESP32 được cấp phát sau khi kết nối đến mạng Wifi khả dụng.
## Tính năng thiết bị "Công tắc cầu thang"
- Thực hiện việc tạo mạng, mở mạng, dừng mở mạng và xóa thiết bị khỏi mạng sử dụng SWs trên ZC và WebLocal.
- Update trạng thái gia nhập, rời mạng của các Switches lên WebLocal.
- Thiết lập 02 chế độ điều khiển sử dụng đồng bộ nút bấm cơ, WebLocal và update chế độ điều khiển lên WebLocal:
    - Chế độ Auto: Sử dụng WebLocal + Nút bấm cơ + cảm biến chuyển động PIR để bật/tắt đèn cầu thang.
    - Chế độ Manual: Sử dụng WebLocal + Nút bấm cơ để bật/tắt đèn cầu thang.
- Sử dụng nút bấm cơ điều khiển đồng bộ 2 công tắc để bật/tắt đèn cầu thang và update trạng thái đèn lên WebLocal.
- Sử dụng cảm biến PIR điều khiển bật/tắt đèn cầu thang và update trạng thái đèn lên WebServer *(điều khiển đồng bộ với nút bấm cơ).*
- Sử dụng Web để điều khiển bật/tắt đèn cầu thang và update trạng thái đèn lên WebServer.
## Hướng phát triển của giải pháp
Trong phạm vi đồ án, tôi lựa chọn giải pháp sử dụng WebLocal để thực hiện việc điều khiển từ xa đèn cầu thang. Sau đồ án này, để tăng thêm tính năng của sản phẩm và nâng cao kiến thức của bản thân, tôi muốn tìm hiểu và làm thêm App điều khiển; cải thiện tính năng Login; sử dụng thêm các giao thức giao tiếp khác như MQTT, WebSocket; sử dụng hệ quản trị database MySQL...

