#include "mbed.h"
#include "bbcar.h"
#include "drivers/DigitalOut.h"

#include "BLEcommunicate.cpp"
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
BusInOut qti_pin(D4,D5,D6,D7);
int level = 0;
double traveledPath = 0;

void feedbackWheel()
{
    traveledPath = car.Dest();
    printf("path: %f\n", traveledPath);
}
int LaserPingNavigation()
{
    parallax_laserping ping1(pin8);
    if ((float)ping1 < 20)
    {
        led1 = 1;
        printf("has\n");
        car.stop();
        return 1;
    }
    else
    {
        led1 = 0;
        printf("no\n");
        return 0;
    }
}
void GoCertainDistance(float distance)
{
    car.goCertainDistance(distance); // 6.5*3.14*3

    while (car.checkDistance(0.5))
    {
        // if (LaserPingNavigation())
        //     break;
        ThisThread::sleep_for(500ms);
    }
    car.stop();
    ThisThread::sleep_for(3s);
    printf("error distance = %f\n", (car.servo0.targetAngle - car.servo0.angle) * 6.5 * 3.14 / 360);
}
void QTInavigation()
{
    parallax_qti qti1(qti_pin);
    int pattern = 0b0110;
    int lastPattern = 0b0110;
    bool isTurnEnd = true;
    car.goStraight(50);

    while (level < 10)
    {
        int hint;
        lastPattern = pattern;
        pattern = qti1;
        // printf("%u\n",pattern);
        // std::cout << pattern << std::endl;
        if (pattern != 0b0000)
            isTurnEnd = true;
        switch (pattern) {
            case 0b1000: car.turn(40, 0.05); printf("1000\n"); break;
            case 0b1100: car.turn(55, 0.35); printf("1100\n"); break;
            case 0b0100: car.turn(60, 0.55); printf("0100\n"); break;
            case 0b0110: car.turn(65, 1); printf("0110\n"); break;
            case 0b0010: car.turn(60, -0.55); printf("0010\n"); break;
            case 0b0011: car.turn(55, -0.35); printf("0011\n"); break;
            case 0b0001: car.turn(40, -0.05); printf("0001\n"); break;
            case 0b0111: hint = 0b0111; printf("0111\n"); break;
            case 0b1110: hint = 0b0111; printf("0111\n"); break;
            case 0b1111: 
            {
                if (hint == 0b0111)
                    {car.turn(50, 0.05); printf("0111\n");}
                else if (hint == 0b1110)
                    {car.turn(50, -0.05); printf("1110\n");}
                else 
                    car.stop(); printf("1111\n"); break;
            }
            default: {
                if (lastPattern == 0b1000 || lastPattern == 0b1100 || lastPattern == 0b0100 || lastPattern == 0b1110)
                    car.turnAround(30, false);
                else if (lastPattern == 0b0001 || lastPattern == 0b0011 || lastPattern == 0b0010 || lastPattern == 0b0111)
                    car.turnAround(30, true);
                isTurnEnd = false;
                car.stop();
            }
        }
        if (LaserPingNavigation() || !isTurnEnd)
        {
            car.turnAround(50, 0);
             isTurnEnd = false;
            printf("turn around\n");
        }
        ThisThread::sleep_for(10ms);
    }
    return;
}
void AvoidWall()
{

}

class ClockService : public ble::GattServer::EventHandler {
public:
    ClockService() :
        _hour_char("0000b391-0000-1000-8000-00805f9b34fb", 0),
        _minute_char("0000b392-0000-1000-8000-00805f9b34fb", 0),
        _second_char("0000b393-0000-1000-8000-00805f9b34fb", 0),
        _clock_service(
            /* uuid */ "0000b390-0000-1000-8000-00805f9b34fb",
            /* characteristics */ _clock_characteristics,
            /* numCharacteristics */ sizeof(_clock_characteristics) /
                                     sizeof(_clock_characteristics[0])
        )
    {
        /* update internal pointers (value, descriptors and characteristics array) */
        _clock_characteristics[0] = &_hour_char;
        _clock_characteristics[1] = &_minute_char;
        _clock_characteristics[2] = &_second_char;

        /* setup authorization handlers */
        _hour_char.setWriteAuthorizationCallback(this, &ClockService::authorize_client_write);
        _minute_char.setWriteAuthorizationCallback(this, &ClockService::authorize_client_write);
        _second_char.setWriteAuthorizationCallback(this, &ClockService::authorize_client_write);
    }

