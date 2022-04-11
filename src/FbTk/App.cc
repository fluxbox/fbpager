// App.cc
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
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.

#include "App.hh"

#include "EventManager.hh"

#include <cassert>
#include <string>

namespace FbTk {

App *App::s_app = 0;

App *App::instance() {
    if (s_app == 0)
        throw std::string("You must create an instance of FbTk::App first!");
    return s_app;
}

App::App(const char *displayname):m_done(false), m_display(0) {
    if (s_app != 0)
        throw std::string("Can't create more than one instance of FbTk::App");
    s_app = this;
    // this allows the use of std::string.c_str(), which returns 
    // a blank string, rather than a null string, so we make them equivalent
    if (displayname != 0 && displayname[0] == '\0')
        displayname = 0;
    m_display = XOpenDisplay(displayname);
}

App::~App() {
    if (m_display != 0) {
        XCloseDisplay(m_display);
        m_display = 0;
    }
    s_app = 0;
}

void App::sync(bool discard) const {
    XSync(m_display, discard);
}

void App::eventLoop() {
    XEvent ev;
    while (!m_done) {
        XNextEvent(m_display, &ev);
        EventManager::instance()->handleEvent(ev);
    }
}


void App::end() {
    m_done = true; //end loop in App::eventLoop
}

}; // end namespace FbTk
