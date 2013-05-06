/*
 * File:   xwindowMngt.cpp
 * Author: oc
 *
 * Created on April 3, 2011, 1:52 AM
 */

#include "globals.h"
#include <unistd.h>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <sys/types.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <linux/limits.h>

#include "xwindowMngt.h"

#define MODULE_FLAG FLAG_XWINDOWS

////////////////////////////////////////////////////////////////////////////////
// X11 Window Helper functions
////////////////////////////////////////////////////////////////////////////////

static int XWinErrorHandler(Display *dpy, XErrorEvent *errorEvent) {
    char errorMsg[255];
    XGetErrorText(dpy, errorEvent->error_code, errorMsg, sizeof (errorMsg));
    ERROR_MSG("X11 Error:\n"
            "\ttype = %d on display %d\n"
            "\tRessource ID = %d\n"
            "\tfailed request serial number = %u\n"
            "\terror code = %d (%s)\n"
            "\tfailed request was %u / %u (see X11/Xproto.h for its identification)"
            , errorEvent->type, errorEvent->display
            , errorEvent->resourceid
            , errorEvent->serial
            , errorEvent->error_code, errorMsg
            , errorEvent->request_code, errorEvent->minor_code
            );
    return 0; //cancel
}

inline static Window XGetParentWindow(Display *dpy, Window window) {
    Window *childList = NULL;
    Window parentWindow = 0, rootWindow = 0;
    unsigned int numberOfChildren = 0;
    // try to get its parent's status
    Status status = XQueryTree(dpy, window, &rootWindow, &parentWindow, &childList, &numberOfChildren);
    DEBUG_VAR(status, "%d");
    if (childList != NULL) {
        XFree(childList);
        childList = NULL;
    }
    return parentWindow;
}

inline static Window XGetTopWindow(Display *dpy, Window window) {
    Window *childList = NULL;
    Window parentWindow = 0, rootWindow = 0;
    unsigned int numberOfChildren = 0;
    // try to get its parent's status
    Status status = XQueryTree(dpy, window, &rootWindow, &parentWindow, &childList, &numberOfChildren);
    DEBUG_VAR(status, "%d");
    if (childList != NULL) {
        XFree(childList);
        childList = NULL;
    }
    if (parentWindow == rootWindow) {
        parentWindow = window;
    }
    return parentWindow;
}

static inline Window XGetRootWindow(Display *dpy, Window window) {
    Window *childList = NULL;
    Window parentWindow = 0, rootWindow = 0;
    unsigned int numberOfChildren = 0;
    // try to get its parent's status
    Status status = XQueryTree(dpy, window, &rootWindow, &parentWindow, &childList, &numberOfChildren);
    DEBUG_VAR(status, "%d");
    if (childList != NULL) {
        XFree(childList);
        childList = NULL;
    }
    return rootWindow;
}

// based on icesh source code

// define symbols from IceWM / WinMgr.h
#define WinLayerDesktop        0L
#define WinLayerBelow          2L
#define WinLayerNormal         4L
#define WinLayerOnTop          6L
#define WinLayerDock           8L
#define WinLayerAboveDock      10L
#define WinLayerMenu           12L
#define WinLayerFullscreen     14L // hack, for now
#define WinLayerAboveAll       15L // for taskbar auto hide

#define WinStateAllWorkspaces  (1 << 0)   /* appears on all workspaces */
#define WinStateMinimized      (1 << 1)   /* to iconbox,taskbar,... */
#define WinStateMaximizedVert  (1 << 2)   /* maximized vertically */
#define WinStateMaximizedHoriz (1 << 3)   /* maximized horizontally */
#define WinStateHidden         (1 << 4)   /* not on taskbar if any, but still accessible */
#define WinStateRollup         (1 << 5)   /* only titlebar visible */
#define WinStateFixedPosition  (1 << 10)  /* fixed position on virtual desktop*/
#define WinStateArrangeIgnore  (1 << 11)  /* ignore for auto arranging */
//#define WinStateDocked         (1 << 9) /* docked, ignore my area for maximizing */
#define WinStateSkipTaskBar    (1 << 24)  /* skip taskbar */
#define WinStateModal          (1 << 25)  /* modal */
#define WinStateBelow          (1 << 26)  /* below layer */
#define WinStateAbove          (1 << 27)  /* above layer */
#define WinStateFullscreen     (1 << 28)  /* fullscreen (no lauout limits) */
#define WinStateWasHidden      (1 << 29)  /* was hidden when parent was minimized/hidden */
#define WinStateWasMinimized   (1 << 30)  /* was minimized when parent was minimized/hidden */
#define WinStateWithdrawn      (1 << 31)  /* managed, but not available to user */

