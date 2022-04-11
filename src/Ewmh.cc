// Ewmh.cc for FbPager
// Copyright (c) 2004 Henrik Kinnunen (fluxgen at users.sourceforge.net)
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
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.

#include "Ewmh.hh"

#include "Pager.hh"
#include "WindowHint.hh"

#include "FbTk/App.hh"
#include "FbTk/FbWindow.hh"
#include "FbRootWindow.hh"
#include "PropertyTools.hh"

#include <X11/Xatom.h>

#include <iostream>
#include <vector>
#include <sys/types.h>
#include <unistd.h>
#include <memory>


using namespace std;
using namespace PropertyTools;


struct PropT {
    PropT(unsigned char *data, unsigned int num):data(data), num(num) {   }
    ~PropT() {
        if (data != 0)
            XFree(data);
    }
    unsigned char *data;
    unsigned int num;
};


typedef std::unique_ptr<PropT> PropTPtr;

PropT *property(const FbTk::FbWindow &win, Atom atom,
                Atom type, unsigned int num) {
    Atom ret_type;
    int fmt;
    unsigned long nitems, bytes_after;
    unsigned char *data = 0;
    win.property(atom,
                 0, num,
                 False,
                 type,
                 &ret_type, &fmt,
                 &nitems,
                 &bytes_after,
                 &data);
    return new PropT(data, nitems);

}

