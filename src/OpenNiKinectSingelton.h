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


class OpenNIKinectSingelton : public boost::noncopyable {
public:
    void registerModule(UKinectModule* uobject);
    void unregisterModule(UKinectModule* uobject);

private:
    OpenNIKinectSingelton();
    ~OpenNIKinectSingelton();
};

#endif	/* OPENNIKINECTSINGELTON_H */

