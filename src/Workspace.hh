// Workspace.hh for FbPager
// Copyright (c) 2003 Henrik Kinnunen (fluxgen at users.sourceforge.net)
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

#ifndef FBPAGER_WORKSPACE_HH
#define FBPAGER_WORKSPACE_HH

#include "FbTk/FbWindow.hh"
#include "FbTk/FbPixmap.hh"
#include "FbTk/NotCopyable.hh"
#include "FbTk/EventHandler.hh"
#include "FbTk/Color.hh"

#include "ClientWindow.hh"

#include <string>
#include <map>
#include <memory>

namespace FbPager {

/**
   Workspace window for fbpager
*/
class Workspace:private FbTk::NotCopyable {
public:

    /// create a workspace with specified name, size, parent and eventhandler
    Workspace(FbTk::EventHandler &evh, 
              FbTk::FbWindow &parent, unsigned int width, unsigned int height, 
              const FbTk::Color &focused_win_color,
              const FbTk::Color &wincolor, 
              const FbTk::Color &border_color,
              const FbTk::Color &background_color, 
              const char *name = 0,
              const bool use_pixmap = true,
              const int window_border_width = 1);
    /// destructor
    ~Workspace();
    /// set new name for this workspace
    void setName(const char *name) { m_name = (name ? name : ""); }
    /// add a window to workspace, and associate a FbWindow to it
    void add(Window win);
    void resize(unsigned int width, unsigned int height);
    /// remove a window from workspace
    void remove(Window win);
    void shadeWindow(Window win);
    void unshadeWindow(Window win);
    void iconifyWindow(Window win);
    void deiconifyWindow(Window win);
    void raiseWindow(Window win);
    void lowerWindow(Window win);
    void updateGeometry(Window win);
    void updateFocusedWindow();
    void setWindowColor(const std::string &focused,
                        const std::string &unfocused,
                        const std::string &bordercolor_str);
    void setAlpha(unsigned char alpha);
    void clearWindows();

    ClientWindow findClient(const FbTk::FbWindow &win) const;

    /**
       @brief Find associated FbWindow to win
       @return 0 on failure
    */
    FbTk::FbWindow *find(Window win);
    /// find associated FbWindow to win
    const FbTk::FbWindow *find(Window win) const;
    /// @return workspace name
    const std::string &name() const { return m_name; }
    /// @return the workspace's FbWindow 
    const FbTk::FbWindow &window() const { return m_window; }
    /// @return the workspace's FbWindow 
    FbTk::FbWindow &window() { return m_window; }

    static void setFocusedWindow(Window win) { s_focused_window = win; }

private:

    void updateBackground(Window win, const FbTk::Color& bg_color);
    
    std::string m_name; ///< name of this workspace
    FbTk::FbWindow m_window; ///< FbWindow of this workspace
    FbTk::EventHandler &m_eventhandler;

    typedef std::map<Window, FbTk::FbWindow*> WindowList;
    WindowList m_windowlist; ///< holds a set of Window to FbWindow association

    FbTk::Color m_window_color, m_focused_window_color, m_window_bordercolor;
    FbTk::FbWindow *m_focused_window;

    static Window s_focused_window;

    bool m_use_pixmap;
    int  m_window_border_width;
    
};

}; // end namespace FbPager

#endif // FBPAGER_WORKSPACE_HH
