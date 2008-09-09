SUBDIRS=FbTk

bin_PROGRAMS=fbpager


fbpager_SOURCES= main.cc FbPager.hh FbPager.cc \
	Workspace.cc Workspace.hh \
	Ewmh.hh Ewmh.cc ClientHandler.hh \
	ClientWindow.hh Pager.hh WindowHint.hh FbRootWindow.hh \
	ScaleWindowToWindow.hh Resources.hh RefBinder.hh
fbpager_CXXFLAGS=-IFbTk
fbpager_LDADD=FbTk/libFbTk.a
