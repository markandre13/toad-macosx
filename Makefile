EXEC=fischland.app/Contents/MacOS/fischland

all: $(EXEC)

#include Makefile.dep

SRC_SHARED=interactor.cc control.cc labelowner.cc buttonbase.cc pushbutton.cc \
	   checkbox.cc penbase.cc focusmanager.cc textarea.cc textfield.cc \
           scrollpane.cc table.cc menuhelper.cc menubutton.cc menubar.cc \
	   popup.cc popupmenu.cc command.cc dialog.cc dialogeditor.cc \
	   colorselector.cc fatradiobutton.cc radiobuttonbase.cc \
	   radiobutton.cc fatcheckbutton.cc layouteditor.cc color.cc \
	   filedialog.cc cursor.cc tableadapter.cc core.cc \
	   figure.cc figuremodel.cc figureeditor.cc figuretool.cc matrix2d.cc \
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
	   figure/transform.cc \
	   figure/rectangle.cc figure/window.cc figure/createtool.cc \
	   geometry.cc \
	   stacktrace.cc \
	   \
	   test_table.cc test_scroll.cc test_dialog.cc test_timer.cc \
	   test_combobox.cc test_cursor.cc test_colordialog.cc test_grab.cc \
	   test_path.cc test_image.cc test_curve.cc test_text.cc test_tablet.cc \
	   test_path_bool.cc
	
SRC_COCOA=window.cc pen.cc

SRC_FISH=fischland/draw.cc fischland/colorpalette.cc fischland/fitcurve.cc \
	 fischland/fpath.cc fischland/lineal.cc fischland/page.cc \
	 fischland/toolbox.cc fischland/colorpicker.cc \
	 fischland/selectiontool.cc fischland/directselectiontool.cc \
	 fischland/rotatetool.cc \
	 fischland/pentool.cc fischland/penciltool.cc \
	 fischland/filltool.cc fischland/filltoolutil.cc \
	 fischland/fischeditor.cc
	 
#fischland/fontdialog.cc

SRC=$(SRC_SHARED) $(SRC_COCOA) $(SRC_FISH)
CXX=g++ -ObjC++ -std=gnu++14
CXXFLAGS=-g -O0 -frtti -Wall -Wno-switch -Wno-unused-variable -Wno-unneeded-internal-declaration

OBJS    = $(SRC:.cc=.o)

$(EXEC): $(OBJS)
	$(CXX) \
	-framework CoreFoundation \
	-framework AppKit \
	$(OBJS) -o $(EXEC)

clean:
	rm -f $(OBJS) $(EXEC) .gdb_history
	find . -name "*~" -exec rm {} \;
	find . -name "*.bak" -exec rm {} \;
	find . -name "DEADJOE" -exec rm {} \;

depend:
	@makedepend -Iinclude -Y $(SRC) 2> /dev/null

Makefile.dep:
	$(CXX) -MM -Iinclude $(CXXFLAGS) $(SRC) > Makefile.dep

run:
	fischland.app/Contents/MacOS/fischland fischland/example.fish

gdb:
	lldb fischland.app/Contents/MacOS/fischland

.SUFFIXES: .cc .M .o

.cc.o:
	$(CXX) -Iinclude $(CXXFLAGS) -c -o $*.o $*.cc

# DO NOT DELETE