namespace FbPager {

class Ewmh_priv {
public:
    Ewmh_priv() {
        Display *disp = FbTk::App::instance()->display();
        state_skip_pager =  XInternAtom(disp, "_NET_WM_STATE_SKIP_PAGER", False);
        state_skip_taskbar =  XInternAtom(disp, "_NET_WM_STATE_SKIP_TASKBAR", False);
        state_sticky =  XInternAtom(disp, "_NET_WM_STATE_STICKY", False);
        state_hidden = XInternAtom(disp, "_NET_WM_STATE_HIDDEN", False);
        state_shaded = XInternAtom(disp, "_NET_WM_STATE_SHADED", False);
        state_above = XInternAtom(disp, "_NET_WM_STATE_ABOVE", False);
        state_below = XInternAtom(disp, "_NET_WM_STATE_BELOW", False);
        wm_desktop = XInternAtom(disp, "_NET_WM_DESKTOP", False);
        wm_state =  XInternAtom(disp, "_NET_WM_STATE", False);
        wm_pid = XInternAtom(disp, "_NET_WM_PID", False);
        wm_type = XInternAtom(disp, "_NET_WM_WINDOW_TYPE", False);
        type_dock = XInternAtom(disp, "_NET_WM_WINDOW_TYPE_DOCK", False);
        number_of_desktops = XInternAtom(disp, "_NET_NUMBER_OF_DESKTOPS", False);
        current_desktop = XInternAtom(disp, "_NET_CURRENT_DESKTOP", False);
        clientlist = XInternAtom(disp, "_NET_CLIENT_LIST", False);
        moveresize_window = XInternAtom(disp, "_NET_MOVERESIZE_WINDOW", False);
        active_window = XInternAtom(disp, "_NET_ACTIVE_WINDOW", False);
        close_window = XInternAtom(disp, "_NET_CLOSE_WINDOW", False);
        desktop_layout = XInternAtom(disp, "_NET_DESKTOP_LAYOUT", False);
    }
    Atom state_skip_pager, state_skip_taskbar, state_sticky,
        state_hidden, state_shaded, state_above, state_below;
    Atom wm_desktop, wm_state, wm_pid, wm_type;
    Atom type_dock;
    Atom number_of_desktops, current_desktop;
    Atom clientlist;
    Atom moveresize_window;
    Atom active_window;
    Atom close_window;
    Atom above, below;
    Atom desktop_layout;
};

Ewmh::Ewmh():
    m_support(true),
    m_data(new Ewmh_priv()) {

    //!! TODO
    // check for ewmh support

}

Ewmh::~Ewmh() {
}


void Ewmh::setFocus(FbTk::FbWindow &win) {

}

void Ewmh::setDesktopLayout(FbTk::FbWindow &root,
                            Orientation orientation,
                            Corner starting_corner,
                            unsigned int columns, unsigned int rows) {
    unsigned int data[4] = { orientation, columns, rows, starting_corner };
    root.changeProperty(m_data->desktop_layout,
                        XA_CARDINAL,
                        32,
                        PropModeReplace,
                        (unsigned char*)data, 4 );
}

void Ewmh::moveResize(FbTk::FbWindow &win) {

    // We can't do this yet, there is a bug in fluxbox 0.9.8
    // and this makes it crash (it's fixed in cvs though)
    /*
            Display *disp = FbTk::App::instance()->display();
            XEvent event;
            event.xclient.display = disp;
            event.xclient.type = ClientMessage;
            event.xclient.window = RootWindow(disp, win.screenNumber());
            event.xclient.message_type = m_data->moveresize_window;
            event.xclient.format = 32;
            event.xclient.data.l[0] = 0;
            event.xclient.data.l[1] = win.x();
            event.xclient.data.l[2] = win.y();
            event.xclient.data.l[3] = win.width();
            event.xclient.data.l[4] = win.height();

            XSendEvent(disp, event.xclient.window, False, SubstructureNotifyMask, &event);
    */
}

void Ewmh::sendToWorkspace(FbTk::FbWindow &win, int workspace) {

}

void Ewmh::closeWindow(FbTk::FbWindow &win) {
    Display *disp = FbTk::App::instance()->display();
    XEvent event;
    event.xclient.display = disp;
    event.xclient.type = ClientMessage;
    event.xclient.window = win.window();
    event.xclient.message_type = m_data->close_window;
    event.xclient.format = 32;
    event.xclient.data.l[0] = 0;
    XSendEvent(disp, RootWindow(disp, DefaultScreen(disp)), False, SubstructureNotifyMask, &event);
}

bool Ewmh::propertyNotify(Pager &pager, XPropertyEvent &event) try {
    if (event.window != DefaultRootWindow(FbTk::App::instance()->display())) {
        return false;
    }
    if (event.atom == m_data->current_desktop) {
        pager.setCurrentWorkspace(getIntProperty(event.window, event.atom));
    } else if (event.atom == m_data->number_of_desktops) {
        pager.updateWorkspaceCount(getIntProperty(event.window, event.atom));
    } else if (event.atom == m_data->active_window) {
        pager.setFocusedWindow(getWinProperty(event.window, event.atom));
    } else if (event.atom == m_data->clientlist) {
        vector<Window> windows;
        getWinArrayProperty(DefaultRootWindow(FbTk::App::instance()->display()), event.atom, windows);
        std::vector< pair<Window, unsigned int > > wins_workspaces;
        for ( unsigned int win = 0; win < windows.size(); ++win ) {
            wins_workspaces.
                push_back( std::make_pair(windows[win],
                                          getIntProperty(windows[win],
                                                         m_data->wm_desktop)));
        }
        pager.addWindows(wins_workspaces);
    } else {
        // did not handle it here
        return false;
    }

    return true;

} catch ( const PropertyException& e ) {
    cerr << "Ewmh Exception: " << e.what() << endl;
    // we handle it
    return true;
}

bool Ewmh::clientMessage(Pager &pager, XClientMessageEvent &event) {
    if (!m_support)
      return false;


    if (event.message_type == m_data->current_desktop) {
        pager.setCurrentWorkspace(event.data.l[0]);
        return true;
    } else if (event.message_type == m_data->number_of_desktops) {
        pager.updateWorkspaceCount(event.data.l[0]);
        return true;
    } else if (event.message_type == m_data->wm_desktop) {
        pager.moveToWorkspace(event.window, event.data.l[0]);
        return true;
    } else if (event.message_type == m_data->clientlist) {
        return true;
    } else if (event.message_type == m_data->active_window) {
        pager.setFocusedWindow(event.window);
        return true;
    }

    return false;
}

void Ewmh::changeWorkspace(int screen_num, int workspace) {
    Display *disp = FbTk::App::instance()->display();
    XEvent event;
    Status stat;
    event.xclient.display = disp;
    event.xclient.type = ClientMessage;
    event.xclient.window = RootWindow(disp, screen_num);
    event.xclient.message_type = m_data->current_desktop;
    event.xclient.format = 32;
    event.xclient.data.l[0] = workspace;
    stat = XSendEvent(disp, event.xclient.window, False, SubstructureNotifyMask, &event);
    if (stat != 1)
        std::cerr << "XSendEvent() returned " << stat << std::endl;
}

void Ewmh::setHints(FbTk::FbWindow &win, WindowHint &hint) {

    int data = getpid();
    win.changeProperty(m_data->wm_pid,
                       XA_CARDINAL,
                       32,
                       PropModeReplace,
                       (unsigned char *)&data, 1);

    if (hint.flags() & WindowHint::WHINT_STICKY) {
        data = 0xFFFFFFFF;
        win.changeProperty(m_data->wm_desktop,
                           XA_CARDINAL,
                           32,
                           PropModeReplace,
                           (unsigned char *)&data, 1);
    }

    if (hint.flags() & WindowHint::WHINT_TYPE_DOCK) {
//        cerr << "Setting type dock." << endl;
        Atom dock = m_data->type_dock;
        win.changeProperty(m_data->wm_type,
                           XA_ATOM,
                           32,
                           PropModeReplace,
                           (unsigned char *)&dock, 1);
    }

    std::vector<Atom> states;
    if (hint.flags() & WindowHint::WHINT_SKIP_TASKBAR)
        states.push_back(m_data->state_skip_taskbar);
    if (hint.flags() & WindowHint::WHINT_SKIP_PAGER)
        states.push_back(m_data->state_skip_pager);
    if (hint.flags() & WindowHint::WHINT_STICKY)
        states.push_back(m_data->state_sticky);
    if (hint.flags() & WindowHint::WHINT_HIDDEN)
        states.push_back(m_data->state_hidden);
    if (hint.flags() & WindowHint::WHINT_LAYER_TOP){
        states.push_back(m_data->state_above);
//        cerr << "Setting state above." << endl;
    }
    if (hint.flags() & WindowHint::WHINT_LAYER_BOTTOM) {
//        cerr<< "Setting state bottom." << endl;
        states.push_back(m_data->state_below);
    }

    Atom *state_atoms = new Atom[states.size()];
    for (size_t i=0; i < states.size(); ++i)
        state_atoms[i] = states[i];

    win.changeProperty(m_data->wm_state,
                       XA_ATOM,
                       32,
                       PropModeReplace,
                       (unsigned char *)state_atoms, states.size());
    delete [] state_atoms;
}


void Ewmh::getHints(const FbTk::FbWindow &win, WindowHint &hint) const {
    PropTPtr p(property(win,
                        m_data->wm_state,
                        XA_ATOM, 0x7FFFFFFF));

    if (p.get() != 0 && p->data != 0) {

        Atom *states = (Atom *)(p->data);
        for (unsigned int i=0; i<p->num; ++i) {
            if (states[i] == m_data->state_skip_pager)
                hint.add(WindowHint::WHINT_SKIP_PAGER);
            else if (states[i] == m_data->state_skip_taskbar)
                hint.add(WindowHint::WHINT_SKIP_TASKBAR);
            else if (states[i] == m_data->state_sticky)
                hint.add(WindowHint::WHINT_STICKY);
            else if (states[i] == m_data->state_shaded)
                hint.add(WindowHint::WHINT_SHADED);
        }
    }

    p.reset(property(win, m_data->wm_type,
                     XA_ATOM, 0x7FFFFFFF));
    if (p.get() != 0 && p->data != 0) {
        // we only care about the first type.
        Atom *atoms = (Atom *)p->data;
        if (atoms[0] == m_data->type_dock)
            hint.add(WindowHint::WHINT_TYPE_DOCK);
    }

    p.reset(property(win, m_data->wm_desktop,
                     XA_CARDINAL, 1));
    if (p.get() != 0 && p->data != 0) {
        int workspace = static_cast<int>(*(p->data));
        hint.setWorkspace(workspace);
    }
}

int Ewmh::numberOfWorkspaces(int screen_num) const {

    PropTPtr p(property(FbRootWindow(screen_num),
                        m_data->number_of_desktops,
                        XA_CARDINAL, 1));
    if (p.get() == 0 || p->data == 0)
        return 0;
    else {
        int num = static_cast<int>(*(p->data));
        return num;
    }

}

} // end namespace FbPager
