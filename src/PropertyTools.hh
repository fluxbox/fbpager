#ifndef PROPERTY_TOOLS_H
#define PROPERTY_TOOLS_H

#include <X11/Xlib.h>
#include <string>
#include <exception>
#include <vector>

namespace PropertyTools {

class PropertyException: public std::exception {
public:
    PropertyException(const std::string &property) throw():
        m_property(property) {
    }

    ~PropertyException() throw () {
    }

    const char *what() const throw() {
        return m_property.c_str();
    }

private:
    std::string m_property;
};

/// @return Atom name.
std::string getAtomName(Atom atom);


/// @return Integer value for property \c atom
unsigned int getIntProperty(Window win, Atom atom) throw (PropertyException);
Window getWinProperty(Window win, Atom atom) throw (PropertyException);
void getWinArrayProperty(Window win, Atom atom, std::vector<Window> &cont) throw (PropertyException);
} // PropertyTools

#endif // PROPERTY_TOOL_H
