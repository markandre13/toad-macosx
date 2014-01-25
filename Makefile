EXEC=fischland.app/Contents/MacOS/fischland

all: $(EXEC)

include Makefile.dep

SRC_SHARED=interactor.cc control.cc labelowner.cc buttonbase.cc pushbutton.cc \
	   checkbox.cc penbase.cc focusmanager.cc textarea.cc textfield.cc \
           scrollpane.cc table.cc menuhelper.cc menubutton.cc menubar.cc \
	   popup.cc popupmenu.cc command.cc dialog.cc dialogeditor.cc \
	   colorselector.cc fatradiobutton.cc radiobuttonbase.cc \
	   radiobutton.cc fatcheckbutton.cc layouteditor.cc color.cc \
	   filedialog.cc cursor.cc tableadapter.cc core.cc \
	   figure.cc figuremodel.cc figureeditor.cc matrix2d.cc \
           simpletimer.cc region.cc polygon.cc springlayout.cc combobox.cc \
           treemodel.cc treeadapter.cc htmlview.cc messagebox.cc \
           layout.cc pointer.cc connect.cc rectangle.cc font.cc eventfilter.cc \
	   arrowbutton.cc scrollbar.cc utf8.cc undo.cc undomanager.cc model.cc \
	   integermodel.cc floatmodel.cc textmodel.cc action.cc bitmap.cc \
	   io/atvparser.cc io/binstream.cc io/serializable.cc io/urlstream.cc \
	   gauge.cc colordialog.cc dragndrop.cc rgbmodel.cc \
	   dnd/dropobject.cc dnd/color.cc dnd/textplain.cc dnd/image.cc \
	   figure/bezier.cc figure/frame.cc figure/image.cc figure/polygon.cc \
	   figure/text.cc figure/circle.cc figure/group.cc figure/line.cc \
	   figure/rectangle.cc figure/window.cc \
	   \
	   test_table.cc test_scroll.cc
		
SRC_COCOA=window.cc pen.cc

SRC_FISH=fischland/draw.cc fischland/colorpalette.cc fischland/fitcurve.cc \
	 fischland/fpath.cc fischland/lineal.cc fischland/page.cc \
	 fischland/toolbox.cc fischland/selectiontool.cc
#fischland/fontdialog.cc

SRC=$(SRC_FISH) $(SRC_SHARED) $(SRC_COCOA)
CXX=g++ -ObjC++
CXXFLAGS=-g -O0 -frtti -Wall -Wno-switch -Wno-unused-variable -Wno-unneeded-internal-declaration

OBJS    = $(SRC:.cc=.o)

$(EXEC): $(OBJS)
	$(CXX) \
	-framework CoreFoundation \
	-framework AppKit \
	-L/usr/X11R6/lib -lfontconfig $(OBJS) -o $(EXEC)

clean:
	rm -f $(OBJS) $(EXEC) .gdb_history
	find . -name "*~" -exec rm {} \;
	find . -name "*.bak" -exec rm {} \;
	find . -name "DEADJOE" -exec rm {} \;

depend: Makefile.dep

Makefile.dep:
	$(CXX) -MM -I/usr/X11R6/include -Iinclude $(CXXFLAGS) $(SRC) > Makefile.dep

run:
	fischland.app/Contents/MacOS/fischland fischland/example.fish

gdb:
	lldb fischland.app/Contents/MacOS/fischland

.SUFFIXES: .cc .M .o

.cc.o:
	$(CXX) -I/usr/X11R6/include -Iinclude $(CXXFLAGS) -c -o $*.o $*.cc
