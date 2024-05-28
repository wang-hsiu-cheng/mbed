#include "mbed.h"
#include "drivers/DigitalOut.h"

#include <events/mbed_events.h>
#include "ble/BLE.h"
#include "gatt_server_process.h"
#include "mbed-trace/mbed_trace.h"

// mbed::DigitalOut led1(LED1, 1);
// mbed::DigitalOut led2(LED2, 1);
// mbed::DigitalOut led3(LED3, 1);
// mbed::DigitalOut *leds[] = {&led1, &led2, &led3};

class GattServerDemo : ble::GattServer::EventHandler
{

    const static uint16_t EXAMPLE_SERVICE_UUID = 0xA000;
    const static uint16_t WRITABLE_CHARACTERISTIC_UUID = 0xA001;

public:
    GattServerDemo()
    {
        const UUID uuid = WRITABLE_CHARACTERISTIC_UUID;
        _writable_characteristic = new ReadWriteGattCharacteristic<uint8_t>(uuid, &_characteristic_value);

        if (!_writable_characteristic)
        {
            printf("Allocation of ReadWriteGattCharacteristic failed\r\n");
        }
    }

    ~GattServerDemo()
    {
    }

    void start(BLE &ble, events::EventQueue &event_queue)
    {
        const UUID uuid = EXAMPLE_SERVICE_UUID;
        GattCharacteristic *charTable[] = {_writable_characteristic};
        GattService example_service(uuid, charTable, 1);

        ble.gattServer().addService(example_service);

        ble.gattServer().setEventHandler(this);

        printf("Example service added with UUID 0xA000\r\n");
        printf("Connect and write to characteristic 0xA001\r\n");
    }

private:
    /**
     * This callback allows the LEDService to receive updates to the ledState Characteristic.
     *
     * @param[in] params Information about the characteristic being updated.
     */
    virtual void onDataWritten(const GattWriteCallbackParams &params)
    {
        if ((params.handle == _writable_characteristic->getValueHandle()) && (params.len == 1))
        {
            printf("New characteristic value written: %x\r\n", *(params.data));
            char command = *(params.data);
            // switch (command)
            // {
            // case 0x69: // i
            //     printf("Initialization received\r");
            //     break;
            // case 0x66: // f
            //     goStraight(1, 100);
            //     break;
            // case 0x6C: // l
            //     turn(1, 100, 0.3);
            //     break;
            // case 0x72: // r
            //     turn(1, 100, -0.3);
            //     break;
            // case 0x62: // b
            //     goStraight(1, -100);
            //     break;
            // case 0x73: // s
            //     stop(1);
            //     break;
            // default:
            //     break;
            // }
        }
    }

private:
    ReadWriteGattCharacteristic<uint8_t> *_writable_characteristic = nullptr;
    uint8_t _characteristic_value = 0;
};