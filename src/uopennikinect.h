/* 
 * File:   uopennikinect.h
 * Author: lmalek
 *
 * Created on 22 luty 2012, 09:55
 */

#ifndef UOPENNIKINECT_H
#define	UOPENNIKINECT_H

#include <urbi/uobject.hh>
#include <urbi/ubinary.hh>
#include <urbi/uimage.hh>

#define DEPTH_SRC "DepthSource"
#define COLOR_SRC "ColorSource"

using namespace urbi;
using namespace std;

class UKinect: public urbi::UObject
{
public:
	UKinect(const std::string& s);
	~UKinect();

	// constructor
	int init(); 

	bool isReady, isActive;
	float mMotorPosition;
	int mLedMode, mRGBMode;
	unsigned long oldDepthFrame, oldColorFrame;

	// is device ready
	urbi::UVar	deviceReady;
	// device ID and count
	urbi::UVar deviceID;
	urbi::UVar deviceCount;
	void setDeviceID(int id);

	// kinect depth and color image
	urbi::UBinary mDepthImage;
	urbi::UBinary mColorImage;
	// captured image
	urbi::UBinary mCaptureImage;

	// RGB or IR mode
	urbi::UVar rgbmode;
	void setRGBMode(int rgb);
	void getRGBMode();

	// accelerometer urbi var
	urbi::UVar kX, kY, kZ;

	// urbi side depthImage, colorImage and captured
	urbi::UVar val, colordata, depthdata, capturedata;
	// std width and height
	urbi::UVar width, height;

	// init kinect device
	urbi::UReturn open();
	// close session
	void close();

	// Kinect Motor
	urbi::UVar motor;
	void setMotor(double pos);
	void getMotor();

	// Kinect led Mode
	urbi::UVar led;
	void setLed(int col);
	void getLed();

	// Called periodically to get image data
	virtual int update (); 	

	// Kinect instance
	OpenNI::Kinect	*mKin;
	// Kinect Listener
        
	// device found?
	void getDevState(urbi::UVar& v);
	// get Accel value
	void getAccel(urbi::UVar& v);
	void GetAccelerometer();

	// get val (return depth image)
	void getVal (urbi::UVar& v);
	// get image data
	void getColorImage (urbi::UVar& v);
	void getDepthImage (urbi::UVar& v);
	void getCaptureImage(urbi::UVar& v);

	// opencv bonus
	// generic create/show/hide window
	void create(const std::string& wName, int x, int y);
	void show(urbi::UBinary uimg, const std::string& wName);
	void hide(const std::string& wName);
	// capture start/stop
	void stopcapture();
	void startcapture(int id);
	void processCam ();

	// cv windows state
	bool isDepthShowed, isColorShowed, isCapture;
	
	// show/hide depth and color windows
	void hidecolor();
	void showcolor();
	void hidedepth();
	void showdepth();
	void showall();
	void hideall();
};


#endif	/* UOPENNIKINECT_H */

