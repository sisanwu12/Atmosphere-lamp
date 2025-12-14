#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLECharacteristic.h>

// 自定义BLE服务和特征UUID
#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

// define led according to pin diagram
int led = 8;


// BLE服务器回调：处理连接/断开事件
class MyServerCallbacks: public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    Serial.println("手机已成功连接ESP32 BLE");
  }

  void onDisconnect(BLEServer* pServer) {
    Serial.println("手机已断开BLE连接，重新开启广播...");
    // 断开后重新启动广播
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

      // 转换为小写，避免大小写敏感问题
      String lowerData = receivedData;
      lowerData.toLowerCase();
      
      // 判断是否收到hello，若是则回复nihao
      if (lowerData == "hello") {
        Serial.println("检测到hello，回复nihao给手机");
        // 设置回复内容并通过notify推送给手机
        pCharacteristic->setValue("nihao");
        pCharacteristic->notify();

        for(int i=0;i<=3;i++)
        {
        digitalWrite(led, HIGH);   // turn the LED off
        delay(1000);               // wait for a second
        digitalWrite(led, LOW);    // turn the LED on
        delay(1000);               // wait for a second
        }
      }
    }
  }
};

void setup() {
  // 初始化串口
  Serial.begin(115200);
  delay(100);
  Serial.println("开始初始化ESP32 BLE...");
  pinMode(led, OUTPUT);
  // 1. 初始化BLE设备，设置设备名称
  BLEDevice::init("My_ESP32");

  // 2. 创建BLE服务器并设置回调
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // 3. 创建BLE服务
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // 4. 创建BLE特征（设置完整权限）
  BLECharacteristic *pCharacteristic = pService->createCharacteristic(
    CHARACTERISTIC_UUID,
    BLECharacteristic::PROPERTY_READ |    // 可读
    BLECharacteristic::PROPERTY_WRITE |   // 可写
    BLECharacteristic::PROPERTY_NOTIFY    // 通知
  );

  // 5. 绑定特征回调
  pCharacteristic->setCallbacks(new MyCharacteristicCallbacks());
  // 设置特征初始值
  pCharacteristic->setValue("Hello from ESP32 BLE");

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
  delay(1000);

  // 可选：心跳包推送（不影响hello回复功能）
  BLEServer *pServer = BLEDevice::getServer();
  BLEService *pService = pServer->getServiceByUUID(SERVICE_UUID);
  BLECharacteristic *pCharacteristic = pService->getCharacteristic(CHARACTERISTIC_UUID);
  
  // 只有在未发送回复时才发送心跳包，避免覆盖回复内容
  String currentValue = pCharacteristic->getValue();
  if (currentValue != "nihao") {
    pCharacteristic->setValue("ESP32心跳包：" + String(millis()/1000) + "s");
    pCharacteristic->notify();
  }
}
