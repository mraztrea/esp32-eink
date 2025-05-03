# Workflow: Hiển thị ảnh từ internet lên màn hình E-Ink

## Yêu cầu chức năng

- Kết nối WiFi với SSID "HungThu", password "12345678"
- Định kỳ 5 phút tải ảnh từ URL
- Hiển thị ảnh đã tải lên màn hình E-Ink 4.2 inch

## Thư viện cần thiết

- WiFi.h - Kết nối WiFi
- HTTPClient.h - Tải dữ liệu từ internet
- WiFiClientSecure.h - Kết nối HTTPS (nếu cần)
- GxEPD2_3C.h - Điều khiển màn hình E-Ink
- Adafruit_GFX.h - Thư viện đồ họa cơ bản
- SPI.h - Giao tiếp SPI với màn hình
- JPEGDecoder.h - Giải mã ảnh JPEG (nếu cần)

## Sơ đồ kết nối phần cứng

- **ESP32:**
  - CS: GPIO 15
  - DC: GPIO 27
  - RST: GPIO 26
  - BUSY: GPIO 25
  - SCLK: GPIO 13
  - MOSI: GPIO 14

## Code mẫu

```cpp
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <GxEPD2_3C.h>
#include <Adafruit_GFX.h>
#include <SPI.h>

// Thông tin WiFi
const char* ssid = "HungThu";
const char* password = "12345678";

// URL ảnh cần tải
const char* imageUrl = "https://dummyimage.com/400x300/000000/fff&text=Thanh+dep+trai";

// Cài đặt màn hình E-Ink
GxEPD2_3C<GxEPD2_420_Z98c, GxEPD2_420_Z98c::HEIGHT> display(
    GxEPD2_420_Z98c(/*CS=*/ 15, /*DC=*/ 27, /*RST=*/ 26, /*BUSY=*/ 25)
);

// Buffer lưu dữ liệu ảnh
uint8_t* imageBuffer = NULL;
size_t imageSize = 0;

// Biến thời gian
unsigned long previousMillis = 0;
const long interval = 5 * 60 * 1000; // 5 phút

void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.println("E-Ink WiFi Image Display");

  // Khởi tạo SPI
  SPI.begin(13, -1, 14, 15);

  // Khởi tạo màn hình
  display.init();
  display.setFullWindow();

  // Hiển thị thông báo đang kết nối
  displayMessage("Đang kết nối WiFi...");

  // Kết nối WiFi
  connectWiFi();

  // Tải và hiển thị ảnh lần đầu
  downloadAndDisplayImage();
}

void loop() {
  unsigned long currentMillis = millis();

  // Kiểm tra xem đã đến thời gian cập nhật ảnh chưa
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    // Kiểm tra kết nối WiFi
    if (WiFi.status() != WL_CONNECTED) {
      displayMessage("Mất kết nối WiFi. Đang kết nối lại...");
      connectWiFi();
    }

    // Tải và hiển thị ảnh mới
    downloadAndDisplayImage();
  }

  // Có thể thêm xử lý thêm ở đây nếu cần
  delay(1000);
}

void connectWiFi() {
  WiFi.begin(ssid, password);

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println();
    Serial.print("Đã kết nối WiFi. IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("Không thể kết nối WiFi. Thử lại sau.");
  }
}

void downloadAndDisplayImage() {
  Serial.println("Bắt đầu tải ảnh...");
  displayMessage("Đang tải ảnh...");

  if (WiFi.status() == WL_CONNECTED) {
    WiFiClientSecure client;
    client.setInsecure(); // Bỏ qua xác thực SSL

    HTTPClient http;
    http.begin(client, imageUrl);

    int httpCode = http.GET();

    if (httpCode == HTTP_CODE_OK) {
      int contentLength = http.getSize();
      Serial.print("Kích thước ảnh: ");
      Serial.println(contentLength);

      // Giải phóng bộ nhớ cũ nếu có
      if (imageBuffer != NULL) {
        free(imageBuffer);
        imageBuffer = NULL;
      }

      // Cấp phát bộ nhớ mới
      imageBuffer = (uint8_t*)malloc(contentLength);

      if (imageBuffer == NULL) {
        Serial.println("Không đủ bộ nhớ để lưu ảnh");
        displayMessage("Lỗi: Không đủ bộ nhớ");
        http.end();
        return;
      }

      // Tải dữ liệu ảnh
      WiFiClient* stream = http.getStreamPtr();
      size_t totalRead = 0;

      while (http.connected() && totalRead < contentLength) {
        size_t available = stream->available();
        if (available) {
          size_t bytesToRead = min(available, contentLength - totalRead);
          size_t readBytes = stream->readBytes(imageBuffer + totalRead, bytesToRead);
          totalRead += readBytes;
        }
        delay(1);
      }

      imageSize = totalRead;
      Serial.print("Đã tải: ");
      Serial.println(imageSize);

      // Hiển thị ảnh lên màn hình
      displayImage();

    } else {
      Serial.print("Lỗi HTTP: ");
      Serial.println(httpCode);
      displayMessage("Lỗi khi tải ảnh");
    }

    http.end();
  } else {
    displayMessage("Không có kết nối WiFi");
  }
}

void displayImage() {
  Serial.println("Hiển thị ảnh lên màn hình E-Ink");

  // Ở đây cần xử lý dữ liệu ảnh để hiển thị lên E-Ink
  // Đối với màn hình 3 màu, cần chuyển đổi dữ liệu ảnh thành dạng phù hợp

  display.setFullWindow();
  display.firstPage();

  do {
    display.fillScreen(GxEPD_WHITE);

    // Hiển thị ảnh từ dữ liệu đã tải
    // Cần phân tích dữ liệu PNG/JPEG và chuyển thành dạng bitmap phù hợp
    // Đoạn code này phụ thuộc vào định dạng ảnh cụ thể

    // Ví dụ hiển thị thông báo thành công
    display.setTextColor(GxEPD_BLACK);
    display.setCursor(50, 150);
    display.println("Đã tải ảnh thành công!");

  } while (display.nextPage());

  Serial.println("Đã cập nhật màn hình");
  display.hibernate();
}

void displayMessage(const char* message) {
  Serial.println(message);

  display.setFullWindow();
  display.firstPage();

  do {
    display.fillScreen(GxEPD_WHITE);
    display.setTextColor(GxEPD_BLACK);
    display.setCursor(10, 100);
    display.println(message);
  } while (display.nextPage());

  display.hibernate();
}
```

