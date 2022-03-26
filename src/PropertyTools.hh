#ifndef PROPERTY_TOOLS_H
#define PROPERTY_TOOLS_H

#include <X11/Xlib.h>
#include <string>
#include <exception>
#include <vector>

namespace PropertyTools {

class PropertyException: public std::exception {
public:
    PropertyException(const std::string &property) noexcept:
        m_property(property) {
    }

    ~PropertyException() noexcept {
    }

    const char *what() const noexcept {
        return m_property.c_str();
    }

private:
    std::string m_property;
};

/// @return Atom name.
std::string getAtomName(Atom atom);


/// @return Integer value for property \c atom
unsigned int getIntProperty(Window win, Atom atom);
Window getWinProperty(Window win, Atom atom);
void getWinArrayProperty(Window win, Atom atom, std::vector<Window> &cont);
} // PropertyTools

#endif // PROPERTY_TOOL_H
