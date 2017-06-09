// L298N 的控制訊號腳位
const byte Left_Motor_IN1 = 13, Left_Motor_IN2 = 12;
const byte Right_Motor_IN3 = 11, Right_Motor_IN4 = 10;

const short rotation_180 = 1200, rotation_90 = 600, rotation_60 = 400, rotation_30 = 200, rotation_20 = 132, rotation_15 = 100;

// SG90 超音波
//Input pin to receive echo pulse
const int Echo_Pin_L = 9, Echo_Pin_R = 7, Echo_Pin_C = 5;
//Output pin to send trigger signal
const int Trig_Pin_L = 8, Trig_Pin_R = 6, Trig_Pin_C = 4;


const int dist_arr_length = 7;
unsigned int dist_C[dist_arr_length], dist_L[dist_arr_length], dist_R[dist_arr_length];
unsigned int dist_C_AVE = 0, dist_L_AVE = 0, dist_R_AVE = 0;
unsigned int dist_C_Throw[15], dist_L_Throw[15], dist_R_Throw[15];
int cut_C = 0, cut_L = 0, cut_R = 0;
const int interval_length_OneDim = 7, 
          interval_C_length_TwoDim = 5, 
          interval_LR_length_TwoDim = 10;

// unsigned int interval_C[interval_length][interval_C_length_TwoDim], 
//              interval_L[interval_length][interval_LR_length_TwoDim], 
//              interval_R[interval_length][interval_LR_length_TwoDim];

unsigned int tmpDist_C, tmpDist_L, tmpDist_R;

const int get_dist_delay = 60;
// 車寬 13.5 cm，場地寬 30 cm，左右邊超音波可能最長距離 16.5 cm
const float dist_LR_max = 16.5;
// 即時檢查目前左右邊超音波可能最長距離 (EX：L = 10, 那 R 一定 <= 6.5 )
const float dist_L_current_max = 16.5, dist_R_current_max = 16.5;
const int dist_CLR_min = 4;
// 每格邊長 30 cm，假設場地最大 30 x 5 片，正面超音波可能最長距離 150 cm
const float dist_C_max = 150;

const int perSecGetDist = 10;
int i, count = 0;

unsigned long previous_time;
unsigned long current_time;
unsigned long calc_time;

bool L_Flag = false, R_Flag = false;
#define true 1
#define false 0

int x = 0;
int count_C = 0, count_L = 0, count_R = 0;
int count_C_Return = 0, count_L_Return = 0;
unsigned int dis_C[20], dis_L[20], dis_R[20];

int j, AVE = 0;


void setup() {
    // 設定L298n控制腳位為輸出
    setCarTire();
    // 超音波 (HC-SR04)
    setUltrasound(Trig_Pin_L);
    setUltrasound(Trig_Pin_R);
    setUltrasound(Trig_Pin_C);
}

void loop() {
    get_dist();
    Serial.println("-------------");
    //delay(500);
}

void get_dist() {
    dist_C_AVE = 0, dist_L_AVE = 0, dist_R_AVE = 0, AVE = 0;
    count_C = 0, count_L = 0, count_R = 0;
    previous_time = millis();

    action();
    print_dist();

    current_time = millis() - previous_time;
    Serial.print("t: ");
    Serial.println(current_time);
}
void action() {
    get_dist_R();
    // 右: 空 (dist >= 4) -> turnRight 15
    if (dist_R_AVE >= dist_CLR_min) {
        turnRight(rotation_30);
        coast(500);
        forward(800);
    } else {
        get_dist_C();
        if (dist_C_AVE >= dist_CLR_min)
            forward(800);
        else {
            get_dist_L();
            if (dist_L_AVE >= dist_CLR_min) {
                turnLeft(rotation_90);
                coast(500);
                //forward(1000);
            } else {
                turnLeft(rotation_180);
                coast(500);
                //forward(1000);
            }
        }
    }
}
void get_dist_R() {
    // 右: 有 (dist < 4) -> turnLeft 15
    for (i = 0; i < dist_arr_length; i++) {
        dist_R[i] = Ultrasound('R');
        // 如果右邊超音波離牆壁距離低於 4 cm 就向左轉校正，
        if (dist_R[i] < dist_CLR_min) {
            turnLeft(rotation_20);
            coast(500);
        }
        // 濾波
        if(i <= 4) {
            if (dist_R[i] < dist_C_max) {
                dist_R_AVE += dist_R[i];
                count_R++;
            }
            if (i == 4) {
                AVE = dist_R_AVE;
                dist_R_AVE = dist_R_AVE / count_R;
                //Serial.println(AVE);
            }
        } else {
            // 第二次 0~5, 第三次 0~6...
            if (!(dist_R[i] > dist_R_AVE + 3 || dist_R[i] < dist_R_AVE - 3)) {
                count_R++;
                AVE += dist_R[i];
                dist_R_AVE = AVE / count_R;
            }
        }

        delay(get_dist_delay);
    }
    // 如果全部都是爆掉就設為場地最長值 (此方法不好)
    if (count_R == 0) dist_R_AVE = dist_C_max;
}
void get_dist_C() {
    // 中 or 左: 有 (dist < 4) -> turnLeft 15
    for (i = 0; i < dist_arr_length; i++) {
        dist_C[i] = Ultrasound('C');
        // 濾波
        if(i <= 4) {
            if (dist_C[i] < dist_C_max) {
                dist_C_AVE += dist_C[i];
                count_C++;
            }
            if (i == 4) {
                AVE = dist_C_AVE;
                dist_C_AVE = dist_C_AVE / count_C;
                //Serial.println(AVE);
            }
        } else {
            // 第二次 0~5, 第三次 0~6...
            if (!(dist_C[i] > dist_C_AVE + 3 || dist_C[i] < dist_C_AVE - 3)) {
                count_C++;
                AVE += dist_C[i];
                dist_C_AVE = AVE / count_C;
            }
        }
        delay(get_dist_delay);
    }
    // 如果全部都是爆掉就設為場地最長值 (此方法不好)
    if (count_C == 0) dist_C_AVE = dist_C_max;
}
void get_dist_L() {
    // 中 or 左: 有 (dist < 4) -> turnLeft 15
    for (i = 0; i < dist_arr_length; i++) {
        dist_L[i] = Ultrasound('L');
        // 濾波
        if(i <= 4) {
            if (dist_L[i] < dist_C_max) {
                dist_L_AVE += dist_L[i];
                count_L++;
            }
            if (i == 4) {
                AVE = dist_L_AVE;
                dist_L_AVE = dist_L_AVE / count_L;
                //Serial.println(AVE);
            }
        } else {
            // 第二次 0~5, 第三次 0~6...
            if (!(dist_L[i] > dist_L_AVE + 3 || dist_L[i] < dist_L_AVE - 3)) {
                count_L++;
                AVE += dist_L[i];
                dist_L_AVE = AVE / count_L;
            }
        }
        delay(get_dist_delay);
    }
    // 如果全部都是爆掉就設為場地最長值 (此方法不好)
    if (count_L == 0) dist_L_AVE = dist_C_max;
}
// 濾波
unsigned int filter(unsigned int distArr[], unsigned int dist_AVE, int count, int i) {
    // // 第一次平均 0~4
    // if(i <= 4) {
    //     if (distArr[i] < dist_C_max) {
    //         dist_AVE += distArr[i];
    //         count++;
    //     }
    //     if (i == 4) {
    //         AVE = dist_AVE;
    //         dist_AVE = dist_AVE / count;
    //         //Serial.println(AVE);
    //     }
    // } else {
    //     // 第二次 0~5, 第三次 0~6...
    //     if (!(distArr[i] > dist_AVE + 3 || distArr[i] < dist_AVE - 3)) {
    //         count++;
    //         AVE += distArr[i];
    //         dist_AVE = AVE / count;
    //     }
    // }
    // Serial.println(dist_AVE);
    // return dist_AVE;
}

