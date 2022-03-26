#include "PropertyTools.hh"
#include "FbTk/App.hh"
#include <X11/Xatom.h>
#include <iostream>

namespace PropertyTools {

std::string getAtomName(Atom atom) {
    char* xstr = XGetAtomName(FbTk::App::instance()->display(), atom);
    std::string name = xstr;
    XFree(xstr);
    return name;
}


unsigned int getIntProperty(Window win, Atom atom) {
    Atom ret_type = 0;
    int fmt = 0;
    unsigned long nitems = 0, bytes_after = 0;
    long *data = 0;
    if ( XGetWindowProperty(FbTk::App::instance()->display(), win,
                            atom, 0, 1, False, XA_CARDINAL,
                            &ret_type, &fmt, &nitems,
                            &bytes_after, (unsigned char**)&data) != Success) {
        std::cerr << __func__ << "() PropertyException, n=" << nitems << std::endl;
        throw PropertyException(getAtomName(atom));
    }
    if (!data) {
        std::cerr << __func__ << "() data == NULL, n=" << nitems << std::endl;
        return 0;
    }

    unsigned int val = (unsigned int)( *data );
    XFree(data);
    return val;
}

Window getWinProperty(Window win, Atom atom) {
    Atom ret_type = 0;
    int fmt = 0;
    unsigned long nitems = 0, bytes_after = 0;
    long *data = 0;
    if ( XGetWindowProperty(FbTk::App::instance()->display(), win,
                            atom, 0, 1, False, XA_WINDOW,
                            &ret_type, &fmt, &nitems,
                            &bytes_after, (unsigned char**)&data) != Success) {
        std::cerr << __func__ << "() PropertyException, n=" << nitems << std::endl;
        throw PropertyException(getAtomName(atom));
    }
    if (!data) {
        std::cerr << __func__ << "() data == NULL, n=" << nitems << std::endl;
        return (Window)0;
    }

    Window val = (Window)( *data );
    XFree(data);
    return val;
}

void getWinArrayProperty(Window win, Atom atom, std::vector<Window> &cont) {
    Atom ret_type = 0;
    int fmt = 0;
    unsigned long nitems = 0, bytes_after = 0;
    long *data = 0;
    if ( XGetWindowProperty(FbTk::App::instance()->display(), win,
                            atom, 0, 0xFFFFFF, False, XA_WINDOW,
                            &ret_type, &fmt, &nitems,
                            &bytes_after, (unsigned char**)&data) != Success) {
        std::cerr << __func__ << "() PropertyException, n=" << nitems << std::endl;
        throw PropertyException(getAtomName(atom));
    }
    if (!data) {
        std::cerr << __func__ << "() data == NULL, n=" << nitems << std::endl;
        return;
    }
    for (unsigned long i = 0; i < nitems; ++i ) {
        cont.push_back(((Window*)data)[i]);
    }

    XFree(data);

}

} // PropertyTools
