#include <string.h>
#include <SoftwareSerial.h>

// HC-06 藍牙模組
const byte BL_RX_Pin = 6, BL_TX_Pin = 7;

// L298N 的控制訊號腳位
const byte Left_Motor_IN1 = 13, Left_Motor_IN2 = 12;
const byte Right_Motor_IN3 = 11, Right_Motor_IN4 = 10;

const short rotation_180 = 1200, rotation_90 = 600, 
            rotation_60 = 400, rotation_30 = 200, 
            rotation_20 = 132, rotation_15 = 100;

const short coast_time = 200;

// SG90 超音波
//Input pin to receive echo pulse
const int Echo_Pin_L = 9, Echo_Pin_R = 3, Echo_Pin_C = 5;
//Output pin to send trigger signal
const int Trig_Pin_L = 8, Trig_Pin_R = 2, Trig_Pin_C = 4;

const int dist_arr_length = 8;
int dist_C[dist_arr_length], dist_L[dist_arr_length], dist_R[dist_arr_length];
int dist_C_AVE = 0, dist_L_AVE = 0, dist_R_AVE = 0;

const int get_dist_delay = 50;
// 車寬 13.5 cm，場地寬 30 cm，左右邊超音波可能最長距離 16.5 cm
const float dist_LR_max = 16.5;
// 即時檢查目前左右邊超音波可能最長距離 (EX：L = 10, 那 R 一定 <= 6.5 )
const float dist_L_current_max = 16.5, dist_R_current_max = 16.5;
const int dist_CLR_min = 5;
// 每格邊長 30 cm，假設場地最大 30 x 5 片，正面超音波可能最長距離 150 cm
const float dist_C_max = 150;
// fot 迴圈變數, 平均數, 執行次數
int i, count = 0, AVE = 0, x = 1;

unsigned long previous_time;
unsigned long current_time;

// 定義連接藍牙模組的序列埠
SoftwareSerial BT(BL_TX_Pin, BL_RX_Pin); // 傳送腳 / 接收腳 (TX / RX)

void setup() {
    Serial.begin(9600);   // 與電腦序列埠連線
    BT.begin(38400);   // 設定藍牙模組的連線速率
    // 設定 L298n 控制腳位為輸出
    setCarTire();
    // 超音波 (HC-SR04)
    setUltrasound(Trig_Pin_L);
    setUltrasound(Trig_Pin_R);
    setUltrasound(Trig_Pin_C);
}

void loop() {
    dist_C_AVE = 0, dist_L_AVE = 0, dist_R_AVE = 0;
    previous_time = millis();

    dist_L_AVE = get_dist('L', dist_L, dist_L_AVE);
    dist_R_AVE = get_dist('R', dist_R, dist_R_AVE);
    print(x + String(" - L: ") + dist_L_AVE + ", R: " + dist_R_AVE);
    

    //action();
    //print(dist_C_AVE + String(", ") + dist_L_AVE + ", " + dist_R_AVE);

    // dist_C_AVE = get_dist('C', dist_C, dist_C_AVE);
    // print(String("C: ") + dist_C_AVE);

    current_time = millis() - previous_time;
    print(String("t: ") + current_time);
    print("---------");
    x++;
    delay(500);
}

int get_dist(char c, int distArr[], int dist_AVE) {
    count = 0, AVE = 0;
    for (i = 0; i < dist_arr_length; i++) {
        distArr[i] = Ultrasound(c);
        // 濾波
        dist_AVE = filter(distArr, dist_AVE, i);
        delay(get_dist_delay);
    }
    // 如果全部都是爆掉就設為場地最長值 (此方法不好)
    return count == 0 ? dist_C_max : dist_AVE;
}

// 濾波
int filter(int distArr[], int dist_AVE, int i) {
    // 第一次平均 0~4
    if (i <= 4) {
        if (distArr[i] < dist_C_max) {
            count++;
            AVE += distArr[i];
        }
        if (i == 4)
            return AVE / count;
    } else {
        // 第二次 0~5, 第三次 0~6...
        if (!(distArr[i] > dist_AVE + 3) || !(distArr[i] < dist_AVE - 3)) {
            count++;
            AVE += distArr[i];
            return AVE / count;
        }
    }
    return dist_AVE;
}

