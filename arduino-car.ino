// SG90 超音波
//Input pin to receive echo pulse
const int Echo_Pin_L = 9, Echo_Pin_R = 7, Echo_Pin_C = 5;
//Output pin to send trigger signal
const int Trig_Pin_L = 8, Trig_Pin_R = 6, Trig_Pin_C = 4;

unsigned int dist_C[15], dist_L[15], dist_R[15];
unsigned int dist_L_AVE;
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
int i_C = 0, i_L = 0, i_R = 0;
unsigned int dis_C[20], dis_L[20], dis_R[20];

void setup() {
    // 超音波 (HC-SR04)
    setUltrasound(Trig_Pin_L);
    setUltrasound(Trig_Pin_R);
    setUltrasound(Trig_Pin_C);
}

void loop() {
    get_dist();
    action(0);
    delay(500);
}

void get_dist() {
    i_C = 0, i_L = 0, i_R = 0;
    cut_C = 0, cut_L = 0, cut_R = 0;
    previous_time = millis();

    get_dist_CLR(0, 0);
    get_dist_CLR(0, 0);
    get_dist_CLR(0, 0);

    current_time = millis() - previous_time;
    Serial.println(current_time);

    print_dist();
}

void get_dist_CLR(bool L_Flag, bool R_Flag) {
    get_dist_C();
    get_dist_L();
    if (L_Flag) get_dist_L();
    if (R_Flag) get_dist_R();
    get_dist_R();
}
void get_dist_C() {
    dist_C[i_C] = Ultrasound('C');
    delay(get_dist_delay);
    i_C++;
}
void get_dist_L() {
    dist_L[i_L] = Ultrasound('L');
    delay(get_dist_delay);
    i_L++;
}
void get_dist_R() {
    dist_R[i_R] = Ultrasound('R');
    delay(get_dist_delay);
    i_R++;
}
void print_dist() {
    for (i = 0; i < i_C; i++) {
        // if (dist_C[i] < dist_C_max) {
        //     interval(interval_L, dist_C[i]);
        // } else {
        //     dist_C_Throw[count] = dist_C[i];
        //     cut_C++;
        // }
        Serial.print(dist_C[i]);
        Serial.print(", ");
    }
    Serial.println("");
    for (i = 0; i < i_L; i++) {
        // if (dist_L[i] < dist_L_current_max) {
        //     interval(interval_L, dist_L[i]);
        // }
        Serial.print(dist_L[i]);
        Serial.print(", ");
        
    }
    Serial.println("");
    for (i = 0; i < i_R; i++) {
        Serial.print(dist_R[i]);
        Serial.print(", ");
    }
    Serial.println("");
}

// void interval(unsigned int interval[][], unsigned int dist) {
//     unsigned int dist_arr_i = dist >= 30 ? interval_length_OneDim : dist / 5;
//     interval[]
//     Serial.print(dist_arr_i);
//     Serial.print(": ");
// }

void action(int dist) {

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













