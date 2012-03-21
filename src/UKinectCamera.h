/* 
 * File:   uopennikinect.h
 * Author: lmalek
 *
 * Created on 22 luty 2012, 09:55
 */

#ifndef UOPENNIKINECT_H
#define	UOPENNIKINECT_H

#include "UKinectModule.h"

class UKinectCamera : public UKinectModule {
public:
    // Urbi constructor. Throw error in case of error.
    UKinectCamera(const std::string& name);
    virtual ~UKinectCamera();

    virtual int update();

    void init();
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
    /* 
     * mode = 0 - slave mode in multi generator work
     * mode = 1 - master mode in multi generator work
     * mode = 2 - single mode
     */
    void getData(int mode=0);

    //
    void changeNotifyImage(urbi::UVar&);

    // Mutex and conditional variable to synchronize threads
    boost::mutex getValMutex;

    // Storage for last captured image.
    urbi::UBinary mBinImage;

    void fpsChanged();

    xn::ImageGenerator imageGenerator;
    xn::ImageMetaData imageMD;
};




#endif	/* UOPENNIKINECT_H */