void action() {
    if (dist_R_AVE > dist_CLR_min) {
        // 因為硬體本身在直走時就會自己稍微右偏前進，所以這裡註解右轉動作
        // turnRight(rotation_15);
        // coast(coast_time);
        forward(500);
        print("Right 15 + For");
    } else {
        // 如果右邊超音波離牆壁距離低於 5 cm 就向左轉校正
        turnLeft(rotation_15);
        coast(coast_time);
        print("Left 15");

        dist_C_AVE = get_dist('C', dist_C, dist_C_AVE);
        print(String("C: ") + dist_C_AVE);
        
        if (dist_C_AVE > dist_CLR_min) {
            forward(500);
            print("For");
        } else {
            backward(200);
            print("Back");
            
            if (dist_L_AVE > dist_CLR_min) {
                turnLeft(rotation_90);
                coast(coast_time);
                print("Left 90");
                //forward(1000);
            } else {
                turnLeft(rotation_180);
                coast(coast_time);
                print("Turn 180");
                //forward(1000);
            }
        }
    }
}

void print(String s) {
    // print(s);
    Serial.println(s);
}

// 設定L298n控制腳位為輸出
void setCarTire() {
    pinMode(Left_Motor_IN1, OUTPUT);
    pinMode(Left_Motor_IN2, OUTPUT);
    pinMode(Right_Motor_IN3, OUTPUT);
    pinMode(Right_Motor_IN4, OUTPUT);
}
// ------------------------------------------------
void forward(int time) {
    motorAForward();
    motorBForward();
    delay(time);
    //print("Forward");
}
 
void backward(int time) {
    motorABackward();
    motorBBackward();
    delay(time);
    //print("Backward");
}
 
void turnRight(int time) {
    motorABackward();
    motorBForward();
    delay(time);
    //print("Right");
}
 
void turnLeft(int time) {
    motorAForward();
    motorBBackward();
    delay(time);
    //print("Left");
}
 
void coast(int time) {
    motorACoast();
    motorBCoast();
    delay(time);
    //print("Stop");
}
 
void brake(int time) {
    motorABrake();
    motorBBrake();
    delay(time);
    //print("brake");
}
// ------------------------------------------------
//motor A controls
void motorAForward() {
    digitalWrite(Left_Motor_IN1, HIGH);
    digitalWrite(Left_Motor_IN2, LOW);
}
 
void motorABackward() {
    digitalWrite(Left_Motor_IN1, LOW);
    digitalWrite(Left_Motor_IN2, HIGH);
}
 
//motor B controls
void motorBForward() {
    digitalWrite(Right_Motor_IN3, HIGH);
    digitalWrite(Right_Motor_IN4, LOW);
}
 
void motorBBackward() {
    digitalWrite(Right_Motor_IN3, LOW);
    digitalWrite(Right_Motor_IN4, HIGH);
}
 
//coasting and braking
void motorACoast() {
    digitalWrite(Left_Motor_IN1, LOW);
    digitalWrite(Left_Motor_IN2, LOW);
}
 
void motorABrake() {
    digitalWrite(Left_Motor_IN1, HIGH);
    digitalWrite(Left_Motor_IN2, HIGH);
}
 
void motorBCoast() {
    digitalWrite(Right_Motor_IN3, LOW);
    digitalWrite(Right_Motor_IN4, LOW);
}
 
void motorBBrake() {
    digitalWrite(Right_Motor_IN3, HIGH);
    digitalWrite(Right_Motor_IN4, HIGH);
}
// ------------------------------------------------
// 超音波 (HC-SR04)
void setUltrasound(int TrigPin) {
    pinMode(TrigPin, OUTPUT);   //set TrigPin as OUTPUT
    Serial.begin(9600);         //uart baud set 9600
}

long Ultrasound(char c) {
    // 音波速度 343.2 m/s, 1/343.2 = 2.91 m/s, 29.1 cm/us
    // 所以 echo, (high 的時間 uS / 2) / 29.1 => 
    // high 的時間 uS/(2 * 29.1) => high 的時間 uS / 58.2
    int EchoPin, TrigPin;
    if (c == 'L') {
        EchoPin = Echo_Pin_L;
        TrigPin = Trig_Pin_L;
    } else if (c == 'R') {
        EchoPin = Echo_Pin_R;
        TrigPin = Trig_Pin_R;
    } else {
        EchoPin = Echo_Pin_C;
        TrigPin = Trig_Pin_C;
    }
    unsigned int dist = ping(EchoPin, TrigPin) / 58;  //calculate distance
    return dist;
}

// 超音波 high 的時間
unsigned long ping(int EchoPin, int TrigPin) { 
    digitalWrite(TrigPin, LOW);
    delayMicroseconds(2);
    // send 10us pulse to HC-SR04 trigger pin
    digitalWrite(TrigPin, HIGH);
    // 超音波只要超過 10us 就好
    delayMicroseconds(10);      //at least 10us HIGH pulse
    digitalWrite(TrigPin, LOW);
    // 傳回 per pulse 時間
    return pulseIn(EchoPin, HIGH);
}






