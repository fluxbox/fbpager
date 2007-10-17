// FbPager.cc for FbPager
// Copyright (c) 2003-2004 Henrik Kinnunen (fluxgen at users.sourceforge.net)
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

// $Id$

#include "FbPager.hh"

// fbtk utils
#include "FbTk/EventManager.hh"
#include "FbTk/App.hh"
#include "FbTk/Color.hh"
#include "FbTk/StringUtil.hh"

// client handlers
#include "Ewmh.hh"
#include "FluxboxHandler.hh"

#include "Workspace.hh"
#include "WindowHint.hh"
#include "FbAtoms.hh"
#include "ScaleWindowToWindow.hh"
#include "Resources.hh"
#include "RefBinder.hh"

// x headers
#include <X11/Xatom.h>
#include <X11/Xutil.h>

// std headers
#include <iostream>
#include <cstdio>
#include <cassert>
#include <functional>
#include <algorithm>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

// copied from fluxbox code
#ifdef HAVE_SSTREAM
#include <sstream>
#define FB_istringstream istringstream
#define FB_ostringstream ostringstream
#elif HAVE_STRSTREAM 
#include <strstream>
#define FB_istringstream istrstream
#define FB_ostringstream ostrstream
#else
#error "You dont have sstream or strstream headers!"
#endif // HAVE_STRSTREAM

using namespace std;

namespace {

int handleXErrors(Display *disp, XErrorEvent *xerror) {
    return 0;
}

template <typename Container>
void destroyList(Container &c) {
    while (!c.empty()) {
        delete c.back();
        c.pop_back();
    }
}

// TODO: is this optimal? i dont think so, but it works so far
bool compareButtonQueues(std::string q1, std::string q2) {

    FbTk::StringUtil::removeFirstWhitespace(q1);
    FbTk::StringUtil::removeFirstWhitespace(q2);
    FbTk::StringUtil::removeTrailingWhitespace(q1);
    FbTk::StringUtil::removeTrailingWhitespace(q2);
    
    return q1 == q2;
}


// little helper to keep  win  visible in  bigwin 
void respectConstraints(int& x, int& y,
                        const FbTk::FbWindow& win, const FbTk::FbWindow& bigwin) {

    const int left = - static_cast<int>((3 * win.width()) / 4);
    const int right = bigwin.width() - win.width() / 4;
    const int top =  - static_cast<int>((3 * win.height()) / 4);
    const int bottom = bigwin.height() - win.height() / 4;

    // make sure window isnt dragged out of visible area of workspace
    if ( x < left || x > right )
        x = win.x();
    if ( y < top || y > bottom )
        y = win.y();
}

} // end anonymous namespace


