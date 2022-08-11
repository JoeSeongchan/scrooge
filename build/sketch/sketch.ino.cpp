#include <Arduino.h>
#line 1 "c:\\Users\\wordn\\arduino_projects\\sketch.ino"
#define TIME_LIMIT 60
#define ZERO_AMPS 0.18
#define ONGOING 1
#define STOPPED 0
#define MILLIS_IN_SECOND 1000
#define SECONDS_IN_MINUTE 10
#define SWITCH_PIN 8
float getVPP();
void relayOn();
void relayOff();
int isTherePowerUse(double ampsRMS);
int isPowerWastedFor60Sec(double ampsRMS);
int isSwitchDown();
const int sensorIn = A0;
int mVperAmp = 66; // use 185 for 5A Module and 100 for 20A Module

double Voltage = 0;
double VRMS = 0;
double AmpsRMS = 0;

int Relaypin1 = 12; // IN1

#line 23 "c:\\Users\\wordn\\arduino_projects\\sketch.ino"
void setup();
#line 30 "c:\\Users\\wordn\\arduino_projects\\sketch.ino"
void loop();
#line 23 "c:\\Users\\wordn\\arduino_projects\\sketch.ino"
void setup()
{
    Serial.begin(9600);         // 시리얼 통신 속도 9600bps
    pinMode(Relaypin1, OUTPUT); // 릴레이 제어 1번핀을 출력으로 설정
    pinMode(SWITCH_PIN, INPUT_PULLUP);
}

void loop()
{
    Voltage = getVPP();
    VRMS = (Voltage / 2.0) * 0.707;     // RMS값 70.7%
    AmpsRMS = (VRMS * 1000) / mVperAmp; // mA 단위 맞춰줌
    Serial.print(AmpsRMS);
    Serial.println(" Amps RMS");
    if (isPowerWastedFor60Sec(AmpsRMS))
    {
        Serial.print("relay off\n");
        relayOff();
    }
    if (isSwitchDown())
    {
        relayOn();
    }
}

int isSwitchDown()
{
    if (digitalRead(SWITCH_PIN) == LOW)
    {
        return 1;
        // Serial.println("LOW");
        // delay(1000);
    }
    else
    {
        return 0;
        // Serial.println("HIGH");
        // delay(1000);
    }
}

// 모니터 연결 전: 0.16
// 모니터 연결 후, 전원 ON: 0.47~0.50
// 모니터 연결 후, 전원 OFF: 0.18
// 모니터가 꺼져 있다는 것을 어떻게 알 수 있는가?
// 기본값과 거의 비슷한 전원 수치를 가지는 때에는 모니터가 꺼져 있다고 인식한다.
float getVPP()
{
    float result;

    int readValue;       // value read from the sensor
    int maxValue = 0;    // store max value here
    int minValue = 1024; // store min value here

    uint32_t start_time = millis();
    while ((millis() - start_time) < 1000) // 1초동안 값을 모아서 AC 전류의 최고점, 최저점을 찾아 평균치를 구함
    {
        readValue = analogRead(sensorIn);
        // see if you have a new maxValue
        if (readValue > maxValue)
        {
            /*record the maximum sensor value*/
            maxValue = readValue;
        }
        if (readValue < minValue)
        {
            /*record the maximum sensor value*/
            minValue = readValue;
        }
    }

    // Subtract min from max
    result = ((maxValue - minValue) * 5.0) / 1024.0; // 5V 분해능, 아날로그핀의 분해능 1024

    return result;
}

void relayOn()
{
    digitalWrite(Relaypin1, LOW); // 릴레이 ON
    delay(2000);                  // 2초 대기
}

void relayOff()
{
    digitalWrite(Relaypin1, HIGH); // 릴레이 OFF
    delay(2000);
}

int isTherePowerUse(double ampsRMS)
{
    if (ampsRMS - ZERO_AMPS > 0.1)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

// 60초 동안 전력이 낭비되는 때를 포착.
int isPowerWastedFor60Sec(double ampsRMS)
{
    static uint32_t startTime = 0;
    uint32_t endTime = 0;
    if (startTime == 0)
    {
        startTime = millis();
        return 0;
    }
    endTime = millis();
    if (endTime - startTime < MILLIS_IN_SECOND * SECONDS_IN_MINUTE)
    {
        if (isTherePowerUse(ampsRMS))
        {
            startTime = millis();
            return 0;
        }
    }
    else
    {
        startTime = 0;
        return 1;
    }
}
