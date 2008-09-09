// FbPager.hh for FbPager
// Copyright (c) 2003-2007 Henrik Kinnunen (fluxgen at fluxbox org)
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

#ifndef FBPAGER_HH
#define FBPAGER_HH

#include "FbTk/FbWindow.hh"
#include "FbTk/EventHandler.hh"
#include "FbTk/Resource.hh"
#include "FbTk/NotCopyable.hh"

#include "ClientWindow.hh"
#include "FbRootWindow.hh"
#include "Pager.hh"

#include <vector>
#include <list>
#include <set>

namespace FbPager {

class Workspace;
class ClientHandler;

/// Shows a pager window with all workspaces and their windows
class FbPager:public FbTk::EventHandler, public Pager, private FbTk::NotCopyable {
public:
    enum Alignment { LEFT_TO_RIGHT, TOP_TO_BOTTOM};
    FbPager(int screen_num, bool withdraw, 
            bool show_resources,
            int layer_flags,
            const char *resource_filename=0);
    ~FbPager();
    void hide();
    void show();

    /// NOT IMPLEMENTED!
    void toggleWithdrawn();
    /**
      @name event handlers
     */
    //@{
    void handleEvent(XEvent &event);
    void buttonPressEvent(XButtonEvent &event);	
    void buttonReleaseEvent(XButtonEvent &event);	
    void motionNotifyEvent(XMotionEvent &event);
    void exposeEvent(XExposeEvent &event);
    //@}

    /// add a window to workspace
    void addWindow(Window win, unsigned int workspace_num);
    void moveToWorkspace(Window win, unsigned int workspace_num);
    void setFocusedWindow(Window win);
    /// remove a window from workspace
    void removeWindow(Window win);
    void raiseWindow(Window win);
    void lowerWindow(Window win);
    /// update number of workspaces
    void updateWorkspaceCount(unsigned int num);
    void setCurrentWorkspace(unsigned int num);
    bool haveWindow(Window win) {
        if (m_windows.find(win) == m_windows.end())
            return false;
        return true;
    }
private:
    void clientMessageEvent(XClientMessageEvent &event);
    void propertyEvent(XPropertyEvent &event);
    void clearWindows();


    void alignWorkspaces();
    
    void sendChangeToWorkspace(unsigned int num);
    void updateWindowHints(Window win);
    void load(const std::string &filename);

    FbRootWindow m_rootwin;
    FbTk::FbWindow m_window;

    typedef std::vector<Workspace *> WorkspaceList;
    typedef std::list<ClientHandler *> ClientHandlerList;
    typedef std::set<Window> WindowList;

    ClientHandlerList m_clienthandlers;
    WorkspaceList m_workspaces;
    WindowList m_windows;
    
    unsigned int m_curr_workspace;

    struct MoveWindow {
        MoveWindow():client(0), curr_window(0), curr_workspace(0) { }
        ClientWindow client;
        FbTk::FbWindow *curr_window;
        Workspace* curr_workspace;
    } m_move_window;

    int m_grab_x, m_grab_y;

    FbTk::ResourceManager m_resmanager;
    FbTk::Resource<int> m_alpha;
    FbTk::Resource<int> m_start_x, m_start_y;
    FbTk::Resource<int> m_workspace_width, m_workspace_height;
    FbTk::Resource<int> m_workspaces_per_row;
    FbTk::Resource<std::string> m_layer;
    FbTk::Resource<bool> m_follow_drag;
    FbTk::Resource<bool> m_follow_move;
    FbTk::Resource<std::string> m_follow_scale;
    FbTk::Resource<std::string> m_change_workspace_button;
    FbTk::Resource<std::string> m_raise_window_button;
    FbTk::Resource<std::string> m_lower_window_button;
    FbTk::Resource<std::string> m_close_window_button;
    FbTk::Resource<std::string> m_exit_button;
    FbTk::Resource<std::string> m_next_workspace_button;
    FbTk::Resource<std::string> m_prev_workspace_button;
    FbTk::Resource<int>         m_move_in_workspace_button;
    FbTk::Resource<int>         m_drag_to_workspace_button;
    
    FbTk::Resource<Alignment>   m_align;
    FbTk::Resource<std::string> m_color_str;
    FbTk::Resource<std::string> m_window_color_str, m_focused_window_color_str;
    FbTk::Resource<std::string> m_window_bordercolor_str;
    FbTk::Resource<std::string> m_background_color_str;
    FbTk::Resource<std::string> m_current_background_color_str;
    FbTk::Resource<int>         m_multiclick_time;
    FbTk::Resource<bool>        m_window_icons;
    FbTk::Resource<int>         m_window_border_width;
    FbTk::Resource<int> m_workspace_border_width;
    FbTk::Resource<int> m_workspace_border_inactive_width;
    FbTk::Resource<std::string> m_workspace_border_active_color,
        m_workspace_border_inactive_color;
    std::string                 m_button_queue;
    XButtonEvent                m_last_button_event;
    int                         m_last_workspace_num;
    
};

} // end namespace FbPager

#endif //FBPAGER_HH
