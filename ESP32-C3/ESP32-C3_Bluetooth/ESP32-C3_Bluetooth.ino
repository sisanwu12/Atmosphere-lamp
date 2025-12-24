/*
蓝牙输入
右转:turnright
左转：turnleft
危险警报闪关灯：danger
关闭所有灯光：offled
刹车：brake
*/
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLECharacteristic.h>
#include <Adafruit_NeoPixel.h>  
// 自定义BLE服务和特征UUID
#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"
// WS2812配置参数
#define LED_PIN     5       
#define LED_COUNT   61      
#define LED_LEFT_COUNT   30  
#define LED_RIGHT_COUNT  30  

Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800); 

// 全局变量：右侧灯闪烁控制
bool isRightBlink = false;       
unsigned long previousRightBlinkMillis = 0; 
const long blinkInterval = 500;  
bool rightLedState = false;     

// 全局变量：左侧灯闪烁控制
bool isLeftBlink = false;        
unsigned long previousLeftBlinkMillis = 0;  
bool leftLedState = false; 

// 全局变量：危险报警闪光灯
bool isDangerBlink = false; 
unsigned long previousDangerBlinkMillis = 0;
bool DangerLedState = false;

// BLE服务器回调：处理连接/断开事件
class MyServerCallbacks: public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    Serial.println("手机已成功连接ESP32 BLE");
  }
  void onDisconnect(BLEServer* pServer) {
    Serial.println("手机已断开BLE连接，重新开启广播...");
    BLEDevice::startAdvertising();
  }
};  

// BLE特征回调：处理手机写入数据事件
class MyCharacteristicCallbacks: public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
    // 获取手机写入的数据
    String receivedData = pCharacteristic->getValue();
    if (receivedData.length() > 0) {
      Serial.println("\n********* 收到手机数据 *********");
      Serial.print("数据内容：");
      Serial.println(receivedData);
      Serial.println("********************************\n");
      String lowerData = receivedData;
      lowerData.toLowerCase();
      
      // 链式条件判断
      if (lowerData == "brake") {
        Serial.println("检测到brake（刹车），控制所有灯珠亮红色并回复brake_ok");
        isRightBlink = false; 
        isLeftBlink = false;  
        isDangerBlink = false; 
        for(int i=0; i<LED_COUNT; i++){
          strip.setPixelColor(i, strip.Color(255, 0, 0)); // 红色
        }
        strip.show(); 
        pCharacteristic->setValue("brake_ok");
        pCharacteristic->notify();
      }
      else if (lowerData == "turnright") {
        Serial.println("检测到turnright（右转），开启右侧灯持续闪烁并回复turnright_ok");
        isRightBlink = true;  
        isLeftBlink = false;  
        isDangerBlink = false; 
        pCharacteristic->setValue("turnright_ok");
        pCharacteristic->notify();
      }
      else if (lowerData == "turnleft") {
        Serial.println("检测到turnleft（左转），开启左侧灯持续闪烁并回复turnleft_ok");
        isLeftBlink = true;   
        isRightBlink = false; 
        isDangerBlink = false; 
        pCharacteristic->setValue("turnleft_ok");
        pCharacteristic->notify();
      }
      else if (lowerData == "offled") {
        Serial.println("检测到offled，停止所有闪烁并关闭所有WS2812灯珠，回复ledoff_ok");
        isRightBlink = false;
        isLeftBlink = false;  
        isDangerBlink = false; 
        strip.clear();
        strip.show();  
        pCharacteristic->setValue("ledoff_ok");
        pCharacteristic->notify();
      }
      else if (lowerData == "danger") { 
        Serial.println("检测到danger（危险报警），开启所有灯珠闪烁并回复danger_ok");
        isDangerBlink = true;
        isRightBlink = false;  
        isLeftBlink = false;
        pCharacteristic->setValue("danger_ok");
        pCharacteristic->notify();
      }
    }
  }
};

