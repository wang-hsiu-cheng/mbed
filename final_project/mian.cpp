#include "mbed.h"
#include "bbcar.h"
#include "drivers/DigitalOut.h"
#include "params.h"

#include <cstdint>
#include <stdint.h>
#include <events/mbed_events.h>
#include "events/EventQueue.h"
#include "ble/BLE.h"
#include "gatt_server_process.h"
#include "mbed-trace/mbed_trace.h"
#include "platform/Callback.h"

// #include "accelerometer.h"

using mbed::callback;
using namespace std::literals::chrono_literals;

Ticker servo_ticker;
Ticker servo_feedback_ticker;
Thread thread;
Thread thread1;
EventQueue servo_queue;
Thread t;

DigitalOut led1(LED1);
PwmIn servo0_f(D13), servo1_f(D10);
PwmOut servo0_c(D11), servo1_c(D12);
BBCar car(servo0_c, servo0_f, servo1_c, servo1_f, servo_ticker, servo_feedback_ticker);
InterruptIn button(BUTTON1);
DigitalInOut pin8(D8);
BusInOut qti_pin(D4, D5, D6, D7);
// int pattern = 0b0110;
// int hint = 0;
// double traveledPath = 0;

void FeedbackWheel()
{
    traveledPath = car.PathLength(100);
}
int LaserPingNavigation()
{
    parallax_laserping ping1(pin8);
    if ((float)ping1 < 20)
    {
        led1 = 1;
        return 1;
    }
    else
    {
        led1 = 0;
        return 0;
    }
}
void GoCertainDistance(int distance)
{
    car.goCertainDistance(distance); // 6.5*3.14*3

    while (car.checkDistance(1))
    {
        ThisThread::sleep_for(500ms);
    }
    car.stop();
    ThisThread::sleep_for(3s);
}
void Parking()
{
    printf("start parking\n");
    car.turnAround(50, true);
    ThisThread::sleep_for(2800ms);
    car.goStraight(-50);
    ThisThread::sleep_for(1500ms);
    car.stop();
    ThisThread::sleep_for(3s);
    car.goStraight(50);
    ThisThread::sleep_for(500ms);
    printf("stop parking\n");
    return;
}
void QTInavigation()
{
    parallax_qti qti1(qti_pin);
    int lastPattern = 0b0110;
    int i = 0, j = 0;
    bool isTurning = false;
    bool canPark = true;
    car.goStraight(50);

    while (1)
    {
        lastPattern = pattern;
        pattern = qti1;
        // printf("%u\n",pattern);
        if (pattern != 0b0000)
            isTurning = false;
        switch (pattern)
        {
        case 0b1000:
            car.turn(50, 0.1);
            printf("1000\n");
            break;
        case 0b1100:
            car.turn(55, 0.3);
            printf("1100\n");
            break;
        case 0b0100:
            car.turn(60, 0.55);
            printf("0100\n");
            break;
        case 0b0110:
            car.goStraight(65);
            printf("0110\n");
            break;
        case 0b0010:
            car.turn(60, -0.55);
            printf("0010\n");
            break;
        case 0b0011:
            car.turn(55, -0.3);
            printf("0011\n");
            break;
        case 0b0001:
            car.turn(50, -0.1);
            printf("0001\n");
            break;
        case 0b0111:
            i++;
            if (i > 20)
            {
                i = 0;
                hint = 1;
            }
            printf("0111\n");
            break;
        case 0b1110:
            j++;
            if (j > 20)
            {
                j = 0;
                hint = 2;
            }
            printf("1110\n");
            break;
        case 0b1001:
            car.goStraight(50);
            if (canPark)
                hint = 4;
            else
                hint = 3;
            printf("1001\n");
            break;
        case 0b1111:
        {
            switch (hint)
            {
            case 1:
                printf("hint: 1\n");
                car.turn(60, -0.05);
                ThisThread::sleep_for(800ms); // wait until turn right finish
                break;
            case 2:
                printf("hint: 2\n");
                car.turn(60, 0.05);
                ThisThread::sleep_for(800ms); // wait until turn left finish
                break;
            case 3:
                printf("hint: 3\n");
                car.stop();
                canPark = true;
                break;
            case 4:
                printf("hint: 4\n");
                Parking();
                canPark = false;
                hint = 0;
                break;
            default:
                printf("hint: 0\n");
                car.goStraight(50);
                break;
            }
            printf("1111\n");
            break;
        }
        default:
            // if (lastPattern == 0b1000 || lastPattern == 0b1100 || lastPattern == 0b0100 || lastPattern == 0b1110)
            //     car.turnAround(30, true);
            // else if (lastPattern == 0b0001 || lastPattern == 0b0011 || lastPattern == 0b0010 || lastPattern == 0b0111)
            //     car.turnAround(30, false);
            // isTurning = true;

            hint = 0;
            break;
        }
        if (LaserPingNavigation()) // see the obstacle and turn around
        {
            car.turnAround(50, true);
            isTurning = true;
        }
        ThisThread::sleep_for(10ms);
    }
    return;
}