#define WIN_STATE_ALL (WinStateAllWorkspaces | WinStateMinimized |\
                       WinStateMaximizedVert | WinStateMaximizedHoriz |\
                       WinStateHidden | WinStateRollup)

inline static Status XSetLayer(Display *dpy,Window root,Window window, long layer) {
    Status status = BadAtom;
    Atom WinLayerAtom = XInternAtom(dpy, "_WIN_LAYER", False);
    if (WinLayerAtom != None) {
       XClientMessageEvent xev;

       memset(&xev, 0, sizeof(xev));

       xev.type = ClientMessage;
       xev.window = window;
       xev.message_type = WinLayerAtom;
       xev.format = 32;
       xev.data.l[0] = layer;
       xev.data.l[1] = CurrentTime;

       status = XSendEvent(dpy, root, False, SubstructureNotifyMask, (XEvent *) &xev);
    }
    return status;
}

inline static Status XSetWinState(Display *dpy,Window root,Window window, long mask, long state) {
    Status status = BadAtom;
    Atom StateAtom = XInternAtom(dpy, "_WIN_STATE", False);
    if (StateAtom != None) {
       XClientMessageEvent xev;

       memset(&xev, 0, sizeof(xev));

       xev.type = ClientMessage;
       xev.window = window;
       xev.message_type = StateAtom;
       xev.format = 32;
       xev.data.l[0] = mask;
       xev.data.l[1] = state;
       xev.data.l[2] = CurrentTime;

       status = XSendEvent(dpy, root, False, SubstructureNotifyMask, (XEvent *) &xev);
    }
    return status;
}

inline static Status XUnIconifyWindow(Display *dpy, Window window) {
    Status status = BadWindow;
    //cf Xlib Programming Manual v1 p325
    XWMHints *wmHints = XGetWMHints(dpy, window);
    if (wmHints != NULL) {
        wmHints->flags |= StateHint;
        wmHints->initial_state = IconicState;
        XSetWMHints(dpy, window, wmHints);
        XFree(wmHints);
        wmHints = NULL;
        status = XMapRaised(dpy, window);
     } else {
        // because XGetWMHints return NULL when using IceWM 
        // source code based on icesh source code and the runOMIRS.sh script, 
        Window root = XGetRootWindow(dpy,window);
        if (root != 0) {
           // 63 = b111111, 12 = b1100
           status = XSetWinState(dpy,root,window,0x3F,WinStateMaximizedVert|WinStateMaximizedHoriz|WinStateAbove);
           if (status != 0) {
              status = XSetLayer(dpy,root,window,WinLayerOnTop);
           } //(status != 0)
        } //(root != 0)
    } // !(wmHints != NULL)
    return status;
}

static Status XGetWindowState(Display *dpy, Window window, unsigned int *windowState) {
    Status status = BadAtom;
    Atom StateAtom = XInternAtom(dpy, "WM_STATE", True);
    if (StateAtom != None) {
        char *StateAtom_name = XGetAtomName(dpy, StateAtom);
        if (StateAtom_name != NULL) {
            Atom actual_type;
            int actual_format;
            unsigned long nitems;
            unsigned long bytes_after;
            unsigned char *property = NULL;

            status = XGetWindowProperty(dpy, window, StateAtom, 0, 1024, False, AnyPropertyType
                    , &actual_type, &actual_format, &nitems, &bytes_after, &property);
            if (Success == status) {
                if (actual_type != None) {
                    if (32 == actual_format) {
                        const unsigned int *data = (unsigned int*) property;
                        *windowState = (*data);
                        DEBUG_VAR(*data, "%u");
                    } else { //(32 == actual_format)
                        ERROR_MSG("XGetWindowProperty WM_STATE bad actual_format value (%d)", actual_format);
                    }
                } else { //if (actual_type != None)
                    DEBUG_MSG("WM_STATE property not set (not the application's root window ?)");
                    // try to get its parent's status
                    Window parentWindow = XGetTopWindow(dpy, window);
                    if ((parentWindow != window) && (parentWindow != 0)) {
                        status = XGetWindowState(dpy, parentWindow, windowState);
                    }
                }
            } else {
                ERROR_MSG("XGetWindowProperty WM_STATE error (%d) ", status);
            }
            if (property != NULL) {
                XFree(property);
                property = NULL;
            }
            XFree(StateAtom_name);
            StateAtom_name = NULL;
        } //if (StateAtom_name != NULL)
    } //if (StateAtom != None)
    return status;
}