void print_dist() {
    Serial.print(dist_C_AVE);
    Serial.print(", ");
    Serial.print(dist_L_AVE);
    Serial.print(", ");
    Serial.println(dist_R_AVE);
}

void func() {
    dist_L_AVE = 0, count = 0;
    Serial.print(x);
    Serial.print(" - ");
    
    for (i = 0; i < perSecGetDist; i++) {
        tmpDist_L = Ultrasound('L');
        Serial.print(tmpDist_L);
        Serial.print(", ");

        if (tmpDist_L < dist_LR_max) {
            dist_L[i] = tmpDist_L;
            dist_L_AVE += tmpDist_L;
        } else {
            dist_L_Throw[count] = tmpDist_L;
            count++;
        }
    }

    Serial.println("");
    Serial.print("out: "); 
    if (count != 0) {
        for (i = 0; i < count; i++) {
            Serial.print(dist_L_Throw[i]);
            Serial.print(", ");
        }
    count = 10 - count; // 將原本爆掉數值數量 改成 可用數值數量
    }
    // 如果有 3/10 以上數值都爆掉，大部分都是超音波離障礙物太近的原因，因此該方向超音波距離就設為 1 cm
    dist_L_AVE = count >= 7 ? dist_L_AVE / count : 1;
    
    Serial.println("");
    current_time = millis();
    Serial.print(current_time);
    Serial.print(" - ");
    Serial.print(count);
    Serial.print(" - ");
    Serial.println(dist_L_AVE);
    Serial.println("--------------------------------------");

    delay(500);
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
    Serial.println("Forward");
}
 
void backward(int time) {
    motorABackward();
    motorBBackward();
    delay(time);
    Serial.println("Backward");
}
 
void turnRight(int time) {
    motorABackward();
    motorBForward();
    delay(time);
    Serial.println("Right");
}
 
void turnLeft(int time) {
    motorAForward();
    motorBBackward();
    delay(time);
    Serial.println("Left");
}
 
void coast(int time) {
    motorACoast();
    motorBCoast();
    delay(time);
    Serial.println("Stop");
}
 
void brake(int time) {
    motorABrake();
    motorBBrake();
    delay(time);
    Serial.println("brake");
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
    if (c == 'C') {
        EchoPin = Echo_Pin_C;
        TrigPin = Trig_Pin_C;
    }
    if (c == 'L') {
        EchoPin = Echo_Pin_L;
        TrigPin = Trig_Pin_L;
    }
    if (c == 'R') {
        EchoPin = Echo_Pin_R;
        TrigPin = Trig_Pin_R;
    }
    unsigned int dist = ping(EchoPin, TrigPin) / 58;  //calculate distance
    return dist;
}

// 超音波 high 的時間
unsigned long ping(int EchoPin, int TrigPin) { 
    // send 10us pulse to HC-SR04 trigger pin
    digitalWrite(TrigPin, HIGH);
    // 超音波只要超過 10us 就好
    delayMicroseconds(10);      //at least 10us HIGH pulse
    digitalWrite(TrigPin, LOW);
    // 傳回 per pulse 時間
    return pulseIn(EchoPin, HIGH);
}

















