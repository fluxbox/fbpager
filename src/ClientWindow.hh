#ifndef CLIENTWINDOW_HH
#define CLIENTWINDOW_HH

#include "FbTk/FbWindow.hh"

class ClientWindow:public FbTk::FbWindow {
public:
    explicit ClientWindow(Window win):FbTk::FbWindow(win) { }
};

#endif // CLIENTWINDOW_HH