////////////////////////////////////////////////////////////////////////////////
// Predicats functors
////////////////////////////////////////////////////////////////////////////////

bool XWindowNamed::operator() (Display *dpy, Window window) {
    bool found(false);
    char *childWindowName = NULL;
    Status status = XFetchName(dpy, window, &childWindowName);
    if ((status != BadWindow) && (childWindowName != NULL)) {
        DEBUG_VAR(childWindowName, "%s");
        found = (windowsName.compare(childWindowName) == 0);
    }
    if (childWindowName != NULL) {
        XFree(childWindowName);
        childWindowName = NULL;
    }
    return found;
}

bool XWindowWhichNameContains::operator() (Display *dpy, Window window) {
    bool found(false);
    char *childWindowName = NULL;
    Status status = XFetchName(dpy, window, &childWindowName);
    if ((status != BadWindow) && (childWindowName != NULL)) {
        DEBUG_VAR(childWindowName, "%s");
        found = (windowsSubStringName.find(childWindowName) != std::string::npos);
    }
    if (childWindowName != NULL) {
        XFree(childWindowName);
        childWindowName = NULL;
    }
    return found;
}

bool WindowOf::operator() (Display *dpy, Window window) {
    bool found(false);
    Atom PIDAtom = XInternAtom(dpy, "_NET_WM_PID", True);
    if (PIDAtom != None) {
        char *PIDAtom_name = XGetAtomName(dpy, PIDAtom);
        if (PIDAtom_name != NULL) {
            Atom actual_type;
            int actual_format;
            unsigned long nitems;
            unsigned long bytes_after;
            unsigned char *property = NULL;

            Status status = XGetWindowProperty(dpy, window, PIDAtom, 0, 1024, False, AnyPropertyType, &actual_type, &actual_format, &nitems, &bytes_after, &property);
            if (Success == status) {
                if (actual_type != None) {
                    if (32 == actual_format) {
                        long *data = (long *) property;
                        DEBUG_VAR(*data, "%d");
                        found = true;

                        XTextProperty clientMachineXProperty;
                        status = XGetWMClientMachine(dpy, window, &clientMachineXProperty);
                        if (status != 0) {
                            if (8 == clientMachineXProperty.format) {
                                DEBUG_VAR(clientMachineXProperty.value, "%s");
                                if (hostname.compare((const char*) clientMachineXProperty.value) != 0) {
                                    // this PID is not located on the same computer !
                                    found = false;
                                }
                            } //(8 == clientMachineXProperty.format)
                        } else { //(status != 0)
                            ERROR_MSG("XGetWMClientMachine error (%d)", status);
                        }
                    } else {
                        ERROR_MSG("XGetWMClientMachine bad data format (%d)", actual_format);
                    }
                } else {
                    ERROR_MSG("property not set (XGetWMClientMachine actual_type is None)");
                }
            } else {
                ERROR_MSG("XGetWindowProperty error %d", status);
            }

            if (property != NULL) {
                XFree(property);
                property = NULL;
            }

            XFree(PIDAtom_name);
            PIDAtom_name = NULL;
        }
    } //if (PIDAtom != None)

    return found;
}