class CarReturnService : public ble::GattServer::EventHandler
{
public:
    CarReturnService() : _distaince_cm("0000b391-0000-1000-8000-00805f9b34fb", 0),
                         _qti_pattern("0000b392-0000-1000-8000-00805f9b34fb", 0),
                         _qti_hint("0000b393-0000-1000-8000-00805f9b34fb", 0),
                         _accelerometer_0("0000b394-0000-1000-8000-00805f9b34fb", 0),
                         _accelerometer_1("0000b395-0000-1000-8000-00805f9b34fb", 0),
                         _accelerometer_2("0000b396-0000-1000-8000-00805f9b34fb", 0),
                         _car_return_service(
                             /* uuid */ "0000b390-0000-1000-8000-00805f9b34fb",
                             /* characteristics */ _carReturn_characteristics,
                             /* numCharacteristics */ sizeof(_carReturn_characteristics) / sizeof(_carReturn_characteristics[0]))
    {
        /* update internal pointers (value, descriptors and characteristics array) */
        _carReturn_characteristics[0] = &_distaince_cm;
        _carReturn_characteristics[1] = &_qti_pattern;
        _carReturn_characteristics[2] = &_qti_hint;
        _carReturn_characteristics[3] = &_accelerometer_0;
        _carReturn_characteristics[4] = &_accelerometer_1;
        _carReturn_characteristics[5] = &_accelerometer_2;

        /* setup authorization handlers */
        _distaince_cm.setWriteAuthorizationCallback(this, &CarReturnService::authorize_client_write);
        _qti_pattern.setWriteAuthorizationCallback(this, &CarReturnService::authorize_client_write);
        _qti_hint.setWriteAuthorizationCallback(this, &CarReturnService::authorize_client_write);
        _accelerometer_0.setWriteAuthorizationCallback(this, &CarReturnService::authorize_client_write);
        _accelerometer_1.setWriteAuthorizationCallback(this, &CarReturnService::authorize_client_write);
        _accelerometer_2.setWriteAuthorizationCallback(this, &CarReturnService::authorize_client_write);
    }

    void start(BLE &ble, events::EventQueue &event_queue)
    {
        _server = &ble.gattServer();
        _event_queue = &event_queue;

        printf("Registering demo service\r\n");
        ble_error_t err = _server->addService(_car_return_service);

        if (err)
        {
            printf("Error %u during demo service registration.\r\n", err);
            return;
        }

        /* register handlers */
        _server->setEventHandler(this);

        printf("car return service registered\r\n");
        printf("service handle: %u\r\n", _car_return_service.getHandle());
        printf("distance handle %u\r\n", _distaince_cm.getValueHandle());
        printf("qti pattern handle %u\r\n", _qti_pattern.getValueHandle());
        printf("qti hint handle %u\r\n", _qti_hint.getValueHandle());

        // event_queue.call_every(100ms, CarReturnService::updateDate);
        _event_queue->call_every(100ms, callback(this, &CarReturnService::updateDate));
    }

