// RefBinder.hh for FbPager
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

#ifndef REFBINDER_HH
#define REFBINDER_HH

namespace FbPager {
/// @brief helper class to bind referens argument
template <typename ActionClass, class ValueType, class ReturnType = void>
class RefBinder {
public:
    //    typedef ReturnType (ActionClass::* Action)(ValueType &);
    RefBinder(ActionClass action, ValueType &v):m_value(v), m_action(action) { }
    template <typename T>
    void operator ()(T *p) {
        ((*p).*m_action)(m_value);
    }
private:
    ValueType &m_value;
    ActionClass m_action;
};

template <typename A, typename B>
inline RefBinder<A, B>
RefBind(A a, B &b) {
    return RefBinder<A, B>(a, b);
}

} // end namespace FbPager

#endif // REFBINDER
