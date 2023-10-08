// 2個のKeiganMotorをパンチルトモータとして利用するプログラム例
// M5StackのBtnA, B, Cでパン角、チルト角を制御する

#include "KeiganBLE.h"

#define PAN_MOTOR_MAC_ADDR "AA:BB:CC:DD:EE:FF"  // 自分のKeiganMotorのMACIDに変えてください
#define TILT_MOTOR_MAC_ADDR "GG:HH:II:JJ:KK:LL"
class PanTiltMotor
{
private:
    KeiganMotor panMotor;
    KeiganMotor tiltMotor;
    float panAngle = 0;
    float tiltAngle = 0;
    bool connected = false;
public:
    PanTiltMotor(){ }
    bool initialConnect(float speed_dps = 90, float initial_pos = 0)
    {
        // モータの接続
        BLEDevice::init("M5AtomLite");   // Initialize BLE
        Serial.print("Connecting to KeiganMotor...\n");
        if (panMotor.connect(PAN_MOTOR_MAC_ADDR))
            Serial.print("PanMotor connected!\n");
        else{
            Serial.print("PanMotor is not found...\n");
            connected = false;
            return false;
        }
        if (tiltMotor.connect(TILT_MOTOR_MAC_ADDR))
            Serial.print("TiltMotor connected!\n");
        else{
            Serial.print("TiltMotor is not found...\n");
            connected = false;
            return false;
        }
        // モータを駆動できる状態にする
        panMotor.enableControl();
        panMotor.setSpeed(speed_dps);
        tiltMotor.enableControl();
        tiltMotor.setSpeed(speed_dps);
        connected = true;
        return true;
    }
    void setAngle(float pan_deg, float tilt_deg){
        if (pan_deg != panAngle && pan_deg < 1000){
            panMotor.stopDoingTaskset();
            panMotor.moveTo(pan_deg);
            panAngle = pan_deg;
        }
        if (tilt_deg != tiltAngle && pan_deg < 1000){
            tiltMotor.stopDoingTaskset();
            tiltMotor.moveTo(tilt_deg);
            tiltAngle = tilt_deg;
        }
    }
    float getPanAngle(){
        return panMotor.getMotorInfo();
    }
    float getTiltAngle(){
        return tiltMotor.getMotorInfo();
    }
};


PanTiltMotor panTiltMotor;  // Keiganパンチルト台ライブラリ
int pan = 0;  
int tilt = 0; 
int max_pan_angle = 180;
int min_pan_angle = -180;
int max_tilt_angle = 180;
int min_tilt_angle = -180;
bool is_current_pan = true;

void DisplayInfo(){
    M5.Lcd.clear(BLACK);  // 黒塗り
    M5.Lcd.setTextColor(CYAN);
    M5.Lcd.setTextSize(3);

    M5.Lcd.setCursor(3, 10);
    M5.Lcd.printf("Pan angle:%d\n", pan);
    M5.Lcd.printf("Tilt angle:%d deg\n", tilt);
    M5.Lcd.printf("BtnA : Change controlled motor\n");
    M5.Lcd.printf("BtnB : %s angle ++\n", (is_current_pan)? "Pan": "Tilt");
    M5.Lcd.printf("BtnC : %s angle --\n", (is_current_pan)? "Pan": "Tilt");
}

void setup()
{
    // M5Stackの設定（Serial:true, I2C:false, Display:true）
    M5.begin(true, false, true);

    M5.Lcd.clear(BLACK);  // 黒塗り
    M5.Lcd.setTextColor(CYAN);
    M5.Lcd.setTextSize(3);
    M5.Lcd.setCursor(3, 10);
    M5.Lcd.printf("Connecting to PanMotor...\n");

    panTiltMotor.initialConnect(90, 0);
    DisplayInfo();
}

void loop()
{
    M5.update();

    if (M5.BtnA.wasReleased()){
        is_current_pan = !is_current_pan;
        DisplayInfo();
    }
    if (M5.BtnB.pressedFor(600)){
        if (is_current_pan){
            pan -= 10;
            if (pan < min_pan_angle) pan = min_pan_angle;
        }
        else{
            tilt -= 10;
            if (tilt < min_tilt_angle) tilt = min_tilt_angle;
        }
        DisplayInfo();
    }    
    if (M5.BtnB.wasReleased()){
        if (is_current_pan){
            pan -= 1;
            if (pan < min_pan_angle) pan = min_pan_angle;
        }
        else{
            tilt -= 1;
            if (tilt < min_tilt_angle) tilt = min_tilt_angle;
        }
        DisplayInfo();
    }
    if (M5.BtnC.pressedFor(600)){
        if (is_current_pan){
            pan += 10;
            if (pan > max_pan_angle) pan = max_pan_angle;
        }
        else{
            tilt += 10;
            if (tilt > max_tilt_angle) tilt = max_tilt_angle;
        }
        DisplayInfo();
    }    
    if (M5.BtnC.wasReleased()){
        if (is_current_pan){
            pan += 1;
            if (pan > max_pan_angle) pan = max_pan_angle;
        }
        else{
            tilt += 1;
            if (tilt > max_tilt_angle) tilt = max_tilt_angle;
        }
        DisplayInfo();
    }

    // パンチルト台の制御
    panTiltMotor.setAngle(pan, tilt);
    Serial.printf("pan : %d, tilt : %d\n", (int)(panTiltMotor.getPanAngle()), (int)(panTiltMotor.getTiltAngle()));
}