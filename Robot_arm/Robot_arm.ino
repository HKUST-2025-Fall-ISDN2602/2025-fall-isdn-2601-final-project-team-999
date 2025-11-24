#include "Servo.h"

// 舵机对象
Servo servo0;
Servo servo1;
Servo servo2;
Servo servo3;
Servo servo4;

// 配置参数（合理调整）
const int threhold = 8000;    // 阈值：摇杆8000（避免抖动），扳机20000（需用力按才触发）
const int mutiple = 3;   // 角度倍数（count范围0~60 → 角度0~60，配合中心90°）
const int debounceTime = 50;  // 按键消抖时间（ms）

// 舵机引脚（修改D0为D2，避免串口冲突）
const int pin0 = 16;  // servo0
const int pin1 = 5;   // servo1
const int pin2 = 4;   // servo2
const int pin3 = 0;   // servo3（原D0→改为D2，避免串口冲突）
const int pin4 = 14;  // servo4

// 全局变量（初始化，避免随机值）
int leftX = 0;
int leftY = 0;
int rightX = 0;
int rightY = 0;
uint16_t lt = 0;
uint16_t rt = 0;
bool aBtn = false;
bool bBtn = false;
int count[5] = {0, 0, 0, 0, 0};  // 初始值：对应舵机中心角度（90°）
unsigned long lastBtnTime = 0;       // 按键消抖计时

void setup() {
  Serial.begin(115200);  // 与Python波特率一致
  // 绑定舵机引脚
  servo0.attach(pin0);
  servo1.attach(pin1);
  servo2.attach(pin2);
  servo3.attach(pin3);
  servo4.attach(pin4);
  
  // 舵机初始位置（以90°为中心，count[x]=30 → 30*1 + 60 = 90°）
  servo1.write( count[0] * mutiple);
  servo2.write( count[1] * mutiple);
  servo3.write(count[2] * mutiple);
  servo0.write( count[3] * mutiple);
  servo4.write(0 + count[4] * mutiple); 
  
  delay(200);  // 等待舵机到位
  Serial.println("舵机初始化完成，等待数据...");
}

void loop() {
  // 1. 读取并解析串口数据（Python发来的字符串）
  leftX=0;
  leftY=0;
  rightY=0;
  rightX=0;
  lt=0;
  rt=0;
  aBtn=false;                                                                                                                                              
  bBtn=false;
  if (Serial.available() > 0) {
    String data = Serial.readStringUntil('\n');
    data.trim();
    
    // 解析10个参数：左X,左Y,右X,右Y,LT,RT,A,B,X,Y
    int params[10] = {0};  // 初始化参数数组，避免随机值
    int paramCount = 0;
    String temp = "";
    
    for (int i = 0; i < data.length(); i++) {
      if (data[i] == ',') {
        if (paramCount < 10) {
          params[paramCount++] = temp.toInt();
          temp = "";
        }
      } else {
        temp += data[i];
      }
    }
    // 读取最后一个参数
    if (paramCount < 10) {
      params[paramCount++] = temp.toInt();
    }
    Serial.print("data");
    Serial.println(data);
    // 更新手柄数据（仅在接收新数据时更新，避免旧状态持续）
    leftX = params[0];
    leftY = params[1];
    rightX = params[2];
    rightY = params[3];
    lt = (uint16_t)params[4];  // 强制转换为无符号数（0~65535）
    rt = (uint16_t)params[5];
    aBtn = (params[6] == 1);   // 确保是布尔值
    bBtn = (params[7] == 1);
  }

  // 2. 左摇杆X → servo1（左右动作，中心90°）
  if (abs(leftX) > threhold) {
    if (leftX < 0 && count[0] > 0) {        // 左推摇杆：count[0]递减 → 角度减小（<90°）
      count[0]--;
    } else if (leftX > 0 && count[0] < 180/mutiple) { // 右推摇杆：count[0]递增 → 角度增大（>90°）
      count[0]++;
    }
  }

  // 3. 左摇杆Y → servo2（上下动作，中心90°）
  if (abs(leftY) > threhold) {
    if (leftY < 0 && count[1] > 0) {        // 上推摇杆：count[1]递减 → 角度减小（<90°）
      count[1]--;
    } else if (leftY > 0 && count[1] < 180/mutiple) { // 下推摇杆：count[1]递增 → 角度增大（>90°）
      count[1]++;
    }
  }

    if (abs(rightX) > threhold) {
    if (rightX < 0 && count[2] > 0) {        // 左推摇杆：count[0]递减 → 角度减小（<90°）
      count[2]--;
    } else if (rightX > 0 && count[2] < 180/mutiple) { // 右推摇杆：count[0]递增 → 角度增大（>90°）
      count[2]++;
    }
  }
//  
  if (lt > 20000) {  // 扳机阈值设为20000（需用力按才触发）
    if (count[3] > 0) {
      count[3]--;
    }
  } 
  
  if (rt > 20000) {
    if (count[3] < 180/mutiple) {
      count[3]++;
    }
  } 
// 
  // 6. A/B键 → servo4（消抖处理，按下一次仅动作一次
    if (aBtn && count[4] < 180/mutiple) {  // A键：count[4]递增 → 角度增大（0~60°）
      count[4]++;
    }
    if (bBtn && count[4] > 0) {   // B键：count[4]递减 → 角度减小（0~60°）
      count[4]--;
    }
    // 松开按键后，重置aBtn/bBtn（避免持续触发）
  // 7. 控制舵机（角度限制在0~180°，避免舵机损坏）
  int servo1Angle = constrain( count[0] * mutiple, 0, 180);
  int servo2Angle = constrain(  count[1] * mutiple, 0, 180);
  int servo3Angle = constrain(  count[2] * mutiple, 0, 180);
  int servo0Angle = constrain(  count[3] * mutiple, 0, 180);
  int servo4Angle = constrain( count[4] * mutiple, 0, 180);

  servo1.write(servo1Angle);
  servo2.write(servo2Angle);
  servo3.write(servo3Angle);
  servo0.write(servo0Angle);
  servo4.write(servo4Angle);

  // （可选）串口回显数据，用于调试（配合虚拟串口工具查看）
  // Serial.print("count: [");
  // for (int i = 0; i < 5; i++) {
  //   Serial.print(count[i]);
  //   if (i < 4) Serial.print(",");
  // }
  // Serial.print("] → 角度: ");
  // Serial.print(servo1Angle); Serial.print(",");
  // Serial.print(servo2Angle); Serial.print(",");
  // Serial.print(servo3Angle); Serial.print(",");
  // Serial.print(servo0Angle); Serial.print(",");
  // Serial.println(servo4Angle);

  delay(3);
}