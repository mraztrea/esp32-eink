# Workflow: Thiết lập màn hình E-Ink 4.2 inch

## Các thư viện cần thiết

- GxEPD2: Thư viện điều khiển màn hình E-Ink
- Adafruit_GFX: Thư viện đồ họa cơ bản (bắt buộc cho GxEPD2)
- Thư viện cho fonts

## Cài đặt thư viện

1. Mở Arduino IDE
2. Chọn "Sketch" > "Include Library" > "Manage Libraries..."
3. Cài đặt các thư viện sau:
   - Adafruit GFX Library
   - GxEPD2

## Kết nối phần cứng

- CS: GPIO 15
- DC: GPIO 27
- RST: GPIO 26
- BUSY: GPIO 25
- SCLK: GPIO 13
- MOSI: GPIO 14

## Thông số màn hình 4.2 inch

- Độ phân giải: 400x300px
- 3 màu: đen, trắng và đỏ

## Mẫu code cơ bản

```cpp
#include <GxEPD2_3C.h>
#include <Adafruit_GFX.h>
#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/FreeMonoBold18pt7b.h>
#include <SPI.h>

// Khởi tạo màn hình
GxEPD2_3C<GxEPD2_420_Z98c, GxEPD2_420_Z98c::HEIGHT> display(
    GxEPD2_420_Z98c(/*CS=*/ 15, /*DC=*/ 27, /*RST=*/ 26, /*BUSY=*/ 25)
);

void setup() {
  Serial.begin(115200);

  // Khởi tạo SPI
  SPI.begin(13, -1, 14, 15); // SCLK=13, MISO=-1 (không dùng), MOSI=14, CS=15

  // Khởi tạo màn hình
  display.init();

  // Xóa màn hình (nền trắng)
  display.clearScreen();

  // Hiển thị nội dung
  displayContent();

  // Chế độ sleep để tiết kiệm điện
  display.hibernate();
}

void displayContent() {
  display.setFullWindow();
  display.firstPage();

  do {
    display.fillScreen(GxEPD_WHITE);

    // Hiển thị văn bản màu đen
    display.setTextColor(GxEPD_BLACK);
    display.setFont(&FreeMonoBold18pt7b);

    // Căn giữa văn bản
    String text1 = "E-INK 4.2INCH";
    int16_t x1, y1;
    uint16_t w1, h1;
    display.getTextBounds(text1, 0, 0, &x1, &y1, &w1, &h1);
    display.setCursor((300 - w1) / 2, 120);
    display.println(text1);

    // Hiển thị văn bản màu đỏ
    display.setTextColor(GxEPD_RED);
    String text2 = "Quy Nguyen";
    int16_t x2, y2;
    uint16_t w2, h2;
    display.getTextBounds(text2, 0, 0, &x2, &y2, &w2, &h2);
    display.setCursor((300 - w2) / 2, 220);
    display.println(text2);

  } while (display.nextPage());
}

void loop() {
  // Không cần làm gì trong loop
}
```

## Lưu ý

- Phải cài đặt Adafruit_GFX trước khi cài GxEPD2
- Màn hình E-Ink chỉ nên được cập nhật khi cần thiết để tiết kiệm pin
- Sử dụng hibernate() khi không cần cập nhật màn hình

## Xử lý lỗi khi upload

### Lỗi "No DFU capable USB device available"

Lỗi này xảy ra khi không thể tìm thấy thiết bị ESP32 ở chế độ bootloader. Cách khắc phục:

1. **Kiểm tra kết nối phần cứng**:

   - Đảm bảo cáp USB kết nối tốt và không bị lỗi
   - Thử dùng cáp USB khác
   - Thử cổng USB khác trên máy tính

2. **Đặt ESP32 vào chế độ bootloader**:

   - Nhấn và giữ nút BOOT trên ESP32
   - Trong khi vẫn giữ nút BOOT, nhấn nút RESET
   - Thả nút RESET trước, sau đó thả nút BOOT
   - Bắt đầu upload lại