// 右侧灯非阻塞闪烁函数
void rightLedBlink() {
  unsigned long currentMillis = millis();
  // 仅当右侧闪烁使能时，执行时序判断
  if (isRightBlink && (currentMillis - previousRightBlinkMillis >= blinkInterval)) {
    previousRightBlinkMillis = currentMillis; 
    rightLedState = !rightLedState;           
    for(int i=31; i<LED_COUNT; i++){
      if (rightLedState) {
        strip.setPixelColor(i, strip.Color(255, 0, 0)); 
      } else {
        strip.setPixelColor(i, strip.Color(0, 0, 0));
      }
    }
    strip.show(); 
  }
}

// 左侧灯非阻塞闪烁函数
void leftLedBlink() {
  unsigned long currentMillis = millis();
  // 仅当左侧闪烁使能时，执行时序判断
  if (isLeftBlink && (currentMillis - previousLeftBlinkMillis >= blinkInterval)) {
    previousLeftBlinkMillis = currentMillis; 
    leftLedState = !leftLedState; 
    for(int i=0; i<LED_LEFT_COUNT; i++){
      if (leftLedState) {
        strip.setPixelColor(i, strip.Color(255, 0, 0)); 
      } else {
        strip.setPixelColor(i, strip.Color(0, 0, 0));  
      }
    }
    strip.show(); 
  }
}

void DangerLedBlink() {
  if (!isDangerBlink) {
    return;
  }
  unsigned long currentMillis = millis();
  if (currentMillis - previousDangerBlinkMillis >= blinkInterval) {
    previousDangerBlinkMillis = currentMillis;
    DangerLedState = !DangerLedState;
    uint32_t ledColor = DangerLedState ? strip.Color(255, 0, 0) : strip.Color(0, 0, 0);
    for (int i = 0; i < LED_COUNT; i++) {
      strip.setPixelColor(i, ledColor);
    }
    strip.show();
  }
}

void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.println("开始初始化ESP32 BLE + WS2812");
  
  strip.begin();
  strip.clear(); 
  strip.show();  

  BLEDevice::init("My_ESP32");

  // 2. 创建BLE服务器并设置回调
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // 3. 创建BLE服务
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // 4. 创建BLE特征，设置完整权限
  BLECharacteristic *pCharacteristic = pService->createCharacteristic(
    CHARACTERISTIC_UUID,
    BLECharacteristic::PROPERTY_READ |    
    BLECharacteristic::PROPERTY_WRITE |  
    BLECharacteristic::PROPERTY_NOTIFY    
  );

  // 5. 绑定特征回调
  pCharacteristic->setCallbacks(new MyCharacteristicCallbacks());
  // 设置特征初始值
  pCharacteristic->setValue("Hello from ESP32 BLE (with danger blink)");
  // 6. 启动BLE服务
  pService->start();
  Serial.println("BLE服务启动成功");
  // 7. 配置并启动BLE广播
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();
  Serial.println("BLE广播已启动，手机可搜索「My_ESP32」");
}

void loop() {
  rightLedBlink();
  leftLedBlink();
  DangerLedBlink();
  // 推送BLE心跳包
  delay(1000); // 心跳包间隔1秒
  BLEServer *pServer = BLEDevice::getServer();
  BLEService *pService = pServer->getServiceByUUID(SERVICE_UUID);
  BLECharacteristic *pCharacteristic = pService->getCharacteristic(CHARACTERISTIC_UUID);
  // 避免心跳包覆盖BLE指令回复信息
  String currentValue = pCharacteristic->getValue();
  if (currentValue != "brake_ok" && currentValue != "turnright_ok" && currentValue != "turnleft_ok" && currentValue != "ledoff_ok" && currentValue != "danger_ok") {
    pCharacteristic->setValue("ESP32心跳包：" + String(millis()/1000) + "s");
    pCharacteristic->notify();
  }
}