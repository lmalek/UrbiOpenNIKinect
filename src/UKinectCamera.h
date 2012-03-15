/* 
 * File:   uopennikinect.h
 * Author: lmalek
 *
 * Created on 22 luty 2012, 09:55
 */

#ifndef UOPENNIKINECT_H
#define	UOPENNIKINECT_H


#include <urbi/uobject.hh>
#include "UKinectModule.h"

class UKinectCamera : public UKinectModule {
public:
    // Urbi constructor. Throw error in case of error.
    UKinectCamera(const std::string& name);
    virtual ~UKinectCamera();

    virtual int update();
    
    void init(int);
private:    
    // Our image variable and dimensions
    urbi::UVar image;
    urbi::UVar width;
    urbi::UVar height;
    urbi::UVar fps;
    urbi::UVar notify;
    urbi::UVar flip;

    bool mGetNewFrame; //
    unsigned int mFrame; // ID of already grabed frame
    unsigned int mAccessFrame; // ID of already retrieved frame

    // Called on access.
    void getImage();

    //
    void changeNotifyImage(urbi::UVar&);

    // Mutex and conditional variable to synchronize threads
    boost::mutex getValMutex;

    // Storage for last captured image.
    urbi::UBinary mBinImage;

    void fpsChanged();

    xn::ImageGenerator imageGenerator;
    xn::ImageMetaData imageMD;
    unsigned char* dataRGBPtr;
};




#endif	/* UOPENNIKINECT_H */