namespace FbPager {


FbPager::FbPager(int screen_num, bool withdraw, 
                 bool use_fbhandler,
                 bool use_ewmhhandler,
                 bool show_resources,
                 int layer_flag,
                 const char *resource_filename):
    m_rootwin(screen_num),
    m_window(m_rootwin,
             0, 0, // pos
             64, 64,  // size
             // event mask
             ButtonPressMask | ButtonReleaseMask | ExposureMask |
             FocusChangeMask | StructureNotifyMask|
             SubstructureRedirectMask),
    m_curr_workspace(0),
    m_move_window(),
    m_resmanager(0, false),
    m_alpha(m_resmanager, 255,
            "fbpager.alpha",
            "FbPager.Alpha"),
    m_start_x(m_resmanager, 0,
              "fbpager.x",
              "FbPager.x"),
    m_start_y(m_resmanager, 0,
              "fbpager.y",
              "FbPager.y"),
    m_workspace_width(m_resmanager, 64,
                      "fbpager.workspace.width",
                      "FbPager.Workspace.Width"),
    m_workspace_height(m_resmanager, 64,
                       "fbpager.workspace.height",
                       "FbPager.Workspace.Height"),
    m_workspaces_per_row(m_resmanager, 6400, // big number...
                         "fbpager.workspacesPerRow",
                         "FbPager.WorkspacesPerRow"),
    m_layer(m_resmanager, "normal", // normal layer
            "fbpager.layer",
            "FbPager.Layer"),
    m_follow_drag(m_resmanager, false,
                  "fbpager.followDrag",
                  "FbPager.FollowDrag"),
    m_follow_move(m_resmanager, false,
                  "fbpager.followMove",
                  "FbPager.FollowMove"),
    m_follow_scale(m_resmanager, "height",
                   "fbpager.followScale",
                   "FbPager.followScale"),
    m_change_workspace_button(m_resmanager, "1", 
                              "fbpager.changeWorkspaceButton", 
                              "FbPager.ChangeWorkspaceButton"),
    m_raise_window_button(m_resmanager, "2", 
                          "fbpager.raiseWindowButton", 
                          "FbPager.RaiseWindowButton"),
    m_lower_window_button(m_resmanager, "3", 
                          "fbpager.lowerWindowButton", 
                          "FbPager.LowerWindowButton"),
    m_close_window_button(m_resmanager, "3 3 1",
                          "fbpager.closeWindowButton",
                          "FbPager.CloseWindowButton"),
    m_exit_button(m_resmanager, "1 3 3",
                  "fbpager.exitButton",
                  "FbPager.ExitButton"),
    m_next_workspace_button(m_resmanager, "4",
                            "fbpager.nextWorkspaceButton",
                            "FbPager.NextWorkspaceButton"),
    m_prev_workspace_button(m_resmanager, "5",
                            "fbpager.prevWorkspaceButton",
                            "FbPager.PrevWorkspaceButton"),
    m_move_in_workspace_button(m_resmanager, 2,
                               "fbpager.moveInWorkspaceButton",
                               "FbPager.MoveInWorkspaceButton"),
    m_drag_to_workspace_button(m_resmanager, 3,
                               "fbpager.dragToWorkspaceButton",
                               "FbPager.DragToWorkspaceButtton"),
    m_align(m_resmanager, LEFT_TO_RIGHT, 
            "fbpager.align", 
            "FbPager.Align"),
    m_color_str(m_resmanager, "white",
                "fbpager.color",
                "FbPager.Color"),
    m_window_color_str(m_resmanager, "white", 
                       "fbpager.windowColor", 
                       "FbPager.WindowColor"),
    m_focused_window_color_str(m_resmanager, "white",
                               "fbpager.focusedWindowColor",
                               "FbPager.FocusedWindowColor"),
    m_window_bordercolor_str(m_resmanager, "black", 
                             "fbpager.windowBorderColor", 
                             "FbPager.WindowBorderColor"),
    m_background_color_str(m_resmanager, "darkgray", 
                           "fbpager.backgroundColor", 
                           "FbPager.BackgroundColor"),
    m_current_background_color_str(m_resmanager, "lightgray", 
                                   "fbpager.currentBackgroundColor", 
                                   "FbPager.CurrentBackgroundColor"),
    m_multiclick_time(m_resmanager, 250,
                      "fbpager.multiClickTime",
                      "FbPager.MultiClickTime"),
    m_window_icons(m_resmanager, false,
                   "fbpager.icons",
                   "FbPager.Icons"),
    m_window_border_width(m_resmanager, 1,
                          "fbpager.windowBorderWidth",
                          "FbPager.WindowBorderWidth"),
    m_workspace_border_width(m_resmanager, 1,
                             "fbpager.workspace.border.active.width",
                             "FbPager.Workspace.Border.active.Width"),
    m_workspace_border_inactive_width(m_resmanager, 1,
                                      "fbpager.workspace.border.inactive.width",
                                      "FbPager.Workspace.Border.Inactive.Width"),
    m_workspace_border_active_color(m_resmanager, "darkgreen",
                                    "fbpager.border.active.color",
                                    "FbPager.Border.Active.Color"),
    m_workspace_border_inactive_color(m_resmanager, "white",
                                      "fbpager.border.inactive.color",
                                      "FbPager.Border.Inactive.Color"),
    m_last_workspace_num(-1) {


    if (use_fbhandler)
        m_clienthandlers.push_back(new FluxboxHandler());

    if (use_ewmhhandler) 
        m_clienthandlers.push_back(new Ewmh());

    XSetErrorHandler(handleXErrors);

    FbTk::EventManager::instance()->add(*this, m_window);

    // setup client window   
    m_window.setBackgroundColor(FbTk::Color((*m_color_str).c_str(), 
                                            m_window.screenNumber()));
    m_window.setAlpha(*m_alpha);


    Display *disp = FbTk::App::instance()->display();
    Atom wmproto[2];
    int wmprot_size = 1;
    wmproto[0] = XInternAtom(disp, "WM_DELETE_WINDOW", False);
    if (FluxboxHandler::handler() != 0) {
        wmprot_size = 2;
        wmproto[1] = FbAtoms::instance()->getFluxboxStructureMessagesAtom();
    }
    XSetWMProtocols(disp, m_window.window(), wmproto, wmprot_size);


    XWMHints wmhints;

    if (!withdraw) {
      
        wmhints.initial_state = NormalState;
    } else // withdraw
        wmhints.initial_state = WithdrawnState;


    // setup no input
    wmhints.input = False;
    // set which hints we're using
    wmhints.flags = StateHint | InputHint;


    // set window name
    std::string name = "FbPager";
    char *name_prop = &name[0];
    XTextProperty windowname;
    XStringListToTextProperty(&name_prop, 1, &windowname);

    // setup class hints
    XClassHint classhints;
    string res_name = "fbpager";
    string res_class = "FbPager";
    classhints.res_name = &res_name[0];
    classhints.res_class = &res_class[0];
    // set window properties
    XSetWMProperties(disp, m_window.window(), &windowname, 0, 0, 0,
                     0, &wmhints, &classhints);

    XFree( windowname.value );

    if (resource_filename != 0)
        load(resource_filename);


    int real_layer_flag = layer_flag;

    // no override from command line?
    if ( real_layer_flag == -1 ) {
        // use resource value
        if ( *m_layer == "bottom" ) {
            real_layer_flag = 0;
        } else if ( *m_layer == "top" ) {
            real_layer_flag = 2;
        }
    }

    int extra_flags = 0;    
    // now setup extra flags
    if ( real_layer_flag != -1 ) {
        if (real_layer_flag == 2) {
            extra_flags = WindowHint::WHINT_LAYER_TOP;
        } else if (real_layer_flag == 0) {
            extra_flags = WindowHint::WHINT_LAYER_BOTTOM;
        }
    }

    // setup some extra wm hints
    if (!withdraw) {
        WindowHint hint;
        hint.add(WindowHint::WHINT_NO_DECOR | WindowHint::WHINT_SKIP_TASKBAR |
                 WindowHint::WHINT_STICKY | WindowHint::WHINT_SKIP_PAGER | 
                 WindowHint::WHINT_HIDDEN | extra_flags );
        ClientHandlerList::iterator it = m_clienthandlers.begin();
        ClientHandlerList::iterator it_end = m_clienthandlers.end();
        for (; it != it_end; ++it)
            (*it)->setHints(m_window, hint);
    }

    // setup workspace count
    ClientHandlerList::iterator it = m_clienthandlers.begin();
    ClientHandlerList::iterator it_end = m_clienthandlers.end();
    int workspaces = 0;
    for (; it != it_end; ++it) {
        workspaces = max((*it)->numberOfWorkspaces(m_window.screenNumber()), 
                         workspaces);
    }

    updateWorkspaceCount(workspaces);
    // finaly show window
    show();
    m_window.move(*m_start_x, *m_start_y);
    
    m_window.resize(m_window.width(), m_window.height());

    m_last_button_event.time= 0;

    if (show_resources) {
        // dump resources to stdout
        m_resmanager.dump(true);
    }
}

FbPager::~FbPager() {
    destroyList(m_clienthandlers);
    destroyList(m_workspaces);
}

void FbPager::show() {
    m_window.show();
}

void FbPager::hide() {
    m_window.hide();
}

void FbPager::handleEvent(XEvent &event) {
    // here we handle events that're not in FbTk::EventHandler interface

    if (event.type == ClientMessage)
        clientMessageEvent(event.xclient);
    else if (event.type == ConfigureNotify) {
        if (event.xconfigure.window != m_window.window()) {
            updateWindowHints(event.xconfigure.window);
        } else if (event.xconfigure.send_event == True){
            if (event.xconfigure.x != m_window.x() ||
                event.xconfigure.y != m_window.y() ||
                event.xconfigure.width != m_window.width() ||
                event.xconfigure.height != m_window.height()) {

                m_window.updateGeometry(event.xconfigure.x,
                                        event.xconfigure.y,
                                        event.xconfigure.width, 
                                        event.xconfigure.height);

                clearWindows(); // update transparency
            }

        }
    } else if (event.type == PropertyNotify) {
        propertyEvent(event.xproperty);
    } else {

    }
}

void FbPager::buttonPressEvent(XButtonEvent &event) {
    
    if ( event.time - m_last_button_event.time > 1000 )
        m_button_queue= "";
   
    FB_ostringstream os;

    // double click
    if (event.time - m_last_button_event.time < *m_multiclick_time &&
        event.button == m_last_button_event.button)
        os << event.button;
    else
        os << " " << event.button;

    // append button string to current queue
    m_button_queue += os.str();

    // save last event so we can compare it to next for double click
    m_last_button_event = event;


    if (event.button == *m_move_in_workspace_button ||
        event.button == *m_drag_to_workspace_button ) {
      
        m_move_window.curr_window = 0;
        m_move_window.client = ClientWindow(0);

        FbTk::FbWindow *win = 0;
        for (size_t w = 0; w < m_workspaces.size(); ++w) {
            win = m_workspaces[w]->find(event.subwindow);
            if (win && win->parent() != 0) {
                m_move_window.client = m_workspaces[w]->findClient(*win);
                m_move_window.curr_window = win;
                m_move_window.curr_workspace= m_workspaces[w];
                m_grab_x = event.x - win->x();
                m_grab_y = event.y - win->y();

                // drag -> move it above all workspaces
                if (event.button == *m_drag_to_workspace_button) {
                    m_move_window.curr_window->reparent(m_window, 
                                                        m_move_window.curr_window->x() + 
                                                        m_move_window.curr_workspace->window().x(), 
                                                        m_move_window.curr_window->y() + 
                                                        m_move_window.curr_workspace->window().y());
                }
                break;
            }
        }
    }

}

void FbPager::buttonReleaseEvent(XButtonEvent &event) {

    ClientWindow client(0);
    if (m_move_window.curr_window != 0 && 
        *m_move_in_workspace_button == event.button) {

        FbTk::FbWindow &win = *m_move_window.curr_window;
        int client_x = win.x(), client_y = win.y();
        scaleFromWindowToWindow(*(win.parent()), 
                                m_rootwin,
                                client_x, client_y);

        m_move_window.client.move(client_x, client_y);
        client = m_move_window.client;
        m_move_window.client = ClientWindow(0);
        m_move_window.curr_window = 0;

    }

    if (m_move_window.curr_window != 0 && *m_drag_to_workspace_button == event.button) {

        if (m_last_workspace_num >= 0 && 
            m_last_workspace_num < m_workspaces.size()) {
           
            Workspace* workspace = m_workspaces[m_last_workspace_num];

            // let someone else send it to the _new_ workspace
            if (workspace != m_move_window.curr_workspace) {
              
                ClientHandlerList::iterator it = m_clienthandlers.begin();
                ClientHandlerList::iterator it_end = m_clienthandlers.end();
                for (; it != it_end; ++it) {
                    (*it)->sendToWorkspace(m_move_window.client, m_last_workspace_num);
                }
            }

            // drop it to the last workspace, the mouse was over
            m_move_window.curr_window->reparent(workspace->window(), 
                                                m_move_window.curr_window->x() - 
                                                workspace->window().x(),
                                                m_move_window.curr_window->y() - 
                                                workspace->window().y());
                   
        }
        else {

            m_move_window.curr_window->reparent(m_move_window.curr_workspace->window(),
                                                m_grab_x, 
                                                m_grab_y);
        }
 
        // move it to the right position
        FbTk::FbWindow &win = *m_move_window.curr_window;
        int client_x = win.x(), client_y = win.y();
        scaleFromWindowToWindow(*(win.parent()), 
                                m_rootwin,
                                client_x, client_y);

        m_move_window.client.move(client_x, client_y);

        // clean up 
        m_move_window.client = ClientWindow(0);
        m_move_window.curr_window = 0;
        m_move_window.curr_workspace = 0;
        m_button_queue.erase();

    } else if (compareButtonQueues(m_button_queue, *m_change_workspace_button)) {
        // change workspace

        // find out which workspace
        Workspace *workspace = 0;
        FbTk::FbWindow *win = 0;
        size_t workspace_it = 0;
        for (; workspace_it < m_workspaces.size(); ++workspace_it) {
            if (m_workspaces[workspace_it]->window() == event.window) {
                workspace = m_workspaces[workspace_it];
                break;
            } else {
                win = m_workspaces[workspace_it]->find(event.window);
                if (win != 0) {
                    workspace = m_workspaces[workspace_it];
                    break;
                }
            }
        }
        
        if (workspace != 0)
            sendChangeToWorkspace(workspace_it);

    } else if (compareButtonQueues(m_button_queue, *m_raise_window_button) &&
               client.window() != 0) {
        // raise, currently the same as focus 

        for_each(m_clienthandlers.begin(),
                 m_clienthandlers.end(),
                 RefBind(&ClientHandler::setFocus, client));

        client.raise();
    } else if (compareButtonQueues(m_button_queue, *m_lower_window_button) &&
               client.window() != 0) {
        client.lower();
    } else if (compareButtonQueues(m_button_queue, *m_close_window_button) &&
               client.window() != 0) {
        for_each(m_clienthandlers.begin(),
                 m_clienthandlers.end(),
                 RefBind(&ClientHandler::closeWindow, client));
    } else if (compareButtonQueues(m_button_queue, *m_exit_button)) {
        // exit
        FbTk::App::instance()->end();
    } else if (compareButtonQueues(m_button_queue, *m_next_workspace_button)) {    
        // next workspace
        sendChangeToWorkspace(m_curr_workspace < m_workspaces.size() - 1 ? 
                              m_curr_workspace + 1 : 0);
    } else if (compareButtonQueues(m_button_queue, *m_prev_workspace_button)) {
        // prev workspace
        sendChangeToWorkspace(m_curr_workspace > 0 ? m_curr_workspace - 1 : 
                              m_workspaces.size() - 1);
    } else {
        // ok, we didn't find any queue that match 
        // so we return without clearing the current queue
        return; 
    }

    m_button_queue.erase();
}

void FbPager::motionNotifyEvent(XMotionEvent &event) {

    if (m_move_window.curr_window != 0 &&  
        m_move_window.curr_workspace != 0) {

        Workspace* workspace= 0;
        size_t w = 0;
        size_t old_workspace= m_last_workspace_num;
        int left, top, right, bottom, x, y;

        // find out, on which workspace the motion started and
        // on which workspace we are now
        for(; w < m_workspaces.size(); ++w) {
            
            if (m_workspaces[w]->window() == event.window)
                workspace= m_workspaces[w];

            left = m_workspaces[w]->window().x();
            top = m_workspaces[w]->window().y();
            right = left + m_workspaces[w]->window().width();
            bottom = top + m_workspaces[w]->window().height();

            // m_window - coordinate - space
            x = m_move_window.curr_workspace->window().x() + event.x;
            y = m_move_window.curr_workspace->window().y() + event.y;
               
            // inside the new workspace?
            if ( left <= x && x <= right && top <= y && y <= bottom ) 
                m_last_workspace_num= w;
        }
              
        int newx = event.x - m_grab_x;
        int newy = event.y - m_grab_y;

        // just move in current workspace
        if (m_last_button_event.button == *m_move_in_workspace_button) {

            respectConstraints(newx, newy, *m_move_window.curr_window, workspace->window());
            m_move_window.curr_window->move(newx, newy);

            if (*m_follow_move) {

                FbTk::FbWindow &win = *m_move_window.curr_window;
                int client_x = win.x(), client_y = win.y();
                scaleFromWindowToWindow(*(win.parent()), 
                                        m_rootwin,
                                        client_x, client_y);
                m_move_window.client.move(client_x, client_y);
            }
            
        // drag between the workspaces
        } else if (m_last_button_event.button == *m_drag_to_workspace_button) {

            newx += m_move_window.curr_workspace->window().x();
            newy += m_move_window.curr_workspace->window().y();
            
            respectConstraints(newx, newy, *m_move_window.curr_window, m_window);

            m_move_window.curr_window->move(newx, newy);

            if (*m_follow_drag && m_last_workspace_num != old_workspace)
                sendChangeToWorkspace(m_last_workspace_num);
        }
    }
}

void FbPager::exposeEvent(XExposeEvent &event) {
    if (m_window == event.window) {
        m_window.clearArea(event.x, event.y,
                           event.width, event.height);
    } else {
        for (size_t w = 0; w < m_workspaces.size(); ++w) {
            if (m_workspaces[w]->window() == event.window) {
                FbTk::FbWindow &win = m_workspaces[w]->window();
                win.clearArea(event.x, event.y,
                              event.width, event.height);
                return;
            } else {
                FbTk::FbWindow *win = m_workspaces[w]->find(event.window);
                if (win != 0) {
                    win->clearArea(event.x, event.y,
                                   event.width, event.height);
                    return;
                }
            }
        }
    }
}

void FbPager::clientMessageEvent(XClientMessageEvent &event) {

    ClientHandlerList::iterator it = m_clienthandlers.begin();
    ClientHandlerList::iterator it_end = m_clienthandlers.end();
    for (; it != it_end; ++it) {
        if ((*it)->clientMessage(*this, event))
            break;
    }

}

void FbPager::propertyEvent(XPropertyEvent &event) {
    updateWindowHints(event.window);
}

void FbPager::addWindow(Window win, unsigned int workspace_num) {
    if (workspace_num >= m_workspaces.size())
        return;

    m_windows.insert(win);

    m_workspaces[workspace_num]->add(win);
    // update window mode
    updateWindowHints(win);
    
}

void FbPager::moveToWorkspace(Window win, unsigned int w) {
    if (w >= 0x7FFFFFFF) {
        updateWindowHints(w);
        return;
    } else if (w >= m_workspaces.size())
        return;

    // remove from old workspace
    for (size_t workspace=0; workspace < m_workspaces.size(); ++workspace) {
        if (w != workspace && m_workspaces[workspace]->find(win) != 0)
            m_workspaces[workspace]->remove(win);
    }

    addWindow(win, w);
}

void FbPager::setFocusedWindow(Window win) {
    Workspace::setFocusedWindow(win);
    for_each(m_workspaces.begin(),
             m_workspaces.end(),
             mem_fun(&Workspace::updateFocusedWindow));
}

void FbPager::removeWindow(Window win) {
    for_each(m_workspaces.begin(),
             m_workspaces.end(),
             bind2nd(mem_fun(&Workspace::remove), win));
    FbTk::EventManager::instance()->remove(win);
    m_windows.erase(win);
}

void FbPager::updateWorkspaceCount(unsigned int num) {
    if (num == 0 || num == m_workspaces.size())
        return;

    if (m_workspaces.size() < num) {
        // setup proper workspace scale
        float screenScale = (float)m_rootwin.width() / m_rootwin.height();
        unsigned int width = *m_workspace_width;
        unsigned int height = *m_workspace_height;
        if ( *m_follow_scale == "height" ) {
            width *= screenScale;
        } else if ( *m_follow_scale == "width" ) {
            height *= screenScale;
        } 
        // add some workspaces until we match number of workspaces
        FbTk::Color wincolor(m_window_color_str->c_str(), m_window.screenNumber());
        FbTk::Color focusedcolor(m_focused_window_color_str->c_str(), m_window.screenNumber());
        FbTk::Color bordercol(m_window_bordercolor_str->c_str(), m_window.screenNumber());
        FbTk::Color backgroundcol(m_background_color_str->c_str(), m_window.screenNumber());

        while (m_workspaces.size() != num) {
            Workspace *w = new Workspace(*this, m_window, 
                                         width, height,
                                         focusedcolor,
                                         wincolor,
                                         bordercol,
                                         backgroundcol,
                                         "workspace",
                                         *m_window_icons,
                                         *m_window_border_width);
            w->setAlpha(*m_alpha);
            w->window().setBorderWidth(*m_workspace_border_width);
            m_workspaces.push_back(w);
        }
    } else { // remove some workspace until we match number or workspaces

        while (m_workspaces.size() != num) {
            delete m_workspaces.back();
            m_workspaces.pop_back();
            // extra precaution
            if (m_workspaces.empty())
                break;
        }
    }

    // update size of our main window and align workspaces
    alignWorkspaces();
    m_window.showSubwindows();
}

void setupWindow( FbTk::FbWindow &window,
                  const std::string &bg_color,
                  const std::string &border_color,
                  unsigned int border_width ) {

    window.setBackgroundColor(FbTk::Color(bg_color.c_str(), 
                                          window.screenNumber()));

    window.setBorderColor(FbTk::
                          Color(border_color.c_str(),
                                window.screenNumber()));

    window.setBorderWidth(border_width);
    window.clear();

}

void FbPager::setCurrentWorkspace(unsigned int num) {
    if (num >= m_workspaces.size() || num == m_curr_workspace)
        return; 

    // set current workspaces background to "normal" background, 
    FbTk::FbWindow &oldWindow = m_workspaces[m_curr_workspace]->window();
    setupWindow(oldWindow,
                *m_background_color_str,
                *m_workspace_border_inactive_color,
                *m_workspace_border_inactive_width);
    
    // set next workspace background
    // update workspace background
    FbTk::FbWindow& newWindow = m_workspaces[num]->window();
    setupWindow(newWindow,
                *m_current_background_color_str,
                *m_workspace_border_active_color,
                *m_workspace_border_width);
    
    newWindow.raise();

    if (*m_workspace_border_inactive_width == 0) {
        oldWindow.move( oldWindow.x(),
                        oldWindow.y() );
    }
    
    m_curr_workspace = num;
}

/**
   Setup workspaces position and size in our main window
*/
void FbPager::alignWorkspaces() {

    const int bevel = *m_workspace_border_width;
    int diff_x = 0;
    int diff_y = 0;
    // assumes at least one workspace
    const FbTk::FbWindow& workspace_win = m_workspaces[0]->window();
    const unsigned int workspace_width = workspace_win.width();
    const unsigned int workspace_height = workspace_win.height();
    
    // Fix alignment of workspaces
    if (*m_align == LEFT_TO_RIGHT)
        diff_x = workspace_width + bevel;
    else // align TOP_TO_BOTTOM
        diff_y = workspace_height + bevel;

    // resize and reposition all workspaces
    WorkspaceList::iterator it = m_workspaces.begin();
    WorkspaceList::iterator it_end = m_workspaces.end();
    int next_pos_x = 0;
    int next_pos_y = 0;
    int workspace = 0;
    int max_x = 0;
    int max_y = 0;
    for (; it != it_end; 
         ++it, next_pos_y += diff_y, next_pos_x += diff_x, ++workspace) {

        if (workspace >= *m_workspaces_per_row) {            
            workspace = 0;
            if (*m_align == LEFT_TO_RIGHT) {
                next_pos_x = bevel;
                next_pos_y += workspace_height + bevel;
            } else {
                next_pos_y = bevel;
                next_pos_x += workspace_width + bevel;
            }
        }

        if (next_pos_x + workspace_width > max_x)
            max_x = next_pos_x + workspace_width;
        if (next_pos_y + workspace_height > max_y)
            max_y = next_pos_y + workspace_height;

        (*it)->window().move(next_pos_x, next_pos_y);
        (*it)->resize(workspace_width, workspace_height);
    }

    m_window.resize(max_x + 2*bevel, max_y + 2*bevel);

    XSizeHints sizehints;
    sizehints.max_width = sizehints.min_width = m_window.width();
    sizehints.max_height = sizehints.min_height = m_window.height();
    sizehints.flags = PMinSize | PMaxSize;
    XSetWMProperties(FbTk::App::instance()->display(), m_window.window(), 0, 0, 0, 0,
                     &sizehints, 0, 0);
    
    for_each(m_clienthandlers.begin(),
             m_clienthandlers.end(),
             RefBind(&ClientHandler::moveResize, m_window));
    
}


void FbPager::sendChangeToWorkspace(unsigned int num) {
    assert(num < m_workspaces.size());
    
    ClientHandlerList::iterator it = m_clienthandlers.begin();
    ClientHandlerList::iterator it_end = m_clienthandlers.end();
    for (; it != it_end; ++it)
        (*it)->changeWorkspace(m_window.screenNumber(), num);

}

void FbPager::updateWindowHints(Window win) {
    WindowHint hint;
    ClientWindow client(win);
    ClientHandlerList::iterator it = m_clienthandlers.begin();
    ClientHandlerList::iterator it_end = m_clienthandlers.end();
    for (; it != it_end; ++it)
        (*it)->getHints(client, hint);

    for (int workspace = 0; workspace < m_workspaces.size(); workspace++) {
        if (m_workspaces[workspace]->find(win) == 0) {
            // if the window is sticky then add it to this workspace too
            // and if it's not skip_pager nor type_dock
            if ((hint.flags() & WindowHint::WHINT_STICKY) &&
                !(hint.flags() & WindowHint::WHINT_SKIP_PAGER) &&
                !(hint.flags() & WindowHint::WHINT_TYPE_DOCK)) {
                m_workspaces[workspace]->add(win);
            } else
                continue; // normal window without sticky

        } else if (!(hint.flags() & WindowHint::WHINT_STICKY) && 
                   hint.workspace() != workspace ||
                   (hint.flags() & WindowHint::WHINT_SKIP_PAGER) ||
                   (hint.flags() & WindowHint::WHINT_TYPE_DOCK)) {
            // if win not sticky and if it's not suppose to be on this
            // workspace then remove it and try next workspace
            m_workspaces[workspace]->remove(win);  
            continue; // next workspace
        }

        m_workspaces[workspace]->updateGeometry(win);

        if (hint.flags() & WindowHint::WHINT_SHADED) {
            m_workspaces[workspace]->shadeWindow(win);
        } else
            m_workspaces[workspace]->unshadeWindow(win);

        if ((hint.flags() & WindowHint::WHINT_ICONIC) && 
            !(hint.flags() & WindowHint::WHINT_SHADED))
            m_workspaces[workspace]->iconifyWindow(win);
        else if (!(hint.flags() & WindowHint::WHINT_SHADED))
            m_workspaces[workspace]->deiconifyWindow(win);
        
    } // end for
}

void FbPager::raiseWindow(Window win) {
    // raise on all workspace
    for_each(m_workspaces.begin(),
             m_workspaces.end(),
             bind2nd(mem_fun(&Workspace::raiseWindow), win));
}

void FbPager::lowerWindow(Window win) {
    // lower on all workspace
    for_each(m_workspaces.begin(),
             m_workspaces.end(),
             bind2nd(mem_fun(&Workspace::lowerWindow), win));
}

void FbPager::load(const std::string &filename) {
    if (!m_resmanager.load(FbTk::StringUtil::expandFilename(filename).c_str()))
        cerr<<"Failed to load resource file: "<<filename<<endl;

    m_window.setAlpha(*m_alpha);
    m_window.setBackgroundColor(FbTk::Color((*m_color_str).c_str(), m_window.screenNumber()));
    FbTk::Color bgcolor(m_background_color_str->c_str(), m_window.screenNumber());
    for (size_t workspace = 0; workspace < m_workspaces.size(); ++workspace) {
        m_workspaces[workspace]->setAlpha(*m_alpha);
        m_workspaces[workspace]->setWindowColor(*m_focused_window_color_str, 
                                                *m_window_color_str, 
                                                *m_window_bordercolor_str);
        m_workspaces[workspace]->window().setBackgroundColor(bgcolor);
    }

}

void FbPager::clearWindows() {
    m_window.clear();
    for_each(m_workspaces.begin(),
             m_workspaces.end(),
             mem_fun(&Workspace::clearWindows));
}

} // end namespace FbPager

