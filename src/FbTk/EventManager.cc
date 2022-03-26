// EventManager.cc
// Copyright (c) 2002 Henrik Kinnunen (fluxgen at linuxmail.org)
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.

// $Id: EventManager.cc,v 1.10 2003/10/14 16:23:16 rathnor Exp $

#include "EventManager.hh"
#include "FbWindow.hh"
#include "App.hh"

#include <iostream>
using namespace std;

namespace FbTk {

EventManager *EventManager::instance() {
    static EventManager ev;
    return &ev;
}

EventManager::~EventManager() {
    m_eventhandlers.clear();
    if (m_eventhandlers.size() != 0)
        cerr << "FbTk::EventManager: Warning: unregistered eventhandlers! " << m_eventhandlers.size() << endl;
}

void EventManager::handleEvent(XEvent &ev) {
    dispatch(ev.xany.window, ev);
}

void EventManager::add(EventHandler &ev, const FbWindow &win) {
    registerEventHandler(ev, win.window());
}

void EventManager::addParent(EventHandler &ev, const FbWindow &win) {
    if (win.window() != 0)
        m_parent[win.window()] = &ev;
}

void EventManager::remove(const FbWindow &win) {
    unregisterEventHandler(win.window());
}

Window EventManager::getEventWindow(XEvent &ev) {
    // we only have cases for events that differ from xany
    switch (ev.type) {
    case CreateNotify:
        // XCreateWindowEvent
        return ev.xcreatewindow.window;
        break;
    case DestroyNotify:
        // XDestroyWindowEvent
        return ev.xdestroywindow.window;
        break;
    case UnmapNotify:
        // XUnmapEvent
        return ev.xunmap.window;
        break;
    case MapNotify:
        // XMapEvent
        return ev.xmap.window;
        break;
    case MapRequest:
        // XMapRequestEvent
        return ev.xmaprequest.window;
        break;
    case ReparentNotify:
        // XReparentEvent
        return ev.xreparent.window;
        break;
    case ConfigureNotify:
        // XConfigureNotify
        return ev.xconfigure.window;
        break;
    case GravityNotify:
        // XGravityNotify
        return ev.xgravity.window;
        break;
    case ConfigureRequest:
        // XConfigureRequestEvent
        return ev.xconfigurerequest.window;
        break;
    case CirculateNotify:
        // XCirculateEvent
        return ev.xcirculate.window;
        break;
    case CirculateRequest:
        // XCirculateRequestEvent
        return ev.xcirculaterequest.window;
        break;
    default:
        return ev.xany.window;
    }
}

void EventManager::registerEventHandler(EventHandler &ev, Window win) {
    if (win != None)
        m_eventhandlers[win] = &ev;
}

void EventManager::unregisterEventHandler(Window win) {
    if (win != None) {
        m_eventhandlers.erase(win);
        m_parent.erase(win);
    }
}

void EventManager::dispatch(Window win, XEvent &ev, bool parent) {
    EventHandler *evhand = 0;
    if (parent) {
        if (m_parent.find(win) == m_parent.end())
            return;
        evhand = m_parent[win];
    } else {
        win = getEventWindow(ev);
        if (m_eventhandlers.find(win) == m_eventhandlers.end())
            return;
        evhand = m_eventhandlers[win];
    }

    if (evhand == 0) {
        cerr << __func__ << "() NULL handler for win 0x" << hex << win << dec << endl;
        return;
    }

    switch (ev.type) {
    case KeyPress:
        evhand->keyPressEvent(ev.xkey);
        break;
    case KeyRelease:
        evhand->keyReleaseEvent(ev.xkey);
        break;
    case ButtonPress:
        evhand->buttonPressEvent(ev.xbutton);
        break;
    case ButtonRelease:
        evhand->buttonReleaseEvent(ev.xbutton);
        break;
    case MotionNotify:
        evhand->motionNotifyEvent(ev.xmotion);
        break;
    case Expose:
        evhand->exposeEvent(ev.xexpose);
        break;
    case EnterNotify:
        evhand->enterNotifyEvent(ev.xcrossing);
        break;
    case LeaveNotify:
        evhand->leaveNotifyEvent(ev.xcrossing);
        break;
    default:
        evhand->handleEvent(ev);
        break;
    };

    // find out which window is the parent and
    // dispatch event
    Window root, parent_win, *children = 0;
    unsigned int num_children;
    if (XQueryTree(FbTk::App::instance()->display(), win,
                   &root, &parent_win, &children, &num_children) != 0) {
        if (children != 0)
            XFree(children);

        if (parent_win != 0 &&
            parent_win != root) {
            if (m_parent.find(parent_win) == m_parent.end())
                return;

            // dispatch event to parent
            dispatch(parent_win, ev, true);
        }
    }

}

}; // end namespace FbTk
