/* 
 * File:   locks.h
 * Author: oc
 *
 * Created on January 15, 2011, 9:02 AM
 */

#ifndef _LOCKS_H
#define	_LOCKS_H

#include "globals.h"

#define MODULE_FLAG FLAG_DEADLOCK

class Lock {
    public:
        Lock() {
        }
        ~Lock() {
        }
        virtual int getRead() = 0;
        virtual int releaseRead() = 0;
        virtual int getWrite() = 0;
        virtual int releaseWrite() = 0;
};

#ifdef _GNU_SOURCE

class rwLock : public Lock{
    pthread_rwlock_t rwlock;
public:
    rwLock() {
        pthread_rwlock_init(&rwlock,NULL);
    }
    ~rwLock() {
        pthread_rwlock_destroy(&rwlock);
    }
    virtual int getRead() {
         const int error(pthread_rwlock_rdlock(&rwlock));
         DEBUG_VAR(error,"%d");
         return error;
    }
    virtual int releaseRead() {
        const int error(pthread_rwlock_unlock(&rwlock));
        DEBUG_VAR(error,"%d");
        return error;
    }
    virtual int getWrite() {
        const int error(pthread_rwlock_wrlock(&rwlock));
        DEBUG_VAR(error,"%d");
        return error;
    }
    virtual int releaseWrite(){
        const int error(pthread_rwlock_unlock(&rwlock));
        DEBUG_VAR(error,"%d");
        return error;
    }
};

#else /* _GNU_SOURCE */

class mutex : public Lock {
    pthread_mutex_t mutex_;
public:
    mutex() {
        pthread_mutex_init(&mutex_,NULL);
    }
    ~mutex() {
        pthread_mutex_destroy(&mutex_);
    }
    virtual int getRead() {
        const int error(pthread_mutex_lock(&mutex_));
        DEBUG_VAR(error,"%d");
        return error;
    }
    virtual int releaseRead() {
        const int error(pthread_mutex_unlock(&mutex_));
        DEBUG_VAR(error,"%d");
        return error;
    }
    virtual int getWrite() {
        const int error(pthread_mutex_lock(&mutex_));
        DEBUG_VAR(error,"%d");
        return error;
    }
    virtual int releaseWrite(){
        const int error(pthread_mutex_unlock(&mutex_));
        DEBUG_VAR(error,"%d");
        return error;
    }
};

#endif /* _GNU_SOURCE */

class readLockMgr {
    Lock *managedLock;
public:
    readLockMgr(Lock *l)
       :managedLock(l) {
          managedLock->getRead();
    }
    readLockMgr(Lock &l)
       :managedLock(&l) {
          managedLock->getRead();
    }
    ~readLockMgr() {
        managedLock->releaseRead();
    }
};

class writeLockMgr {
    Lock *managedLock;
public:
    writeLockMgr(Lock *l)
       :managedLock(l) {
          managedLock->getWrite();
    }
    writeLockMgr(Lock &l)
       :managedLock(&l) {
          managedLock->getWrite();
    }
    ~writeLockMgr() {
        managedLock->releaseWrite();
    }
};

#undef MODULE_FLAG

#endif	/* _LOCKS_H */

