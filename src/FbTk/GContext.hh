// GContext.hh for FbTk - fluxbox toolkit
// Copyright (c) 2003 Henrik Kinnunen (fluxgen at users.sourceforge.net)
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
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.

// $Id: GContext.hh,v 1.8 2004/01/11 12:53:46 fluxgen Exp $

#ifndef FBTK_GCONTEXT_HH
#define FBTK_GCONTEXT_HH

#include "Color.hh"
//#include "FbPixmap.hh"

#include <X11/Xlib.h>

namespace FbTk {

class FbDrawable;
class Font;

/// wrapper for X GC
class GContext {
public:
    /// for FbTk drawable
    explicit GContext(const FbTk::FbDrawable &drawable);
    /// for X drawable
    explicit GContext(Drawable drawable);
    GContext(Drawable d, const FbTk::GContext &gc);
    virtual ~GContext();

    inline void setForeground(const FbTk::Color &color) {
        setForeground(color.pixel());
    }

    inline void setForeground(long pixel_value) {
        XSetForeground(m_display, m_gc,
                       pixel_value);
    }

    inline void setBackground(const FbTk::Color &color) {
        setBackground(color.pixel());
    }

    inline void setBackground(long pixel_value) {
        XSetBackground(m_display, m_gc, pixel_value);
    }

    /// not implemented
    static inline void setFont(const FbTk::Font &) {}

    /// set font id
    inline void setFont(int fid) {
        XSetFont(m_display, m_gc, fid);
    }

    /*
    inline void setClipMask(const FbTk::FbPixmap &mask) {
        XSetClipMask(m_display, m_gc, mask.drawable());
    }
    */
    inline void setClipOrigin(int x, int y) {
        XSetClipOrigin(m_display, m_gc, x, y);
    }

    inline void setGraphicsExposure(bool value) {
        XSetGraphicsExposures(m_display, m_gc, value);
    }

    inline void setFunction(int func) {
        XSetFunction(m_display, m_gc, func);
    }

    inline void setSubwindowMode(int mode) {
        XSetSubwindowMode(m_display, m_gc, mode);
    }
    inline void setFillStyle(int style) {
        XSetFillStyle(m_display, m_gc, style);
    }
    inline void setFillRule(int rule) {
        XSetFillRule(m_display, m_gc, rule);
    }

    void copy(GC gc);
    void copy(const GContext &gc);
    inline GContext &operator = (const GContext &copy_gc) { copy(copy_gc); return *this; }
    inline GContext &operator = (GC copy_gc) { copy(copy_gc); return *this; }
    inline GC gc() const { return m_gc; }

private:
    GContext(const GContext &cont);

    static Display *m_display; // worth caching
    GC m_gc;
};

} // end namespace FbTk

#endif // FBTK_GCONTEXT_HH
