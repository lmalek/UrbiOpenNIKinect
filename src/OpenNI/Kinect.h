/* 
 * File:   opennikinect.h
 * Author: lmalek
 *
 * Created on 22 luty 2012, 09:59
 */

#ifndef OPENNIKINECT_H
#define	OPENNIKINECT_H

#include <XnCppWrapper.h>
#include <XnUSB.h>
#include <stdexcept>

namespace OpenNI {
    
    class Kinect {
    public:

        enum {
            DEPTH_WIDTH = 640,
            DEPTH_HEIGHT = 480,
            COLOR_WIDTH = 640,
            COLOR_HEIGHT = 480,
        };

        enum {
            Led_Off = 0x0,
            Led_Green = 0x1,
            Led_Red = 0x2,
            Led_Yellow = 0x3,
            Led_BlinkingYellow = 0x4,
            Led_BlinkingGreen = 0x5,
            Led_AlternateRedYellow = 0x6,
            Led_AlternateRedGreen = 0x7
        };

        enum {
            RGB_mode_bayer,
            RGB_mode_IR
        };

        Kinect();
        Kinect(const Kinect& orig);
        virtual ~Kinect();

        /**
         * Open device.
         * @return true if succeeded, false - overwise
         */
        void Open() throw (std::runtime_error);

        /**
         * Close device.
         */
        void Close();
        
        bool Opened();
        void SetMotorPosition(double pos);
        void SetLedMode(int NewMode);
        bool GetAcceleroData(float *x, float *y, float *z);

        void Run();
        void Stop();

        void SetRGBMode(int newmode);

        unsigned short mDepthBuffer[DEPTH_WIDTH * DEPTH_HEIGHT];
        unsigned char mColorBuffer[COLOR_WIDTH * COLOR_HEIGHT * 4];

        void KinectDisconnected();
        void DepthReceived();
        void ColorReceived();

        void ParseColorBuffer();
        void ParseColorBuffer32();

        void ParseIRBuffer();

        void ParseDepthBuffer();

    private:
        enum { MaxDevs = 16 };
        XN_USB_DEV_HANDLE m_devs[MaxDevs];
        XnUInt32 m_num;
        bool m_isOpen;

        xn::Context g_Context;
        xn::ScriptNode g_scriptNode;
        xn::DepthGenerator g_DepthGenerator;
        xn::UserGenerator g_UserGenerator;
        xn::Player g_Player;
        
        void OpenMotor() throw (std::runtime_error);
        void CloseMotor();
        void Init() throw (std::runtime_error);
        
        std::string sample_xml_path; "SamplesConfig.xml"
    };
};

#endif	/* OPENNIKINECT_H */

