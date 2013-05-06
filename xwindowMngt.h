/* 
 * File:   xwindowMgnt.h
 * Author: oc
 *
 * Created on April 3, 2011, 1:52 AM
 */

#ifndef _XWINDOWMGR_H
#define	_XWINDOWMGR_H

#include <unistd.h>
#include <cstdlib>
#include <errno.h>
#include <string>
#include <X11/Xlib.h>

#define MODULE_FLAG FLAG_XWINDOWS

// Predicats functors

class XWindowPredicat {
public:
    XWindowPredicat(){
    }

    virtual ~XWindowPredicat(){
    }
    
    virtual bool operator() (Display *dpy, Window window) = 0;
};

class XWindowNamed : public XWindowPredicat {
    std::string windowsName;
public:
    
    XWindowNamed(const char *name) :windowsName(name){
    }

    XWindowNamed(const std::string &name) :windowsName(name){
    }

    virtual ~XWindowNamed() {

    }

    virtual bool operator() (Display *dpy, Window window);
};

class XWindowWhichNameContains : public XWindowPredicat {
    std::string windowsSubStringName;
public:

    XWindowWhichNameContains(const char *name) :windowsSubStringName(name){
    }

    XWindowWhichNameContains(const std::string &name) :windowsSubStringName(name){
    }

    virtual ~XWindowWhichNameContains() {

    }

    virtual bool operator() (Display *dpy, Window window);
};

class XWindowOfClass : public XWindowPredicat {
    std::string xWinclass;
public:

    XWindowOfClass(const char *name) :xWinclass(name){
    }

    XWindowOfClass(const std::string &name) :xWinclass(name){
    }

    virtual ~XWindowOfClass() {

    }

    virtual bool operator() (Display *dpy, Window window);
};

class WindowOf : public XWindowPredicat {
    pid_t pid;
    std::string hostname;
public:

    WindowOf(const pid_t processID, const char *host = NULL) :pid(processID) {
        if (NULL == host) {
            hostname.reserve(PATH_MAX);                        
            if (gethostname(&hostname[0],hostname.capacity()) != 0) {
                const int error = errno;
                ERROR_MSG("gethostname error %d (%m)",error);
            }
        } else {
            hostname = host;
        }
    }

    virtual ~WindowOf(){
    }

    virtual bool operator() (Display *dpy, Window window);
};

// Actions functors

typedef enum XWindowState_ {
   Normal
  ,Iconic
  ,Unknown
} XWindowState;

class XWindowAction {
public:
    XWindowAction(){
    }

    virtual ~XWindowAction(){
    }
    virtual int operator() (Display *dpy, Window window,int screen_number) = 0;
	virtual int operator() (Display *dpy, Window window,int screen_number,XWindowState &state) = 0;
};

class SwapState : public XWindowAction {
public:
    SwapState(){
    }
    virtual ~SwapState(){
    }
    virtual int operator() (Display *dpy, Window window,int screen_number);
	virtual int operator() (Display *dpy, Window window,int screen_number,XWindowState &state);
};

class Iconify : public XWindowAction {
public:
    Iconify(){
    }
    virtual ~Iconify(){
    }
    virtual int operator() (Display *dpy, Window window,int screen_number);
	virtual int operator() (Display *dpy, Window window,int screen_number,XWindowState &state);
};

class UnIconify : public XWindowAction {
public:
    UnIconify(){
    }
    virtual ~UnIconify(){
    }
    virtual int operator() (Display *dpy, Window window,int screen_number);
	virtual int operator() (Display *dpy, Window window,int screen_number,XWindowState &state);
};

// Algorithms

int findWindowAndDoAction(XWindowPredicat *predicat,XWindowAction *action);

int getWindowState(XWindowPredicat *predicat,XWindowState &state);

#undef MODULE_FLAG

#endif	/* _XWINDOWMGR_H */

