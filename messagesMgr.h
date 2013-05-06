/* 
 * File:   messagesMgr.h
 * Author: oc
 *
 * Created on January 15, 2011, 2:20 PM
 */

#ifndef _MESSAGESMGR_H
#define	_MESSAGESMGR_H

#include "globals.h"
#include "messagesQueue.h"
#include "message.h"

#define MODULE_FLAG FLAG_RUNTIME
#define  MESSAGE_QUEUE_NAME "/"PACKAGE

class MessagesMgr {
protected:
    messageQueueId mq;
public:

    MessagesMgr()
    : mq(-1) {        
    }

    MessagesMgr(const MessagesMgr &orig) {

    }

    virtual ~MessagesMgr() {
        if (mq != -1) {
            closeMessagesQueue(mq);
        }
    }

    virtual int get(Message &msg) = 0;
    virtual int send(Message &msg) = 0;
};

class ReadMessagesMgr : public MessagesMgr {
public:
    ReadMessagesMgr() {
        int error(EXIT_SUCCESS);
        error = createMessagesQueue(MESSAGE_QUEUE_NAME, O_RDONLY, &mq);
    }
    ReadMessagesMgr(ReadMessagesMgr &orig) {

    }
    ~ReadMessagesMgr(){
        
    }
    virtual int get(Message &msg){
        char message[8192]; /* WARNING: this value can't be changed else the read will fail */
        size_t messageLength = sizeof (message);
        unsigned int messagePriority = 0;
        int error = readFromMessagesQueue(mq, message, &messageLength, &messagePriority);
        if (EXIT_SUCCESS == error) {
            if (messageLength == sizeof(Message)){
                msg.setContent(message);
            } else {
                error = EPROTO;
                ERROR_MSG("bad message received (size = %d)",messageLength);
            }
        } /* error already logged */
        return error;
    }
    virtual int send(Message &msg){
        return ENOSYS;
    }
};

class WriteMessagesMgr : public MessagesMgr {
public:
    WriteMessagesMgr() {
        int error(EXIT_SUCCESS);
        error = createMessagesQueue(MESSAGE_QUEUE_NAME, O_WRONLY, &mq);
    }
    WriteMessagesMgr(WriteMessagesMgr &orig) {

    }
    ~WriteMessagesMgr() {
        
    }
    virtual int get(Message &msg){
        return ENOSYS;
    }
    virtual int send(Message &msg){
        int error = writeToMessagesQueue(mq,msg.getBuffer(), sizeof (Message), 0);
        return error; /* error already logged */
    }
};

#undef MODULE_FLAG

#endif	/* _MESSAGESMGR_H */