    void start(BLE &ble, events::EventQueue &event_queue)
    {
        _server = &ble.gattServer();
        _event_queue = &event_queue;

        printf("Registering demo service\r\n");
        ble_error_t err = _server->addService(_clock_service);

        if (err) {
            printf("Error %u during demo service registration.\r\n", err);
            return;
        }

        /* register handlers */
        _server->setEventHandler(this);

        printf("clock service registered\r\n");
        printf("service handle: %u\r\n", _clock_service.getHandle());
        printf("hour characteristic value handle %u\r\n", _hour_char.getValueHandle());
        printf("minute characteristic value handle %u\r\n", _minute_char.getValueHandle());
        printf("second characteristic value handle %u\r\n", _second_char.getValueHandle());

        // event_queue.call_every(100ms, ClockService::updateDate);
        _event_queue->call_every(100ms, callback(this, &ClockService::updateDate));
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
        printf("connection handle: %u\r\n", params.connHandle);
        printf("attribute handle: %u", params.handle);
        if (params.handle == _hour_char.getValueHandle()) {
            printf(" (hour characteristic)\r\n");
        } else if (params.handle == _minute_char.getValueHandle()) {
            printf(" (minute characteristic)\r\n");
        } else if (params.handle == _second_char.getValueHandle()) {
            printf(" (second characteristic)\r\n");
        } else {
            printf("\r\n");
        }
        printf("write operation: %u\r\n", params.writeOp);
        printf("offset: %u\r\n", params.offset);
        printf("length: %u\r\n", params.len);
        printf("data: ");

        for (size_t i = 0; i < params.len; ++i) {
            printf("%02X", params.data[i]);
        }

        printf("\r\n");
          }

          /**
           * Handler called after an attribute has been read.
     */
    void onDataRead(const GattReadCallbackParams &params) override
    {
        printf("data read:\r\n");
        printf("connection handle: %u\r\n", params.connHandle);
        printf("attribute handle: %u", params.handle);
        if (params.handle == _hour_char.getValueHandle()) {
            printf(" (hour characteristic)\r\n");
        } else if (params.handle == _minute_char.getValueHandle()) {
            printf(" (minute characteristic)\r\n");
        } else if (params.handle == _second_char.getValueHandle()) {
            printf(" (second characteristic)\r\n");
        } else {
            printf("\r\n");
        }
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

        if (e->offset != 0) {
            printf("Error invalid offset\r\n");
            e->authorizationReply = AUTH_CALLBACK_REPLY_ATTERR_INVALID_OFFSET;
            return;
        }

        if (e->len != 1) {
            printf("Error invalid len\r\n");
            e->authorizationReply = AUTH_CALLBACK_REPLY_ATTERR_INVALID_ATT_VAL_LENGTH;
            return;
        }

        if ((e->data[0] >= 60) ||
            ((e->data[0] >= 24) && (e->handle == _hour_char.getValueHandle()))) {
            printf("Error invalid data\r\n");
            e->authorizationReply = AUTH_CALLBACK_REPLY_ATTERR_WRITE_NOT_PERMITTED;
            return;
        }

        e->authorizationReply = AUTH_CALLBACK_REPLY_SUCCESS;
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
        ble_error_t err = _second_char.get(*_server, second);
        err = _hour_char.get(*_server, second);
        err = _hour_char.get(*_server, second);
        if (err) {
            printf("read of the second value returned error %u\r\n", err);
            return;
        }

        // accelerometer.GetAcceleromterCalibratedData(calibratedAccelerationData);

        err = _second_char.set(*_server, traveledPath);
        err = _minute_char.set(*_server, traveledPath);
        err = _hour_char.set(*_server, traveledPath);
        if (err) {
            printf("write of the second value returned error %u\r\n", err);
            return;
        }

    }


private:
template<typename T>
    class ReadWriteNotifyIndicateCharacteristic : public GattCharacteristic {
    public:
        /**
         * Construct a characteristic that can be read or written and emit
         * notification or indication.
         *
         * @param[in] uuid The UUID of the characteristic.
         * @param[in] initial_value Initial value contained by the characteristic.
         */
        ReadWriteNotifyIndicateCharacteristic(const UUID & uuid, const T& initial_value) :
            GattCharacteristic(
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
                /* variable len */ false
            ),
            _value(initial_value) {
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
        ble_error_t get(GattServer &server, T& dst) const
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

    GattService _clock_service;
    GattCharacteristic* _clock_characteristics[3];

    ReadWriteNotifyIndicateCharacteristic<uint8_t> _hour_char;
    ReadWriteNotifyIndicateCharacteristic<uint8_t> _minute_char;
    ReadWriteNotifyIndicateCharacteristic<uint8_t> _second_char;
};

void BLEsend()
{
    mbed_trace_init();

    BLE &ble = BLE::Instance();
    events::EventQueue event_queue;
    ClockService demo_service;

    /* this process will handle basic ble setup and advertising for us */
    GattServerProcess ble_process(event_queue, ble);

    /* once it's done it will let us continue with our demo */
    ble_process.on_init(callback(&demo_service, &ClockService::start));

    ble_process.start();
}
void Car()
{
    // go certain distance
    // GoCertainDistance(10);

    // QTI Line Following Kit
    QTInavigation();

    // avoid wall
    AvoidWall();
}
int main()
{
    // thread.start(BLEsend);
    thread1.start(Car);
   // simple test
//    car.goStraight(200);
//    ThisThread::sleep_for(5s);
//    car.stop();
//    ThisThread::sleep_for(5s);

    car.initPathDist();
    t.start(callback(&servo_queue, &EventQueue::dispatch_forever));
    servo_queue.call_every(1s, feedbackWheel);
}