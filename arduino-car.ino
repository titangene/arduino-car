// L298N 的控制訊號腳位
const byte Left_Motor_IN1 = 13, Left_Motor_IN2 = 12;
const byte Right_Motor_IN3 = 11, Right_Motor_IN4 = 10;

const short rotation_180 = 1200, rotation_90 = 600, 
            rotation_60 = 400, rotation_30 = 200, 
            rotation_20 = 132, rotation_15 = 100;

// SG90 超音波
//Input pin to receive echo pulse
const int Echo_Pin_L = 9, Echo_Pin_R = 7, Echo_Pin_C = 5;
//Output pin to send trigger signal
const int Trig_Pin_L = 8, Trig_Pin_R = 6, Trig_Pin_C = 4;

const int dist_arr_length = 7;
int dist_C[dist_arr_length], dist_L[dist_arr_length], dist_R[dist_arr_length];
int dist_C_AVE = 0, dist_L_AVE = 0, dist_R_AVE = 0;

const int get_dist_delay = 60;
// 車寬 13.5 cm，場地寬 30 cm，左右邊超音波可能最長距離 16.5 cm
const float dist_LR_max = 16.5;
// 即時檢查目前左右邊超音波可能最長距離 (EX：L = 10, 那 R 一定 <= 6.5 )
const float dist_L_current_max = 16.5, dist_R_current_max = 16.5;
const int dist_CLR_min = 5;
// 每格邊長 30 cm，假設場地最大 30 x 5 片，正面超音波可能最長距離 150 cm
const float dist_C_max = 150;

const int perSecGetDist = 10;
int i, count = 0, AVE = 0;

unsigned long previous_time;
unsigned long current_time;
unsigned long calc_time;

void setup() {
    // 設定L298n控制腳位為輸出
    setCarTire();
    // 超音波 (HC-SR04)
    setUltrasound(Trig_Pin_L);
    setUltrasound(Trig_Pin_R);
    setUltrasound(Trig_Pin_C);
}

void loop() {
    dist_C_AVE = 0, dist_L_AVE = 0, dist_R_AVE = 0;
    previous_time = millis();

    action();
    print_dist();

    current_time = millis() - previous_time;
    Serial.print("t: ");
    Serial.println(current_time);
    Serial.println("-------------");
    //delay(500);
}

int get_dist(char c, int distArr[], int dist_AVE) {
    count = 0, AVE = 0;
    // 右: 有 (dist < 4) -> turnLeft 15
    for (i = 0; i < dist_arr_length; i++) {
        distArr[i] = Ultrasound(c);
        // 如果右邊超音波離牆壁距離低於 5 cm 就向左轉校正
        if (c == 'R' && distArr[i] <= dist_CLR_min) {
            turnLeft(rotation_20);
            coast(500);
            Serial.println("Left");
        }
        // 濾波
        dist_AVE = filter(distArr, dist_AVE, i);
        delay(get_dist_delay);
    }
    // 如果全部都是爆掉就設為場地最長值 (此方法不好)
    //if (count == 0) dist_AVE = dist_C_max;
    return count == 0 ? dist_C_max : dist_AVE;
}

// 濾波
int filter(int distArr[], int dist_AVE, int i) {
    // 第一次平均 0~4
    if(i <= 4) {
        if (distArr[i] < dist_C_max) {
            AVE += distArr[i];
            count++;
        }
        if (i == 4)
            dist_AVE = AVE / count;
    } else {
        // 第二次 0~5, 第三次 0~6...
        if (!(distArr[i] > dist_AVE + 3 || distArr[i] < dist_AVE - 3)) {
            AVE += distArr[i];
            count++;
            dist_AVE = AVE / count;
        }
    }
    //Serial.println(dist_AVE);
    return dist_AVE;
}

void action() {
    dist_R_AVE = get_dist('R', dist_R, dist_R_AVE);
    // 右: 空 (dist >= 4) -> turnRight 15
    if (dist_R_AVE >= dist_CLR_min) {
        turnRight(rotation_30);
        coast(800);
        forward(800);
        Serial.println("Left + Forward");
    } else {
        dist_C_AVE = get_dist('C', dist_C, dist_C_AVE);
        if (dist_C_AVE >= dist_CLR_min) {
            forward(800);
            Serial.println("Forward");
        } else {
            dist_L_AVE = get_dist('L', dist_L, dist_L_AVE);
            if (dist_L_AVE >= dist_CLR_min) {
                backward(200);
                turnLeft(rotation_90);
                coast(800);
                Serial.println("Backward + Left");
                //forward(1000);
            } else {
                backward(200);
                turnLeft(rotation_180);
                coast(500);
                Serial.println("Turn 180");
                //forward(1000);
            }
        }
    }
}

void print_dist() {
    Serial.print(dist_C_AVE);
    Serial.print(", ");
    Serial.print(dist_L_AVE);
    Serial.print(", ");
    Serial.println(dist_R_AVE);
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
    //Serial.println("Forward");
}
 
void backward(int time) {
    motorABackward();
    motorBBackward();
    delay(time);
    //Serial.println("Backward");
}
 
void turnRight(int time) {
    motorABackward();
    motorBForward();
    delay(time);
    //Serial.println("Right");
}
 
void turnLeft(int time) {
    motorAForward();
    motorBBackward();
    delay(time);
    //Serial.println("Left");
}
 
void coast(int time) {
    motorACoast();
    motorBCoast();
    delay(time);
    //Serial.println("Stop");
}
 
void brake(int time) {
    motorABrake();
    motorBBrake();
    delay(time);
    //Serial.println("brake");
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
    switch(c) {
        case 'C':  
            EchoPin = Echo_Pin_C;
            TrigPin = Trig_Pin_C;
            break;  
        case 'L':  
            EchoPin = Echo_Pin_L;
            TrigPin = Trig_Pin_L;
            break;  
        case 'R':  
            EchoPin = Echo_Pin_R;
            TrigPin = Trig_Pin_R;
            break;  
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

