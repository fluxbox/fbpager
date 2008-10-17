// Workspace.cc for FbPager
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

#include "Workspace.hh"

#include "FbTk/App.hh"
#include "FbTk/Color.hh"
#include "FbTk/EventManager.hh"

#include "ScaleWindowToWindow.hh"
#include "FbRootWindow.hh"

#include <X11/Xutil.h>

#include <iostream>
#include <algorithm>
#include <functional>
using namespace std;

namespace FbPager {

Window Workspace::s_focused_window = 0;

Workspace::Workspace(FbTk::EventHandler &evh,
                     FbTk::FbWindow &parent, unsigned int width, unsigned int height,
                     const FbTk::Color &focused_win_color,
                     const FbTk::Color &wincolor, const FbTk::Color &border_color,
                     const FbTk::Color &background_color,
                     const char *name,
                     const bool use_pixmap,
                     const int window_border_width):
    m_name(name ? name : ""),
    m_window(parent,
             0, 0, // position
             width, height, // size
             ButtonPressMask | ButtonReleaseMask | ExposureMask | ButtonMotionMask),
    m_eventhandler(evh),
    m_window_color(wincolor),
    m_focused_window_color(focused_win_color),
    m_window_bordercolor(border_color),
    m_focused_window(0),
    m_use_pixmap(use_pixmap),
    m_window_border_width(window_border_width) {

    m_window.setBackgroundColor(background_color);
    FbTk::EventManager::instance()->add(evh, m_window);
}

Workspace::~Workspace() {
    WindowList::iterator it = m_windowlist.begin();
    WindowList::iterator it_end = m_windowlist.end();
    for (; it != it_end; ++it) {
        FbTk::EventManager::instance()->remove(*(*it).second);
        FbTk::EventManager::instance()->remove((*it).first);
        delete (*it).second;
    }

    FbTk::EventManager::instance()->remove(m_window);
}

void Workspace::add(Window win) {
    FbTk::FbWindow *fbwin = find(win);
    if (fbwin != 0)
        delete fbwin;


    fbwin = new FbTk::FbWindow(m_window,  // parent
                               0, 0, // pos
                               10, 10, // size
                               // event mask
                               ExposureMask);
    //    fbwin->setAlpha(m_window.alpha());

    XSelectInput(FbTk::App::instance()->display(), win,
                 PropertyChangeMask | // for shade/iconic state
                 StructureNotifyMask // for pos and size
                 );
    // add to event manager
    FbTk::EventManager::instance()->add(m_eventhandler, *fbwin);
    FbTk::EventManager::instance()->add(m_eventhandler, win);

    // add window to list
    m_windowlist[win] = fbwin;

    // update pos and size
    updateGeometry(win);

    fbwin->show();
    fbwin->setBorderWidth(m_window_border_width);
    fbwin->setBorderColor(m_window_bordercolor);

    updateBackground(win, m_window_color);
}

void Workspace::resize(unsigned int width, unsigned int height) {
    m_window.resize(width, height);
    WindowList::iterator it = m_windowlist.begin();
    WindowList::iterator it_end = m_windowlist.end();
    for (; it != it_end; ++it) {
        updateGeometry((*it).first);
    }
}


void Workspace::shadeWindow(Window win) {
    FbTk::FbWindow *fbwin = find(win);
    if (fbwin == 0)
        return;

    fbwin->resize(fbwin->width(), m_window.height()/30);
}

void Workspace::unshadeWindow(Window win) {
    updateGeometry(win);
}

void Workspace::iconifyWindow(Window win) {
    FbTk::FbWindow *fbwin = find(win);
    if (fbwin == 0)
        return;

    fbwin->hide();
}

void Workspace::deiconifyWindow(Window win) {
    FbTk::FbWindow *fbwin = find(win);
    if (fbwin != 0)
        return;
    fbwin->show();
}

void Workspace::lowerWindow(Window win) {
    FbTk::FbWindow *fbwin = find(win);
    if (fbwin == 0)
        return;
    fbwin->lower();
}

void Workspace::raiseWindow(Window win) {
    FbTk::FbWindow *fbwin = find(win);
    if (fbwin == 0)
        return;
    fbwin->raise();
}

void Workspace::setWindowColor(const std::string &focused,
                               const std::string &unfocused,
                               const std::string &bordercolor_str) {
    m_window_color = FbTk::Color(unfocused.c_str(), m_window.screenNumber());
    m_focused_window_color = FbTk::Color(focused.c_str(), m_window.screenNumber());
    m_window_bordercolor = FbTk::Color(bordercolor_str.c_str(), m_window.screenNumber());

    WindowList::iterator it = m_windowlist.begin();
    WindowList::iterator it_end = m_windowlist.end();
    for (; it != it_end; ++it) {
        (*it).second->setBorderColor(m_window_bordercolor);
        updateBackground((*it).first, m_window_color);
    }
    updateFocusedWindow();
}


void Workspace::setAlpha(unsigned char alpha) {
    m_window.setAlpha(alpha);
    WindowList::iterator it = m_windowlist.begin();
    WindowList::iterator it_end = m_windowlist.end();
    for (; it != it_end; ++it) {
        (*it).second->setAlpha(alpha);
    }
}

void Workspace::clearWindows() {
    m_window.clear();
    WindowList::iterator it = m_windowlist.begin();
    WindowList::iterator it_end = m_windowlist.end();
    for (; it != it_end; ++it) {
        if (m_use_pixmap) {
            updateBackground((*it).first,m_window_color);
        }
        (*it).second->clear();
    }
}

void Workspace::remove(Window win) {
    FbTk::FbWindow *fbwin = find(win);
    if (fbwin == 0)
        return;
    FbTk::EventManager::instance()->remove(*fbwin);
    //    FbTk::EventManager::instance()->remove(win);

    delete fbwin;

    if (fbwin == m_focused_window)
        m_focused_window = 0;

    m_windowlist.erase(win);
}


void Workspace::removeAll() {
    WindowList::iterator it = m_windowlist.begin();
    for (; it != m_windowlist.end(); ++it) {
        FbTk::EventManager::instance()->remove(*(it->second));
        delete it->second;
    }
    m_windowlist.clear();
    s_focused_window = 0;
}

ClientWindow Workspace::findClient(const FbTk::FbWindow &win) const {
    WindowList::const_iterator it = m_windowlist.begin();
    WindowList::const_iterator it_end = m_windowlist.end();
    for (; it != it_end; ++it) {
        if (*(*it).second == win)
            return ClientWindow((*it).first);
    }
    return ClientWindow(0);
}

FbTk::FbWindow *Workspace::find(Window win) {
    WindowList::iterator it = m_windowlist.begin();
    WindowList::iterator it_end = m_windowlist.end();
    for (; it != it_end; ++it) {
        if ((*it).second->window() == win ||
            (*it).first == win)
            return it->second;
    }
    return 0;
}

const FbTk::FbWindow *Workspace::find(Window win) const {
    WindowList::const_iterator it = m_windowlist.begin();
    WindowList::const_iterator it_end = m_windowlist.end();
    for (; it != it_end; ++it) {
        if ((*it).second->window() == win ||
            (*it).first == win)
            return it->second;
    }
    return 0;
}

void Workspace::updateFocusedWindow() {
    if (s_focused_window == 0)
        return;

    FbTk::FbWindow *fbwin = find(s_focused_window);
    if (fbwin == m_focused_window)
        return;

    if (m_focused_window != 0) {

        updateBackground(m_focused_window->window(), m_window_color);
        m_focused_window->clear();
    }

    if (fbwin == 0) {
        m_focused_window = 0;
        return;
    }

    WindowList::const_iterator it = m_windowlist.begin();
    WindowList::const_iterator it_end = m_windowlist.end();
    for (; it != it_end; ++it) {
        if ((*it).second == fbwin )
            break;
    }

    if (it == it_end)
        return;

    updateBackground((*it).first, m_focused_window_color);
    fbwin->clear();
    m_focused_window = fbwin;
}

void Workspace::updateGeometry(Window win) {

    FbTk::FbWindow* fbwin= find(win);
    if (!fbwin)
        return;

    int x = 0, y = 0;
    Window child;
    ClientWindow client(win);
    FbRootWindow rootwin(m_window.screenNumber());
    rootwin.translateCoordinates(client,
                                 client.x(), client.y(),
                                 x, y,
                                 child);
    x = -x;
    y = -y;
    unsigned int w = client.width(), h = client.height();
    scaleFromWindowToWindow(rootwin, m_window, x, y);
    scaleFromWindowToWindow(rootwin, m_window, w, h);
    w -= fbwin->borderWidth();
    if (w == 0)
        w = 1;
    if (h == 0)
        h = 1;

    fbwin->moveResize(x, y, w, h);
    updateBackground(win, m_window_color);
}

void Workspace::updateBackground(Window win, const FbTk::Color &bg_color) {

    FbTk::FbWindow* fbwin= find(win);
    if (!fbwin )
        return;


    if (m_use_pixmap) {
        XWMHints *hints = XGetWMHints(FbTk::App::instance()->display(), win);
        if (hints && (hints->flags & IconPixmapHint) && hints->icon_pixmap != 0) {
            FbTk::FbPixmap fbpix;
            fbpix.copy(hints->icon_pixmap);
            fbpix.scale(fbwin->width(), fbwin->height());
            fbwin->setBackgroundPixmap(fbpix.drawable());
        }
        else
            fbwin->setBackgroundColor(bg_color);

        XFree(hints);
    }
    else
        fbwin->setBackgroundColor(bg_color);
}

}; // end namespace FbPager

