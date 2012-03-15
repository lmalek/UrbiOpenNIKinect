/* 
 * File:   UKinectModule.h
 * Author: lmalek
 *
 * Created on 7 marzec 2012, 15:39
 */

#ifndef UKINECTMODULE_H
#define	UKINECTMODULE_H

#include <urbi/uobject.hh>
#include <string>

class UKinectModule : public urbi::UObject {
public:
    UKinectModule(const std::string&);
    virtual ~UKinectModule();
    int init();
private:
};

#endif	/* UKINECTMODULE_H */

