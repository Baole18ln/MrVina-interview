#define BLYNK_TEMPLATE_ID "TMPL6b9XSxUof"
#define BLYNK_TEMPLATE_NAME "mrvina"
#define BLYNK_AUTH_TOKEN "gNt73WTETyn1wFtpnrNmer8k752SR23u"

#include <WiFi.h>  
#include <BlynkSimpleEsp32.h>  
#include <NTPClient.h>
#include <WiFiUdp.h>

char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "Wokwi-GUEST";
char pass[] = "";

const int button = 4;
const int led = 23;
const int relay = 16;
int buttonStatus = 0;
int pressCount = 0;
unsigned long lastPressTime = 0;
unsigned long pressStartTime = 0;
unsigned long previousMillis = 0;
unsigned long intervalOn = 0;
unsigned long intervalOff = 0;
bool ledState = false;
bool isHolding = false;
bool isWiFiConnected = false;
bool isBlynkConnected = false;

int V1_state = 0;
int V2_state = 0;
int V3_state = 0;
int V5_hour, V5_minute, V7_hour, V7_minute, V9_minute, V9_second, V11_minute, V11_second;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 7 * 3600, 60000); // UTC+7 (VN)
String V5_text = "0315"; // Biến lưu trữ dữ liệu từ text input
String V6_text = "0600";
String V7_text = "0059";
String V8_text = "0059";
String V9_text = "0515";
String V10_text = "0515";
String V11_text = "0045";
String V12_text = "0045";

BLYNK_WRITE(V5) {
  V5_text = param.asStr(); // Đọc giá trị từ text input
  Serial.print("Dữ liệu từ Blynk V5: ");
  Serial.println(V5_text);
}

BLYNK_WRITE(V6) {
  V6_text = param.asStr();
  Serial.print("Dữ liệu từ Blynk V6: ");
  Serial.println(V6_text);
}

BLYNK_WRITE(V7) {
  V7_text = param.asStr();
  Serial.print("Dữ liệu từ Blynk V7: ");
  Serial.println(V7_text);
}

BLYNK_WRITE(V8) {
  V8_text = param.asStr();
  Serial.print("Dữ liệu từ Blynk V8: ");
  Serial.println(V8_text);
}

BLYNK_WRITE(V9) {
  V9_text = param.asStr();
  Serial.print("Dữ liệu từ Blynk V9: ");
  Serial.println(V9_text);
}

BLYNK_WRITE(V10) {
  V10_text = param.asStr();
  Serial.print("Dữ liệu từ Blynk V10: ");
  Serial.println(V10_text);
}

BLYNK_WRITE(V11) {
  V11_text = param.asStr();
  Serial.print("Dữ liệu từ Blynk V11: ");
  Serial.println(V11_text);
}

BLYNK_WRITE(V12) {
  V12_text = param.asStr();
  Serial.print("Dữ liệu từ Blynk V12: ");
  Serial.println(V12_text);
}

BLYNK_WRITE(V1) {
  V1_state = param.asInt();
}
BLYNK_WRITE(V2) {
  V2_state = param.asInt();
}
BLYNK_WRITE(V3) {
  V3_state = param.asInt();
}

void setup() {
  pinMode(led, OUTPUT);
  pinMode(button, INPUT);
  pinMode(relay, OUTPUT);
  Serial.begin(9600);

  Blynk.begin(auth, ssid, pass);
  connectToWiFi();
  if (isWiFiConnected) {
    connectToBlynk();
    timeClient.begin(); // Khởi động NTPClient
  }
}

void loop() {
  buttonStatus = digitalRead(button);
  extractTimeParts();
  
  if (buttonStatus == HIGH) {
    if (pressStartTime == 0) {
      pressStartTime = millis();
    }

    while (digitalRead(button) == HIGH); // Đợi nhả nút

    unsigned long pressDuration = millis() - pressStartTime;

    if (pressDuration >= 5000) {
      if (!isHolding) {
        Serial.println("Giữ nút 5s: LED sáng liên tục và bắt đầu kết nối WiFi + Blynk");
        digitalWrite(led, HIGH);
        isHolding = true;
        connectToWiFi();
        if (isWiFiConnected) {
          connectToBlynk();
        }
      }
    } else {
      if (isHolding) {
        Serial.println("Nhấn thêm lần nữa: LED tắt và trở lại chế độ nháy");
        digitalWrite(led, LOW);
        isHolding = false;
      } else {
        pressCount++;
        pressCount = min(pressCount, 3);
        Serial.print("Số lần nhấn: ");
        Serial.println(pressCount);
        lastPressTime = millis();
      }
    }

    pressStartTime = 0;
    delay(100);
  }

  if (!isHolding && pressCount > 0 && millis() - lastPressTime > 500) {
    Serial.print("LED nhấp nháy: ");
    Serial.print(pressCount);
    Serial.println(" lần");

    for (int i = 0; i < pressCount; i++) {
      digitalWrite(led, HIGH);
      delay(500);
      digitalWrite(led, LOW);
      delay(500);
    }

    if (pressCount == 1) {
      V1_state = !V1_state;
      Blynk.virtualWrite(V3, V1_state);
    } else if (pressCount == 2) {
      V2_state = !V2_state;
      Blynk.virtualWrite(V4, V2_state);
    } else if (pressCount == 3) {
      V3_state = !V3_state;
      Blynk.virtualWrite(V2, V3_state);
    }
    pressCount = 0;
  }

  if (isBlynkConnected) {
    Blynk.run();
  }

  updateTime(); // Cập nhật thời gian thực
}

