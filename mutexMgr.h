/* 
 * File:   mutexMgr.h
 * Author: oc
 *
 * Created on January 23, 2011, 11:23 AM
 */

#ifndef _MUTEX_MGR_H
#define	_MUTEX_MGR_H

#include <pthread.h>
#include "debug.h"

#define MODULE_FLAG FLAG_DEADLOCK

class MutexMgr {
    MutexMgr(const MutexMgr& orig) {
        //copy not allowed
    }
    pthread_mutex_t *mutex;
public:
    MutexMgr(pthread_mutex_t &m)
    :mutex(&m){
        const int error = pthread_mutex_lock(mutex);
        if (error != 0) {
            ERROR_MSG("pthread_mutex_lock error %d",error);
        }
    }
    virtual ~MutexMgr() {
        const int error = pthread_mutex_unlock(mutex);
        if (error != 0) {
            ERROR_MSG("pthread_mutex_unlock error %d",error);
        }
    }
};
#undef MODULE_FLAG
#endif	/* _MUTEX_MGR_H */

