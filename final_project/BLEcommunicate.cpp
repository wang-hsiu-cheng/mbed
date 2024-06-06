#include "mbed.h"
#include "drivers/DigitalOut.h"
#include "params.h"

#include <events/mbed_events.h>
#include "ble/BLE.h"
#include "gatt_server_process.h"
#include "mbed-trace/mbed_trace.h"

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
        traveledPath = car.PathLength(100);
        ble_error_t err = _qti_hint.get(*_server, second);
        err = _distaince_cm.get(*_server, second);
        err = _distaince_cm.get(*_server, second);

        bag1 = (int)traveledPath % 255;
        bagNumber = (int)traveledPath / 255;
        _qti_hint.set(*_server, bagNumber);
        _qti_pattern.set(*_server, pattern);
        _distaince_cm.set(*_server, bag1);
        _accelerometer_0.set(*_server, deltaVelocity0 * 10);
        _accelerometer_1.set(*_server, deltaVelocity1 * 10);
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