interactor.o: include/toad/interactor.hh include/toad/types.hh
control.o: include/toad/control.hh include/toad/window.hh
control.o: include/toad/interactor.hh include/toad/types.hh
control.o: include/toad/cursor.hh include/toad/color.hh
control.o: include/toad/io/serializable.hh include/toad/io/atvparser.hh
control.o: include/toad/region.hh
labelowner.o: include/toad/labelowner.hh include/toad/control.hh
labelowner.o: include/toad/window.hh include/toad/interactor.hh
labelowner.o: include/toad/types.hh include/toad/cursor.hh
labelowner.o: include/toad/color.hh include/toad/io/serializable.hh
labelowner.o: include/toad/io/atvparser.hh include/toad/region.hh
buttonbase.o: include/toad/core.hh include/toad/window.hh
buttonbase.o: include/toad/interactor.hh include/toad/types.hh
buttonbase.o: include/toad/cursor.hh include/toad/color.hh
buttonbase.o: include/toad/io/serializable.hh include/toad/io/atvparser.hh
buttonbase.o: include/toad/region.hh include/toad/pen.hh
buttonbase.o: include/toad/penbase.hh include/toad/font.hh
buttonbase.o: include/toad/pointer.hh include/toad/matrix2d.hh
buttonbase.o: include/toad/bitmap.hh include/toad/connect.hh
buttonbase.o: include/toad/buttonbase.hh include/toad/labelowner.hh
buttonbase.o: include/toad/control.hh
pushbutton.o: include/toad/core.hh include/toad/window.hh
pushbutton.o: include/toad/interactor.hh include/toad/types.hh
pushbutton.o: include/toad/cursor.hh include/toad/color.hh
pushbutton.o: include/toad/io/serializable.hh include/toad/io/atvparser.hh
pushbutton.o: include/toad/region.hh include/toad/pen.hh
pushbutton.o: include/toad/penbase.hh include/toad/font.hh
pushbutton.o: include/toad/pointer.hh include/toad/matrix2d.hh
pushbutton.o: include/toad/bitmap.hh include/toad/connect.hh
pushbutton.o: include/toad/buttonbase.hh include/toad/labelowner.hh
pushbutton.o: include/toad/control.hh include/toad/pushbutton.hh
checkbox.o: include/toad/pen.hh include/toad/penbase.hh include/toad/color.hh
checkbox.o: include/toad/types.hh include/toad/io/serializable.hh
checkbox.o: include/toad/io/atvparser.hh include/toad/font.hh
checkbox.o: include/toad/pointer.hh include/toad/matrix2d.hh
checkbox.o: include/toad/window.hh include/toad/interactor.hh
checkbox.o: include/toad/cursor.hh include/toad/region.hh
checkbox.o: include/toad/checkbox.hh include/toad/labelowner.hh
checkbox.o: include/toad/control.hh include/toad/boolmodel.hh
checkbox.o: include/toad/model.hh include/toad/connect.hh
penbase.o: include/toad/penbase.hh include/toad/color.hh
penbase.o: include/toad/types.hh include/toad/io/serializable.hh
penbase.o: include/toad/io/atvparser.hh include/toad/font.hh
penbase.o: include/toad/pointer.hh include/toad/matrix2d.hh
focusmanager.o: include/toad/focusmanager.hh include/toad/window.hh
focusmanager.o: include/toad/interactor.hh include/toad/types.hh
focusmanager.o: include/toad/cursor.hh include/toad/color.hh
focusmanager.o: include/toad/io/serializable.hh include/toad/io/atvparser.hh
focusmanager.o: include/toad/region.hh include/toad/eventfilter.hh
textarea.o: include/toad/textarea.hh include/toad/core.hh
textarea.o: include/toad/window.hh include/toad/interactor.hh
textarea.o: include/toad/types.hh include/toad/cursor.hh
textarea.o: include/toad/color.hh include/toad/io/serializable.hh
textarea.o: include/toad/io/atvparser.hh include/toad/region.hh
textarea.o: include/toad/pen.hh include/toad/penbase.hh include/toad/font.hh
textarea.o: include/toad/pointer.hh include/toad/matrix2d.hh
textarea.o: include/toad/bitmap.hh include/toad/connect.hh
textarea.o: include/toad/control.hh include/toad/textmodel.hh
textarea.o: include/toad/model.hh include/toad/undo.hh
textarea.o: include/toad/scrollbar.hh include/toad/integermodel.hh
textarea.o: include/toad/numbermodel.hh include/toad/simpletimer.hh
textarea.o: include/toad/undomanager.hh include/toad/action.hh
textarea.o: include/toad/utf8.hh
textfield.o: include/toad/textfield.hh include/toad/textarea.hh
textfield.o: include/toad/core.hh include/toad/window.hh
textfield.o: include/toad/interactor.hh include/toad/types.hh
textfield.o: include/toad/cursor.hh include/toad/color.hh
textfield.o: include/toad/io/serializable.hh include/toad/io/atvparser.hh
textfield.o: include/toad/region.hh include/toad/pen.hh
textfield.o: include/toad/penbase.hh include/toad/font.hh
textfield.o: include/toad/pointer.hh include/toad/matrix2d.hh
textfield.o: include/toad/bitmap.hh include/toad/connect.hh
textfield.o: include/toad/control.hh include/toad/textmodel.hh
textfield.o: include/toad/model.hh include/toad/undo.hh
textfield.o: include/toad/scrollbar.hh include/toad/integermodel.hh
textfield.o: include/toad/numbermodel.hh
scrollpane.o: include/toad/scrollpane.hh include/toad/window.hh
scrollpane.o: include/toad/interactor.hh include/toad/types.hh
scrollpane.o: include/toad/cursor.hh include/toad/color.hh
scrollpane.o: include/toad/io/serializable.hh include/toad/io/atvparser.hh
scrollpane.o: include/toad/region.hh include/toad/penbase.hh
scrollpane.o: include/toad/font.hh include/toad/pointer.hh
scrollpane.o: include/toad/matrix2d.hh include/toad/scrollbar.hh
scrollpane.o: include/toad/control.hh include/toad/integermodel.hh
scrollpane.o: include/toad/numbermodel.hh include/toad/model.hh
scrollpane.o: include/toad/connect.hh include/toad/textmodel.hh
scrollpane.o: include/toad/undo.hh
table.o: include/toad/table.hh include/toad/pen.hh include/toad/penbase.hh
table.o: include/toad/color.hh include/toad/types.hh
table.o: include/toad/io/serializable.hh include/toad/io/atvparser.hh
table.o: include/toad/font.hh include/toad/pointer.hh
table.o: include/toad/matrix2d.hh include/toad/region.hh
table.o: include/toad/scrollpane.hh include/toad/window.hh
table.o: include/toad/interactor.hh include/toad/cursor.hh
table.o: include/toad/model.hh include/toad/connect.hh
table.o: include/toad/dragndrop.hh include/toad/figure.hh
table.o: include/toad/bitmap.hh include/toad/figuremodel.hh
menuhelper.o: include/toad/menuhelper.hh include/toad/window.hh
menuhelper.o: include/toad/interactor.hh include/toad/types.hh
menuhelper.o: include/toad/cursor.hh include/toad/color.hh
menuhelper.o: include/toad/io/serializable.hh include/toad/io/atvparser.hh
menuhelper.o: include/toad/region.hh include/toad/layout.hh
menuhelper.o: include/toad/eventfilter.hh include/toad/connect.hh
menuhelper.o: include/toad/menubutton.hh include/toad/io/urlstream.hh
menuhelper.o: include/toad/action.hh include/toad/model.hh
menuhelper.o: include/toad/pointer.hh
menubutton.o: include/toad/menubutton.hh include/toad/menuhelper.hh
menubutton.o: include/toad/window.hh include/toad/interactor.hh
menubutton.o: include/toad/types.hh include/toad/cursor.hh
menubutton.o: include/toad/color.hh include/toad/io/serializable.hh
menubutton.o: include/toad/io/atvparser.hh include/toad/region.hh
menubutton.o: include/toad/layout.hh include/toad/eventfilter.hh
menubutton.o: include/toad/connect.hh include/toad/pen.hh
menubutton.o: include/toad/penbase.hh include/toad/font.hh
menubutton.o: include/toad/pointer.hh include/toad/matrix2d.hh
menubutton.o: include/toad/popup.hh include/toad/action.hh
menubutton.o: include/toad/model.hh include/toad/command.hh
menubutton.o: include/toad/bitmap.hh include/toad/focusmanager.hh
menubar.o: include/toad/menubar.hh include/toad/menuhelper.hh
menubar.o: include/toad/window.hh include/toad/interactor.hh
menubar.o: include/toad/types.hh include/toad/cursor.hh include/toad/color.hh
menubar.o: include/toad/io/serializable.hh include/toad/io/atvparser.hh
menubar.o: include/toad/region.hh include/toad/layout.hh
menubar.o: include/toad/eventfilter.hh include/toad/connect.hh
menubar.o: include/toad/focusmanager.hh include/toad/pushbutton.hh
menubar.o: include/toad/buttonbase.hh include/toad/labelowner.hh
menubar.o: include/toad/control.hh include/toad/textfield.hh
menubar.o: include/toad/textarea.hh include/toad/core.hh include/toad/pen.hh
menubar.o: include/toad/penbase.hh include/toad/font.hh
menubar.o: include/toad/pointer.hh include/toad/matrix2d.hh
menubar.o: include/toad/bitmap.hh include/toad/textmodel.hh
menubar.o: include/toad/model.hh include/toad/undo.hh
menubar.o: include/toad/scrollbar.hh include/toad/integermodel.hh
menubar.o: include/toad/numbermodel.hh include/toad/action.hh
menubar.o: include/toad/utf8.hh
popup.o: include/toad/popup.hh include/toad/menuhelper.hh
popup.o: include/toad/window.hh include/toad/interactor.hh
popup.o: include/toad/types.hh include/toad/cursor.hh include/toad/color.hh
popup.o: include/toad/io/serializable.hh include/toad/io/atvparser.hh
popup.o: include/toad/region.hh include/toad/layout.hh
popup.o: include/toad/eventfilter.hh include/toad/connect.hh
popupmenu.o: include/toad/popupmenu.hh include/toad/menuhelper.hh
popupmenu.o: include/toad/window.hh include/toad/interactor.hh
popupmenu.o: include/toad/types.hh include/toad/cursor.hh
popupmenu.o: include/toad/color.hh include/toad/io/serializable.hh
popupmenu.o: include/toad/io/atvparser.hh include/toad/region.hh
popupmenu.o: include/toad/layout.hh include/toad/eventfilter.hh
popupmenu.o: include/toad/connect.hh include/toad/focusmanager.hh
command.o: include/toad/command.hh include/toad/pointer.hh
command.o: include/toad/window.hh include/toad/interactor.hh
command.o: include/toad/types.hh include/toad/cursor.hh include/toad/color.hh
command.o: include/toad/io/serializable.hh include/toad/io/atvparser.hh
command.o: include/toad/region.hh
dialog.o: include/toad/pen.hh include/toad/penbase.hh include/toad/color.hh
dialog.o: include/toad/types.hh include/toad/io/serializable.hh
dialog.o: include/toad/io/atvparser.hh include/toad/font.hh
dialog.o: include/toad/pointer.hh include/toad/matrix2d.hh
dialog.o: include/toad/window.hh include/toad/interactor.hh
dialog.o: include/toad/cursor.hh include/toad/region.hh
dialog.o: include/toad/dialog.hh include/toad/layout.hh
dialog.o: include/toad/eventfilter.hh include/toad/figuremodel.hh
dialog.o: include/toad/model.hh include/toad/connect.hh
dialog.o: include/toad/layouteditor.hh include/toad/control.hh
dialog.o: include/toad/dialogeditor.hh include/toad/figure.hh
dialog.o: include/toad/bitmap.hh include/toad/labelowner.hh
dialog.o: include/toad/io/urlstream.hh include/toad/focusmanager.hh
dialog.o: include/toad/scrollbar.hh include/toad/integermodel.hh
dialog.o: include/toad/numbermodel.hh include/toad/textmodel.hh
dialog.o: include/toad/undo.hh include/toad/pushbutton.hh
dialog.o: include/toad/buttonbase.hh include/toad/fatradiobutton.hh
dialog.o: include/toad/radiobuttonbase.hh include/toad/textfield.hh
dialog.o: include/toad/textarea.hh include/toad/core.hh
dialog.o: include/toad/figureeditor.hh include/toad/scrollpane.hh
dialog.o: include/toad/boolmodel.hh include/toad/floatmodel.hh
dialog.o: include/toad/figure/createtool.hh include/toad/figuretool.hh
dialog.o: include/toad/colorselector.hh include/toad/dnd/color.hh
dialog.o: include/toad/dragndrop.hh fischland/directselectiontool.hh
dialog.o: include/toad/undomanager.hh include/toad/action.hh
dialogeditor.o: include/toad/dialogeditor.hh include/toad/dialog.hh
dialogeditor.o: include/toad/window.hh include/toad/interactor.hh
dialogeditor.o: include/toad/types.hh include/toad/cursor.hh
dialogeditor.o: include/toad/color.hh include/toad/io/serializable.hh
dialogeditor.o: include/toad/io/atvparser.hh include/toad/region.hh
dialogeditor.o: include/toad/layout.hh include/toad/eventfilter.hh
dialogeditor.o: include/toad/figuremodel.hh include/toad/model.hh
dialogeditor.o: include/toad/pointer.hh include/toad/connect.hh
dialogeditor.o: include/toad/pushbutton.hh include/toad/buttonbase.hh
dialogeditor.o: include/toad/labelowner.hh include/toad/control.hh
dialogeditor.o: include/toad/fatcheckbutton.hh include/toad/boolmodel.hh
dialogeditor.o: include/toad/layouteditor.hh include/toad/focusmanager.hh
colorselector.o: include/toad/colorselector.hh include/toad/dnd/color.hh
colorselector.o: include/toad/dragndrop.hh include/toad/types.hh
colorselector.o: include/toad/pointer.hh include/toad/connect.hh
colorselector.o: include/toad/color.hh include/toad/io/serializable.hh
colorselector.o: include/toad/io/atvparser.hh include/toad/figureeditor.hh
colorselector.o: include/toad/figure.hh include/toad/penbase.hh
colorselector.o: include/toad/font.hh include/toad/matrix2d.hh
colorselector.o: include/toad/window.hh include/toad/interactor.hh
colorselector.o: include/toad/cursor.hh include/toad/region.hh
colorselector.o: include/toad/bitmap.hh include/toad/figuremodel.hh
colorselector.o: include/toad/model.hh include/toad/scrollpane.hh
colorselector.o: include/toad/undo.hh include/toad/boolmodel.hh
colorselector.o: include/toad/textmodel.hh include/toad/integermodel.hh
colorselector.o: include/toad/numbermodel.hh include/toad/floatmodel.hh
colorselector.o: include/toad/figure/createtool.hh include/toad/figuretool.hh
colorselector.o: include/toad/core.hh include/toad/pen.hh
colorselector.o: include/toad/colordialog.hh include/toad/dialog.hh
colorselector.o: include/toad/layout.hh include/toad/eventfilter.hh
colorselector.o: include/toad/rgbmodel.hh include/toad/textarea.hh
colorselector.o: include/toad/control.hh include/toad/scrollbar.hh
colorselector.o: include/toad/pushbutton.hh include/toad/buttonbase.hh
colorselector.o: include/toad/labelowner.hh include/toad/undomanager.hh
colorselector.o: include/toad/action.hh
fatradiobutton.o: include/toad/pen.hh include/toad/penbase.hh
fatradiobutton.o: include/toad/color.hh include/toad/types.hh
fatradiobutton.o: include/toad/io/serializable.hh
fatradiobutton.o: include/toad/io/atvparser.hh include/toad/font.hh
fatradiobutton.o: include/toad/pointer.hh include/toad/matrix2d.hh
fatradiobutton.o: include/toad/window.hh include/toad/interactor.hh
fatradiobutton.o: include/toad/cursor.hh include/toad/region.hh
fatradiobutton.o: include/toad/buttonbase.hh include/toad/connect.hh
fatradiobutton.o: include/toad/labelowner.hh include/toad/control.hh
fatradiobutton.o: include/toad/fatradiobutton.hh
fatradiobutton.o: include/toad/radiobuttonbase.hh include/toad/model.hh
radiobuttonbase.o: include/toad/pen.hh include/toad/penbase.hh
radiobuttonbase.o: include/toad/color.hh include/toad/types.hh
radiobuttonbase.o: include/toad/io/serializable.hh
radiobuttonbase.o: include/toad/io/atvparser.hh include/toad/font.hh
radiobuttonbase.o: include/toad/pointer.hh include/toad/matrix2d.hh
radiobuttonbase.o: include/toad/window.hh include/toad/interactor.hh
radiobuttonbase.o: include/toad/cursor.hh include/toad/region.hh
radiobuttonbase.o: include/toad/radiobuttonbase.hh include/toad/buttonbase.hh
radiobuttonbase.o: include/toad/connect.hh include/toad/labelowner.hh
radiobuttonbase.o: include/toad/control.hh include/toad/model.hh
radiobutton.o: include/toad/pen.hh include/toad/penbase.hh
radiobutton.o: include/toad/color.hh include/toad/types.hh
radiobutton.o: include/toad/io/serializable.hh include/toad/io/atvparser.hh
radiobutton.o: include/toad/font.hh include/toad/pointer.hh
radiobutton.o: include/toad/matrix2d.hh include/toad/window.hh
radiobutton.o: include/toad/interactor.hh include/toad/cursor.hh
radiobutton.o: include/toad/region.hh include/toad/radiobutton.hh
radiobutton.o: include/toad/radiobuttonbase.hh include/toad/buttonbase.hh
radiobutton.o: include/toad/connect.hh include/toad/labelowner.hh
radiobutton.o: include/toad/control.hh include/toad/model.hh
fatcheckbutton.o: include/toad/fatcheckbutton.hh include/toad/buttonbase.hh
fatcheckbutton.o: include/toad/connect.hh include/toad/labelowner.hh
fatcheckbutton.o: include/toad/control.hh include/toad/window.hh
fatcheckbutton.o: include/toad/interactor.hh include/toad/types.hh
fatcheckbutton.o: include/toad/cursor.hh include/toad/color.hh
fatcheckbutton.o: include/toad/io/serializable.hh
fatcheckbutton.o: include/toad/io/atvparser.hh include/toad/region.hh
fatcheckbutton.o: include/toad/boolmodel.hh include/toad/model.hh
fatcheckbutton.o: include/toad/pointer.hh include/toad/pen.hh
fatcheckbutton.o: include/toad/penbase.hh include/toad/font.hh
fatcheckbutton.o: include/toad/matrix2d.hh
layouteditor.o: include/toad/layouteditor.hh include/toad/control.hh
layouteditor.o: include/toad/window.hh include/toad/interactor.hh
layouteditor.o: include/toad/types.hh include/toad/cursor.hh
layouteditor.o: include/toad/color.hh include/toad/io/serializable.hh
layouteditor.o: include/toad/io/atvparser.hh include/toad/region.hh
color.o: include/toad/color.hh include/toad/types.hh
color.o: include/toad/io/serializable.hh include/toad/io/atvparser.hh
filedialog.o: include/toad/filedialog.hh include/toad/dialog.hh
filedialog.o: include/toad/window.hh include/toad/interactor.hh
filedialog.o: include/toad/types.hh include/toad/cursor.hh
filedialog.o: include/toad/color.hh include/toad/io/serializable.hh
filedialog.o: include/toad/io/atvparser.hh include/toad/region.hh
filedialog.o: include/toad/layout.hh include/toad/eventfilter.hh
filedialog.o: include/toad/figuremodel.hh include/toad/model.hh
filedialog.o: include/toad/pointer.hh include/toad/connect.hh
filedialog.o: include/toad/textmodel.hh include/toad/undo.hh
filedialog.o: include/toad/boolmodel.hh include/toad/table.hh
filedialog.o: include/toad/pen.hh include/toad/penbase.hh
filedialog.o: include/toad/font.hh include/toad/matrix2d.hh
filedialog.o: include/toad/scrollpane.hh include/toad/dragndrop.hh
filedialog.o: include/toad/pushbutton.hh include/toad/buttonbase.hh
filedialog.o: include/toad/labelowner.hh include/toad/control.hh
filedialog.o: include/toad/stl/vector.hh include/toad/textfield.hh
filedialog.o: include/toad/textarea.hh include/toad/core.hh
filedialog.o: include/toad/bitmap.hh include/toad/scrollbar.hh
filedialog.o: include/toad/integermodel.hh include/toad/numbermodel.hh
filedialog.o: include/toad/combobox.hh include/toad/checkbox.hh
filedialog.o: include/toad/messagebox.hh include/toad/stl/deque.hh
cursor.o: include/toad/pen.hh include/toad/penbase.hh include/toad/color.hh
cursor.o: include/toad/types.hh include/toad/io/serializable.hh
cursor.o: include/toad/io/atvparser.hh include/toad/font.hh
cursor.o: include/toad/pointer.hh include/toad/matrix2d.hh
cursor.o: include/toad/window.hh include/toad/interactor.hh
cursor.o: include/toad/cursor.hh include/toad/region.hh
cursor.o: include/toad/bitmap.hh
tableadapter.o: include/toad/table.hh include/toad/pen.hh
tableadapter.o: include/toad/penbase.hh include/toad/color.hh
tableadapter.o: include/toad/types.hh include/toad/io/serializable.hh
tableadapter.o: include/toad/io/atvparser.hh include/toad/font.hh
tableadapter.o: include/toad/pointer.hh include/toad/matrix2d.hh
tableadapter.o: include/toad/region.hh include/toad/scrollpane.hh
tableadapter.o: include/toad/window.hh include/toad/interactor.hh
tableadapter.o: include/toad/cursor.hh include/toad/model.hh
tableadapter.o: include/toad/connect.hh include/toad/dragndrop.hh
tableadapter.o: include/toad/utf8.hh
core.o: include/toad/core.hh include/toad/window.hh
core.o: include/toad/interactor.hh include/toad/types.hh
core.o: include/toad/cursor.hh include/toad/color.hh
core.o: include/toad/io/serializable.hh include/toad/io/atvparser.hh
core.o: include/toad/region.hh include/toad/pen.hh include/toad/penbase.hh
core.o: include/toad/font.hh include/toad/pointer.hh include/toad/matrix2d.hh
core.o: include/toad/bitmap.hh include/toad/connect.hh include/toad/figure.hh
core.o: include/toad/figuremodel.hh include/toad/model.hh
core.o: include/toad/command.hh include/toad/dialogeditor.hh
core.o: include/toad/dialog.hh include/toad/layout.hh
core.o: include/toad/eventfilter.hh fischland/fontdialog.hh
core.o: include/toad/textfield.hh include/toad/textarea.hh
core.o: include/toad/control.hh include/toad/textmodel.hh
core.o: include/toad/undo.hh include/toad/scrollbar.hh
core.o: include/toad/integermodel.hh include/toad/numbermodel.hh
core.o: include/toad/table.hh include/toad/scrollpane.hh
core.o: include/toad/dragndrop.hh
figure.o: include/toad/figure.hh include/toad/penbase.hh
figure.o: include/toad/color.hh include/toad/types.hh
figure.o: include/toad/io/serializable.hh include/toad/io/atvparser.hh
figure.o: include/toad/font.hh include/toad/pointer.hh
figure.o: include/toad/matrix2d.hh include/toad/window.hh
figure.o: include/toad/interactor.hh include/toad/cursor.hh
figure.o: include/toad/region.hh include/toad/bitmap.hh
figure.o: include/toad/figuremodel.hh include/toad/model.hh
figure.o: include/toad/connect.hh include/toad/figureeditor.hh
figure.o: include/toad/scrollpane.hh include/toad/undo.hh
figure.o: include/toad/boolmodel.hh include/toad/textmodel.hh
figure.o: include/toad/integermodel.hh include/toad/numbermodel.hh
figure.o: include/toad/floatmodel.hh include/toad/figure/createtool.hh
figure.o: include/toad/figuretool.hh include/toad/core.hh include/toad/pen.hh
figure.o: include/toad/dialog.hh include/toad/layout.hh
figure.o: include/toad/eventfilter.hh include/toad/menuhelper.hh
figure.o: include/toad/springlayout.hh
figuremodel.o: include/toad/figure.hh include/toad/penbase.hh
figuremodel.o: include/toad/color.hh include/toad/types.hh
figuremodel.o: include/toad/io/serializable.hh include/toad/io/atvparser.hh
figuremodel.o: include/toad/font.hh include/toad/pointer.hh
figuremodel.o: include/toad/matrix2d.hh include/toad/window.hh
figuremodel.o: include/toad/interactor.hh include/toad/cursor.hh
figuremodel.o: include/toad/region.hh include/toad/bitmap.hh
figuremodel.o: include/toad/figuremodel.hh include/toad/model.hh
figuremodel.o: include/toad/connect.hh include/toad/figureeditor.hh
figuremodel.o: include/toad/scrollpane.hh include/toad/undo.hh
figuremodel.o: include/toad/boolmodel.hh include/toad/textmodel.hh
figuremodel.o: include/toad/integermodel.hh include/toad/numbermodel.hh
figuremodel.o: include/toad/floatmodel.hh include/toad/figure/createtool.hh
figuremodel.o: include/toad/figuretool.hh include/toad/core.hh
figuremodel.o: include/toad/pen.hh include/toad/undomanager.hh
figuremodel.o: include/toad/action.hh include/toad/io/binstream.hh
figureeditor.o: include/toad/figureeditor.hh include/toad/figure.hh
figureeditor.o: include/toad/penbase.hh include/toad/color.hh
figureeditor.o: include/toad/types.hh include/toad/io/serializable.hh
figureeditor.o: include/toad/io/atvparser.hh include/toad/font.hh
figureeditor.o: include/toad/pointer.hh include/toad/matrix2d.hh
figureeditor.o: include/toad/window.hh include/toad/interactor.hh
figureeditor.o: include/toad/cursor.hh include/toad/region.hh
figureeditor.o: include/toad/bitmap.hh include/toad/figuremodel.hh
figureeditor.o: include/toad/model.hh include/toad/connect.hh
figureeditor.o: include/toad/scrollpane.hh include/toad/undo.hh
figureeditor.o: include/toad/boolmodel.hh include/toad/textmodel.hh
figureeditor.o: include/toad/integermodel.hh include/toad/numbermodel.hh
figureeditor.o: include/toad/floatmodel.hh include/toad/figure/createtool.hh
figureeditor.o: include/toad/figuretool.hh include/toad/core.hh
figureeditor.o: include/toad/pen.hh include/toad/colordialog.hh
figureeditor.o: include/toad/dialog.hh include/toad/layout.hh
figureeditor.o: include/toad/eventfilter.hh include/toad/rgbmodel.hh
figureeditor.o: include/toad/textarea.hh include/toad/control.hh
figureeditor.o: include/toad/scrollbar.hh include/toad/dnd/color.hh
figureeditor.o: include/toad/dragndrop.hh include/toad/checkbox.hh
figureeditor.o: include/toad/labelowner.hh include/toad/action.hh
figureeditor.o: include/toad/undomanager.hh include/toad/stacktrace.hh
figuretool.o: include/toad/figuretool.hh include/toad/core.hh
figuretool.o: include/toad/window.hh include/toad/interactor.hh
figuretool.o: include/toad/types.hh include/toad/cursor.hh
figuretool.o: include/toad/color.hh include/toad/io/serializable.hh
figuretool.o: include/toad/io/atvparser.hh include/toad/region.hh
figuretool.o: include/toad/pen.hh include/toad/penbase.hh
figuretool.o: include/toad/font.hh include/toad/pointer.hh
figuretool.o: include/toad/matrix2d.hh include/toad/bitmap.hh
figuretool.o: include/toad/connect.hh include/toad/figure.hh
figuretool.o: include/toad/figuremodel.hh include/toad/model.hh
matrix2d.o: include/toad/matrix2d.hh include/toad/types.hh
matrix2d.o: include/toad/io/serializable.hh include/toad/io/atvparser.hh
simpletimer.o: include/toad/simpletimer.hh include/toad/types.hh
region.o: include/toad/region.hh include/toad/types.hh
polygon.o: include/toad/types.hh
springlayout.o: include/toad/springlayout.hh include/toad/layout.hh
springlayout.o: include/toad/io/serializable.hh include/toad/io/atvparser.hh
springlayout.o: include/toad/eventfilter.hh include/toad/window.hh
springlayout.o: include/toad/interactor.hh include/toad/types.hh
springlayout.o: include/toad/cursor.hh include/toad/color.hh
springlayout.o: include/toad/region.hh
combobox.o: include/toad/combobox.hh include/toad/table.hh
combobox.o: include/toad/pen.hh include/toad/penbase.hh include/toad/color.hh
combobox.o: include/toad/types.hh include/toad/io/serializable.hh
combobox.o: include/toad/io/atvparser.hh include/toad/font.hh
combobox.o: include/toad/pointer.hh include/toad/matrix2d.hh
combobox.o: include/toad/region.hh include/toad/scrollpane.hh
combobox.o: include/toad/window.hh include/toad/interactor.hh
combobox.o: include/toad/cursor.hh include/toad/model.hh
combobox.o: include/toad/connect.hh include/toad/dragndrop.hh
combobox.o: include/toad/buttonbase.hh include/toad/labelowner.hh
combobox.o: include/toad/control.hh include/toad/scrollbar.hh
combobox.o: include/toad/integermodel.hh include/toad/numbermodel.hh
combobox.o: include/toad/textmodel.hh include/toad/undo.hh
treemodel.o: include/toad/treemodel.hh include/toad/table.hh
treemodel.o: include/toad/pen.hh include/toad/penbase.hh
treemodel.o: include/toad/color.hh include/toad/types.hh
treemodel.o: include/toad/io/serializable.hh include/toad/io/atvparser.hh
treemodel.o: include/toad/font.hh include/toad/pointer.hh
treemodel.o: include/toad/matrix2d.hh include/toad/region.hh
treemodel.o: include/toad/scrollpane.hh include/toad/window.hh
treemodel.o: include/toad/interactor.hh include/toad/cursor.hh
treemodel.o: include/toad/model.hh include/toad/connect.hh
treemodel.o: include/toad/dragndrop.hh
treeadapter.o: include/toad/treeadapter.hh include/toad/treemodel.hh
treeadapter.o: include/toad/table.hh include/toad/pen.hh
treeadapter.o: include/toad/penbase.hh include/toad/color.hh
treeadapter.o: include/toad/types.hh include/toad/io/serializable.hh
treeadapter.o: include/toad/io/atvparser.hh include/toad/font.hh
treeadapter.o: include/toad/pointer.hh include/toad/matrix2d.hh
treeadapter.o: include/toad/region.hh include/toad/scrollpane.hh
treeadapter.o: include/toad/window.hh include/toad/interactor.hh
treeadapter.o: include/toad/cursor.hh include/toad/model.hh
treeadapter.o: include/toad/connect.hh include/toad/dragndrop.hh
htmlview.o: include/toad/htmlview.hh include/toad/scrollpane.hh
htmlview.o: include/toad/window.hh include/toad/interactor.hh
htmlview.o: include/toad/types.hh include/toad/cursor.hh
htmlview.o: include/toad/color.hh include/toad/io/serializable.hh
htmlview.o: include/toad/io/atvparser.hh include/toad/region.hh
htmlview.o: include/toad/penbase.hh include/toad/font.hh
htmlview.o: include/toad/pointer.hh include/toad/matrix2d.hh
htmlview.o: include/toad/scrollbar.hh include/toad/control.hh
htmlview.o: include/toad/integermodel.hh include/toad/numbermodel.hh
htmlview.o: include/toad/model.hh include/toad/connect.hh
htmlview.o: include/toad/textmodel.hh include/toad/undo.hh
htmlview.o: include/toad/io/urlstream.hh include/toad/action.hh
htmlview.o: include/toad/filedialog.hh include/toad/dialog.hh
htmlview.o: include/toad/layout.hh include/toad/eventfilter.hh
htmlview.o: include/toad/figuremodel.hh include/toad/boolmodel.hh
htmlview.o: include/toad/table.hh include/toad/pen.hh
htmlview.o: include/toad/dragndrop.hh include/toad/pushbutton.hh
htmlview.o: include/toad/buttonbase.hh include/toad/labelowner.hh
htmlview.o: include/toad/stl/vector.hh include/toad/undomanager.hh
htmlview.o: include/toad/messagebox.hh include/toad/bitmap.hh
htmlview.o: include/toad/utf8.hh
messagebox.o: include/toad/pen.hh include/toad/penbase.hh
messagebox.o: include/toad/color.hh include/toad/types.hh
messagebox.o: include/toad/io/serializable.hh include/toad/io/atvparser.hh
messagebox.o: include/toad/font.hh include/toad/pointer.hh
messagebox.o: include/toad/matrix2d.hh include/toad/bitmap.hh
messagebox.o: include/toad/window.hh include/toad/interactor.hh
messagebox.o: include/toad/cursor.hh include/toad/region.hh
messagebox.o: include/toad/pushbutton.hh include/toad/buttonbase.hh
messagebox.o: include/toad/connect.hh include/toad/labelowner.hh
messagebox.o: include/toad/control.hh include/toad/messagebox.hh
layout.o: include/toad/layout.hh include/toad/io/serializable.hh
layout.o: include/toad/io/atvparser.hh include/toad/eventfilter.hh
layout.o: include/toad/window.hh include/toad/interactor.hh
layout.o: include/toad/types.hh include/toad/cursor.hh include/toad/color.hh
layout.o: include/toad/region.hh include/toad/textarea.hh
layout.o: include/toad/core.hh include/toad/pen.hh include/toad/penbase.hh
layout.o: include/toad/font.hh include/toad/pointer.hh
layout.o: include/toad/matrix2d.hh include/toad/bitmap.hh
layout.o: include/toad/connect.hh include/toad/control.hh
layout.o: include/toad/textmodel.hh include/toad/model.hh
layout.o: include/toad/undo.hh include/toad/scrollbar.hh
layout.o: include/toad/integermodel.hh include/toad/numbermodel.hh
layout.o: include/toad/pushbutton.hh include/toad/buttonbase.hh
layout.o: include/toad/labelowner.hh include/toad/io/urlstream.hh
layout.o: include/toad/layouteditor.hh
pointer.o: include/toad/pointer.hh
connect.o: include/toad/connect.hh
rectangle.o: include/toad/types.hh
font.o: include/toad/font.hh include/toad/pointer.hh include/toad/types.hh
eventfilter.o: include/toad/eventfilter.hh
arrowbutton.o: include/toad/arrowbutton.hh include/toad/pushbutton.hh
arrowbutton.o: include/toad/buttonbase.hh include/toad/connect.hh
arrowbutton.o: include/toad/labelowner.hh include/toad/control.hh
arrowbutton.o: include/toad/window.hh include/toad/interactor.hh
arrowbutton.o: include/toad/types.hh include/toad/cursor.hh
arrowbutton.o: include/toad/color.hh include/toad/io/serializable.hh
arrowbutton.o: include/toad/io/atvparser.hh include/toad/region.hh
arrowbutton.o: include/toad/simpletimer.hh include/toad/pen.hh
arrowbutton.o: include/toad/penbase.hh include/toad/font.hh
arrowbutton.o: include/toad/pointer.hh include/toad/matrix2d.hh
scrollbar.o: include/toad/scrollbar.hh include/toad/control.hh
scrollbar.o: include/toad/window.hh include/toad/interactor.hh
scrollbar.o: include/toad/types.hh include/toad/cursor.hh
scrollbar.o: include/toad/color.hh include/toad/io/serializable.hh
scrollbar.o: include/toad/io/atvparser.hh include/toad/region.hh
scrollbar.o: include/toad/integermodel.hh include/toad/numbermodel.hh
scrollbar.o: include/toad/model.hh include/toad/pointer.hh
scrollbar.o: include/toad/connect.hh include/toad/textmodel.hh
scrollbar.o: include/toad/undo.hh include/toad/pen.hh include/toad/penbase.hh
scrollbar.o: include/toad/font.hh include/toad/matrix2d.hh
scrollbar.o: include/toad/arrowbutton.hh include/toad/pushbutton.hh
scrollbar.o: include/toad/buttonbase.hh include/toad/labelowner.hh
scrollbar.o: include/toad/simpletimer.hh
utf8.o: include/toad/utf8.hh
undo.o: include/toad/undo.hh
undomanager.o: include/toad/undomanager.hh include/toad/interactor.hh
undomanager.o: include/toad/types.hh include/toad/action.hh
undomanager.o: include/toad/window.hh include/toad/cursor.hh
undomanager.o: include/toad/color.hh include/toad/io/serializable.hh
undomanager.o: include/toad/io/atvparser.hh include/toad/region.hh
undomanager.o: include/toad/connect.hh include/toad/model.hh
undomanager.o: include/toad/pointer.hh include/toad/undo.hh
model.o: include/toad/model.hh include/toad/pointer.hh
model.o: include/toad/connect.hh include/toad/undomanager.hh
model.o: include/toad/interactor.hh include/toad/types.hh
model.o: include/toad/action.hh include/toad/window.hh include/toad/cursor.hh
model.o: include/toad/color.hh include/toad/io/serializable.hh
model.o: include/toad/io/atvparser.hh include/toad/region.hh
model.o: include/toad/undo.hh
integermodel.o: include/toad/integermodel.hh include/toad/numbermodel.hh
integermodel.o: include/toad/model.hh include/toad/pointer.hh
integermodel.o: include/toad/connect.hh include/toad/textmodel.hh
integermodel.o: include/toad/undo.hh include/toad/io/serializable.hh
integermodel.o: include/toad/io/atvparser.hh
floatmodel.o: include/toad/floatmodel.hh include/toad/numbermodel.hh
floatmodel.o: include/toad/model.hh include/toad/pointer.hh
floatmodel.o: include/toad/connect.hh include/toad/textmodel.hh
floatmodel.o: include/toad/undo.hh include/toad/io/serializable.hh
floatmodel.o: include/toad/io/atvparser.hh
textmodel.o: include/toad/textmodel.hh include/toad/model.hh
textmodel.o: include/toad/pointer.hh include/toad/connect.hh
textmodel.o: include/toad/undo.hh include/toad/io/serializable.hh
textmodel.o: include/toad/io/atvparser.hh include/toad/undomanager.hh
textmodel.o: include/toad/interactor.hh include/toad/types.hh
textmodel.o: include/toad/action.hh include/toad/window.hh
textmodel.o: include/toad/cursor.hh include/toad/color.hh
textmodel.o: include/toad/region.hh
action.o: include/toad/action.hh include/toad/window.hh
action.o: include/toad/interactor.hh include/toad/types.hh
action.o: include/toad/cursor.hh include/toad/color.hh
action.o: include/toad/io/serializable.hh include/toad/io/atvparser.hh
action.o: include/toad/region.hh include/toad/connect.hh
action.o: include/toad/model.hh include/toad/pointer.hh
bitmap.o: include/toad/bitmap.hh include/toad/penbase.hh
bitmap.o: include/toad/color.hh include/toad/types.hh
bitmap.o: include/toad/io/serializable.hh include/toad/io/atvparser.hh
bitmap.o: include/toad/font.hh include/toad/pointer.hh
bitmap.o: include/toad/matrix2d.hh
io/atvparser.o: io/atvparser.hh
io/binstream.o: include/toad/io/binstream.hh include/toad/types.hh
io/serializable.o: io/serializable.hh include/toad/io/atvparser.hh
io/urlstream.o: include/toad/io/urlstream.hh
gauge.o: include/toad/gauge.hh include/toad/arrowbutton.hh
gauge.o: include/toad/pushbutton.hh include/toad/buttonbase.hh
gauge.o: include/toad/connect.hh include/toad/labelowner.hh
gauge.o: include/toad/control.hh include/toad/window.hh
gauge.o: include/toad/interactor.hh include/toad/types.hh
gauge.o: include/toad/cursor.hh include/toad/color.hh
gauge.o: include/toad/io/serializable.hh include/toad/io/atvparser.hh
gauge.o: include/toad/region.hh include/toad/simpletimer.hh
gauge.o: include/toad/integermodel.hh include/toad/numbermodel.hh
gauge.o: include/toad/model.hh include/toad/pointer.hh
gauge.o: include/toad/textmodel.hh include/toad/undo.hh
colordialog.o: include/toad/colordialog.hh include/toad/dialog.hh
colordialog.o: include/toad/window.hh include/toad/interactor.hh
colordialog.o: include/toad/types.hh include/toad/cursor.hh
colordialog.o: include/toad/color.hh include/toad/io/serializable.hh
colordialog.o: include/toad/io/atvparser.hh include/toad/region.hh
colordialog.o: include/toad/layout.hh include/toad/eventfilter.hh
colordialog.o: include/toad/figuremodel.hh include/toad/model.hh
colordialog.o: include/toad/pointer.hh include/toad/connect.hh
colordialog.o: include/toad/rgbmodel.hh include/toad/floatmodel.hh
colordialog.o: include/toad/numbermodel.hh include/toad/textmodel.hh
colordialog.o: include/toad/undo.hh include/toad/textarea.hh
colordialog.o: include/toad/core.hh include/toad/pen.hh
colordialog.o: include/toad/penbase.hh include/toad/font.hh
colordialog.o: include/toad/matrix2d.hh include/toad/bitmap.hh
colordialog.o: include/toad/control.hh include/toad/scrollbar.hh
colordialog.o: include/toad/integermodel.hh include/toad/dnd/color.hh
colordialog.o: include/toad/dragndrop.hh include/toad/pushbutton.hh
colordialog.o: include/toad/buttonbase.hh include/toad/labelowner.hh
colordialog.o: include/toad/textfield.hh include/toad/gauge.hh
colordialog.o: include/toad/arrowbutton.hh include/toad/simpletimer.hh
dragndrop.o: include/toad/dragndrop.hh include/toad/types.hh
dragndrop.o: include/toad/pointer.hh include/toad/connect.hh
dragndrop.o: include/toad/window.hh include/toad/interactor.hh
dragndrop.o: include/toad/cursor.hh include/toad/color.hh
dragndrop.o: include/toad/io/serializable.hh include/toad/io/atvparser.hh
dragndrop.o: include/toad/region.hh
rgbmodel.o: include/toad/rgbmodel.hh include/toad/model.hh
rgbmodel.o: include/toad/pointer.hh include/toad/connect.hh
rgbmodel.o: include/toad/floatmodel.hh include/toad/numbermodel.hh
rgbmodel.o: include/toad/textmodel.hh include/toad/undo.hh
rgbmodel.o: include/toad/io/serializable.hh include/toad/io/atvparser.hh
rgbmodel.o: include/toad/textarea.hh include/toad/core.hh
rgbmodel.o: include/toad/window.hh include/toad/interactor.hh
rgbmodel.o: include/toad/types.hh include/toad/cursor.hh
rgbmodel.o: include/toad/color.hh include/toad/region.hh include/toad/pen.hh
rgbmodel.o: include/toad/penbase.hh include/toad/font.hh
rgbmodel.o: include/toad/matrix2d.hh include/toad/bitmap.hh
rgbmodel.o: include/toad/control.hh include/toad/scrollbar.hh
rgbmodel.o: include/toad/integermodel.hh
dnd/dropobject.o: include/toad/dnd/dropobject.hh include/toad/dragndrop.hh
dnd/dropobject.o: include/toad/types.hh include/toad/pointer.hh
dnd/dropobject.o: include/toad/connect.hh
dnd/color.o: include/toad/dnd/color.hh include/toad/dragndrop.hh
dnd/color.o: include/toad/types.hh include/toad/pointer.hh
dnd/color.o: include/toad/connect.hh include/toad/color.hh
dnd/color.o: include/toad/io/serializable.hh include/toad/io/atvparser.hh
dnd/textplain.o: include/toad/dnd/textplain.hh include/toad/connect.hh
dnd/textplain.o: include/toad/dragndrop.hh include/toad/types.hh
dnd/textplain.o: include/toad/pointer.hh
dnd/image.o: include/toad/dnd/image.hh include/toad/dragndrop.hh
dnd/image.o: include/toad/types.hh include/toad/pointer.hh
dnd/image.o: include/toad/connect.hh include/toad/bitmap.hh
dnd/image.o: include/toad/penbase.hh include/toad/color.hh
dnd/image.o: include/toad/io/serializable.hh include/toad/io/atvparser.hh
dnd/image.o: include/toad/font.hh include/toad/matrix2d.hh
figure/frame.o: include/toad/figure.hh include/toad/penbase.hh
figure/frame.o: include/toad/color.hh include/toad/types.hh
figure/frame.o: include/toad/io/serializable.hh include/toad/io/atvparser.hh
figure/frame.o: include/toad/font.hh include/toad/pointer.hh
figure/frame.o: include/toad/matrix2d.hh include/toad/window.hh
figure/frame.o: include/toad/interactor.hh include/toad/cursor.hh
figure/frame.o: include/toad/region.hh include/toad/bitmap.hh
figure/frame.o: include/toad/figuremodel.hh include/toad/model.hh
figure/frame.o: include/toad/connect.hh include/toad/figureeditor.hh
figure/frame.o: include/toad/scrollpane.hh include/toad/undo.hh
figure/frame.o: include/toad/boolmodel.hh include/toad/textmodel.hh
figure/frame.o: include/toad/integermodel.hh include/toad/numbermodel.hh
figure/frame.o: include/toad/floatmodel.hh include/toad/figure/createtool.hh
figure/frame.o: include/toad/figuretool.hh include/toad/core.hh
figure/frame.o: include/toad/pen.hh
figure/image.o: include/toad/figure.hh include/toad/penbase.hh
figure/image.o: include/toad/color.hh include/toad/types.hh
figure/image.o: include/toad/io/serializable.hh include/toad/io/atvparser.hh
figure/image.o: include/toad/font.hh include/toad/pointer.hh
figure/image.o: include/toad/matrix2d.hh include/toad/window.hh
figure/image.o: include/toad/interactor.hh include/toad/cursor.hh
figure/image.o: include/toad/region.hh include/toad/bitmap.hh
figure/image.o: include/toad/figuremodel.hh include/toad/model.hh
figure/image.o: include/toad/connect.hh include/toad/figureeditor.hh
figure/image.o: include/toad/scrollpane.hh include/toad/undo.hh
figure/image.o: include/toad/boolmodel.hh include/toad/textmodel.hh
figure/image.o: include/toad/integermodel.hh include/toad/numbermodel.hh
figure/image.o: include/toad/floatmodel.hh include/toad/figure/createtool.hh
figure/image.o: include/toad/figuretool.hh include/toad/core.hh
figure/image.o: include/toad/pen.hh include/toad/filedialog.hh
figure/image.o: include/toad/dialog.hh include/toad/layout.hh
figure/image.o: include/toad/eventfilter.hh include/toad/table.hh
figure/image.o: include/toad/dragndrop.hh include/toad/pushbutton.hh
figure/image.o: include/toad/buttonbase.hh include/toad/labelowner.hh
figure/image.o: include/toad/control.hh include/toad/stl/vector.hh
figure/polygon.o: include/toad/figure.hh include/toad/penbase.hh
figure/polygon.o: include/toad/color.hh include/toad/types.hh
figure/polygon.o: include/toad/io/serializable.hh
figure/polygon.o: include/toad/io/atvparser.hh include/toad/font.hh
figure/polygon.o: include/toad/pointer.hh include/toad/matrix2d.hh
figure/polygon.o: include/toad/window.hh include/toad/interactor.hh
figure/polygon.o: include/toad/cursor.hh include/toad/region.hh
figure/polygon.o: include/toad/bitmap.hh include/toad/figuremodel.hh
figure/polygon.o: include/toad/model.hh include/toad/connect.hh
figure/polygon.o: include/toad/figureeditor.hh include/toad/scrollpane.hh
figure/polygon.o: include/toad/undo.hh include/toad/boolmodel.hh
figure/polygon.o: include/toad/textmodel.hh include/toad/integermodel.hh
figure/polygon.o: include/toad/numbermodel.hh include/toad/floatmodel.hh
figure/polygon.o: include/toad/figure/createtool.hh
figure/polygon.o: include/toad/figuretool.hh include/toad/core.hh
figure/polygon.o: include/toad/pen.hh include/toad/action.hh
figure/polygon.o: include/toad/popupmenu.hh include/toad/menuhelper.hh
figure/polygon.o: include/toad/layout.hh include/toad/eventfilter.hh
figure/text.o: include/toad/figure.hh include/toad/penbase.hh
figure/text.o: include/toad/color.hh include/toad/types.hh
figure/text.o: include/toad/io/serializable.hh include/toad/io/atvparser.hh
figure/text.o: include/toad/font.hh include/toad/pointer.hh
figure/text.o: include/toad/matrix2d.hh include/toad/window.hh
figure/text.o: include/toad/interactor.hh include/toad/cursor.hh
figure/text.o: include/toad/region.hh include/toad/bitmap.hh
figure/text.o: include/toad/figuremodel.hh include/toad/model.hh
figure/text.o: include/toad/connect.hh include/toad/figureeditor.hh
figure/text.o: include/toad/scrollpane.hh include/toad/undo.hh
figure/text.o: include/toad/boolmodel.hh include/toad/textmodel.hh
figure/text.o: include/toad/integermodel.hh include/toad/numbermodel.hh
figure/text.o: include/toad/floatmodel.hh include/toad/figure/createtool.hh
figure/text.o: include/toad/figuretool.hh include/toad/core.hh
figure/text.o: include/toad/pen.hh
figure/circle.o: include/toad/figure.hh include/toad/penbase.hh
figure/circle.o: include/toad/color.hh include/toad/types.hh
figure/circle.o: include/toad/io/serializable.hh include/toad/io/atvparser.hh
figure/circle.o: include/toad/font.hh include/toad/pointer.hh
figure/circle.o: include/toad/matrix2d.hh include/toad/window.hh
figure/circle.o: include/toad/interactor.hh include/toad/cursor.hh
figure/circle.o: include/toad/region.hh include/toad/bitmap.hh
figure/circle.o: include/toad/figuremodel.hh include/toad/model.hh
figure/circle.o: include/toad/connect.hh include/toad/figureeditor.hh
figure/circle.o: include/toad/scrollpane.hh include/toad/undo.hh
figure/circle.o: include/toad/boolmodel.hh include/toad/textmodel.hh
figure/circle.o: include/toad/integermodel.hh include/toad/numbermodel.hh
figure/circle.o: include/toad/floatmodel.hh include/toad/figure/createtool.hh
figure/circle.o: include/toad/figuretool.hh include/toad/core.hh
figure/circle.o: include/toad/pen.hh
figure/group.o: include/toad/figure.hh include/toad/penbase.hh
figure/group.o: include/toad/color.hh include/toad/types.hh
figure/group.o: include/toad/io/serializable.hh include/toad/io/atvparser.hh
figure/group.o: include/toad/font.hh include/toad/pointer.hh
figure/group.o: include/toad/matrix2d.hh include/toad/window.hh
figure/group.o: include/toad/interactor.hh include/toad/cursor.hh
figure/group.o: include/toad/region.hh include/toad/bitmap.hh
figure/group.o: include/toad/figuremodel.hh include/toad/model.hh
figure/group.o: include/toad/connect.hh include/toad/figureeditor.hh
figure/group.o: include/toad/scrollpane.hh include/toad/undo.hh
figure/group.o: include/toad/boolmodel.hh include/toad/textmodel.hh
figure/group.o: include/toad/integermodel.hh include/toad/numbermodel.hh
figure/group.o: include/toad/floatmodel.hh include/toad/figure/createtool.hh
figure/group.o: include/toad/figuretool.hh include/toad/core.hh
figure/group.o: include/toad/pen.hh
figure/line.o: include/toad/figure.hh include/toad/penbase.hh
figure/line.o: include/toad/color.hh include/toad/types.hh
figure/line.o: include/toad/io/serializable.hh include/toad/io/atvparser.hh
figure/line.o: include/toad/font.hh include/toad/pointer.hh
figure/line.o: include/toad/matrix2d.hh include/toad/window.hh
figure/line.o: include/toad/interactor.hh include/toad/cursor.hh
figure/line.o: include/toad/region.hh include/toad/bitmap.hh
figure/line.o: include/toad/figuremodel.hh include/toad/model.hh
figure/line.o: include/toad/connect.hh include/toad/figureeditor.hh
figure/line.o: include/toad/scrollpane.hh include/toad/undo.hh
figure/line.o: include/toad/boolmodel.hh include/toad/textmodel.hh
figure/line.o: include/toad/integermodel.hh include/toad/numbermodel.hh
figure/line.o: include/toad/floatmodel.hh include/toad/figure/createtool.hh
figure/line.o: include/toad/figuretool.hh include/toad/core.hh
figure/line.o: include/toad/pen.hh
figure/transform.o: include/toad/figure.hh include/toad/penbase.hh
figure/transform.o: include/toad/color.hh include/toad/types.hh
figure/transform.o: include/toad/io/serializable.hh
figure/transform.o: include/toad/io/atvparser.hh include/toad/font.hh
figure/transform.o: include/toad/pointer.hh include/toad/matrix2d.hh
figure/transform.o: include/toad/window.hh include/toad/interactor.hh
figure/transform.o: include/toad/cursor.hh include/toad/region.hh
figure/transform.o: include/toad/bitmap.hh include/toad/figuremodel.hh
figure/transform.o: include/toad/model.hh include/toad/connect.hh
figure/transform.o: include/toad/figureeditor.hh include/toad/scrollpane.hh
figure/transform.o: include/toad/undo.hh include/toad/boolmodel.hh
figure/transform.o: include/toad/textmodel.hh include/toad/integermodel.hh
figure/transform.o: include/toad/numbermodel.hh include/toad/floatmodel.hh
figure/transform.o: include/toad/figure/createtool.hh
figure/transform.o: include/toad/figuretool.hh include/toad/core.hh
figure/transform.o: include/toad/pen.hh
figure/rectangle.o: include/toad/figure.hh include/toad/penbase.hh
figure/rectangle.o: include/toad/color.hh include/toad/types.hh
figure/rectangle.o: include/toad/io/serializable.hh
figure/rectangle.o: include/toad/io/atvparser.hh include/toad/font.hh
figure/rectangle.o: include/toad/pointer.hh include/toad/matrix2d.hh
figure/rectangle.o: include/toad/window.hh include/toad/interactor.hh
figure/rectangle.o: include/toad/cursor.hh include/toad/region.hh
figure/rectangle.o: include/toad/bitmap.hh include/toad/figuremodel.hh
figure/rectangle.o: include/toad/model.hh include/toad/connect.hh
figure/rectangle.o: include/toad/figureeditor.hh include/toad/scrollpane.hh
figure/rectangle.o: include/toad/undo.hh include/toad/boolmodel.hh
figure/rectangle.o: include/toad/textmodel.hh include/toad/integermodel.hh
figure/rectangle.o: include/toad/numbermodel.hh include/toad/floatmodel.hh
figure/rectangle.o: include/toad/figure/createtool.hh
figure/rectangle.o: include/toad/figuretool.hh include/toad/core.hh
figure/rectangle.o: include/toad/pen.hh
figure/window.o: include/toad/figure.hh include/toad/penbase.hh
figure/window.o: include/toad/color.hh include/toad/types.hh
figure/window.o: include/toad/io/serializable.hh include/toad/io/atvparser.hh
figure/window.o: include/toad/font.hh include/toad/pointer.hh
figure/window.o: include/toad/matrix2d.hh include/toad/window.hh
figure/window.o: include/toad/interactor.hh include/toad/cursor.hh
figure/window.o: include/toad/region.hh include/toad/bitmap.hh
figure/window.o: include/toad/figuremodel.hh include/toad/model.hh
figure/window.o: include/toad/connect.hh include/toad/figureeditor.hh
figure/window.o: include/toad/scrollpane.hh include/toad/undo.hh
figure/window.o: include/toad/boolmodel.hh include/toad/textmodel.hh
figure/window.o: include/toad/integermodel.hh include/toad/numbermodel.hh
figure/window.o: include/toad/floatmodel.hh include/toad/figure/createtool.hh
figure/window.o: include/toad/figuretool.hh include/toad/core.hh
figure/window.o: include/toad/pen.hh include/toad/labelowner.hh
figure/window.o: include/toad/control.hh
figure/createtool.o: include/toad/figureeditor.hh include/toad/figure.hh
figure/createtool.o: include/toad/penbase.hh include/toad/color.hh
figure/createtool.o: include/toad/types.hh include/toad/io/serializable.hh
figure/createtool.o: include/toad/io/atvparser.hh include/toad/font.hh
figure/createtool.o: include/toad/pointer.hh include/toad/matrix2d.hh
figure/createtool.o: include/toad/window.hh include/toad/interactor.hh
figure/createtool.o: include/toad/cursor.hh include/toad/region.hh
figure/createtool.o: include/toad/bitmap.hh include/toad/figuremodel.hh
figure/createtool.o: include/toad/model.hh include/toad/connect.hh
figure/createtool.o: include/toad/scrollpane.hh include/toad/undo.hh
figure/createtool.o: include/toad/boolmodel.hh include/toad/textmodel.hh
figure/createtool.o: include/toad/integermodel.hh include/toad/numbermodel.hh
figure/createtool.o: include/toad/floatmodel.hh
figure/createtool.o: include/toad/figure/createtool.hh
figure/createtool.o: include/toad/figuretool.hh include/toad/core.hh
figure/createtool.o: include/toad/pen.hh
geometry.o: include/toad/geometry.hh include/toad/types.hh
geometry.o: include/toad/vector.hh include/toad/penbase.hh
geometry.o: include/toad/color.hh include/toad/io/serializable.hh
geometry.o: include/toad/io/atvparser.hh include/toad/font.hh
geometry.o: include/toad/pointer.hh include/toad/matrix2d.hh
geometry.o: include/toad/window.hh include/toad/interactor.hh
geometry.o: include/toad/cursor.hh include/toad/region.hh
geometry.o: include/toad/bitmap.hh include/toad/figuremodel.hh
geometry.o: include/toad/model.hh include/toad/connect.hh
stacktrace.o: include/toad/stacktrace.hh
test_table.o: include/toad/table.hh include/toad/pen.hh
test_table.o: include/toad/penbase.hh include/toad/color.hh
test_table.o: include/toad/types.hh include/toad/io/serializable.hh
test_table.o: include/toad/io/atvparser.hh include/toad/font.hh
test_table.o: include/toad/pointer.hh include/toad/matrix2d.hh
test_table.o: include/toad/region.hh include/toad/scrollpane.hh
test_table.o: include/toad/window.hh include/toad/interactor.hh
test_table.o: include/toad/cursor.hh include/toad/model.hh
test_table.o: include/toad/connect.hh include/toad/dragndrop.hh
test_table.o: include/toad/checkbox.hh include/toad/labelowner.hh
test_table.o: include/toad/control.hh include/toad/boolmodel.hh
test_table.o: include/toad/radiobutton.hh include/toad/radiobuttonbase.hh
test_table.o: include/toad/buttonbase.hh include/toad/dialog.hh
test_table.o: include/toad/layout.hh include/toad/eventfilter.hh
test_table.o: include/toad/figuremodel.hh include/toad/stl/vector.hh
test_table.o: include/toad/utf8.hh
test_scroll.o: include/toad/table.hh include/toad/pen.hh
test_scroll.o: include/toad/penbase.hh include/toad/color.hh
test_scroll.o: include/toad/types.hh include/toad/io/serializable.hh
test_scroll.o: include/toad/io/atvparser.hh include/toad/font.hh
test_scroll.o: include/toad/pointer.hh include/toad/matrix2d.hh
test_scroll.o: include/toad/region.hh include/toad/scrollpane.hh
test_scroll.o: include/toad/window.hh include/toad/interactor.hh
test_scroll.o: include/toad/cursor.hh include/toad/model.hh
test_scroll.o: include/toad/connect.hh include/toad/dragndrop.hh
test_scroll.o: include/toad/checkbox.hh include/toad/labelowner.hh
test_scroll.o: include/toad/control.hh include/toad/boolmodel.hh
test_scroll.o: include/toad/radiobutton.hh include/toad/radiobuttonbase.hh
test_scroll.o: include/toad/buttonbase.hh include/toad/dialog.hh
test_scroll.o: include/toad/layout.hh include/toad/eventfilter.hh
test_scroll.o: include/toad/figuremodel.hh include/toad/stl/vector.hh
test_scroll.o: include/toad/utf8.hh
test_dialog.o: include/toad/dialog.hh include/toad/window.hh
test_dialog.o: include/toad/interactor.hh include/toad/types.hh
test_dialog.o: include/toad/cursor.hh include/toad/color.hh
test_dialog.o: include/toad/io/serializable.hh include/toad/io/atvparser.hh
test_dialog.o: include/toad/region.hh include/toad/layout.hh
test_dialog.o: include/toad/eventfilter.hh include/toad/figuremodel.hh
test_dialog.o: include/toad/model.hh include/toad/pointer.hh
test_dialog.o: include/toad/connect.hh include/toad/textarea.hh
test_dialog.o: include/toad/core.hh include/toad/pen.hh
test_dialog.o: include/toad/penbase.hh include/toad/font.hh
test_dialog.o: include/toad/matrix2d.hh include/toad/bitmap.hh
test_dialog.o: include/toad/control.hh include/toad/textmodel.hh
test_dialog.o: include/toad/undo.hh include/toad/scrollbar.hh
test_dialog.o: include/toad/integermodel.hh include/toad/numbermodel.hh
test_dialog.o: include/toad/checkbox.hh include/toad/labelowner.hh
test_dialog.o: include/toad/boolmodel.hh include/toad/pushbutton.hh
test_dialog.o: include/toad/buttonbase.hh include/toad/radiobutton.hh
test_dialog.o: include/toad/radiobuttonbase.hh
test_timer.o: include/toad/window.hh include/toad/interactor.hh
test_timer.o: include/toad/types.hh include/toad/cursor.hh
test_timer.o: include/toad/color.hh include/toad/io/serializable.hh
test_timer.o: include/toad/io/atvparser.hh include/toad/region.hh
test_timer.o: include/toad/simpletimer.hh
test_combobox.o: include/toad/combobox.hh include/toad/table.hh
test_combobox.o: include/toad/pen.hh include/toad/penbase.hh
test_combobox.o: include/toad/color.hh include/toad/types.hh
test_combobox.o: include/toad/io/serializable.hh include/toad/io/atvparser.hh
test_combobox.o: include/toad/font.hh include/toad/pointer.hh
test_combobox.o: include/toad/matrix2d.hh include/toad/region.hh
test_combobox.o: include/toad/scrollpane.hh include/toad/window.hh
test_combobox.o: include/toad/interactor.hh include/toad/cursor.hh
test_combobox.o: include/toad/model.hh include/toad/connect.hh
test_combobox.o: include/toad/dragndrop.hh
test_cursor.o: include/toad/window.hh include/toad/interactor.hh
test_cursor.o: include/toad/types.hh include/toad/cursor.hh
test_cursor.o: include/toad/color.hh include/toad/io/serializable.hh
test_cursor.o: include/toad/io/atvparser.hh include/toad/region.hh
test_cursor.o: include/toad/pen.hh include/toad/penbase.hh
test_cursor.o: include/toad/font.hh include/toad/pointer.hh
test_cursor.o: include/toad/matrix2d.hh
test_colordialog.o: include/toad/colordialog.hh include/toad/dialog.hh
test_colordialog.o: include/toad/window.hh include/toad/interactor.hh
test_colordialog.o: include/toad/types.hh include/toad/cursor.hh
test_colordialog.o: include/toad/color.hh include/toad/io/serializable.hh
test_colordialog.o: include/toad/io/atvparser.hh include/toad/region.hh
test_colordialog.o: include/toad/layout.hh include/toad/eventfilter.hh
test_colordialog.o: include/toad/figuremodel.hh include/toad/model.hh
test_colordialog.o: include/toad/pointer.hh include/toad/connect.hh
test_colordialog.o: include/toad/rgbmodel.hh include/toad/floatmodel.hh
test_colordialog.o: include/toad/numbermodel.hh include/toad/textmodel.hh
test_colordialog.o: include/toad/undo.hh include/toad/textarea.hh
test_colordialog.o: include/toad/core.hh include/toad/pen.hh
test_colordialog.o: include/toad/penbase.hh include/toad/font.hh
test_colordialog.o: include/toad/matrix2d.hh include/toad/bitmap.hh
test_colordialog.o: include/toad/control.hh include/toad/scrollbar.hh
test_colordialog.o: include/toad/integermodel.hh include/toad/dnd/color.hh
test_colordialog.o: include/toad/dragndrop.hh
test_grab.o: include/toad/window.hh include/toad/interactor.hh
test_grab.o: include/toad/types.hh include/toad/cursor.hh
test_grab.o: include/toad/color.hh include/toad/io/serializable.hh
test_grab.o: include/toad/io/atvparser.hh include/toad/region.hh
test_grab.o: include/toad/pen.hh include/toad/penbase.hh include/toad/font.hh
test_grab.o: include/toad/pointer.hh include/toad/matrix2d.hh
test_grab.o: include/toad/connect.hh
test_path.o: include/toad/window.hh include/toad/interactor.hh
test_path.o: include/toad/types.hh include/toad/cursor.hh
test_path.o: include/toad/color.hh include/toad/io/serializable.hh
test_path.o: include/toad/io/atvparser.hh include/toad/region.hh
test_path.o: include/toad/pen.hh include/toad/penbase.hh include/toad/font.hh
test_path.o: include/toad/pointer.hh include/toad/matrix2d.hh
test_path.o: include/toad/figure.hh include/toad/bitmap.hh
test_path.o: include/toad/figuremodel.hh include/toad/model.hh
test_path.o: include/toad/connect.hh include/toad/vector.hh
test_path.o: include/toad/geometry.hh
test_image.o: include/toad/window.hh include/toad/interactor.hh
test_image.o: include/toad/types.hh include/toad/cursor.hh
test_image.o: include/toad/color.hh include/toad/io/serializable.hh
test_image.o: include/toad/io/atvparser.hh include/toad/region.hh
test_image.o: include/toad/pen.hh include/toad/penbase.hh
test_image.o: include/toad/font.hh include/toad/pointer.hh
test_image.o: include/toad/matrix2d.hh include/toad/bitmap.hh
test_image.o: include/toad/connect.hh
test_curve.o: include/toad/window.hh include/toad/interactor.hh
test_curve.o: include/toad/types.hh include/toad/cursor.hh
test_curve.o: include/toad/color.hh include/toad/io/serializable.hh
test_curve.o: include/toad/io/atvparser.hh include/toad/region.hh
test_curve.o: include/toad/pen.hh include/toad/penbase.hh
test_curve.o: include/toad/font.hh include/toad/pointer.hh
test_curve.o: include/toad/matrix2d.hh include/toad/bitmap.hh
test_curve.o: include/toad/connect.hh
test_text.o: include/toad/window.hh include/toad/interactor.hh
test_text.o: include/toad/types.hh include/toad/cursor.hh
test_text.o: include/toad/color.hh include/toad/io/serializable.hh
test_text.o: include/toad/io/atvparser.hh include/toad/region.hh
test_text.o: include/toad/pen.hh include/toad/penbase.hh include/toad/font.hh
test_text.o: include/toad/pointer.hh include/toad/matrix2d.hh
test_text.o: include/toad/utf8.hh
test_tablet.o: include/toad/window.hh include/toad/interactor.hh
test_tablet.o: include/toad/types.hh include/toad/cursor.hh
test_tablet.o: include/toad/color.hh include/toad/io/serializable.hh
test_tablet.o: include/toad/io/atvparser.hh include/toad/region.hh
test_tablet.o: include/toad/pen.hh include/toad/penbase.hh
test_tablet.o: include/toad/font.hh include/toad/pointer.hh
test_tablet.o: include/toad/matrix2d.hh include/toad/figure.hh
test_tablet.o: include/toad/bitmap.hh include/toad/figuremodel.hh
test_tablet.o: include/toad/model.hh include/toad/connect.hh
test_tablet.o: include/toad/vector.hh include/toad/geometry.hh
test_path_bool.o: include/toad/window.hh include/toad/interactor.hh
test_path_bool.o: include/toad/types.hh include/toad/cursor.hh
test_path_bool.o: include/toad/color.hh include/toad/io/serializable.hh
test_path_bool.o: include/toad/io/atvparser.hh include/toad/region.hh
test_path_bool.o: include/toad/pen.hh include/toad/penbase.hh
test_path_bool.o: include/toad/font.hh include/toad/pointer.hh
test_path_bool.o: include/toad/matrix2d.hh include/toad/figure.hh
test_path_bool.o: include/toad/bitmap.hh include/toad/figuremodel.hh
test_path_bool.o: include/toad/model.hh include/toad/connect.hh
test_path_bool.o: include/toad/vector.hh include/toad/geometry.hh
window.o: include/toad/core.hh include/toad/window.hh
window.o: include/toad/interactor.hh include/toad/types.hh
window.o: include/toad/cursor.hh include/toad/color.hh
window.o: include/toad/io/serializable.hh include/toad/io/atvparser.hh
window.o: include/toad/region.hh include/toad/pen.hh include/toad/penbase.hh
window.o: include/toad/font.hh include/toad/pointer.hh
window.o: include/toad/matrix2d.hh include/toad/bitmap.hh
window.o: include/toad/connect.hh include/toad/layout.hh
window.o: include/toad/eventfilter.hh include/toad/focusmanager.hh
window.o: include/toad/io/urlstream.hh include/toad/command.hh
window.o: include/toad/stacktrace.hh
pen.o: include/toad/core.hh include/toad/window.hh include/toad/interactor.hh
pen.o: include/toad/types.hh include/toad/cursor.hh include/toad/color.hh
pen.o: include/toad/io/serializable.hh include/toad/io/atvparser.hh
pen.o: include/toad/region.hh include/toad/pen.hh include/toad/penbase.hh
pen.o: include/toad/font.hh include/toad/pointer.hh include/toad/matrix2d.hh
pen.o: include/toad/bitmap.hh include/toad/connect.hh
fischland/draw.o: fischland/fischland.hh include/toad/cursor.hh
fischland/draw.o: fischland/fischeditor.hh include/toad/figureeditor.hh
fischland/draw.o: include/toad/figure.hh include/toad/penbase.hh
fischland/draw.o: include/toad/color.hh include/toad/types.hh
fischland/draw.o: include/toad/io/serializable.hh
fischland/draw.o: include/toad/io/atvparser.hh include/toad/font.hh
fischland/draw.o: include/toad/pointer.hh include/toad/matrix2d.hh
fischland/draw.o: include/toad/window.hh include/toad/interactor.hh
fischland/draw.o: include/toad/region.hh include/toad/bitmap.hh
fischland/draw.o: include/toad/figuremodel.hh include/toad/model.hh
fischland/draw.o: include/toad/connect.hh include/toad/scrollpane.hh
fischland/draw.o: include/toad/undo.hh include/toad/boolmodel.hh
fischland/draw.o: include/toad/textmodel.hh include/toad/integermodel.hh
fischland/draw.o: include/toad/numbermodel.hh include/toad/floatmodel.hh
fischland/draw.o: include/toad/figure/createtool.hh
fischland/draw.o: include/toad/figuretool.hh include/toad/core.hh
fischland/draw.o: include/toad/pen.hh fischland/page.hh
fischland/draw.o: include/toad/dialog.hh include/toad/layout.hh
fischland/draw.o: include/toad/eventfilter.hh include/toad/treeadapter.hh
fischland/draw.o: include/toad/treemodel.hh include/toad/table.hh
fischland/draw.o: include/toad/dragndrop.hh fischland/toolbox.hh
fischland/draw.o: fischland/colorpalette.hh include/toad/combobox.hh
fischland/draw.o: include/toad/textfield.hh include/toad/textarea.hh
fischland/draw.o: include/toad/control.hh include/toad/scrollbar.hh
fischland/draw.o: include/toad/dnd/color.hh fischland/lineal.hh
fischland/draw.o: fischland/config.h include/toad/menubar.hh
fischland/draw.o: include/toad/menuhelper.hh include/toad/undomanager.hh
fischland/draw.o: include/toad/action.hh include/toad/command.hh
fischland/draw.o: include/toad/fatradiobutton.hh
fischland/draw.o: include/toad/radiobuttonbase.hh include/toad/buttonbase.hh
fischland/draw.o: include/toad/labelowner.hh include/toad/pushbutton.hh
fischland/draw.o: include/toad/colorselector.hh include/toad/filedialog.hh
fischland/draw.o: include/toad/stl/vector.hh include/toad/arrowbutton.hh
fischland/draw.o: include/toad/simpletimer.hh include/toad/exception.hh
fischland/draw.o: include/toad/fatcheckbutton.hh include/toad/htmlview.hh
fischland/draw.o: include/toad/popupmenu.hh include/toad/messagebox.hh
fischland/draw.o: include/toad/springlayout.hh fischland/rotatetool.hh
fischland/colorpalette.o: fischland/colorpalette.hh include/toad/window.hh
fischland/colorpalette.o: include/toad/interactor.hh include/toad/types.hh
fischland/colorpalette.o: include/toad/cursor.hh include/toad/color.hh
fischland/colorpalette.o: include/toad/io/serializable.hh
fischland/colorpalette.o: include/toad/io/atvparser.hh include/toad/region.hh
fischland/colorpalette.o: include/toad/combobox.hh include/toad/table.hh
fischland/colorpalette.o: include/toad/pen.hh include/toad/penbase.hh
fischland/colorpalette.o: include/toad/font.hh include/toad/pointer.hh
fischland/colorpalette.o: include/toad/matrix2d.hh include/toad/scrollpane.hh
fischland/colorpalette.o: include/toad/model.hh include/toad/connect.hh
fischland/colorpalette.o: include/toad/dragndrop.hh include/toad/textfield.hh
fischland/colorpalette.o: include/toad/textarea.hh include/toad/core.hh
fischland/colorpalette.o: include/toad/bitmap.hh include/toad/control.hh
fischland/colorpalette.o: include/toad/textmodel.hh include/toad/undo.hh
fischland/colorpalette.o: include/toad/scrollbar.hh
fischland/colorpalette.o: include/toad/integermodel.hh
fischland/colorpalette.o: include/toad/numbermodel.hh
fischland/colorpalette.o: include/toad/dnd/color.hh
fischland/colorpalette.o: include/toad/figureeditor.hh include/toad/figure.hh
fischland/colorpalette.o: include/toad/figuremodel.hh
fischland/colorpalette.o: include/toad/boolmodel.hh
fischland/colorpalette.o: include/toad/floatmodel.hh
fischland/colorpalette.o: include/toad/figure/createtool.hh
fischland/colorpalette.o: include/toad/figuretool.hh
fischland/colorpalette.o: include/toad/fatcheckbutton.hh
fischland/colorpalette.o: include/toad/buttonbase.hh
fischland/colorpalette.o: include/toad/labelowner.hh
fischland/colorpalette.o: include/toad/pushbutton.hh
fischland/fitcurve.o: include/toad/figure.hh include/toad/penbase.hh
fischland/fitcurve.o: include/toad/color.hh include/toad/types.hh
fischland/fitcurve.o: include/toad/io/serializable.hh
fischland/fitcurve.o: include/toad/io/atvparser.hh include/toad/font.hh
fischland/fitcurve.o: include/toad/pointer.hh include/toad/matrix2d.hh
fischland/fitcurve.o: include/toad/window.hh include/toad/interactor.hh
fischland/fitcurve.o: include/toad/cursor.hh include/toad/region.hh
fischland/fitcurve.o: include/toad/bitmap.hh include/toad/figuremodel.hh
fischland/fitcurve.o: include/toad/model.hh include/toad/connect.hh
fischland/fpath.o: fischland/fpath.hh include/toad/figure.hh
fischland/fpath.o: include/toad/penbase.hh include/toad/color.hh
fischland/fpath.o: include/toad/types.hh include/toad/io/serializable.hh
fischland/fpath.o: include/toad/io/atvparser.hh include/toad/font.hh
fischland/fpath.o: include/toad/pointer.hh include/toad/matrix2d.hh
fischland/fpath.o: include/toad/window.hh include/toad/interactor.hh
fischland/fpath.o: include/toad/cursor.hh include/toad/region.hh
fischland/fpath.o: include/toad/bitmap.hh include/toad/figuremodel.hh
fischland/fpath.o: include/toad/model.hh include/toad/connect.hh
fischland/fpath.o: include/toad/action.hh include/toad/popupmenu.hh
fischland/fpath.o: include/toad/menuhelper.hh include/toad/layout.hh
fischland/fpath.o: include/toad/eventfilter.hh include/toad/figureeditor.hh
fischland/fpath.o: include/toad/scrollpane.hh include/toad/undo.hh
fischland/fpath.o: include/toad/boolmodel.hh include/toad/textmodel.hh
fischland/fpath.o: include/toad/integermodel.hh include/toad/numbermodel.hh
fischland/fpath.o: include/toad/floatmodel.hh
fischland/fpath.o: include/toad/figure/createtool.hh
fischland/fpath.o: include/toad/figuretool.hh include/toad/core.hh
fischland/fpath.o: include/toad/pen.hh
fischland/lineal.o: fischland/lineal.hh include/toad/figureeditor.hh
fischland/lineal.o: include/toad/figure.hh include/toad/penbase.hh
fischland/lineal.o: include/toad/color.hh include/toad/types.hh
fischland/lineal.o: include/toad/io/serializable.hh
fischland/lineal.o: include/toad/io/atvparser.hh include/toad/font.hh
fischland/lineal.o: include/toad/pointer.hh include/toad/matrix2d.hh
fischland/lineal.o: include/toad/window.hh include/toad/interactor.hh
fischland/lineal.o: include/toad/cursor.hh include/toad/region.hh
fischland/lineal.o: include/toad/bitmap.hh include/toad/figuremodel.hh
fischland/lineal.o: include/toad/model.hh include/toad/connect.hh
fischland/lineal.o: include/toad/scrollpane.hh include/toad/undo.hh
fischland/lineal.o: include/toad/boolmodel.hh include/toad/textmodel.hh
fischland/lineal.o: include/toad/integermodel.hh include/toad/numbermodel.hh
fischland/lineal.o: include/toad/floatmodel.hh
fischland/lineal.o: include/toad/figure/createtool.hh
fischland/lineal.o: include/toad/figuretool.hh include/toad/core.hh
fischland/lineal.o: include/toad/pen.hh
fischland/page.o: fischland/page.hh include/toad/core.hh
fischland/page.o: include/toad/window.hh include/toad/interactor.hh
fischland/page.o: include/toad/types.hh include/toad/cursor.hh
fischland/page.o: include/toad/color.hh include/toad/io/serializable.hh
fischland/page.o: include/toad/io/atvparser.hh include/toad/region.hh
fischland/page.o: include/toad/pen.hh include/toad/penbase.hh
fischland/page.o: include/toad/font.hh include/toad/pointer.hh
fischland/page.o: include/toad/matrix2d.hh include/toad/bitmap.hh
fischland/page.o: include/toad/connect.hh include/toad/dialog.hh
fischland/page.o: include/toad/layout.hh include/toad/eventfilter.hh
fischland/page.o: include/toad/figuremodel.hh include/toad/model.hh
fischland/page.o: include/toad/treeadapter.hh include/toad/treemodel.hh
fischland/page.o: include/toad/table.hh include/toad/scrollpane.hh
fischland/page.o: include/toad/dragndrop.hh fischland/fischland.hh
fischland/page.o: include/toad/pushbutton.hh include/toad/buttonbase.hh
fischland/page.o: include/toad/labelowner.hh include/toad/control.hh
fischland/page.o: include/toad/textfield.hh include/toad/textarea.hh
fischland/page.o: include/toad/textmodel.hh include/toad/undo.hh
fischland/page.o: include/toad/scrollbar.hh include/toad/integermodel.hh
fischland/page.o: include/toad/numbermodel.hh
fischland/toolbox.o: fischland/toolbox.hh include/toad/dialog.hh
fischland/toolbox.o: include/toad/window.hh include/toad/interactor.hh
fischland/toolbox.o: include/toad/types.hh include/toad/cursor.hh
fischland/toolbox.o: include/toad/color.hh include/toad/io/serializable.hh
fischland/toolbox.o: include/toad/io/atvparser.hh include/toad/region.hh
fischland/toolbox.o: include/toad/layout.hh include/toad/eventfilter.hh
fischland/toolbox.o: include/toad/figuremodel.hh include/toad/model.hh
fischland/toolbox.o: include/toad/pointer.hh include/toad/connect.hh
fischland/toolbox.o: include/toad/figureeditor.hh include/toad/figure.hh
fischland/toolbox.o: include/toad/penbase.hh include/toad/font.hh
fischland/toolbox.o: include/toad/matrix2d.hh include/toad/bitmap.hh
fischland/toolbox.o: include/toad/scrollpane.hh include/toad/undo.hh
fischland/toolbox.o: include/toad/boolmodel.hh include/toad/textmodel.hh
fischland/toolbox.o: include/toad/integermodel.hh include/toad/numbermodel.hh
fischland/toolbox.o: include/toad/floatmodel.hh
fischland/toolbox.o: include/toad/figure/createtool.hh
fischland/toolbox.o: include/toad/figuretool.hh include/toad/core.hh
fischland/toolbox.o: include/toad/pen.hh include/toad/table.hh
fischland/toolbox.o: include/toad/dragndrop.hh fischland/selectiontool.hh
fischland/toolbox.o: include/toad/undomanager.hh include/toad/action.hh
fischland/toolbox.o: fischland/filltool.hh fischland/fpath.hh
fischland/toolbox.o: fischland/pentool.hh fischland/penciltool.hh
fischland/toolbox.o: fischland/directselectiontool.hh fischland/rotatetool.hh
fischland/toolbox.o: fischland/colorpicker.hh fischland/colorpalette.hh
fischland/toolbox.o: include/toad/combobox.hh include/toad/textfield.hh
fischland/toolbox.o: include/toad/textarea.hh include/toad/control.hh
fischland/toolbox.o: include/toad/scrollbar.hh include/toad/dnd/color.hh
fischland/toolbox.o: fischland/fischland.hh fischland/cairo.hh
fischland/toolbox.o: fischland/config.h include/toad/pushbutton.hh
fischland/toolbox.o: include/toad/buttonbase.hh include/toad/labelowner.hh
fischland/toolbox.o: include/toad/fatradiobutton.hh
fischland/toolbox.o: include/toad/radiobuttonbase.hh
fischland/toolbox.o: include/toad/fatcheckbutton.hh
fischland/toolbox.o: include/toad/colordialog.hh include/toad/rgbmodel.hh
fischland/toolbox.o: include/toad/gauge.hh include/toad/arrowbutton.hh
fischland/toolbox.o: include/toad/simpletimer.hh include/toad/messagebox.hh
fischland/toolbox.o: fischland/fontdialog.hh
fischland/colorpicker.o: fischland/colorpicker.hh
fischland/colorpicker.o: include/toad/figureeditor.hh include/toad/figure.hh
fischland/colorpicker.o: include/toad/penbase.hh include/toad/color.hh
fischland/colorpicker.o: include/toad/types.hh
fischland/colorpicker.o: include/toad/io/serializable.hh
fischland/colorpicker.o: include/toad/io/atvparser.hh include/toad/font.hh
fischland/colorpicker.o: include/toad/pointer.hh include/toad/matrix2d.hh
fischland/colorpicker.o: include/toad/window.hh include/toad/interactor.hh
fischland/colorpicker.o: include/toad/cursor.hh include/toad/region.hh
fischland/colorpicker.o: include/toad/bitmap.hh include/toad/figuremodel.hh
fischland/colorpicker.o: include/toad/model.hh include/toad/connect.hh
fischland/colorpicker.o: include/toad/scrollpane.hh include/toad/undo.hh
fischland/colorpicker.o: include/toad/boolmodel.hh include/toad/textmodel.hh
fischland/colorpicker.o: include/toad/integermodel.hh
fischland/colorpicker.o: include/toad/numbermodel.hh
fischland/colorpicker.o: include/toad/floatmodel.hh
fischland/colorpicker.o: include/toad/figure/createtool.hh
fischland/colorpicker.o: include/toad/figuretool.hh include/toad/core.hh
fischland/colorpicker.o: include/toad/pen.hh fischland/colorpalette.hh
fischland/colorpicker.o: include/toad/combobox.hh include/toad/table.hh
fischland/colorpicker.o: include/toad/dragndrop.hh include/toad/textfield.hh
fischland/colorpicker.o: include/toad/textarea.hh include/toad/control.hh
fischland/colorpicker.o: include/toad/scrollbar.hh include/toad/dnd/color.hh
fischland/colorpicker.o: fischland/toolbox.hh include/toad/dialog.hh
fischland/colorpicker.o: include/toad/layout.hh include/toad/eventfilter.hh
fischland/selectiontool.o: fischland/selectiontool.hh
fischland/selectiontool.o: include/toad/figureeditor.hh
fischland/selectiontool.o: include/toad/figure.hh include/toad/penbase.hh
fischland/selectiontool.o: include/toad/color.hh include/toad/types.hh
fischland/selectiontool.o: include/toad/io/serializable.hh
fischland/selectiontool.o: include/toad/io/atvparser.hh include/toad/font.hh
fischland/selectiontool.o: include/toad/pointer.hh include/toad/matrix2d.hh
fischland/selectiontool.o: include/toad/window.hh include/toad/interactor.hh
fischland/selectiontool.o: include/toad/cursor.hh include/toad/region.hh
fischland/selectiontool.o: include/toad/bitmap.hh include/toad/figuremodel.hh
fischland/selectiontool.o: include/toad/model.hh include/toad/connect.hh
fischland/selectiontool.o: include/toad/scrollpane.hh include/toad/undo.hh
fischland/selectiontool.o: include/toad/boolmodel.hh
fischland/selectiontool.o: include/toad/textmodel.hh
fischland/selectiontool.o: include/toad/integermodel.hh
fischland/selectiontool.o: include/toad/numbermodel.hh
fischland/selectiontool.o: include/toad/floatmodel.hh
fischland/selectiontool.o: include/toad/figure/createtool.hh
fischland/selectiontool.o: include/toad/figuretool.hh include/toad/core.hh
fischland/selectiontool.o: include/toad/pen.hh include/toad/undomanager.hh
fischland/selectiontool.o: include/toad/action.hh
fischland/directselectiontool.o: fischland/directselectiontool.hh
fischland/directselectiontool.o: include/toad/figureeditor.hh
fischland/directselectiontool.o: include/toad/figure.hh
fischland/directselectiontool.o: include/toad/penbase.hh
fischland/directselectiontool.o: include/toad/color.hh include/toad/types.hh
fischland/directselectiontool.o: include/toad/io/serializable.hh
fischland/directselectiontool.o: include/toad/io/atvparser.hh
fischland/directselectiontool.o: include/toad/font.hh include/toad/pointer.hh
fischland/directselectiontool.o: include/toad/matrix2d.hh
fischland/directselectiontool.o: include/toad/window.hh
fischland/directselectiontool.o: include/toad/interactor.hh
fischland/directselectiontool.o: include/toad/cursor.hh
fischland/directselectiontool.o: include/toad/region.hh
fischland/directselectiontool.o: include/toad/bitmap.hh
fischland/directselectiontool.o: include/toad/figuremodel.hh
fischland/directselectiontool.o: include/toad/model.hh
fischland/directselectiontool.o: include/toad/connect.hh
fischland/directselectiontool.o: include/toad/scrollpane.hh
fischland/directselectiontool.o: include/toad/undo.hh
fischland/directselectiontool.o: include/toad/boolmodel.hh
fischland/directselectiontool.o: include/toad/textmodel.hh
fischland/directselectiontool.o: include/toad/integermodel.hh
fischland/directselectiontool.o: include/toad/numbermodel.hh
fischland/directselectiontool.o: include/toad/floatmodel.hh
fischland/directselectiontool.o: include/toad/figure/createtool.hh
fischland/directselectiontool.o: include/toad/figuretool.hh
fischland/directselectiontool.o: include/toad/core.hh include/toad/pen.hh
fischland/directselectiontool.o: include/toad/undomanager.hh
fischland/directselectiontool.o: include/toad/action.hh
fischland/rotatetool.o: fischland/rotatetool.hh include/toad/figureeditor.hh
fischland/rotatetool.o: include/toad/figure.hh include/toad/penbase.hh
fischland/rotatetool.o: include/toad/color.hh include/toad/types.hh
fischland/rotatetool.o: include/toad/io/serializable.hh
fischland/rotatetool.o: include/toad/io/atvparser.hh include/toad/font.hh
fischland/rotatetool.o: include/toad/pointer.hh include/toad/matrix2d.hh
fischland/rotatetool.o: include/toad/window.hh include/toad/interactor.hh
fischland/rotatetool.o: include/toad/cursor.hh include/toad/region.hh
fischland/rotatetool.o: include/toad/bitmap.hh include/toad/figuremodel.hh
fischland/rotatetool.o: include/toad/model.hh include/toad/connect.hh
fischland/rotatetool.o: include/toad/scrollpane.hh include/toad/undo.hh
fischland/rotatetool.o: include/toad/boolmodel.hh include/toad/textmodel.hh
fischland/rotatetool.o: include/toad/integermodel.hh
fischland/rotatetool.o: include/toad/numbermodel.hh
fischland/rotatetool.o: include/toad/floatmodel.hh
fischland/rotatetool.o: include/toad/figure/createtool.hh
fischland/rotatetool.o: include/toad/figuretool.hh include/toad/core.hh
fischland/rotatetool.o: include/toad/pen.hh fischland/fischland.hh
fischland/pentool.o: fischland/pentool.hh fischland/fpath.hh
fischland/pentool.o: include/toad/figure.hh include/toad/penbase.hh
fischland/pentool.o: include/toad/color.hh include/toad/types.hh
fischland/pentool.o: include/toad/io/serializable.hh
fischland/pentool.o: include/toad/io/atvparser.hh include/toad/font.hh
fischland/pentool.o: include/toad/pointer.hh include/toad/matrix2d.hh
fischland/pentool.o: include/toad/window.hh include/toad/interactor.hh
fischland/pentool.o: include/toad/cursor.hh include/toad/region.hh
fischland/pentool.o: include/toad/bitmap.hh include/toad/figuremodel.hh
fischland/pentool.o: include/toad/model.hh include/toad/connect.hh
fischland/pentool.o: include/toad/figureeditor.hh include/toad/scrollpane.hh
fischland/pentool.o: include/toad/undo.hh include/toad/boolmodel.hh
fischland/pentool.o: include/toad/textmodel.hh include/toad/integermodel.hh
fischland/pentool.o: include/toad/numbermodel.hh include/toad/floatmodel.hh
fischland/pentool.o: include/toad/figure/createtool.hh
fischland/pentool.o: include/toad/figuretool.hh include/toad/core.hh
fischland/pentool.o: include/toad/pen.hh fischland/fischland.hh
fischland/penciltool.o: fischland/penciltool.hh fischland/fpath.hh
fischland/penciltool.o: include/toad/figure.hh include/toad/penbase.hh
fischland/penciltool.o: include/toad/color.hh include/toad/types.hh
fischland/penciltool.o: include/toad/io/serializable.hh
fischland/penciltool.o: include/toad/io/atvparser.hh include/toad/font.hh
fischland/penciltool.o: include/toad/pointer.hh include/toad/matrix2d.hh
fischland/penciltool.o: include/toad/window.hh include/toad/interactor.hh
fischland/penciltool.o: include/toad/cursor.hh include/toad/region.hh
fischland/penciltool.o: include/toad/bitmap.hh include/toad/figuremodel.hh
fischland/penciltool.o: include/toad/model.hh include/toad/connect.hh
fischland/penciltool.o: include/toad/boolmodel.hh
fischland/penciltool.o: include/toad/integermodel.hh
fischland/penciltool.o: include/toad/numbermodel.hh include/toad/textmodel.hh
fischland/penciltool.o: include/toad/undo.hh include/toad/figureeditor.hh
fischland/penciltool.o: include/toad/scrollpane.hh include/toad/floatmodel.hh
fischland/penciltool.o: include/toad/figure/createtool.hh
fischland/penciltool.o: include/toad/figuretool.hh include/toad/core.hh
fischland/penciltool.o: include/toad/pen.hh fischland/fischland.hh
fischland/penciltool.o: include/toad/undomanager.hh include/toad/action.hh
fischland/penciltool.o: include/toad/textfield.hh include/toad/textarea.hh
fischland/penciltool.o: include/toad/control.hh include/toad/scrollbar.hh
fischland/penciltool.o: include/toad/checkbox.hh include/toad/labelowner.hh
fischland/filltool.o: fischland/filltool.hh include/toad/figureeditor.hh
fischland/filltool.o: include/toad/figure.hh include/toad/penbase.hh
fischland/filltool.o: include/toad/color.hh include/toad/types.hh
fischland/filltool.o: include/toad/io/serializable.hh
fischland/filltool.o: include/toad/io/atvparser.hh include/toad/font.hh
fischland/filltool.o: include/toad/pointer.hh include/toad/matrix2d.hh
fischland/filltool.o: include/toad/window.hh include/toad/interactor.hh
fischland/filltool.o: include/toad/cursor.hh include/toad/region.hh
fischland/filltool.o: include/toad/bitmap.hh include/toad/figuremodel.hh
fischland/filltool.o: include/toad/model.hh include/toad/connect.hh
fischland/filltool.o: include/toad/scrollpane.hh include/toad/undo.hh
fischland/filltool.o: include/toad/boolmodel.hh include/toad/textmodel.hh
fischland/filltool.o: include/toad/integermodel.hh
fischland/filltool.o: include/toad/numbermodel.hh include/toad/floatmodel.hh
fischland/filltool.o: include/toad/figure/createtool.hh
fischland/filltool.o: include/toad/figuretool.hh include/toad/core.hh
fischland/filltool.o: include/toad/pen.hh fischland/fpath.hh
fischland/filltoolutil.o: fischland/filltool.hh include/toad/figureeditor.hh
fischland/filltoolutil.o: include/toad/figure.hh include/toad/penbase.hh
fischland/filltoolutil.o: include/toad/color.hh include/toad/types.hh
fischland/filltoolutil.o: include/toad/io/serializable.hh
fischland/filltoolutil.o: include/toad/io/atvparser.hh include/toad/font.hh
fischland/filltoolutil.o: include/toad/pointer.hh include/toad/matrix2d.hh
fischland/filltoolutil.o: include/toad/window.hh include/toad/interactor.hh
fischland/filltoolutil.o: include/toad/cursor.hh include/toad/region.hh
fischland/filltoolutil.o: include/toad/bitmap.hh include/toad/figuremodel.hh
fischland/filltoolutil.o: include/toad/model.hh include/toad/connect.hh
fischland/filltoolutil.o: include/toad/scrollpane.hh include/toad/undo.hh
fischland/filltoolutil.o: include/toad/boolmodel.hh include/toad/textmodel.hh
fischland/filltoolutil.o: include/toad/integermodel.hh
fischland/filltoolutil.o: include/toad/numbermodel.hh
fischland/filltoolutil.o: include/toad/floatmodel.hh
fischland/filltoolutil.o: include/toad/figure/createtool.hh
fischland/filltoolutil.o: include/toad/figuretool.hh include/toad/core.hh
fischland/filltoolutil.o: include/toad/pen.hh fischland/fpath.hh
fischland/fischeditor.o: fischland/fischeditor.hh
fischland/fischeditor.o: include/toad/figureeditor.hh include/toad/figure.hh
fischland/fischeditor.o: include/toad/penbase.hh include/toad/color.hh
fischland/fischeditor.o: include/toad/types.hh
fischland/fischeditor.o: include/toad/io/serializable.hh
fischland/fischeditor.o: include/toad/io/atvparser.hh include/toad/font.hh
fischland/fischeditor.o: include/toad/pointer.hh include/toad/matrix2d.hh
fischland/fischeditor.o: include/toad/window.hh include/toad/interactor.hh
fischland/fischeditor.o: include/toad/cursor.hh include/toad/region.hh
fischland/fischeditor.o: include/toad/bitmap.hh include/toad/figuremodel.hh
fischland/fischeditor.o: include/toad/model.hh include/toad/connect.hh
fischland/fischeditor.o: include/toad/scrollpane.hh include/toad/undo.hh
fischland/fischeditor.o: include/toad/boolmodel.hh include/toad/textmodel.hh
fischland/fischeditor.o: include/toad/integermodel.hh
fischland/fischeditor.o: include/toad/numbermodel.hh
fischland/fischeditor.o: include/toad/floatmodel.hh
fischland/fischeditor.o: include/toad/figure/createtool.hh
fischland/fischeditor.o: include/toad/figuretool.hh include/toad/core.hh
fischland/fischeditor.o: include/toad/pen.hh fischland/page.hh
fischland/fischeditor.o: include/toad/dialog.hh include/toad/layout.hh
fischland/fischeditor.o: include/toad/eventfilter.hh
fischland/fischeditor.o: include/toad/treeadapter.hh
fischland/fischeditor.o: include/toad/treemodel.hh include/toad/table.hh
fischland/fischeditor.o: include/toad/dragndrop.hh
