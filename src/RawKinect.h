// Forked from: https://groups.google.com/d/msg/openni-dev/T_CeVW_d8ig/dsBKONIpNyQJ

#include <XnUSB.h>
#include <cstdio>

/**
 * Class to control Kinect's motor.
 */
class RawKinect {
public:

    enum {
        MaxDevs = 16
    };

    enum LedColor{
        LED_OFF = 0,
        LED_GREEN = 1,
        LED_RED = 2,
        LED_YELLOW = 3, //(actually orange)
        LED_BLINK_YELLOW = 4, //(actually orange)
        LED_BLINK_GREEN = 5,
        LED_BLINK_RED_YELLOW = 6 //(actually red/orange) 
    };

public:
    RawKinect();
    virtual ~RawKinect();

    /**
     * Open device.
     * @return true if succeeded, false - overwise
     */
    bool open();

    /**
     * Close device.
     */
    void close();

    /**
     * Move motor up or down to specified angle value.
     * @param angle angle value
     * @return true if succeeded, false - overwise
     */
    bool moveMotor(int angle);

    /**
     * Control kinect led 
     * @param color led control value
     */
    bool setLed(LedColor color);
    
    bool getAccelerometer(float &x, float &y, float &z);

private:
    XN_USB_DEV_HANDLE m_devs[MaxDevs];
    XnUInt32 m_num;
    bool m_isOpen;
};

