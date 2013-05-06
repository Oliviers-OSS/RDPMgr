/* 
 * File:   message.h
 * Author: oc
 *
 * Created on January 15, 2011, 2:00 PM
 */

#ifndef _MESSAGE_H
#define	_MESSAGE_H

#define DISPLAY_MSG         1
#define SIGCHILD_MSG        2
#define CHILD_EXEC_ERROR    3
#define STOP_MSG            4
#define ICONIZE_MSG         5
#define MAXIMIZE_MSG        6

#define MODULE_FLAG FLAG_RUNTIME

class Message {
    unsigned int content;

#ifdef _DEBUG_
    #define DECLARE_MSG(x) case x: DEBUG_MSG("Message: " #x); break;
    void dump() {
        switch(content) {
            DECLARE_MSG(DISPLAY_MSG);
            DECLARE_MSG(SIGCHILD_MSG);
            DECLARE_MSG(CHILD_EXEC_ERROR);
			DECLARE_MSG(STOP_MSG);
			DECLARE_MSG(ICONIZE_MSG);
			DECLARE_MSG(MAXIMIZE_MSG);
            default:
                NOTICE_MSG("bad message value %d",content);
        }
    }
    #undef DECLARE_MSG   
#endif /* _DEBUG_ */

public:

    Message(unsigned int value = 0)
        :content(value){
    }

    const char *getBuffer() {
        return (const char *)&content;
    }

    unsigned int getContent() {
        DUMP();
        return content;
    }

    void setContent(const char *buffer) {
        memcpy(&content,buffer,sizeof(content));
		DUMP();
    }
	
	void setContent(const unsigned int buffer) {
        content = buffer;
		DUMP();
    }
};

#undef MODULE_FLAG

#endif	/* _MESSAGE_H */