bool XWindowOfClass::operator() (Display *dpy, Window window) {
    bool found(false);
    Atom WClassAtom = XInternAtom(dpy, "WM_CLASS", True);
    if (WClassAtom != None) {
        char *PIDAtom_name = XGetAtomName(dpy, WClassAtom);
        if (PIDAtom_name != NULL) {
            Atom actual_type;
            int actual_format;
            unsigned long nitems;
            unsigned long bytes_after;
            unsigned char *property = NULL;

            Status status = XGetWindowProperty(dpy, window, WClassAtom, 0, 1024, False, AnyPropertyType, &actual_type, &actual_format, &nitems, &bytes_after, &property);
            if (Success == status) {
                if (actual_type != None) {
                    if (8 == actual_format) {
                        register char *data = (char *) property;
                        const char *end = (char *) property + nitems;
                        while ((data < end) && (!found)) {
                            DEBUG_VAR(data, "%s");
                            found = (xWinclass.compare(data) == 0);
                            if (!found) {
                                const size_t n = strlen(data) + 1;
                                data += n;
                            }
                        } //while ((data < end) && (!found))
                    } else {
                        ERROR_MSG("XGetWMClientMachine bad data format (%d)", actual_format);
                    }
                } else {
                    ERROR_MSG("property not set (XGetWMClientMachine actual_type is None)");
                }
            } else {
                ERROR_MSG("XGetWindowProperty error %d", status);
            }

            if (property != NULL) {
                XFree(property);
                property = NULL;
            }

            XFree(PIDAtom_name);
            PIDAtom_name = NULL;
        }
    } //if (PIDAtom != None)

    return found;
}

////////////////////////////////////////////////////////////////////////////////
// Actions functors
////////////////////////////////////////////////////////////////////////////////

int SwapState::operator() (Display *dpy, Window window, int screen_number) {
    unsigned int windowState(0);
    Status status = XGetWindowState(dpy,window,&windowState);
    if (Success == status) {
        if (1 == windowState) {
            DEBUG_MSG("window state: Normal");
            status = XIconifyWindow(dpy, window, screen_number);
            DEBUG_VAR(status, "%d");
        } else if (3 == windowState) {
            DEBUG_MSG("window state: Iconic");
            status = XUnIconifyWindow(dpy, window);
            DEBUG_VAR(status, "%d");
        }
    }    
    return status;
}

int SwapState::operator() (Display *dpy, Window window,int screen_number,XWindowState &state) {
    unsigned int windowState(0);
    Status status = XGetWindowState(dpy,window,&windowState);
    if (Success == status) {
        if (1 == windowState) {
            DEBUG_MSG("window state: Normal");
            status = XIconifyWindow(dpy, window, screen_number);
            DEBUG_VAR(status, "%d");
			if (0 == status) {
			   state = Iconic;
			}
        } else if (3 == windowState) {
            DEBUG_MSG("window state: Iconic");
            status = XUnIconifyWindow(dpy, window);
            DEBUG_VAR(status, "%d");
			if (0 == status) {
			   state = Normal;
			}
        }
    }    
    return status;
}

int Iconify::operator() (Display *dpy, Window window, int screen_number) {
    Status status = XIconifyWindow(dpy, window, screen_number);
    DEBUG_VAR(status, "%d");
    return status;
}

int Iconify::operator() (Display *dpy, Window window, int screen_number,XWindowState &state) {
    Status status = XIconifyWindow(dpy, window, screen_number);
    DEBUG_VAR(status, "%d");
	if (0 == status) {
	   state = Iconic;
	}
    return status;
}

int UnIconify::operator() (Display *dpy, Window window, int screen_number) {
    Status status = XUnIconifyWindow(dpy, window);
    DEBUG_VAR(status, "%d");
    return status;
}

int UnIconify::operator() (Display *dpy, Window window, int screen_number,XWindowState &state) {
    Status status = XUnIconifyWindow(dpy, window);
    DEBUG_VAR(status, "%d");
	if (0 == status) {
	   state = Normal;
	}
    return status;
}

////////////////////////////////////////////////////////////////////////////////
// Algorithms
////////////////////////////////////////////////////////////////////////////////

// findWindowAndDoAction
static inline Status findWindowAndDoAction(Display *dpy, Window window, int screen, unsigned int level, XWindowPredicat *predicat, XWindowAction *action) {
    Status status;
    Window *childList = NULL;
    Window parentWindow, rootWindow;
    unsigned int numberOfChildren = 0;

    status = XQueryTree(dpy, window, &rootWindow, &parentWindow, &childList, &numberOfChildren);
    if (status != BadWindow) {
        unsigned int i;
        Window *childListItem = childList;
        level++;
        DEBUG_VAR(numberOfChildren, "%d");
        for (i = 0; i < numberOfChildren; i++) {
            const Window window(*childListItem);
            if ((*predicat)(dpy, window)) {
                DEBUG_VAR(level, "%d");
                // found
                (*action)(dpy, window, screen);
            } else {
                status = findWindowAndDoAction(dpy, window, screen, level, predicat, action);
            }
            childListItem++;
        } //for (i = 0; i < numberOfChildren; i++)
    } //(status != BadWindow)

    if (childList != NULL) {
        XFree(childList);
        childList = NULL;
    }
    DEBUG_VAR(status,"%d");

    return status;
}

