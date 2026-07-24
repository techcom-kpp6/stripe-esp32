#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <XPT2046_Touchscreen.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
// #include "qrcode.h"

// --- กำหนดขาต่อใช้งาน TFT และ Touch ---
#define TFT_DC     2
#define TFT_CS    15
#define TFT_RST    4
#define TOUCH_CS  21   
#define TOUCH_IRQ 36   

#define RELAY_PIN 5
const unsigned long relayDuration = 5000; 

const char* ssid = "ROD AIS_2.4G";
const char* password = "0660644465";
// const char* server = "https://stripe-esp32.onrender.com/check";

// ลิงก์ Stripe Buy Button สำหรับแต่ละราคา
// const char* stripeUrl20  = "https://buy.stripe.com/test_4gMfZh6Dx2si1w0dQQ5kk02";
// const char* stripeUrl50  = "https://buy.stripe.com/test_dRm3cv0f9ff4eiMdQQ5kk04";
// const char* stripeUrl100 = "https://buy.stripe.com/test_dRm5kD7HB3wm5MgeUU5kk05";

// ตัวแปรเก็บลิงก์ปัจจุบันและยอดเงินที่เลือก
// String currentStripeUrl = stripeUrl20;
// String selectedAmount = "0.00 THB";
string logger = 0; 

enum ScreenState { STATE_STANDBY, STATE_SHOW_QR, STATE_PAID };
ScreenState currentState = STATE_STANDBY;

unsigned long relayOnTime = 0;
bool relayState = false;

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);
XPT2046_Touchscreen ts(TOUCH_CS);

// ==========================================
// ฟังก์ชันวาด QR Code (ปรับสเกลขยายใหญ่ขึ้น)
// ==========================================
// extern "C" void draw_qr(esp_qrcode_handle_t qr) {
//   int size = esp_qrcode_get_size(qr);
  
  // คำนวณ Scale ให้อัตโนมัติ เพื่อให้ QR ใหญ่เต็มพื้นที่ (สูงสุดไม่เกินขนาดจอ)
  // int scale = 200 / size; 
  // if (scale < 1) scale = 1;

  // int qrSizeOnScreen = size * scale;
  // int xOffset = (240 - qrSizeOnScreen) / 2;
  // int yOffset = (320 - qrSizeOnScreen) / 2 - 25;

  // วาด Quiet Zone สีขาว
  // tft.fillRect(xOffset - 8, yOffset - 8, qrSizeOnScreen + 16, qrSizeOnScreen + 16, ILI9341_WHITE);

  // วาดพิกเซล QR Code
  // for (int y = 0; y < size; y++) {
  //   for (int x = 0; x < size; x++) {
  //     if (esp_qrcode_get_module(qr, x, y)) {
  //       tft.fillRect(xOffset + (x * scale), yOffset + (y * scale), scale, scale, ILI9341_BLACK);
  //     }
  //   }
  // }

  // แสดงยอดเงินที่เราเลือกไว้
//   tft.fillRect(0, yOffset + qrSizeOnScreen + 12, 240, 35, ILI9341_WHITE);
//   tft.setTextColor(ILI9341_BLACK);
//   tft.setTextSize(2);
//   tft.setCursor(20, yOffset + qrSizeOnScreen + 15);
//   tft.print("Price: ");
//   tft.setTextColor(ILI9341_BLUE);
//   tft.print(selectedAmount);

//   tft.setTextColor(ILI9341_RED);
//   tft.setTextSize(1);
//   tft.setCursor(45, yOffset + qrSizeOnScreen + 48);
//   tft.println("Scan QR Code to Pay...");
// }

// void drawQRCodeScreen() {
//   tft.fillScreen(ILI9341_WHITE);

//   esp_qrcode_config_t cfg = {};
//   cfg.max_qrcode_version = 10;
//   cfg.qrcode_ecc_level = 0;
//   cfg.display_func = draw_qr;

//   // ใช้ตัวแปร currentStripeUrl ส่งเข้าไปสร้าง QR
//   esp_qrcode_generate(&cfg, currentStripeUrl.c_str());
// }