    /* GattServer::EventHandler */
private:
    /**
     * Handler called when a notification or an indication has been sent.
     */
    void onDataSent(const GattDataSentCallbackParams &params) override
    {
        printf("sent updates\r\n");
    }

    /**
     * Handler called after an attribute has been written.
     */
    void onDataWritten(const GattWriteCallbackParams &params) override
    {
        printf("data written:\r\n");
    }

    /**
     * Handler called after an attribute has been read.
     */
    void onDataRead(const GattReadCallbackParams &params) override
    {
        printf("data read:\r\n");
        return;
    }

    /**
     * Handler called after a client has subscribed to notification or indication.
     *
     * @param handle Handle of the characteristic value affected by the change.
     */
    void onUpdatesEnabled(const GattUpdatesEnabledCallbackParams &params) override
    {
        printf("update enabled on handle %d\r\n", params.attHandle);
    }

    /**
     * Handler called after a client has cancelled his subscription from
     * notification or indication.
     *
     * @param handle Handle of the characteristic value affected by the change.
     */
    void onUpdatesDisabled(const GattUpdatesDisabledCallbackParams &params) override
    {
        printf("update disabled on handle %d\r\n", params.attHandle);
    }

    /**
     * Handler called when an indication confirmation has been received.
     *
     * @param handle Handle of the characteristic value that has emitted the
     * indication.
     */
    void onConfirmationReceived(const GattConfirmationReceivedCallbackParams &params) override
    {
        printf("confirmation received on handle %d\r\n", params.attHandle);
    }

private:
    /**
     * Handler called when a write request is received.
     *
     * This handler verify that the value submitted by the client is valid before
     * authorizing the operation.
     */
    void authorize_client_write(GattWriteAuthCallbackParams *e)
    {
        printf("characteristic %u write authorization\r\n", e->handle);

        // if (e->offset != 0)
        // {
        //     printf("Error invalid offset\r\n");
        //     e->authorizationReply = AUTH_CALLBACK_REPLY_ATTERR_INVALID_OFFSET;
        //     return;
        // }

        // if (e->len != 1)
        // {
        //     printf("Error invalid len\r\n");
        //     e->authorizationReply = AUTH_CALLBACK_REPLY_ATTERR_INVALID_ATT_VAL_LENGTH;
        //     return;
        // }

        // if ((e->data[0] >= 60) ||
        //     ((e->data[0] >= 24) && (e->handle == _distaince_cm.getValueHandle())))
        // {
        //     printf("Error invalid data\r\n");
        //     e->authorizationReply = AUTH_CALLBACK_REPLY_ATTERR_WRITE_NOT_PERMITTED;
        //     return;
        // }

        // e->authorizationReply = AUTH_CALLBACK_REPLY_SUCCESS;
    }

    /**
     * Increment the second counter.
     */
    void updateDate(void)
    {
        // double traveledPath = UpdateDate();
        // Accelerometer accelerometer;
        // double rawAccelerationData[3];
        // double calibratedAccelerationData[3];
        uint8_t second = 0;
        uint8_t bag1 = 0;
        uint8_t bagNumber = 0;
        // traveledPath = car.PathLength(100);
        ble_error_t err = _qti_hint.get(*_server, second);
        err = _distaince_cm.get(*_server, second);
        err = _distaince_cm.get(*_server, second);
        if (err)
        {
            printf("read of the second value returned error %u\r\n", err);
            return;
        }

        // accelerometer.GetAcceleromterCalibratedData(calibratedAccelerationData);
        bag1 = (int)traveledPath % 255;
        bagNumber = (int)traveledPath / 255;
        err = _qti_hint.set(*_server, bagNumber);
        err = _qti_pattern.set(*_server, pattern);
        err = _distaince_cm.set(*_server, bag1);
        // if (err)
        // {
        //     printf("write of the second value returned error %u\r\n", err);
        //     return;
        // }
    }

private:
    template <typename T>
    class ReadWriteNotifyIndicateCharacteristic : public GattCharacteristic
    {
    public:
        /**
         * Construct a characteristic that can be read or written and emit
         * notification or indication.
         *
         * @param[in] uuid The UUID of the characteristic.
         * @param[in] initial_value Initial value contained by the characteristic.
         */
        ReadWriteNotifyIndicateCharacteristic(const UUID &uuid, const T &initial_value) : GattCharacteristic(
                                                                                              /* UUID */ uuid,
                                                                                              /* Initial value */ &_value,
                                                                                              /* Value size */ sizeof(_value),
                                                                                              /* Value capacity */ sizeof(_value),
                                                                                              /* Properties */ GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_READ |
                                                                                                  GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_WRITE |
                                                                                                  GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY |
                                                                                                  GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_INDICATE,
                                                                                              /* Descriptors */ nullptr,
                                                                                              /* Num descriptors */ 0,
                                                                                              /* variable len */ false),
                                                                                          _value(initial_value)
        {
        }

