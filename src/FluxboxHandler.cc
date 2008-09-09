// FluxboxHandler.cc for FbPager
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

#include "FluxboxHandler.hh"

#include "Pager.hh"
#include "WindowHint.hh"
#include "FbRootWindow.hh"
#include "ClientWindow.hh"

#include "FbTk/App.hh"

#include <X11/Xutil.h>

#include <iostream>
using namespace std;

namespace {

void sendClientMessage(FbTk::FbWindow &src, 
                       FbTk::FbWindow &dest,
                       Atom the_atom, 
                       unsigned long data1, unsigned long data2 = 0,
                       unsigned long data3 = 0, unsigned long data4 = 0) {
    // fill in client structure
    XEvent clientevent;
    clientevent.xclient.display = FbTk::App::instance()->display();
    clientevent.xclient.type = ClientMessage;
    clientevent.xclient.window = dest.window();
    clientevent.xclient.message_type = the_atom;
    clientevent.xclient.format = 32;
    clientevent.xclient.data.l[0] = data1;
    clientevent.xclient.data.l[1] = data2;
    clientevent.xclient.data.l[2] = data3;
    clientevent.xclient.data.l[3] = data4;
    // send event to root window
    src.sendEvent(false, SubstructureRedirectMask, clientevent);
}

} // end anonymous namespace

struct FluxboxHints {
    unsigned long flags, attrib, workspace, stack, decoration;
    enum Attrib { 
        ATTRIB_SHADED = 0x01, 
        ATTRIB_STICK = 0x08, 
        ATTRIB_WORKSPACE = 0x10,
        ATTRIB_DECOR = 0x40
    };
    enum Decor { NO_DECOR = 0 };
};

namespace FbPager {

FluxboxHandler *FluxboxHandler::s_handler = 0;

FluxboxHandler::FluxboxHandler():
    m_fbatoms(FbTk::App::instance()->display()) {
    s_handler = this;
}

void FluxboxHandler::setFocus(FbTk::FbWindow &client) {
    FbRootWindow root(client.screenNumber());
    sendClientMessage(root, client,
                      m_fbatoms.getFluxboxChangeWindowFocusAtom(), 0);
}

void FluxboxHandler::sendToWorkspace(FbTk::FbWindow &client, int workspace) {
    FbRootWindow root(client.screenNumber());
    sendClientMessage(root, client,
                      m_fbatoms.getFluxboxChangeAttributesAtom(),
                      FluxboxHints::ATTRIB_WORKSPACE,
                      0,
                      workspace);
}

bool FluxboxHandler::clientMessage(Pager &pager, XClientMessageEvent &event) {
    const FbAtoms &fbatoms = *FbAtoms::instance();
    if (event.data.l[0] == fbatoms.getFluxboxNotifyWorkspaceCountAtom()) {
        pager.updateWorkspaceCount(event.data.l[1]);
        return true;
    } else if (event.data.l[0] == fbatoms.getFluxboxNotifyCurrentWorkspaceAtom()) {
        pager.setCurrentWorkspace(event.data.l[1]);
        return true;
    } else if (event.data.l[0] == fbatoms.getFluxboxNotifyWindowAddAtom()) {
        pager.addWindow(event.data.l[1], event.data.l[2]);
        return true;
    } else if (event.data.l[0] == fbatoms.getFluxboxNotifyWindowDelAtom()) {
        pager.removeWindow(event.data.l[1]);
        return true;
    } else if (event.data.l[0] == fbatoms.getFluxboxNotifyWindowRaiseAtom()) {
        pager.raiseWindow(event.data.l[1]);
        return true;
    } else if (event.data.l[0] == fbatoms.getFluxboxNotifyWindowLowerAtom()) {
        pager.lowerWindow(event.data.l[1]);
        return true;
    } else if (event.data.l[0] == fbatoms.getFluxboxNotifyWindowFocusAtom()) {
        pager.setFocusedWindow(event.data.l[1]);
        return true;
    }

    return false;
}

void FluxboxHandler::changeWorkspace(int screen_num, int workspace) {
    FbRootWindow root(screen_num);
    sendClientMessage(root, root, FbAtoms::instance()->getFluxboxChangeWorkspaceAtom(), workspace);    
}

void FluxboxHandler::setHints(FbTk::FbWindow &win, WindowHint &hints) {
    FluxboxHints net_hints;
    net_hints.decoration = (hints.flags() & WindowHint::WHINT_NO_DECOR) ? FluxboxHints::NO_DECOR : 1;
    net_hints.attrib = (hints.flags() & WindowHint::WHINT_STICKY) ? FluxboxHints::ATTRIB_STICK : 0;
    // use Decoration and sticky flag
    net_hints.flags = FluxboxHints::ATTRIB_DECOR | FluxboxHints::ATTRIB_STICK; 
    Atom hint_atom = m_fbatoms.getFluxboxHintsAtom();
    win.changeProperty(hint_atom, hint_atom,
                       32, PropModeReplace,
                       (unsigned char *)(&net_hints),
                       5); // number of fluxbox hints in the structure
}

void FluxboxHandler::getHints(const FbTk::FbWindow &win, WindowHint &hint) const {
    Atom real_type;
    int format;
    unsigned long num_elements, not_used;
    FluxboxHints *win_hints;

    if (!win.property(m_fbatoms.getFluxboxAttributesAtom(), 0L, 
                     5,  // five elements in our structure
                     false, 
                     m_fbatoms.getFluxboxAttributesAtom(), 
                     &real_type, &format, &num_elements, &not_used,
                     (unsigned char**)&win_hints) || 
        num_elements != 5) // must have five elements
        return; // failure

    hint.setWorkspace(win_hints->workspace);

    // get shaded state
    bool shaded = false;
    if (win_hints->flags & FluxboxHints::ATTRIB_SHADED)
        hint.add(WindowHint::WHINT_SHADED);

    // get sticky state
    bool sticky = false;
    if (win_hints->flags & FluxboxHints::ATTRIB_STICK)
        hint.add(WindowHint::WHINT_STICKY);

    // get icon state
    long *state;
    bool iconified = false;
    if (!win.property(m_fbatoms.getWMStateAtom(), 0, 1,
                      False, m_fbatoms.getWMStateAtom(),
                      &real_type, &format, &num_elements, &not_used,  
                      (unsigned char**)&state) || state == 0)
        cerr<<"Warning: Couldn't get WM_STATE property"<<endl;
    else if (state[0] == IconicState)
        hint.add(WindowHint::WHINT_ICONIC);

    return;
}

int FluxboxHandler::numberOfWorkspaces(int screen_num) const {
    Atom real_type;
    int format;
    unsigned long num_elements, not_used;
    unsigned char *data = 0;
    Display *disp = FbTk::App::instance()->display();
    if (XGetWindowProperty(disp, RootWindow(disp, screen_num), 
                           m_fbatoms.getFluxboxNotifyWorkspaceCountAtom(), 0, 1,
                           False, 0,
                           &real_type, &format, &num_elements, &not_used,  
                           (unsigned char**)&data) == Success) {
        if (data != 0)
            return static_cast<int>(*data);
    }

    return 0;
}

} // end namespace FbPager
