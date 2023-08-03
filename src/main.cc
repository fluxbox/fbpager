// main.cc for FbPager
// Copyright (c) 2003-2004 Henrik Kinnunen (fluxgen at users.sourceforge.net)
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

// $Id$

#include "FbPager.hh"
#include "FbTk/App.hh"

#include <cstdlib>
#include <iostream>
#include <string>
#include <string.h>
#include "version.h"

using namespace std;

void showVersion() {
    cout<<"FbPager "<<__fbpager_version<<": (c) 2004-2007 Henrik Kinnunen (fluxgen<at>fluxbox org)"<<endl;
}

void showUsage(const char * const appname) {
    showVersion();
    cout<<"Usage: "<<endl
        <<appname<<" [argument(s)]"<<endl
        <<"Arguments: "<<endl
        <<"  -display           <displayname>"<<endl
        <<"  -w                 withdrawn state (i.e in the Slit)"<<endl
        <<"  -rc <filename>     resource file (default: ~/.fluxbox/fbpager)"<<endl
        <<"  -sr                show resources"<<endl
        <<"  -s <screen number> (default: 0)"<<endl
        <<"  -v                 show version"<<endl
        <<"  -h                 show this help"<<endl
        <<"  -l <layer>         layer, [top|normal|bottom]" << endl;
}

int main(const int argc, const char **argv) {
    int screen_num = 0;
    std::string displayname;
    bool withdraw = false;
    std::string resourcefile("~/.fluxbox/fbpager");
    bool show_resources = false;
    int layer = -1;
    for (int arg = 1; arg < argc; ++arg) {
        string sarg = argv[arg];
        // if argument starts with "--" remove the first '-'
        if (sarg[0] == '-' && sarg[1] == '-')
            sarg.erase(0, 1);

        if (sarg == "-display" && arg < argc - 1) {
            displayname = argv[++arg];
        } else if (sarg == "-w")
            withdraw = true;
        else if (sarg == "-rc" && arg < argc - 1)
            resourcefile = argv[++arg];
        else if (sarg == "-v") {
            showVersion();
            exit(EXIT_SUCCESS);
        } else if (sarg == "-s" && arg < argc - 1) {
            screen_num = atoi(argv[++arg]);
        } else if (sarg == "-sr") {
            show_resources = true;
        } else if (sarg == "-l") {
            ++arg;
            if (strcmp(argv[arg], "top")== 0) {
                layer = 2;
            } else if (strcmp(argv[arg], "bottom") == 0) {
                layer = 0;
            }
        } else {
            showUsage(argv[0]);
            exit(EXIT_SUCCESS);
        }
    }

    try {
        FbTk::App app(displayname.c_str());
        if (app.display() == 0) {
            cerr<<"Could not open display!"<<endl;
            cerr<<"Make sure you have an X server running."<<endl;
            throw string("Display connection");
        }

        FbPager::FbPager fbpager(screen_num, withdraw,
                                 show_resources,
                                 layer,
                                 resourcefile.c_str());
        if (!show_resources)
            app.eventLoop();

    } catch (std::string err) {
        cerr<<"Error: "<<err<<endl;
    } catch (...) {
        cerr<<"Unknown exception caught!"<<endl;
    }
}
