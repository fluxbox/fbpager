// ClientHandler.hh for FbPager
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

#ifndef CLIENTHANDLER_HH
#define CLIENTHANDLER_HH

#include <X11/Xlib.h>
namespace FbTk {
class FbWindow;
}

namespace FbPager {

class WindowHint;
class Pager;

class ClientHandler {
public:
    enum Orientation {
        HORIZONTAL,
        VERTICAL
    };
    enum Corner {
        TOPLEFT,
        TOPRIGHT,
        BOTTOMRIGHT,
        BOTTOMLEFT
    };
    ClientHandler() { }
    virtual ~ClientHandler() { }
    virtual void setFocus(FbTk::FbWindow &win) = 0;
    virtual void moveResize(FbTk::FbWindow &win) = 0;
    virtual void sendToWorkspace(FbTk::FbWindow &win, int workspace) = 0;
    virtual void closeWindow(FbTk::FbWindow &win) = 0;
    virtual bool clientMessage(Pager &pager, XClientMessageEvent &event) = 0;
    virtual void changeWorkspace(int screen_num, int workspace) = 0;
    virtual void setHints(FbTk::FbWindow &win, WindowHint &hint) = 0;
    virtual void getHints(const FbTk::FbWindow &win, WindowHint &hint) const = 0;
    virtual int numberOfWorkspaces(int screen_num) const = 0;
    virtual void setDesktopLayout(FbTk::FbWindow &root,
                                  Orientation orientation,
                                  Corner starting_corner,
                                  unsigned int columns, unsigned int rows) = 0;
    virtual bool propertyNotify( Pager &pager, XPropertyEvent &event) = 0;
};

} // end namespace FbPager

#endif // CLIENTHANDLER_HH
