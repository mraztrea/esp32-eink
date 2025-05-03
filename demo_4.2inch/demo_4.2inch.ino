#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <GxEPD2_3C.h>
#include <Adafruit_GFX.h>
#include <Fonts/FreeMonoBold9pt7b.h>
// Thêm font lớn hơn
#include <Fonts/FreeMonoBold18pt7b.h>
#include <SPI.h>
#include <JPEGDecoder.h> // Thêm thư viện JPEGDecoder

// Thông tin WiFi
const char* ssid = "Z117_3";
const char* password = "chiquynhbo";

// URL ảnh cần tải
const char* imageUrl = "https://dummyimage.com/400x300/000000/fff.jpg&text=Thanh+dep+trai";

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
  Serial.println("E-Ink WiFi Image Display - Tự động cập nhật ảnh từ internet");
  
  // Khởi tạo SPI
  SPI.begin(13, -1, 14, 15);
  
  // Khởi tạo màn hình
  Serial.println("Khởi tạo màn hình E-Ink");
  display.init();
  display.setFullWindow();
  
  // Hiển thị thông báo đang kết nối
  displayMessage("Dang ket noi WiFi...");
  
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
    
    Serial.println("Đã đến thời gian cập nhật ảnh (5 phút)");
    
    // Kiểm tra kết nối WiFi
    if (WiFi.status() != WL_CONNECTED) {
      displayMessage("Mat ket noi WiFi. Dang ket noi lai...");
      connectWiFi();
    }
    
    // Tải và hiển thị ảnh mới
    downloadAndDisplayImage();
  }
  
  delay(1000);
}

void connectWiFi() {
  Serial.println("Kết nối WiFi với SSID: " + String(ssid));
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
    displayMessage("Da ket noi WiFi!");
  } else {
    Serial.println("Không thể kết nối WiFi. Thử lại sau.");
    displayMessage("Loi ket noi WiFi!");
  }
}

void downloadAndDisplayImage() {
  Serial.println("Bắt đầu tải ảnh từ URL: " + String(imageUrl));
  displayMessage("Dang tai anh...");
  
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
        displayMessage("Loi: Khong du bo nho");
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
          
          // In tiến trình tải
          if (totalRead % 10000 == 0 || totalRead == contentLength) {
            Serial.print("Đã tải: ");
            Serial.print(totalRead);
            Serial.print(" / ");
            Serial.println(contentLength);
          }
        }
        delay(1);
      }
      
      imageSize = totalRead;
      Serial.println("Đã tải xong ảnh từ internet");
      
      // Hiển thị ảnh lên màn hình
      displayImage();
      
    } else {
      Serial.print("Lỗi HTTP: ");
      Serial.println(httpCode);
      displayMessage("Loi khi tai anh: " + String(httpCode));
    }
    
    http.end();
  } else {
    displayMessage("Khong co ket noi WiFi");
  }
}

void displayImage() {
  Serial.println("Xử lý và hiển thị ảnh lên màn hình E-Ink");
  
  display.setFullWindow();
  display.firstPage();
  
  do {
    display.fillScreen(GxEPD_WHITE);
    
    if (imageBuffer != NULL && imageSize > 0) {
      // Giải mã và hiển thị JPEG
      if (drawJpegFromBuffer(imageBuffer, imageSize)) {
        Serial.println("Hiển thị ảnh thành công");
      } else {
        // Nếu không thể hiển thị, hiển thị thông báo lỗi
        display.setTextColor(GxEPD_BLACK);
        display.setTextSize(1);
        display.setCursor(50, 100);
        display.println("Loi giai ma JPEG");
        display.setCursor(50, 120);
        display.println("Kich thuoc: " + String(imageSize) + " bytes");
      }
    } else {
      // Hiển thị thông báo không có ảnh
      display.setTextColor(GxEPD_BLACK);
      display.setTextSize(2);
      display.setCursor(50, 50);
      display.println("Thanh Dep Trai");
      
      display.setTextSize(1);
      display.setCursor(50, 100);
      display.println("Khong co du lieu anh");
      display.setCursor(50, 120);
      display.println("Dang thu tai lai...");
    }
    
    // Vẽ thời gian cập nhật
    display.setTextColor(GxEPD_RED);
    display.setCursor(50, 200);
    int minutes = millis() / 60000;
    int seconds = (millis() / 1000) % 60;
    display.println("Thoi gian: " + String(minutes) + ":" + (seconds < 10 ? "0" : "") + String(seconds));
    
  } while (display.nextPage());
  
  Serial.println("Đã cập nhật màn hình thành công");
  display.hibernate();
}