// ==========================================
// วาดหน้า Standby แบบมี 3 ปุ่มให้เลือกราคา
// ==========================================
void drawStandbyScreen() {
  tft.fillScreen(ILI9341_BLACK);

  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(2);
  tft.setCursor(20, 30);
  tft.println("SELECT MENU");

  // ปุ่มที่ 1: 20 บาท (Y: 70 -> 120)
  tft.fillRoundRect(20, 70, 200, 50, 10, ILI9341_WHITE);
  tft.drawRoundRect(20, 70, 200, 50, 10, ILI9341_BLACK);
  tft.setCursor(75, 85);
  tft.println("Load Items");

  // ปุ่มที่ 2: 50 บาท (Y: 140 -> 190)
  tft.fillRoundRect(20, 140, 200, 50, 10, ILI9341_WHITE);
  tft.drawRoundRect(20, 140, 200, 50, 10, ILI9341_BLACK);
  tft.setCursor(75, 155);
  tft.println("Remove Items");

  // ปุ่มที่ 3: 100 บาท (Y: 210 -> 260)
  // tft.fillRoundRect(20, 210, 200, 50, 10, ILI9341_BLUE);
  // tft.drawRoundRect(20, 210, 200, 50, 10, ILI9341_WHITE);
  // tft.setCursor(70, 225);
  // tft.println("100 THB");
}
void drawPasswordScreen() {
  tft.fillScreen(ILI9341_DARK);

  tft.fillRoundRect(120, 110, 40, ILI9341_WHITE);
  tft.setTextColor(ILI9341_BLACK);
  tft.setTextSize(4);
  tft.setCursor(108, 97);
  tft.println("V"); 

  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(2);
  tft.setCursor(45, 180);
  tft.println("Payment Success!");

  tft.setTextSize(1);
  tft.setCursor(65, 220);
  tft.println("Relay is working...");
}
void drawSuccessScreen() {
  tft.fillScreen(ILI9341_DARK);

  tft.fillCircle(120, 110, 40, ILI9341_WHITE);
  tft.setTextColor(ILI9341_BLACK);
  tft.setTextSize(4);
  tft.setCursor(108, 97);
  tft.println("V"); 

  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(2);
  tft.setCursor(45, 180);
  tft.println("Payment Success!");

  tft.setTextSize(1);
  tft.setCursor(65, 220);
  tft.println("Relay is working...");
}

// ==========================================
// Setup & Loop
// ==========================================
void setup() {
  Serial.begin(115200);

  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, HIGH);

  SPI.begin(18, 19, 23);
  ts.begin();
  ts.setRotation(0);
  
  tft.begin();
  tft.setRotation(0);
  
  drawStandbyScreen();

  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.print("\nWiFi Connected!");
}

void loop() {
  // 1. ตรวจสอบการสัมผัสปุ่มราคา 3 ปุ่ม (ในหน้า Standby)
  if (currentState == STATE_STANDBY && ts.touched()) {
    TS_Point p = ts.getPoint();
    
    // คำนวณสเกลพิกัด TouchScreen เป็น พิกเซลจอ (240x320)
    int touchX = map(p.x, 200, 3700, 0, 240);
    int touchY = map(p.y, 3800, 240, 320, 0);
    
    Serial.print("X: "); Serial.print(touchX);
    Serial.print(" | Y: "); Serial.println(touchY);
        // ปุ่มที่ 1: เลือกฝาก
    if (touchY >= 70 && touchY <= 120) {

      // selectedAmount = "20.00 THB";
      // currentStripeUrl = stripeUrl20;
      // drawQRCodeScreen();
      // currentState = STATE_SHOW_QR;
    }  
    // ปุ่มที่ 2: 50 บาท
    // else if (touchY >= 140 && touchY <= 190) {
    //   selectedAmount = "50.00 THB";
    //   currentStripeUrl = stripeUrl50;
    //   drawQRCodeScreen();
    //   currentState = STATE_SHOW_QR;
    } 
    // ปุ่มที่ 3: 100 บาท
    // else if (touchY >= 210 && touchY <= 260) {
    //   selectedAmount = "100.00 THB";
    //   currentStripeUrl = stripeUrl100;
    //   drawQRCodeScreen();
    //   currentState = STATE_SHOW_QR;
    // }
    // delay(400); 
  }

  // 2. เช็กสถานะชำระเงินจาก Server
  // if (currentState == STATE_SHOW_QR && WiFi.status() == WL_CONNECTED) {
  //   WiFiClientSecure client;
  //   client.setInsecure(); 

  //   HTTPClient http;
  //   http.begin(client, server);
  //   int code = http.GET();

  //   if (code == 200) {
  //     String payload = http.getString();
  //     DynamicJsonDocument doc(512);
  //     DeserializationError error = deserializeJson(doc, payload);

  //     if (!error) {
  //       String status = doc["status"].as<String>();

  //       if (status == "ON") {
  //         digitalWrite(RELAY_PIN, LOW); // เปิด Relay
  //         relayState = true;
  //         relayOnTime = millis();

  //         drawSuccessScreen();
  //         currentState = STATE_PAID;
  //         Serial.println("🔌 Payment Verified! Relay ON");
  //       }
  //     }
  //   }
  //   http.end();
  // }

  // 3. ปิด Relay อัตโนมัติแล้วกลับสู่หน้าเลือกราคา
  if (currentState == STATE_PAID && relayState) {
    if (millis() - relayOnTime >= relayDuration) {
      digitalWrite(RELAY_PIN, HIGH); 
      relayState = false;

      drawStandbyScreen();
      currentState = STATE_STANDBY;
    }
  }
  println("relayOnTime: ", relayOnTime;)
  delay(1000); 
}