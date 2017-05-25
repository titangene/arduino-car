#include <Servo.h>         // Include Servo Motor library

// L298N 的控制訊號腳位
#define Left_Motor_IN1 = 13;
#define Left_Motor_IN2 = 12;
#define Right_Motor_IN3 = 11;
#define Right_Motor_IN4 = 10;

// SG90 超音波
//Input pin to receive echo pulse
#define Echo_Pin_L = 9;
#define Echo_Pin_R = 7;
#define Echo_Pin_C = 5;
//Output pin to send trigger signal
#define Trig_Pin_L = 8;
#define Trig_Pin_R = 6;
#define Trig_Pin_C = 4;
unsigned long dist_L, dist_R, dist_C;

// 伺服馬達 (SG90)
#define ServoPin = 3;
Servo servoMotor;  // create servo object to control a servo

const byte MIN_Range = 5, MAX_Range = 7, OVER_Range = 10;
const short OUT_Range = 3400;
const short rotation_180 = 1200, rotation_90 = 600, rotation_60 = 400, rotation_30 = 200, rotation_15 = 100;
byte servo_Pos = 0;    // variable to store the servo position
String logStr = "";

void setup() {
    // 設定L298n控制腳位為輸出
    setCarTire();
    // 超音波 (HC-SR04)
    setUltrasound(Trig_Pin_L);
    setUltrasound(Trig_Pin_R);
    setUltrasound(Trig_Pin_C);
    // 伺服馬達 (SG90)
    //setServoMotor(ServoPin);
}

void loop() {
    dist_L = Ultrasound(Echo_Pin_L, Trig_Pin_L);
    dist_R = Ultrasound(Echo_Pin_R, Trig_Pin_R);
    dist_C = Ultrasound(Echo_Pin_C, Trig_Pin_C);

    Serial.print(logStr); Serial.print(" - ");
    Serial.print(dist_L); Serial.print(", ");
    Serial.print(dist_C); Serial.print(", ");
    Serial.println(dist_R);

    carMove();    
    delay(50);

    // forward(1500);
    // backward(1500);
    // turnLeft(600);
    // turnRight(600);
    // coast(300);

    //ServoMotor_180(Echo_Pin_C, Trig_Pin_C);
}

void carMove() {
    if (dist_C > MAX_Range) {
        if (dist_L > MIN_Range && dist_R > MIN_Range) {
            forward(500);
        } else if (dist_L <= MIN_Range && dist_R > MIN_Range) {
            turnLeft(rotation_15);
            coast(500);
        } else if (dist_L > MIN_Range && dist_R <= MIN_Range) {
            turnRight(rotation_15);
            coast(500);
        } else {
            turnRight(rotation_180);
            coast(500);
        }
        
    } else {
        coast(500);
        backward(300);

        if (dist_L > MIN_Range && dist_R > MIN_Range) {
            rotateRandom();
        } else if (dist_L <= MIN_Range && dist_R > MIN_Range) {
            turnLeft(rotation_90);
            coast(500);
        } else if (dist_L > MIN_Range && dist_R <= MIN_Range) {
            turnRight(rotation_90);
            coast(500);
        } else {
            turnRight(rotation_180);
            coast(500);
        }
    }
}

void chooseRotation() {
    coast(500);
    backward(500);

    if (dist_R < MIN_Range && dist_L > MAX_Range) {
        turnLeft(rotation_90);
        coast(500);
    } else if (dist_L < MIN_Range && dist_R > MAX_Range) {
        turnRight(rotation_90);
        coast(500);
    } else {
        rotateRandom();
    }
}

void rotateRandom() {
    // random number 0 ~ 1
    long randomNumber = random(2);
    if (randomNumber == 0)
        turnLeft(rotation_90);
    else
        turnRight(rotation_90);
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
    logStr = "F";
}
 
void backward(int time) {
    motorABackward();
    motorBBackward();
    delay(time);
    logStr = "B";
}
 
void turnLeft(int time) {
    motorABackward();
    motorBForward();
    delay(time);
    logStr = "L";
}
 
void turnRight(int time) {
    motorAForward();
    motorBBackward();
    delay(time);
    logStr = "R";
}
 
void coast(int time) {
    motorACoast();
    motorBCoast();
    delay(time);
    logStr = "C";
}
 
void brake(int time) {
    motorABrake();
    motorBBrake();
    delay(time);
    logStr = "K";
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
void setUltrasound(byte TrigPin) {
    pinMode(TrigPin, OUTPUT);   //set TrigPin as OUTPUT
    Serial.begin(9600);         //uart baud set 9600
}

long Ultrasound(byte EchoPin, byte TrigPin) {
    // 音波速度每秒 343.2 公尺, 1/343.2 = 2.91 ms, 1公尺 2.91 ms, 每公分 29.1 us
    // 所以 echo, (high 的時間 uS / 2) / 29.1 => 
    // high 的時間 uS/(2 * 29.1) => high 的時間 uS / 58.2
    unsigned long dist = ping(EchoPin, TrigPin) / 58;  //calculate distance
    return dist;
}

// 超音波 high 的時間
unsigned long ping(byte EchoPin, byte TrigPin) { 
    //send 10us pulse to HC-SR04 trigger pin
    digitalWrite(TrigPin, HIGH);
    // 超音波只要超過 10us 就好
    delayMicroseconds(10);      //at least 10us HIGH pulse
    digitalWrite(TrigPin, LOW);
    // 傳回 per pulse 時間
    return pulseIn(EchoPin, HIGH);
}
// ------------------------------------------------
// 伺服馬達 (SG90)
void setServoMotor(byte ServoPin) {
    servoMotor.attach(ServoPin, 500, 2400); // 修正脈衝寬度範圍
    servoMotor.write(90); //置中
    delay(10000); 
}

void ServoMotor_180(byte EchoPin, byte TrigPin) {
    //Ultrasound(EchoPin, TrigPin);
    for (pos = 0; servo_Pos <= 180; servo_Pos += 1) { // goes from 0 degrees to 180 degrees
        // in steps of 1 degree
        servoMotor.write(pos);              // tell servo to go to position in variable 'pos'
        delay(15);                       // waits 15ms for the servo to reach the position

        //Ultrasound(EchoPin, TrigPin);
    }
    for (pos = 180; servo_Pos >= 0; servo_Pos -= 1) { // goes from 180 degrees to 0 degrees
        servoMotor.write(pos);              // tell servo to go to position in variable 'pos'
        delay(15);                       // waits 15ms for the servo to reach the position

        //Ultrasound(EchoPin, TrigPin);
    }
}