## Lưu ý quan trọng

1. **Xử lý ảnh PNG/JPEG**:

   - ESP32 có bộ nhớ giới hạn nên việc xử lý ảnh lớn có thể gặp khó khăn
   - Cần thư viện giải mã phù hợp như JPEGDecoder hoặc PNGdec
   - Có thể cần server trung gian để chuyển đổi ảnh sang định dạng phù hợp với E-Ink

2. **Tiết kiệm pin**:

   - Nếu dùng pin, cần thêm chế độ deep sleep giữa các lần cập nhật
   - Ví dụ: hiển thị xong -> deep sleep 5 phút -> wake up -> tải ảnh mới

3. **Bảo mật**:

   - Trong code mẫu, chúng ta bỏ qua xác thực SSL (`client.setInsecure()`)
   - Trong môi trường thực tế, nên cấu hình chứng chỉ SSL đúng cách

4. **Xử lý lỗi mạng**:
   - Thêm cơ chế retry khi tải ảnh thất bại
   - Hiển thị ảnh cũ nếu không tải được ảnh mới

## Tối ưu code

- Cân nhắc giảm kích thước ảnh tải về hoặc sử dụng định dạng hiệu quả hơn
- Sử dụng các thư viện xử lý ảnh nhẹ nhàng, phù hợp với ESP32
- Sử dụng deep sleep để tiết kiệm pin giữa các lần cập nhật
