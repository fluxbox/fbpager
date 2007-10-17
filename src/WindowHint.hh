#ifndef WINDOWHINTS_HH
#define WINDOWHINTS_HH

namespace FbPager {
class WindowHint {
public:
    enum Hint {
        WHINT_ICONIC =         0x0001,
        WHINT_STICKY    =      0x0002,
        WHINT_SHADED    =      0x0004,
        WHINT_SKIP_PAGER  =    0x0008,
        WHINT_SKIP_TASKBAR  =  0x0010,
        WHINT_NO_DECOR  =      0x0020,
        WHINT_HIDDEN =         0x0040,
        WHINT_TYPE_DOCK =      0x0080,
        WHINT_LAYER_BOTTOM =   0x0100,
        WHINT_LAYER_TOP    =   0x0200,
    };

    WindowHint():m_flags(0), 
                  m_workspace(0) { 
    }
    inline void add(int hint) { m_flags |= hint; }
    inline int flags() const { return m_flags; }
    inline void setWorkspace(int num) { m_workspace = num; }
    inline int workspace() const { return m_workspace; }
private:
    int m_flags;
    int m_workspace;
};

} // end namespace FbPager

#endif // WINDOWHINTS_HH
