// L298N 的控制訊號腳位
const byte IN1_L = 13, IN2_L = 12, IN3_R = 11, IN4_R = 10;

// SG90 超音波
//Input pin to receive echo pulse
const byte EchoPin_L = 9, EchoPin_R = 7, EchoPin_C = 5;
//Output pin to send trigger signal
const byte TrigPin_L = 8, TrigPin_R = 6, TrigPin_C = 4;
long dist_L, dist_R, dist_C;

// 伺服馬達 (SG90)
const byte PWM = 3;

#include <Servo.h>

Servo myservo;  // create servo object to control a servo
int pos = 0;    // variable to store the servo position

const byte min_R = 5, max_R = 7, over_R = 10, out_R = 3400;
String logStr = "";

void setup() {
    // 設定L298n控制腳位為輸出
    setCarTire();
    // 超音波 (HC-SR04)
    setUltrasound(TrigPin_L);
    setUltrasound(TrigPin_R);
    setUltrasound(TrigPin_C);
    // 伺服馬達 (SG90)
    //setServoMotor(PWM);
}

boolean run = true;

void loop() {
    dist_L = Ultrasound(EchoPin_L, TrigPin_L);
    dist_R = Ultrasound(EchoPin_R, TrigPin_R);
    dist_C = Ultrasound(EchoPin_C, TrigPin_C);

    Serial.print(logStr); Serial.print(" - ");
    Serial.print(dist_L); Serial.print(", ");
    Serial.print(dist_C); Serial.print(", ");
    Serial.println(dist_R);

    // carMove();    
    //delay(500);
    if (dist_C > max_R) {
        forward(500);
    } else {
        backward(500);
    }
    // forward(1500);
    // coast(300);
    // turnRight(600);
    // coast(300);
    // backward(1500);
    // coast(300);
    // turnLeft(600);
    // coast(300);

    //ServoMotor_180(EchoPin_C, TrigPin_C);
}

void carMove() {
    // 控制馬達B 正轉 // 10
    //digitalWrite(IN3_R, HIGH);
    //digitalWrite(IN4_R, LOW);

    // if (dist_C > max_R) {
    //     if (dist_L > max_R && dist_R > max_R) {
    //         forward();
    //     } else {
    //         if (dist_L < min_R && dist_R > max_R) {
    //             TRight();
    //         }
    //         if (dist_R < min_R && dist_L > max_R) {
    //             TLeft();
    //         }
    //     }
    // } else {
    //     if (dist_L > max_R && dist_R > max_R) {
    //         reverse();
    //     } else {
    //         if (dist_L < min_R && dist_R > max_R) {
    //             TRight();
    //         }
    //         if (dist_R < min_R && dist_L > max_R) {
    //             TLeft();
    //         }
    //     }
    // }

    

    // if (dist_C < min_R) {
    //     reverse();
    // }
    // if (dist_L > min_R && dist_C > min_R && dist_R > min_R) {
    //     forward();
    // }
    // if (dist_L < min_R && dist_R > max_R) {
    //     TLeft();
    // }
    // if (dist_L > max_R && dist_R < min_R) {
    //     TRight();
    // }

    // reverse();
    // TLeft();
    // TRight();
    // stop();
}

// 設定L298n控制腳位為輸出
void setCarTire() {
    pinMode(IN1_L, OUTPUT);
    pinMode(IN2_L, OUTPUT);
    pinMode(IN3_R, OUTPUT);
    pinMode(IN4_R, OUTPUT);
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
    digitalWrite(IN1_L, HIGH);
    digitalWrite(IN2_L, LOW);
}
 
void motorABackward() {
    digitalWrite(IN1_L, LOW);
    digitalWrite(IN2_L, HIGH);
}
 
//motor B controls
void motorBForward() {
    digitalWrite(IN3_R, HIGH);
    digitalWrite(IN4_R, LOW);
}
 
void motorBBackward() {
    digitalWrite(IN3_R, LOW);
    digitalWrite(IN4_R, HIGH);
}
 
//coasting and braking
void motorACoast() {
    digitalWrite(IN1_L, LOW);
    digitalWrite(IN2_L, LOW);
}
 
void motorABrake() {
    digitalWrite(IN1_L, HIGH);
    digitalWrite(IN2_L, HIGH);
}
 
void motorBCoast() {
    digitalWrite(IN3_R, LOW);
    digitalWrite(IN4_R, LOW);
}
 
void motorBBrake() {
    digitalWrite(IN3_R, HIGH);
    digitalWrite(IN4_R, HIGH);
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
void setServoMotor(byte PWM) {
    myservo.attach(PWM, 500, 2400); // 修正脈衝寬度範圍
    myservo.write(90); //置中
    delay(10000); 
}

void ServoMotor_180(byte EchoPin, byte TrigPin) {
    //Ultrasound(EchoPin, TrigPin);
    for (pos = 0; pos <= 180; pos += 1) { // goes from 0 degrees to 180 degrees
        // in steps of 1 degree
        myservo.write(pos);              // tell servo to go to position in variable 'pos'
        delay(15);                       // waits 15ms for the servo to reach the position

        //Ultrasound(EchoPin, TrigPin);
    }
    for (pos = 180; pos >= 0; pos -= 1) { // goes from 180 degrees to 0 degrees
        myservo.write(pos);              // tell servo to go to position in variable 'pos'
        delay(15);                       // waits 15ms for the servo to reach the position

        //Ultrasound(EchoPin, TrigPin);
    }
}

