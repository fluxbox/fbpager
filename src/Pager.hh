#ifndef PAGER_HH
#define PAGER_HH

#include <vector>
#include <utility>

namespace FbPager {

/// @brief pager interface
class Pager {
public:
    /// add a window to workspace
    virtual void addWindow(Window win, unsigned int workspace_num) = 0;
    virtual void moveToWorkspace(Window win, unsigned int workspace_num) = 0;
    virtual void setFocusedWindow(Window win) = 0;
    /// remove a window from workspace
    virtual void removeWindow(Window win) = 0;
    virtual void raiseWindow(Window win) = 0;
    virtual void lowerWindow(Window win) = 0;
    /// update number of workspaces
    virtual void updateWorkspaceCount(unsigned int num) = 0;
    virtual void setCurrentWorkspace(unsigned int num) = 0;
    virtual bool haveWindow(Window win) = 0;
    /// Window and workspace pair container
    typedef std::vector< std::pair<Window, unsigned int> > WindowsAndWorkspaces;

    virtual void addWindows(const WindowsAndWorkspaces& windows) = 0;

};

} // end FbPager namespace

#endif // PAGER_HH