// Hàm giải mã và hiển thị JPEG
bool drawJpegFromBuffer(uint8_t* buffer, size_t size) {
  boolean decoded = JpegDec.decodeArray(buffer, size);
  
  if (!decoded) {
    Serial.println("Lỗi giải mã JPEG");
    return false;
  }
  
  // Thông tin ảnh đã giải mã
  uint32_t width = JpegDec.width;
  uint32_t height = JpegDec.height;
  uint32_t MCUWidth = JpegDec.MCUWidth;
  uint32_t MCUHeight = JpegDec.MCUHeight;
  
  Serial.print("Kích thước ảnh giải mã: ");
  Serial.print(width); Serial.print("x"); Serial.println(height);
  
  // Lấy toạ độ điểm bắt đầu để căn giữa ảnh
  uint16_t xOffset = (display.width() - width) / 2;
  uint16_t yOffset = (display.height() - height) / 2;
  
  // Vẽ từng MCU (Minimum Coded Unit)
  while (JpegDec.read()) {
    uint16_t x = JpegDec.MCUx * MCUWidth + xOffset;
    uint16_t y = JpegDec.MCUy * MCUHeight + yOffset;
    
    // Lấy dữ liệu pixel
    uint16_t* pImg = JpegDec.pImage;
    
    // Vẽ khối MCU 8x8 hoặc 16x16 pixel
    for (int16_t dy = 0; dy < MCUHeight; dy++) {
      for (int16_t dx = 0; dx < MCUWidth; dx++) {
        if ((x + dx) < display.width() && (y + dy) < display.height()) {
          uint16_t pixel = *pImg++;
          
          // Chuyển đổi màu RGB565 thành màu E-Ink (3 màu)
          uint8_t r = (pixel & 0xF800) >> 11;
          uint8_t g = (pixel & 0x07E0) >> 5;
          uint8_t b = (pixel & 0x001F);
          
          // Tính độ xám (grayscale) - cân bằng màu phù hợp với mắt người
          uint8_t gray = (r * 77 + g * 150 + b * 29) >> 8;
          
          // Quyết định hiển thị màu gì (đen, trắng hoặc đỏ)
          uint16_t eink_color;
          if (gray < 85) {
            eink_color = GxEPD_BLACK;
          } else if (gray < 170) {
            eink_color = GxEPD_RED;
          } else {
            eink_color = GxEPD_WHITE;
          }
          
          // Vẽ pixel
          display.drawPixel(x + dx, y + dy, eink_color);
        }
      }
    }
  }
  
  return true;
}

void displayMessage(String message) {
  Serial.println(message);
  
  display.setFullWindow();
  display.firstPage();
  
  do {
    display.fillScreen(GxEPD_WHITE);
    display.setTextColor(GxEPD_BLACK);
    display.setTextSize(1);
    
    // Vẽ khung
    display.drawRect(10, 10, display.width() - 20, display.height() - 20, GxEPD_BLACK);
    
    // Hiển thị tiêu đề
    display.setTextSize(2);
    display.setCursor(40, 50);
    display.println("Nhu Quynh Xinh Gai - E-Ink");
    
    // Vẽ dòng ngăn cách
    display.drawLine(30, 70, display.width() - 30, 70, GxEPD_BLACK);
    
    // Hiển thị thông báo
    display.setTextSize(1);
    display.setCursor(30, 100);
    display.println(message);
    
    // Hiển thị thông tin WiFi nếu đã kết nối
    if (WiFi.status() == WL_CONNECTED) {
      display.setCursor(30, 130);
      display.println("IP: " + WiFi.localIP().toString());
    }
    
  } while (display.nextPage());
  
  display.hibernate();
}
