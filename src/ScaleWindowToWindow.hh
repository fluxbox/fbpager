#ifndef SCALEWINDOWTOWINDOW_HH
#define SCALEWINDOWTOWINDOW_HH

#include "FbTk/FbWindow.hh"

namespace FbPager {
template <typename T>
inline void scaleFromWindowToWindow(const FbTk::FbWindow &src, 
                                    const FbTk::FbWindow &dest,
                                    T &x, T &y) {
    float xdiv = static_cast<float>(src.width()) / static_cast<float>(dest.width());
    float ydiv = static_cast<float>(src.height()) / static_cast<float>(dest.height());
    x = static_cast<T>((float)(x) / xdiv);
    y = static_cast<T>((float)(y) / ydiv);
}

} // end namespace FbPager

#endif // SCALEWINDOWTOWINDOW_HH