        /**
         * Get the value of this characteristic.
         *
         * @param[in] server GattServer instance that contain the characteristic
         * value.
         * @param[in] dst Variable that will receive the characteristic value.
         *
         * @return BLE_ERROR_NONE in case of success or an appropriate error code.
         */
        ble_error_t get(GattServer &server, T &dst) const
        {
            uint16_t value_length = sizeof(dst);
            return server.read(getValueHandle(), &dst, &value_length);
        }

        /**
         * Assign a new value to this characteristic.
         *
         * @param[in] server GattServer instance that will receive the new value.
         * @param[in] value The new value to set.
         * @param[in] local_only Flag that determine if the change should be kept
         * locally or forwarded to subscribed clients.
         */
        ble_error_t set(GattServer &server, const uint8_t &value, bool local_only = false) const
        {
            return server.write(getValueHandle(), &value, sizeof(value), local_only);
        }

    private:
        uint8_t _value;
    };

private:
    GattServer *_server = nullptr;
    events::EventQueue *_event_queue = nullptr;

    GattService _car_return_service;
    GattCharacteristic *_carReturn_characteristics[6];

    ReadWriteNotifyIndicateCharacteristic<uint8_t> _distaince_cm;
    ReadWriteNotifyIndicateCharacteristic<uint8_t> _qti_pattern;
    ReadWriteNotifyIndicateCharacteristic<uint8_t> _qti_hint;
    ReadWriteNotifyIndicateCharacteristic<uint8_t> _accelerometer_0;
    ReadWriteNotifyIndicateCharacteristic<uint8_t> _accelerometer_1;
    ReadWriteNotifyIndicateCharacteristic<uint8_t> _accelerometer_2;
};

void BLEsend()
{
    mbed_trace_init();

    BLE &ble = BLE::Instance();
    events::EventQueue event_queue;
    CarReturnService demo_service;

    /* this process will handle basic ble setup and advertising for us */
    GattServerProcess ble_process(event_queue, ble);

    /* once it's done it will let us continue with our demo */
    ble_process.on_init(callback(&demo_service, &CarReturnService::start));

    ble_process.start();
}
void Car()
{
    // go certain distance
    // GoCertainDistance(10);

    // QTI Line Following Kit
    QTInavigation();
}
int main()
{
    // simple test
    //    car.goStraight(200);
    //    ThisThread::sleep_for(5s);
    //    car.stop();
    //    ThisThread::sleep_for(5s);

    thread1.start(Car);
    mbed_trace_init();
    t.start(callback(&servo_queue, &EventQueue::dispatch_forever));
    servo_queue.call_every(100ms, FeedbackWheel);

    BLE &ble = BLE::Instance();
    events::EventQueue event_queue;
    CarReturnService demo_service;

    /* this process will handle basic ble setup and advertising for us */
    GattServerProcess ble_process(event_queue, ble);

    /* once it's done it will let us continue with our demo */
    ble_process.on_init(callback(&demo_service, &CarReturnService::start));

    car.initPathDist();
    ble_process.start();
}