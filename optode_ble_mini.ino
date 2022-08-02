#include <ArduinoBLE.h>



#define _SP   Serial.print
#define _SPN  Serial.println
#define _dW   digitalWrite



// --------------------------------------
// serial when developing = 1
// serial when installed at optode = 0
#define _SERIAL_ENABLE  0
// --------------------------------------



// must be defined here, outside
BLEService svc("2323");
BLECharacteristic char_out("2324", BLERead | BLENotify, 10);
BLECharacteristic  char_in("2325", BLERead | BLEWrite, 10);



// pins
#define PIN_DISPLAY_OUT   D10
#define PIN_WIFI_OUT      D9
#define PIN_DISPLAY_IN    A0
#define PIN_WIFI_IN       A1



void setup()
{
    #if _SERIAL_ENABLE == 1
        Serial.begin(9600);
        while (!Serial);
    #endif


    // BLE hardware check
    if (!BLE.begin())
    {
        _SPN("starting BLE failed!");
        while (1);
    }


    // BLE services and characteristics
    svc.addCharacteristic(char_out);
    svc.addCharacteristic(char_in);
    BLE.addService(svc);


    // BLE CONN (* 1.25) -> 800 = 1s, 100 = 125 ms
    BLE.setConnectionInterval(100, 100);


    // BLE ADV (* 0.625) -> 320 = 200 ms
    BLE.setLocalName("BLE_optode_1");
    BLE.setAdvertisedService(svc);
    BLE.setAdvertisingInterval(160);
    BLE.advertise();


    // display initial info
    _SP("my xiao BLE MAC address is: ");
    _SPN(BLE.address());


    // pins
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(PIN_DISPLAY_OUT, OUTPUT);
}



void _act_do()
{
    _dW(PIN_DISPLAY_OUT, 1);
    delay(3000);
    _dW(PIN_DISPLAY_OUT, 0);
}



void _act_wo()
{
    _dW(PIN_WIFI_OUT, 1);
    delay(100);
    _dW(PIN_WIFI_OUT, 0);
}



void _act_di()
{
    int a = analogRead(PIN_DISPLAY_IN);
    _SP("adc display value: ");
    _SPN(a);
    if (a > 512) char_out.writeValue("dis_on");
    else char_out.writeValue("dis_off");
}



void _act_wi()
{
    int a = analogRead(PIN_WIFI_IN);
    _SP("adc display value: ");
    _SPN(a);
    if (a > 512) char_out.writeValue("wif_on");
    else char_out.writeValue("wif_off");
}



void _act_led()
{
    char_out.writeValue("led_ok");
  
    for (uint8_t i = 0; i < 10; i++)
    {
        _dW(LED_BUILTIN, HIGH);
        delay(100);
        _dW(LED_BUILTIN, LOW);
        delay(100);
    }
    

    // end up w/ LED on
    _dW(LED_BUILTIN, LOW);
}




void loop()
{
    BLEDevice central = BLE.central();
    if (central)
    {
        _SP("\n\n\nConnected to central: ");
        _SPN(central.address());


        // on BLE connection, LED = LOW = shining
        _dW(LED_BUILTIN, LOW);


        // -----------------
        // command loop
        // -----------------
        
        while (central.connected())
        {
            if (char_in.written())
            {
                // ignore bad commands
                int len = char_in.valueLength();
                if (len > 2) continue;
        
        
                // get command and parse it
                char v[10] = {0};
                strncpy(v, (const char *)char_in.value(), len);
                _SP("-> ");
                _SPN(v);
        
        
                // parse command
                if (!strncmp("DO", v, len) || !strncmp("do", v, len))
                {
                    _act_do();
                }
                if (!strncmp("WO", v, len) || !strncmp("wo", v, len))
                {
                    _act_wo();
                }
                if (!strncmp("DI", v, len) || !strncmp("di", v, len))
                {
                    _act_di();
                }
                if (!strncmp("WI", v, len) || !strncmp("wi", v, len))
                {
                    _act_wi();
                }
                if (!strncmp("L", v, 1))
                {
                    _act_led();
                }
                
            }            
        }


        // BLE disconnection
        _SP("Disconnected from central: ");
        _SPN(central.address());
        _dW(LED_BUILTIN, HIGH);
  }
}