int findWindowAndDoAction(XWindowPredicat *predicat, XWindowAction *action) {
    int error = EXIT_SUCCESS;
    const char *display = getenv("DISPLAY");
    if (*display != 0) {
        Display *dpy = XOpenDisplay((char*) display);
        if (dpy != NULL) {
            const int screenCount = ScreenCount(dpy);
            int screen = 0;
            DEBUG_VAR(screenCount, "%d");
            XSetErrorHandler(XWinErrorHandler);
            //XSetIOErrorHandler(XWinIOErrorHandler);
            for (screen = 0; screen < screenCount; screen++) {                
                unsigned int level = 0;
                Window displayRootWindow = XRootWindow(dpy, screen);
                /*Status status =*/ findWindowAndDoAction(dpy, displayRootWindow, screen, level, predicat, action);
            } //for (screen = 0; screen < screenCount; screen++)
            XCloseDisplay(dpy);
            dpy = NULL;
        } else { //(dpy != NULL)
		    error = ENXIO;
            ERROR_MSG("can't open X11 display");
        }
    } else {
	    error = ENOENT;
        ERROR_MSG("can't get DISPLAY value from the program's environment");
    }
    return error;
}

// getWindowState

static inline Status getWindowState(Display *dpy, Window window, int screen, unsigned int level, XWindowPredicat *predicat, XWindowState &state) {
    Status status;
    Window *childList = NULL;
    Window parentWindow, rootWindow;
    unsigned int numberOfChildren = 0;

    status = XQueryTree(dpy, window, &rootWindow, &parentWindow, &childList, &numberOfChildren);
    if (status != BadWindow) {
        unsigned int i;
        Window *childListItem = childList;
        level++;
        DEBUG_VAR(numberOfChildren, "%d");
        for (i = 0; i < numberOfChildren; i++) {
            const Window window(*childListItem);
            if ((*predicat)(dpy, window)) {
                DEBUG_VAR(level, "%d");
                // found
				unsigned int windowState(0);
				status = XGetWindowState(dpy,window,&windowState);
				DEBUG_VAR(windowState,"%d");
				switch(windowState) {
				  case 1:
                     DEBUG_MSG("window state: Normal");
					 state = Normal;
					 break;
                  case 3:
					 DEBUG_MSG("window state: Iconic");
					 state = Iconic;                     
					 break;
				}                
            } else {
                status = getWindowState(dpy, window, screen, level, predicat, state);
            }
            childListItem++;
        } //for (i = 0; i < numberOfChildren; i++)
    } //(status != BadWindow)

    if (childList != NULL) {
        XFree(childList);
        childList = NULL;
    }
    DEBUG_VAR(status,"%d");

    return status;
}

int getWindowState(XWindowPredicat *predicat,XWindowState &state) {
	int error = EXIT_SUCCESS;
    const char *display = getenv("DISPLAY");
	state = Unknown; 
    if (*display != 0) {
        Display *dpy = XOpenDisplay((char*) display);
        if (dpy != NULL) {
            const int screenCount = ScreenCount(dpy);
            int screen = 0;
            DEBUG_VAR(screenCount, "%d");
            XSetErrorHandler(XWinErrorHandler);
            //XSetIOErrorHandler(XWinIOErrorHandler);
            for (screen = 0; screen < screenCount; screen++) {                
                unsigned int level = 0;
                Window displayRootWindow = XRootWindow(dpy, screen);
                /*Status status =*/ getWindowState(dpy, displayRootWindow, screen, level, predicat, state);
            } //for (screen = 0; screen < screenCount; screen++)
            XCloseDisplay(dpy);
            dpy = NULL;
        } else { //(dpy != NULL)
		    error = ENXIO;
            ERROR_MSG("can't open X11 display");
        }
    } else {
	    error = ENOENT;
        ERROR_MSG("can't get DISPLAY value from the program's environment");
    }
    return error;
}
