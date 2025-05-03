# Workflow: Hiển thị ảnh từ internet lên màn hình E-Ink

## Cách hiển thị ảnh lên màn hình E-Ink

Có nhiều cách để hiển thị ảnh lên màn hình E-Ink 4.2 inch (400x300 pixel):

### 1. Sử dụng thư viện JPEGDecoder

```cpp
#include <JPEGDecoder.h>

// Trong hàm displayImage()
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
  uint32_t MCUsInRow = JpegDec.MCUsPerRow;
  uint32_t MCUsInCol = JpegDec.MCUsPerCol;

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

          // Tính độ xám (grayscale)
          uint8_t gray = (r * 77 + g * 150 + b * 29) >> 8; // RGB to grayscale

          // Quyết định hiển thị màu gì (đen, trắng hoặc đỏ)
          uint16_t eink_color;
          if (gray < 85) {
            eink_color = GxEPD_BLACK;  // Đen
          } else if (gray < 170) {
            eink_color = GxEPD_RED;    // Đỏ
          } else {
            eink_color = GxEPD_WHITE;  // Trắng
          }

          // Vẽ pixel
          display.drawPixel(x + dx, y + dy, eink_color);
        }
      }
    }
  }

  return true;
}
```

### 2. Chuyển ảnh thành PROGMEM array trước

Đây là phương pháp tối ưu nhất nếu ảnh không thay đổi thường xuyên:

```cpp
// Khai báo ảnh dạng C array
#include "image_data.h"  // File chứa biến image_data[]

void displayStaticImage() {
  for (int y = 0; y < 300; y++) {
    for (int x = 0; x < 400; x++) {
      // Lấy giá trị màu từ mảng
      uint8_t pixel = image_data[y * 400 + x];
      // Hiển thị pixel
      display.drawPixel(x, y, pixel);
    }
  }
}
```

### 3. Sử dụng thư viện drawBitmap có sẵn

Nếu bạn có dữ liệu ảnh dạng bitmap:

```cpp
// Chuyển đổi dữ liệu từ buffer thành bitmap E-Ink
void convertToEinkBitmap(uint8_t* buffer, size_t size, uint8_t* bitmap, int width, int height) {
  // Code chuyển đổi từ định dạng ảnh (JPEG/PNG) sang bitmap
  // ...
}

void displayBitmap(uint8_t* bitmap, int width, int height) {
  display.drawBitmap(0, 0, bitmap, width, height, GxEPD_BLACK);
}
```

## Tối ưu hiển thị ảnh cho E-Ink

1. **Dithering**: Cải thiện chất lượng ảnh khi hiển thị ảnh màu trên E-Ink 3 màu:

```cpp
uint16_t dither(int x, int y, uint8_t gray) {
  // Floyd-Steinberg dithering
  int threshold = (((x & 1) ^ (y & 1)) * 16);
  if (gray < threshold)
    return GxEPD_BLACK;
  else if (gray < 128)
    return GxEPD_RED;
  else
    return GxEPD_WHITE;
}
```

2. **Xử lý ảnh trước**: Nén ảnh thành định dạng phù hợp với E-Ink trước khi tải:

```cpp
// Tạo server trung gian để xử lý ảnh
// Thay vì tải trực tiếp: https://dummyimage.com/400x300/...
// Sử dụng: https://your-server.com/convert?url=https://dummyimage.com/400x300/...
```

## Code hoàn chỉnh cho hiển thị ảnh JPEG

```cpp
#include <JPEGDecoder.h>

// Thêm vào phần khai báo
#include <JPEGDecoder.h>

// Sửa lại hàm displayImage()
void displayImage() {
  Serial.println("Xử lý và hiển thị ảnh lên màn hình E-Ink");

  display.setFullWindow();
  display.firstPage();

  do {
    display.fillScreen(GxEPD_WHITE);

    // Giải mã và hiển thị JPEG nếu tải thành công
    if (imageBuffer != NULL && imageSize > 0) {
      if (drawJpegFromBuffer(imageBuffer, imageSize)) {
        Serial.println("Hiển thị ảnh thành công");
      } else {
        // Nếu không thể hiển thị, hiển thị thông báo lỗi
        display.setTextColor(GxEPD_BLACK);
        display.setTextSize(1);
        display.setCursor(50, 100);
        display.println("Loi hien thi anh");
        display.setCursor(50, 120);
        display.println("Kich thuoc: " + String(imageSize) + " bytes");
      }
    } else {
      // Nếu không có dữ liệu ảnh
      display.setTextColor(GxEPD_BLACK);
      display.setTextSize(1);
      display.setCursor(50, 100);
      display.println("Khong co du lieu anh");
    }

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
```

## Cài đặt thư viện cần thiết

1. Mở Arduino IDE
2. Vào Tools > Manage Libraries...
3. Tìm kiếm và cài đặt "JPEGDecoder" (thư viện của Bodmer)
4. Khởi động lại Arduino IDE

## Lưu ý quan trọng

1. Màn hình E-ink chỉ có thể hiển thị 3 màu (đen, trắng, đỏ), nên việc hiển thị ảnh màu sẽ cần thuật toán chuyển đổi
2. Thời gian cập nhật màn hình E-ink khá chậm
3. ESP32 có giới hạn bộ nhớ, nên có thể không đủ để xử lý ảnh lớn
4. Nên sử dụng các định dạng ảnh nhẹ, tối ưu cho E-ink

## Giải pháp thay thế

Nếu ESP32 không đủ mạnh để xử lý các định dạng ảnh phức tạp:

1. Sử dụng server để chuyển đổi ảnh thành bitmap đơn giản trước khi gửi đến ESP32
2. Sử dụng các định dạng ảnh đơn giản (BMP đơn sắc hoặc bitmap riêng)
3. Chuyển đổi ảnh thành mã C/hex và nhúng vào code (phù hợp nếu ảnh không thay đổi)
