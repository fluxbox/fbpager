// Ewmh.hh for FbPager
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

#ifndef EWMH_HH
#define EWMH_HH

#include "ClientHandler.hh"
#include <memory>

namespace FbPager {

class Ewmh_priv;

class Ewmh: public ClientHandler {
public:
    Ewmh();
    ~Ewmh();
    void setFocus(FbTk::FbWindow &win);
    void moveResize(FbTk::FbWindow &win);
    void sendToWorkspace(FbTk::FbWindow &win, int workspace);
    void closeWindow(FbTk::FbWindow &win);
    bool clientMessage(Pager &pager, XClientMessageEvent &event);
    void changeWorkspace(int screen_num, int workspace);
    void setHints(FbTk::FbWindow &win, WindowHint &hint);
    void getHints(const FbTk::FbWindow &win, WindowHint &hint) const;
    int numberOfWorkspaces(int screen_num) const;
    void setDesktopLayout(FbTk::FbWindow &root,
                          Orientation orientation,
                          Corner starting_corner,
                          unsigned int columns, unsigned int rows);
    bool propertyNotify( Pager &pager, XPropertyEvent &event);
private:
    bool m_support;
    std::unique_ptr<Ewmh_priv> m_data;
};

} // end namespace FbPager

#endif // EWMH_HH

