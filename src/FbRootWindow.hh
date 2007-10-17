#ifndef FBROOTWINDOW_HH
#define FBROOTWINDOW_HH

#include "FbTk/FbWindow.hh"
#include "FbTk/App.hh"

class FbRootWindow: public FbTk::FbWindow {
public:
    explicit FbRootWindow(int screen_num):
        FbTk::FbWindow(RootWindow(FbTk::App::instance()->display(), 
                                  screen_num)) { }

};

#endif // FBROOTWINDOW_HH