3. **Kiểm tra cài đặt trong Arduino IDE**:

   - Chọn đúng board trong menu Tools > Board
   - Chọn đúng cổng COM trong menu Tools > Port
   - Với một số board ESP32, cần cài đặt tốc độ upload thấp hơn (Tools > Upload Speed)

4. **Cài đặt driver USB**:

   - Cài đặt driver CP210x (Silicon Labs) hoặc CH340 tùy theo chip trên board ESP32
   - Kiểm tra trong Device Manager (Windows) xem thiết bị có được nhận diện không

5. **Đối với ESP32-S2/S3**:
   - Bật tùy chọn USB CDC On Boot (Tools > USB CDC On Boot > Enabled)
   - Đối với một số board, cần chọn USB Mode là "Hardware CDC and JTAG" thay vì DFU

### Lỗi khác khi upload

Nếu gặp các lỗi khác, thử các giải pháp sau:

- Khởi động lại IDE
- Khởi động lại máy tính
- Đảm bảo có đủ quyền truy cập vào cổng USB
- Kiểm tra xem có ứng dụng nào khác đang chiếm cổng serial không

## Debug ESP32

Có nhiều cách để debug code trên ESP32:

### 1. Debug thông qua Serial Monitor (đơn giản nhất)

```cpp
void setup() {
  Serial.begin(115200); // Khởi tạo giao tiếp Serial với baud rate 115200
  Serial.println("Khởi động hệ thống");

  // Thêm thông báo debug vào các điểm quan trọng trong code
  Serial.println("Bắt đầu khởi tạo SPI");
  SPI.begin(13, -1, 14, 15);
  Serial.println("SPI đã khởi tạo xong");

  // ...
}
```

Để xem output:

1. Upload code lên ESP32
2. Mở Serial Monitor (biểu tượng kính lúp hoặc Ctrl+Shift+M)
3. Chọn đúng tốc độ Baud rate (115200)

### 2. Debug thông qua JTAG (cần hardware hỗ trợ)

Nếu gặp lỗi "OpenOCD: GDB Server Quit Unexpectedly":

1. **Kiểm tra kết nối hardware**:

   - Đảm bảo bạn có hardware adapter hỗ trợ debug (như ESP-PROG)
   - Kết nối đúng các chân JTAG (TDO, TDI, TCK, TMS)

2. **Cài đặt driver và công cụ**:

   - Cài đặt đúng driver cho adapter debug
   - Cài đặt OpenOCD hỗ trợ ESP32
   - Cài đặt plugin ESP32 cho IDE (nếu dùng VSCode hoặc PlatformIO)

3. **Giải pháp thay thế**:
   - Nếu không có hardware chuyên dụng, hãy sử dụng Serial Monitor
   - Thêm nhiều lệnh `Serial.println()` vào code để theo dõi tiến trình
   - Kiểm tra trong Serial Monitor các thông báo lỗi

### 3. Giải quyết vấn đề sau khi upload code

Nếu màn hình E-Ink không hiển thị sau khi upload:

1. Kiểm tra kết nối phần cứng:

   - Các chân kết nối giữa ESP32 và màn hình E-Ink đã đúng chưa
   - Nguồn điện cấp cho màn hình đã đủ chưa

2. Debug qua Serial:

   ```cpp
   // Thêm những dòng debug này vào code
   Serial.println("Bắt đầu khởi tạo màn hình");
   display.init();
   Serial.println("Màn hình đã khởi tạo xong");

   Serial.println("Bắt đầu xóa màn hình");
   display.clearScreen();
   Serial.println("Đã xóa màn hình xong");
   ```

3. Kiểm tra lỗi thư viện:
   - Xác nhận đã cài đặt đúng phiên bản GxEPD2 và Adafruit_GFX
   - Xác nhận đã khai báo đúng model màn hình (GxEPD2_420_Z98c)
