/* 2023年 youseegreen */

#include <M5Stack.h>
#include <BLEDevice.h>
#include <BLEClient.h>

#define DEG2RAD 0.01745329     // PI/180
#define RAD2DEG 57.3           // 180/PI
// https://keigan.zendesk.com/hc/ja/articles/4407181689497
#define KM_SERVICE_UUID "f140ea35-8936-4d35-a0ed-dfcd795baa8c"
#define MOTOR_TX_UUID "f1400001-8936-4d35-a0ed-dfcd795baa8c"
#define MOTOR_LED_UUID "f1400003-8936-4d35-a0ed-dfcd795baa8c"
#define MOTOR_MEASUREMENT_UUID "f1400004-8936-4d35-a0ed-dfcd795baa8c"
#define MOTOR_IMU_MEASUREMENT_UUID "f1400005-8936-4d35-a0ed-dfcd795baa8c"
#define MOTOR_RX_UUID "f1400006-8936-4d35-a0ed-dfcd795baa8c"

// 他の機能を実装したい場合 https://keigan.zendesk.com/hc/ja/articles/4407181689497 を参照
// リトルエンディアンに注意
class KeiganMotor
{
private:
    bool connected = false;
    BLEClient *pClient = NULL;
    BLERemoteCharacteristic *pTxCharacteristic = NULL;
    BLERemoteCharacteristic *pMeasurementCharacteristic = NULL;
    BLERemoteCharacteristic *pRxCharacteristic = NULL;

public:
    MyMotor() {}


    /// @brief モータとBLE接続する
    /// @param macAddr 接続するモータのMACアドレス
    /// @return 接続できたかどうか
    bool connect(const char *macAddr)
    {
        pClient = BLEDevice::createClient();
        connected = pClient->connect(BLEAddress(macAddr), BLE_ADDR_TYPE_RANDOM);
        if (!connected)
            return false;
        BLERemoteService *pRemoteService = pClient->getService(KM_SERVICE_UUID);
        pTxCharacteristic = pRemoteService->getCharacteristic(MOTOR_TX_UUID);
        pMeasurementCharacteristic = pRemoteService->getCharacteristic(MOTOR_MEASUREMENT_UUID);
        pRxCharacteristic = pRemoteService->getCharacteristic(MOTOR_RX_UUID);
        return true;
    }


    /// @brief モータとBLE通信しているか確認する
    /// @return 接続しているかどうか
    bool isConnected(){
        return connected;
    }


    /// @brief BLE通信を終了する
    void disconnect()
    {
        // FIXME
    }    


    /// @brief モータを動作可能状態にする
    void enableControl()
    {
        uint8_t cmd[] = {0x51, 0x00, 0x00, 0x00, 0x00};
        pTxCharacteristic->writeValue(cmd, sizeof(cmd));
    }


    /// @brief モータの回転速度を設定する
    /// @param dps 所望の回転速度（degree per sec）
    void setSpeed(float dps)
    { 
        float rps = dps * DEG2RAD;
        u_int8_t *p = (uint8_t *)&rps;
        // values=float2bytes(speed)
        // struct.pack("!f", float_value)
        uint8_t cmd[] = {0x58, 0x00, 0x00, p[3], p[2], p[1], p[0], 0x00, 0x00};
        pTxCharacteristic->writeValue(cmd, sizeof(cmd));
    }


    /// @brief 現在のモータ角度をpos_deg度として設定する
    /// @param pos_deg 
    void presetPosition(float pos_deg = 0.0)
    {
        float pos_rad = pos_deg * DEG2RAD;
        u_int8_t *p = (uint8_t *)&pos_rad;
        uint8_t cmd[] = {0x5A, 0x00, 0x00, p[3], p[2], p[1], p[0], 0x00, 0x00};
        pTxCharacteristic->writeValue(cmd, sizeof(cmd));
    }


    /// @brief モータを正回転させる（止めるまで回転し続ける）
    void runForward()
    {
        uint8_t cmd[] = {0x60, 0x00, 0x00, 0x00, 0x00};
        pTxCharacteristic->writeValue(cmd, sizeof(cmd));
    }    


    /// @brief モータを逆回転させる（止めるまで回転し続ける）
    void runReverse()
    {
        uint8_t cmd[] = {0x61, 0x00, 0x00, 0x00, 0x00};
        pTxCharacteristic->writeValue(cmd, sizeof(cmd));
    }


    /// @brief モータをpos_degで指定した角度に回転させる
    /// @param pos_deg 
    void moveTo(float pos_deg)
    {
        float pos_rad = pos_deg * DEG2RAD;
        u_int8_t *p = (uint8_t *)&pos_rad;
        uint8_t cmd[] = {0x66, 0x00, 0x00, p[3], p[2], p[1], p[0], 0x00, 0x00};
        pTxCharacteristic->writeValue(cmd, sizeof(cmd));
    }


    /// @brief モータをdistance_degだけ回転させる
    /// @param distance_deg 
    void moveBy(float distance_deg)
    {
        float distance_rad = distance_deg * DEG2RAD;
        u_int8_t *p = (uint8_t *)&distance_rad;
        uint8_t cmd[] = {0x68, 0x00, 0x00, p[3], p[2], p[1], p[0], 0x00, 0x00};
        pTxCharacteristic->writeValue(cmd, sizeof(cmd));
    }       


    /// @brief モータの励磁を停止する（トルク0にする）
    void free()
    {
        uint8_t cmd[] = {0x6C, 0x00, 0x00, 0x00, 0x00};
        pTxCharacteristic->writeValue(cmd, sizeof(cmd));
    }


    /// @brief モータを停止させる
    void stop()
    {
        uint8_t cmd[] = {0x6D, 0x00, 0x00, 0x00, 0x00};
        pTxCharacteristic->writeValue(cmd, sizeof(cmd));
    }


    /// @brief モータを停止させる？
    void stopDoingTaskset(){
        uint8_t cmd[] = {0x82, 0x00, 0x00, 0x00, 0x00};
        pTxCharacteristic->writeValue(cmd, sizeof(cmd));
    }


    /// @brief モータの現在の角度を取得する
    /// @return モータの現在の角度[deg]
    float getMotorInfo(){
        std::string retVal = pMeasurementCharacteristic->readValue();
        char* arrs = (char *)retVal.c_str();
        char dst[4] = {arrs[3], arrs[2], arrs[1], arrs[0]};
        float ret = 0;
        memcpy(&ret, dst, sizeof(float));
        return ret * RAD2DEG;
    }
    
};
