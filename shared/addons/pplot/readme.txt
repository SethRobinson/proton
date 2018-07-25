-- Blurb Seth added:

PPLOT is an open source ("do what you want with it") library written by Pier Philipsen to help visually graphing/plotting. 

Visit http://pplot.sourceforge.net/ for info and docs, I didn't include them here due to our /shared getting too bloated as it is. 

----------
The main directory is called "generic". This directory contains the
files needed to build the pure C++ PPlot library. To see some plots
you have to use one of the APIs for which PPlot has been ported, i.e.
QT, wxWindows or Cocoa.

HOW TO BUILD THE DEMO?

* MacOSX: see the "cocoa" directory for more details
* QT on Linux,FreeBSD: see the "linux" directory for more details
* wxWindows: compile files in "wxWindows" directory

HOW TO BUILD FOR NORMAL USE?

* MacOSX: see the "cocoa" directory for more details
* QT, or wxWindows
You need to compile all the files in the "generic" directory and one
file that implements PPlot in a native widget on the target platform,
i.e.
   qt\QPPlot.cpp
or
   wxWindows\wxPPlot.cpp

HOW TO ENABLE SCRIPTING?

See "ruby" and "python" directories for more detail. 

