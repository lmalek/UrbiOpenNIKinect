/* 
 * File:   OpenNiKinectSingelton.h
 * Author: lmalek
 *
 * Created on 7 marzec 2012, 15:38
 */

#ifndef OPENNIKINECTSINGELTON_H
#define	OPENNIKINECTSINGELTON_H

#include <boost/noncopyable.hpp>

#include "UKinectModule.h"
#include <XnCppWrapper.h>

class OpenNIKinectSingelton : public boost::noncopyable {
public:
    static OpenNIKinectSingelton& getInstance();
    xn::Context& getContext();
    
    ~OpenNIKinectSingelton();

private:
    OpenNIKinectSingelton();
    
    xn::Context context;
};

#endif	/* OPENNIKINECTSINGELTON_H */