void connectToWiFi() {
  Serial.println("Đang kết nối WiFi...");
  WiFi.begin(ssid, pass);
  
  int timeout = 10;
  while (WiFi.status() != WL_CONNECTED && timeout > 0) {
    delay(1000);
    Serial.print(".");
    timeout--;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi đã kết nối!");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
    isWiFiConnected = true;
  } else {
    Serial.println("\nKết nối WiFi thất bại!");
    isWiFiConnected = false;
  }
}

void connectToBlynk() {
  Serial.println("Đang kết nối Blynk...");
  Blynk.begin(auth, ssid, pass);
  if (Blynk.connected()) {
    Serial.println("Blynk đã kết nối!");
    isBlynkConnected = true;
  } else {
    Serial.println("Kết nối Blynk thất bại!");
    isBlynkConnected = false;
  }
}

void updateTime() {
  timeClient.update();
  String formattedTime = timeClient.getFormattedTime();
  int currentHour = timeClient.getHours();
  int currentMinute = timeClient.getMinutes();
  int currentSecond = timeClient.getSeconds();

  // Cập nhật lại các giá trị giờ và phút từ các biến
  extractTimeParts();

  // Chuyển thời gian hiện tại và thời gian cài đặt sang phút từ 00:00
  int currentTotalMinutes = currentHour * 60 + currentMinute;
  int V5_totalMinutes = V5_hour * 60 + V5_minute;
  int V7_totalMinutes = V7_hour * 60 + V7_minute;

  // Kiểm tra nếu đang trong khoảng thời gian bật LED
  if (currentTotalMinutes >= V5_totalMinutes && currentTotalMinutes < V7_totalMinutes) {
    if (!ledState && (millis() - previousMillis >= intervalOff)) {
      previousMillis = millis();
      ledState = true;
      digitalWrite(led, HIGH);
    }
    
    if (ledState && (millis() - previousMillis >= intervalOn)) {
      previousMillis = millis();
      ledState = false;
      digitalWrite(led, LOW);
    }

  } else {
    digitalWrite(led, LOW);
    ledState = false;
  }
}


void extractTimeParts() {
  int V5_hour = V5_text.substring(0, 2).toInt();
  int V5_minute = V5_text.substring(2, 4).toInt();
  
  int V6_hour = V6_text.substring(0, 2).toInt();
  int V6_minute = V6_text.substring(2, 4).toInt();
  
  int V7_hour = V7_text.substring(0, 2).toInt();
  int V7_minute = V7_text.substring(2, 4).toInt();
  
  int V8_hour = V8_text.substring(0, 2).toInt();
  int V8_minute = V8_text.substring(2, 4).toInt();
  
  int V9_minute = V9_text.substring(0, 2).toInt();
  int V9_second = V9_text.substring(2, 4).toInt();
  
  int V10_minute = V10_text.substring(0, 2).toInt();
  int V10_second = V10_text.substring(2, 4).toInt();
  
  int V11_minute = V11_text.substring(0, 2).toInt();
  int V11_second = V11_text.substring(2, 4).toInt();
  
  int V12_minute = V12_text.substring(0, 2).toInt();
  int V12_second = V12_text.substring(2, 4).toInt();
  intervalOn = (V11_minute * 60 + V11_second) * 1000;
  intervalOff = (V9_minute * 60 + V9_second) * 1000;
  // Serial.println("===== Tách giá trị thành công =====");
  // Serial.printf("V5: %02d:%02d\n", V5_hour, V5_minute);
  // Serial.printf("V6: %02d:%02d\n", V6_hour, V6_minute);
  // Serial.printf("V7: %02d:%02d\n", V7_hour, V7_minute);
  // Serial.printf("V8: %02d:%02d\n", V8_hour, V8_minute);
  // Serial.printf("V9: %02d:%02d\n", V9_minute, V9_second);
  // Serial.printf("V10: %02d:%02d\n", V10_minute, V10_second);
  // Serial.printf("V11: %02d:%02d\n", V11_minute, V11_second);
  // Serial.printf("V12: %02d:%02d\n", V12_minute, V12_second);
  // delay(1000);
}
