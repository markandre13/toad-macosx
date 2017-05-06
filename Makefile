.PHONY: all run depend test gdb doc

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
	   gauge.cc colordialog.cc dragndrop.cc rgbmodel.cc types.cc \
	   dnd/dropobject.cc dnd/color.cc dnd/textplain.cc dnd/image.cc \
	   figure/toolbox.cc \
	   figure/toolpanel.cc \
	   figure/toolbutton.cc \
	   figure/frame.cc figure/image.cc figure/arrow.cc \
	   figure/text.cc figure/circle.cc figure/group.cc \
	   figure/transform.cc figure/perspectivetransform.cc \
	   figure/rectangle.cc figure/window.cc \
	   figure/selectiontool.cc \
	   figure/nodetool.cc \
	   figure/shapetool.cc \
	   figure/texttool.cc \
	   figure/connecttool.cc figure/connectfigure.cc \
	   vector.cc geometry.cc wordprocessor.cc \
	   stacktrace.cc \
	   \
	   test_table.cc test_scroll.cc test_dialog.cc test_timer.cc \
	   test_combobox.cc test_cursor.cc test_colordialog.cc test_grab.cc \
	   test_path.cc test_image.cc test_curve.cc test_text.cc test_tablet.cc \
	   test_path_bool.cc test_guitar.cc test_vector_buffer.cc test_path_offset.cc \
	   test_toolbar.cc \
	   \
	   booleanop.cc \
	   \
	   bop12/booleanop.cc bop12/polygon.cc bop12/utilities.cc
	
SRC_COCOA=window.cc mouseevent.cc pen.cc

SRC_FISH=fischland/draw.cc fischland/colorpalette.cc fischland/fitcurve.cc \
	 fischland/fpath.cc fischland/lineal.cc fischland/page.cc \
	 fischland/fishbox.cc fischland/colorpicker.cc \
	 fischland/rotatetool.cc \
	 fischland/pentool.cc fischland/penciltool.cc \
	 fischland/filltool.cc fischland/filltoolutil.cc \
	 fischland/fischeditor.cc

SRC_TEST=test/main.cc test/util.cc test/gtest-all.cc \
	 test/display.cc test/signal.cc \
	 test/figureeditor.cc test/figureeditor-render.cc \
	 test/wordprocessor.cc \
	 test/serializable.cc \
	 test/rectangle.cc \
	 test/booleanop.cc test/lineintersection.cc test/fitcurve.cc

#fischland/fontdialog.cc

SRC=$(SRC_SHARED) $(SRC_COCOA) $(SRC_FISH)
CXX=g++ -ObjC++ -std=gnu++1z
#CXX=clang --language=objective-c++ --std=gnu++1z
CXXFLAGS=-g -O0 \
	 -frtti -fsanitize=address -fno-omit-frame-pointer -fno-optimize-sibling-calls \
	 -Wall \
	 -Winconsistent-missing-override \
	 -Werror=inconsistent-missing-override \
	 -Werror=overloaded-virtual \
	 -Wno-switch \
	 -Wno-unused-variable \
	 -Wno-unneeded-internal-declaration

OBJS    = $(SRC:.cc=.o)

$(EXEC): $(OBJS)
	@mkdir -p fischland.app/Contents/MacOS
	$(CXX) \
	-fsanitize=address \
	-framework CoreFoundation \
	-framework AppKit \
	$(OBJS) -o $(EXEC)
	@echo Ok

TEST_SRC=$(SRC_TEST) $(SRC_SHARED) $(SRC_COCOA)
TEST_OBJ=$(TEST_SRC:.cc=.o)

test.app/Contents/MacOS/test: $(TEST_OBJ)
	@mkdir -p test.app/Contents/MacOS
	$(CXX) \
	-fsanitize=address \
	-framework CoreFoundation \
	-framework AppKit \
	$(TEST_OBJ) -o test.app/Contents/MacOS/test
	@echo Ok

test: test.app/Contents/MacOS/test
	./test.app/Contents/MacOS/test --gtest_filter="FigureEditor.*"
#	./test.app/Contents/MacOS/test --gtest_filter="Rectangle.*"
#	./test.app/Contents/MacOS/test --gtest_filter="Serializeable.List"
#	./test.app/Contents/MacOS/test --gtest_filter="FigureEditor.RelatedFigures"
#	./test.app/Contents/MacOS/test

doc:
	cd doc && /Applications/Doxygen.app/Contents/Resources/doxygen

clean:
	rm -f $(OBJS) $(EXEC) .gdb_history
	find . -name "*~" -exec rm {} \;
	find . -name "*.bak" -exec rm {} \;
	find . -name "DEADJOE" -exec rm {} \;

depend:
	@makedepend -Iinclude -Y $(SRC) $(TEST_SRC) 2> /dev/null

Makefile.dep:
	$(CXX) -MM -Iinclude $(CXXFLAGS) $(SRC) $(TEST_SRC) > Makefile.dep

run:
	fischland.app/Contents/MacOS/fischland fischland/example.fish

gdb:
	lldb fischland.app/Contents/MacOS/fischland

.SUFFIXES: .cc .M .o

.cc.o:
	$(CXX) -Iinclude $(CXXFLAGS) -c -o $*.o $*.cc

# DO NOT DELETE

interactor.o: include/toad/interactor.hh include/toad/types.hh
interactor.o: include/toad/io/serializable.hh include/toad/io/atvparser.hh
interactor.o: include/toad/connect.hh
control.o: include/toad/control.hh include/toad/window.hh
control.o: include/toad/interactor.hh include/toad/types.hh
control.o: include/toad/io/serializable.hh include/toad/io/atvparser.hh
control.o: include/toad/connect.hh include/toad/cursor.hh
control.o: include/toad/color.hh include/toad/region.hh
labelowner.o: include/toad/labelowner.hh include/toad/control.hh
labelowner.o: include/toad/window.hh include/toad/interactor.hh
labelowner.o: include/toad/types.hh include/toad/io/serializable.hh
labelowner.o: include/toad/io/atvparser.hh include/toad/connect.hh
labelowner.o: include/toad/cursor.hh include/toad/color.hh
labelowner.o: include/toad/region.hh
buttonbase.o: include/toad/core.hh include/toad/window.hh
buttonbase.o: include/toad/interactor.hh include/toad/types.hh
buttonbase.o: include/toad/io/serializable.hh include/toad/io/atvparser.hh
buttonbase.o: include/toad/connect.hh include/toad/cursor.hh
buttonbase.o: include/toad/color.hh include/toad/region.hh
buttonbase.o: include/toad/pen.hh include/toad/penbase.hh
buttonbase.o: include/toad/font.hh include/toad/pointer.hh
buttonbase.o: include/toad/matrix2d.hh include/toad/bitmap.hh
buttonbase.o: include/toad/buttonbase.hh include/toad/labelowner.hh
buttonbase.o: include/toad/control.hh
pushbutton.o: include/toad/core.hh include/toad/window.hh
pushbutton.o: include/toad/interactor.hh include/toad/types.hh
pushbutton.o: include/toad/io/serializable.hh include/toad/io/atvparser.hh
pushbutton.o: include/toad/connect.hh include/toad/cursor.hh
pushbutton.o: include/toad/color.hh include/toad/region.hh
pushbutton.o: include/toad/pen.hh include/toad/penbase.hh
pushbutton.o: include/toad/font.hh include/toad/pointer.hh
pushbutton.o: include/toad/matrix2d.hh include/toad/bitmap.hh
pushbutton.o: include/toad/buttonbase.hh include/toad/labelowner.hh
pushbutton.o: include/toad/control.hh include/toad/pushbutton.hh
checkbox.o: include/toad/pen.hh include/toad/penbase.hh include/toad/color.hh
checkbox.o: include/toad/types.hh include/toad/io/serializable.hh
checkbox.o: include/toad/io/atvparser.hh include/toad/font.hh
checkbox.o: include/toad/pointer.hh include/toad/matrix2d.hh
checkbox.o: include/toad/window.hh include/toad/interactor.hh
checkbox.o: include/toad/connect.hh include/toad/cursor.hh
checkbox.o: include/toad/region.hh include/toad/checkbox.hh
checkbox.o: include/toad/labelowner.hh include/toad/control.hh
checkbox.o: include/toad/boolmodel.hh include/toad/model.hh
penbase.o: include/toad/penbase.hh include/toad/color.hh
penbase.o: include/toad/types.hh include/toad/io/serializable.hh
penbase.o: include/toad/io/atvparser.hh include/toad/font.hh
penbase.o: include/toad/pointer.hh include/toad/matrix2d.hh
focusmanager.o: include/toad/focusmanager.hh include/toad/window.hh
focusmanager.o: include/toad/interactor.hh include/toad/types.hh
focusmanager.o: include/toad/io/serializable.hh include/toad/io/atvparser.hh
focusmanager.o: include/toad/connect.hh include/toad/cursor.hh
focusmanager.o: include/toad/color.hh include/toad/region.hh
focusmanager.o: include/toad/eventfilter.hh
textarea.o: include/toad/textarea.hh include/toad/core.hh
textarea.o: include/toad/window.hh include/toad/interactor.hh
textarea.o: include/toad/types.hh include/toad/io/serializable.hh
textarea.o: include/toad/io/atvparser.hh include/toad/connect.hh
textarea.o: include/toad/cursor.hh include/toad/color.hh
textarea.o: include/toad/region.hh include/toad/pen.hh
textarea.o: include/toad/penbase.hh include/toad/font.hh
textarea.o: include/toad/pointer.hh include/toad/matrix2d.hh
textarea.o: include/toad/bitmap.hh include/toad/control.hh
textarea.o: include/toad/textmodel.hh include/toad/model.hh
textarea.o: include/toad/undo.hh include/toad/scrollbar.hh
textarea.o: include/toad/integermodel.hh include/toad/numbermodel.hh
textarea.o: include/toad/simpletimer.hh include/toad/undomanager.hh
textarea.o: include/toad/action.hh include/toad/utf8.hh
textfield.o: include/toad/textfield.hh include/toad/textarea.hh
textfield.o: include/toad/core.hh include/toad/window.hh
textfield.o: include/toad/interactor.hh include/toad/types.hh
textfield.o: include/toad/io/serializable.hh include/toad/io/atvparser.hh
textfield.o: include/toad/connect.hh include/toad/cursor.hh
textfield.o: include/toad/color.hh include/toad/region.hh include/toad/pen.hh
textfield.o: include/toad/penbase.hh include/toad/font.hh
textfield.o: include/toad/pointer.hh include/toad/matrix2d.hh
textfield.o: include/toad/bitmap.hh include/toad/control.hh
textfield.o: include/toad/textmodel.hh include/toad/model.hh
textfield.o: include/toad/undo.hh include/toad/scrollbar.hh
textfield.o: include/toad/integermodel.hh include/toad/numbermodel.hh
scrollpane.o: include/toad/scrollpane.hh include/toad/window.hh
scrollpane.o: include/toad/interactor.hh include/toad/types.hh
scrollpane.o: include/toad/io/serializable.hh include/toad/io/atvparser.hh
scrollpane.o: include/toad/connect.hh include/toad/cursor.hh
scrollpane.o: include/toad/color.hh include/toad/region.hh
scrollpane.o: include/toad/penbase.hh include/toad/font.hh
scrollpane.o: include/toad/pointer.hh include/toad/matrix2d.hh
scrollpane.o: include/toad/scrollbar.hh include/toad/control.hh
scrollpane.o: include/toad/integermodel.hh include/toad/numbermodel.hh
scrollpane.o: include/toad/model.hh include/toad/textmodel.hh
scrollpane.o: include/toad/undo.hh
table.o: include/toad/table.hh include/toad/pen.hh include/toad/penbase.hh
table.o: include/toad/color.hh include/toad/types.hh
table.o: include/toad/io/serializable.hh include/toad/io/atvparser.hh
table.o: include/toad/font.hh include/toad/pointer.hh
table.o: include/toad/matrix2d.hh include/toad/region.hh
table.o: include/toad/scrollpane.hh include/toad/window.hh
table.o: include/toad/interactor.hh include/toad/connect.hh
table.o: include/toad/cursor.hh include/toad/model.hh
table.o: include/toad/dragndrop.hh include/toad/figure.hh
table.o: include/toad/bitmap.hh include/toad/figuremodel.hh
table.o: include/toad/wordprocessor.hh
menuhelper.o: include/toad/menuhelper.hh include/toad/window.hh
menuhelper.o: include/toad/interactor.hh include/toad/types.hh
menuhelper.o: include/toad/io/serializable.hh include/toad/io/atvparser.hh
menuhelper.o: include/toad/connect.hh include/toad/cursor.hh
menuhelper.o: include/toad/color.hh include/toad/region.hh
menuhelper.o: include/toad/layout.hh include/toad/eventfilter.hh
menuhelper.o: include/toad/menubutton.hh include/toad/io/urlstream.hh
menuhelper.o: include/toad/action.hh include/toad/model.hh
menuhelper.o: include/toad/pointer.hh
menubutton.o: include/toad/menubutton.hh include/toad/menuhelper.hh
menubutton.o: include/toad/window.hh include/toad/interactor.hh
menubutton.o: include/toad/types.hh include/toad/io/serializable.hh
menubutton.o: include/toad/io/atvparser.hh include/toad/connect.hh
menubutton.o: include/toad/cursor.hh include/toad/color.hh
menubutton.o: include/toad/region.hh include/toad/layout.hh
menubutton.o: include/toad/eventfilter.hh include/toad/pen.hh
menubutton.o: include/toad/penbase.hh include/toad/font.hh
menubutton.o: include/toad/pointer.hh include/toad/matrix2d.hh
menubutton.o: include/toad/popup.hh include/toad/action.hh
menubutton.o: include/toad/model.hh include/toad/command.hh
menubutton.o: include/toad/bitmap.hh include/toad/focusmanager.hh
menubar.o: include/toad/menubar.hh include/toad/menuhelper.hh
menubar.o: include/toad/window.hh include/toad/interactor.hh
menubar.o: include/toad/types.hh include/toad/io/serializable.hh
menubar.o: include/toad/io/atvparser.hh include/toad/connect.hh
menubar.o: include/toad/cursor.hh include/toad/color.hh
menubar.o: include/toad/region.hh include/toad/layout.hh
menubar.o: include/toad/eventfilter.hh include/toad/focusmanager.hh
menubar.o: include/toad/pushbutton.hh include/toad/buttonbase.hh
menubar.o: include/toad/labelowner.hh include/toad/control.hh
menubar.o: include/toad/textfield.hh include/toad/textarea.hh
menubar.o: include/toad/core.hh include/toad/pen.hh include/toad/penbase.hh
menubar.o: include/toad/font.hh include/toad/pointer.hh
menubar.o: include/toad/matrix2d.hh include/toad/bitmap.hh
menubar.o: include/toad/textmodel.hh include/toad/model.hh
menubar.o: include/toad/undo.hh include/toad/scrollbar.hh
menubar.o: include/toad/integermodel.hh include/toad/numbermodel.hh
menubar.o: include/toad/action.hh include/toad/utf8.hh
popup.o: include/toad/popup.hh include/toad/menuhelper.hh
popup.o: include/toad/window.hh include/toad/interactor.hh
popup.o: include/toad/types.hh include/toad/io/serializable.hh
popup.o: include/toad/io/atvparser.hh include/toad/connect.hh
popup.o: include/toad/cursor.hh include/toad/color.hh include/toad/region.hh
popup.o: include/toad/layout.hh include/toad/eventfilter.hh
popupmenu.o: include/toad/popupmenu.hh include/toad/menuhelper.hh
popupmenu.o: include/toad/window.hh include/toad/interactor.hh
popupmenu.o: include/toad/types.hh include/toad/io/serializable.hh
popupmenu.o: include/toad/io/atvparser.hh include/toad/connect.hh
popupmenu.o: include/toad/cursor.hh include/toad/color.hh
popupmenu.o: include/toad/region.hh include/toad/layout.hh
popupmenu.o: include/toad/eventfilter.hh include/toad/focusmanager.hh
command.o: include/toad/command.hh include/toad/pointer.hh
command.o: include/toad/window.hh include/toad/interactor.hh
command.o: include/toad/types.hh include/toad/io/serializable.hh
command.o: include/toad/io/atvparser.hh include/toad/connect.hh
command.o: include/toad/cursor.hh include/toad/color.hh
command.o: include/toad/region.hh
dialog.o: include/toad/pen.hh include/toad/penbase.hh include/toad/color.hh
dialog.o: include/toad/types.hh include/toad/io/serializable.hh
dialog.o: include/toad/io/atvparser.hh include/toad/font.hh
dialog.o: include/toad/pointer.hh include/toad/matrix2d.hh
dialog.o: include/toad/window.hh include/toad/interactor.hh
dialog.o: include/toad/connect.hh include/toad/cursor.hh
dialog.o: include/toad/region.hh include/toad/dialog.hh
dialog.o: include/toad/layout.hh include/toad/eventfilter.hh
dialog.o: include/toad/figuremodel.hh include/toad/model.hh
dialog.o: include/toad/layouteditor.hh include/toad/control.hh
dialog.o: include/toad/dialogeditor.hh include/toad/figure.hh
dialog.o: include/toad/bitmap.hh include/toad/wordprocessor.hh
dialog.o: include/toad/labelowner.hh include/toad/io/urlstream.hh
dialog.o: include/toad/focusmanager.hh include/toad/undomanager.hh
dialog.o: include/toad/action.hh include/toad/undo.hh
dialog.o: include/toad/scrollbar.hh include/toad/integermodel.hh
dialog.o: include/toad/numbermodel.hh include/toad/textmodel.hh
dialog.o: include/toad/pushbutton.hh include/toad/buttonbase.hh
dialog.o: include/toad/fatradiobutton.hh include/toad/radiobuttonbase.hh
dialog.o: include/toad/textfield.hh include/toad/textarea.hh
dialog.o: include/toad/core.hh include/toad/figureeditor.hh
dialog.o: include/toad/scrollpane.hh include/toad/boolmodel.hh
dialog.o: include/toad/floatmodel.hh include/toad/figure/toolbox.hh
dialog.o: include/toad/colorselector.hh include/toad/dnd/color.hh
dialog.o: include/toad/dragndrop.hh include/toad/figure/nodetool.hh
dialog.o: include/toad/figuretool.hh
dialogeditor.o: include/toad/dialogeditor.hh include/toad/dialog.hh
dialogeditor.o: include/toad/window.hh include/toad/interactor.hh
dialogeditor.o: include/toad/types.hh include/toad/io/serializable.hh
dialogeditor.o: include/toad/io/atvparser.hh include/toad/connect.hh
dialogeditor.o: include/toad/cursor.hh include/toad/color.hh
dialogeditor.o: include/toad/region.hh include/toad/layout.hh
dialogeditor.o: include/toad/eventfilter.hh include/toad/figuremodel.hh
dialogeditor.o: include/toad/model.hh include/toad/pointer.hh
dialogeditor.o: include/toad/pushbutton.hh include/toad/buttonbase.hh
dialogeditor.o: include/toad/labelowner.hh include/toad/control.hh
dialogeditor.o: include/toad/fatcheckbutton.hh include/toad/boolmodel.hh
dialogeditor.o: include/toad/layouteditor.hh include/toad/focusmanager.hh
colorselector.o: include/toad/colorselector.hh include/toad/dnd/color.hh
colorselector.o: include/toad/dragndrop.hh include/toad/types.hh
colorselector.o: include/toad/io/serializable.hh include/toad/io/atvparser.hh
colorselector.o: include/toad/pointer.hh include/toad/connect.hh
colorselector.o: include/toad/color.hh include/toad/figureeditor.hh
colorselector.o: include/toad/figure.hh include/toad/penbase.hh
colorselector.o: include/toad/font.hh include/toad/matrix2d.hh
colorselector.o: include/toad/window.hh include/toad/interactor.hh
colorselector.o: include/toad/cursor.hh include/toad/region.hh
colorselector.o: include/toad/bitmap.hh include/toad/figuremodel.hh
colorselector.o: include/toad/model.hh include/toad/wordprocessor.hh
colorselector.o: include/toad/pen.hh include/toad/scrollpane.hh
colorselector.o: include/toad/undo.hh include/toad/boolmodel.hh
colorselector.o: include/toad/textmodel.hh include/toad/integermodel.hh
colorselector.o: include/toad/numbermodel.hh include/toad/floatmodel.hh
colorselector.o: include/toad/figure/toolbox.hh include/toad/action.hh
colorselector.o: include/toad/colordialog.hh include/toad/dialog.hh
colorselector.o: include/toad/layout.hh include/toad/eventfilter.hh
colorselector.o: include/toad/rgbmodel.hh include/toad/textarea.hh
colorselector.o: include/toad/core.hh include/toad/control.hh
colorselector.o: include/toad/scrollbar.hh include/toad/pushbutton.hh
colorselector.o: include/toad/buttonbase.hh include/toad/labelowner.hh
colorselector.o: include/toad/undomanager.hh
fatradiobutton.o: include/toad/pen.hh include/toad/penbase.hh
fatradiobutton.o: include/toad/color.hh include/toad/types.hh
fatradiobutton.o: include/toad/io/serializable.hh
fatradiobutton.o: include/toad/io/atvparser.hh include/toad/font.hh
fatradiobutton.o: include/toad/pointer.hh include/toad/matrix2d.hh
fatradiobutton.o: include/toad/window.hh include/toad/interactor.hh
fatradiobutton.o: include/toad/connect.hh include/toad/cursor.hh
fatradiobutton.o: include/toad/region.hh include/toad/buttonbase.hh
fatradiobutton.o: include/toad/labelowner.hh include/toad/control.hh
fatradiobutton.o: include/toad/fatradiobutton.hh
fatradiobutton.o: include/toad/radiobuttonbase.hh include/toad/model.hh
radiobuttonbase.o: include/toad/pen.hh include/toad/penbase.hh
radiobuttonbase.o: include/toad/color.hh include/toad/types.hh
radiobuttonbase.o: include/toad/io/serializable.hh
radiobuttonbase.o: include/toad/io/atvparser.hh include/toad/font.hh
radiobuttonbase.o: include/toad/pointer.hh include/toad/matrix2d.hh
radiobuttonbase.o: include/toad/window.hh include/toad/interactor.hh
radiobuttonbase.o: include/toad/connect.hh include/toad/cursor.hh
radiobuttonbase.o: include/toad/region.hh include/toad/radiobuttonbase.hh
radiobuttonbase.o: include/toad/buttonbase.hh include/toad/labelowner.hh
radiobuttonbase.o: include/toad/control.hh include/toad/model.hh
radiobutton.o: include/toad/pen.hh include/toad/penbase.hh
radiobutton.o: include/toad/color.hh include/toad/types.hh
radiobutton.o: include/toad/io/serializable.hh include/toad/io/atvparser.hh
radiobutton.o: include/toad/font.hh include/toad/pointer.hh
radiobutton.o: include/toad/matrix2d.hh include/toad/window.hh
radiobutton.o: include/toad/interactor.hh include/toad/connect.hh
radiobutton.o: include/toad/cursor.hh include/toad/region.hh
radiobutton.o: include/toad/radiobutton.hh include/toad/radiobuttonbase.hh
radiobutton.o: include/toad/buttonbase.hh include/toad/labelowner.hh
radiobutton.o: include/toad/control.hh include/toad/model.hh
fatcheckbutton.o: include/toad/fatcheckbutton.hh include/toad/buttonbase.hh
fatcheckbutton.o: include/toad/connect.hh include/toad/labelowner.hh
fatcheckbutton.o: include/toad/control.hh include/toad/window.hh
fatcheckbutton.o: include/toad/interactor.hh include/toad/types.hh
fatcheckbutton.o: include/toad/io/serializable.hh
fatcheckbutton.o: include/toad/io/atvparser.hh include/toad/cursor.hh
fatcheckbutton.o: include/toad/color.hh include/toad/region.hh
fatcheckbutton.o: include/toad/boolmodel.hh include/toad/model.hh
fatcheckbutton.o: include/toad/pointer.hh include/toad/pen.hh
fatcheckbutton.o: include/toad/penbase.hh include/toad/font.hh
fatcheckbutton.o: include/toad/matrix2d.hh
layouteditor.o: include/toad/layouteditor.hh include/toad/control.hh
layouteditor.o: include/toad/window.hh include/toad/interactor.hh
layouteditor.o: include/toad/types.hh include/toad/io/serializable.hh
layouteditor.o: include/toad/io/atvparser.hh include/toad/connect.hh
layouteditor.o: include/toad/cursor.hh include/toad/color.hh
layouteditor.o: include/toad/region.hh
color.o: include/toad/color.hh include/toad/types.hh
color.o: include/toad/io/serializable.hh include/toad/io/atvparser.hh
filedialog.o: include/toad/filedialog.hh include/toad/dialog.hh
filedialog.o: include/toad/window.hh include/toad/interactor.hh
filedialog.o: include/toad/types.hh include/toad/io/serializable.hh
filedialog.o: include/toad/io/atvparser.hh include/toad/connect.hh
filedialog.o: include/toad/cursor.hh include/toad/color.hh
filedialog.o: include/toad/region.hh include/toad/layout.hh
filedialog.o: include/toad/eventfilter.hh include/toad/figuremodel.hh
filedialog.o: include/toad/model.hh include/toad/pointer.hh
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
cursor.o: include/toad/connect.hh include/toad/cursor.hh
cursor.o: include/toad/region.hh include/toad/bitmap.hh
tableadapter.o: include/toad/table.hh include/toad/pen.hh
tableadapter.o: include/toad/penbase.hh include/toad/color.hh
tableadapter.o: include/toad/types.hh include/toad/io/serializable.hh
tableadapter.o: include/toad/io/atvparser.hh include/toad/font.hh
tableadapter.o: include/toad/pointer.hh include/toad/matrix2d.hh
tableadapter.o: include/toad/region.hh include/toad/scrollpane.hh
tableadapter.o: include/toad/window.hh include/toad/interactor.hh
tableadapter.o: include/toad/connect.hh include/toad/cursor.hh
tableadapter.o: include/toad/model.hh include/toad/dragndrop.hh
tableadapter.o: include/toad/utf8.hh
core.o: include/toad/core.hh include/toad/window.hh
core.o: include/toad/interactor.hh include/toad/types.hh
core.o: include/toad/io/serializable.hh include/toad/io/atvparser.hh
core.o: include/toad/connect.hh include/toad/cursor.hh include/toad/color.hh
core.o: include/toad/region.hh include/toad/pen.hh include/toad/penbase.hh
core.o: include/toad/font.hh include/toad/pointer.hh include/toad/matrix2d.hh
core.o: include/toad/bitmap.hh include/toad/figure.hh
core.o: include/toad/figuremodel.hh include/toad/model.hh
core.o: include/toad/wordprocessor.hh include/toad/command.hh
core.o: include/toad/dialogeditor.hh include/toad/dialog.hh
core.o: include/toad/layout.hh include/toad/eventfilter.hh
core.o: fischland/fontdialog.hh include/toad/textfield.hh
core.o: include/toad/textarea.hh include/toad/control.hh
core.o: include/toad/textmodel.hh include/toad/undo.hh
core.o: include/toad/scrollbar.hh include/toad/integermodel.hh
core.o: include/toad/numbermodel.hh include/toad/table.hh
core.o: include/toad/scrollpane.hh include/toad/dragndrop.hh
figure.o: include/toad/figure.hh include/toad/penbase.hh
figure.o: include/toad/color.hh include/toad/types.hh
figure.o: include/toad/io/serializable.hh include/toad/io/atvparser.hh
figure.o: include/toad/font.hh include/toad/pointer.hh
figure.o: include/toad/matrix2d.hh include/toad/window.hh
figure.o: include/toad/interactor.hh include/toad/connect.hh
figure.o: include/toad/cursor.hh include/toad/region.hh
figure.o: include/toad/bitmap.hh include/toad/figuremodel.hh
figure.o: include/toad/model.hh include/toad/wordprocessor.hh
figure.o: include/toad/pen.hh include/toad/figureeditor.hh
figure.o: include/toad/scrollpane.hh include/toad/undo.hh
figure.o: include/toad/boolmodel.hh include/toad/textmodel.hh
figure.o: include/toad/integermodel.hh include/toad/numbermodel.hh
figure.o: include/toad/floatmodel.hh include/toad/figure/toolbox.hh
figure.o: include/toad/action.hh include/toad/dialog.hh
figure.o: include/toad/layout.hh include/toad/eventfilter.hh
figure.o: include/toad/menuhelper.hh include/toad/springlayout.hh
figuremodel.o: include/toad/figure.hh include/toad/penbase.hh
figuremodel.o: include/toad/color.hh include/toad/types.hh
figuremodel.o: include/toad/io/serializable.hh include/toad/io/atvparser.hh
figuremodel.o: include/toad/font.hh include/toad/pointer.hh
figuremodel.o: include/toad/matrix2d.hh include/toad/window.hh
figuremodel.o: include/toad/interactor.hh include/toad/connect.hh
figuremodel.o: include/toad/cursor.hh include/toad/region.hh
figuremodel.o: include/toad/bitmap.hh include/toad/figuremodel.hh
figuremodel.o: include/toad/model.hh include/toad/wordprocessor.hh
figuremodel.o: include/toad/pen.hh include/toad/figureeditor.hh
figuremodel.o: include/toad/scrollpane.hh include/toad/undo.hh
figuremodel.o: include/toad/boolmodel.hh include/toad/textmodel.hh
figuremodel.o: include/toad/integermodel.hh include/toad/numbermodel.hh
figuremodel.o: include/toad/floatmodel.hh include/toad/figure/toolbox.hh
figuremodel.o: include/toad/action.hh include/toad/undomanager.hh
figuremodel.o: include/toad/io/binstream.hh
figureeditor.o: include/toad/figureeditor.hh include/toad/figure.hh
figureeditor.o: include/toad/penbase.hh include/toad/color.hh
figureeditor.o: include/toad/types.hh include/toad/io/serializable.hh
figureeditor.o: include/toad/io/atvparser.hh include/toad/font.hh
figureeditor.o: include/toad/pointer.hh include/toad/matrix2d.hh
figureeditor.o: include/toad/window.hh include/toad/interactor.hh
figureeditor.o: include/toad/connect.hh include/toad/cursor.hh
figureeditor.o: include/toad/region.hh include/toad/bitmap.hh
figureeditor.o: include/toad/figuremodel.hh include/toad/model.hh
figureeditor.o: include/toad/wordprocessor.hh include/toad/pen.hh
figureeditor.o: include/toad/scrollpane.hh include/toad/undo.hh
figureeditor.o: include/toad/boolmodel.hh include/toad/textmodel.hh
figureeditor.o: include/toad/integermodel.hh include/toad/numbermodel.hh
figureeditor.o: include/toad/floatmodel.hh include/toad/figure/toolbox.hh
figureeditor.o: include/toad/action.hh include/toad/figuretool.hh
figureeditor.o: include/toad/core.hh include/toad/colordialog.hh
figureeditor.o: include/toad/dialog.hh include/toad/layout.hh
figureeditor.o: include/toad/eventfilter.hh include/toad/rgbmodel.hh
figureeditor.o: include/toad/textarea.hh include/toad/control.hh
figureeditor.o: include/toad/scrollbar.hh include/toad/dnd/color.hh
figureeditor.o: include/toad/dragndrop.hh include/toad/checkbox.hh
figureeditor.o: include/toad/labelowner.hh include/toad/undomanager.hh
figureeditor.o: include/toad/stacktrace.hh
figuretool.o: include/toad/figuretool.hh include/toad/core.hh
figuretool.o: include/toad/window.hh include/toad/interactor.hh
figuretool.o: include/toad/types.hh include/toad/io/serializable.hh
figuretool.o: include/toad/io/atvparser.hh include/toad/connect.hh
figuretool.o: include/toad/cursor.hh include/toad/color.hh
figuretool.o: include/toad/region.hh include/toad/pen.hh
figuretool.o: include/toad/penbase.hh include/toad/font.hh
figuretool.o: include/toad/pointer.hh include/toad/matrix2d.hh
figuretool.o: include/toad/bitmap.hh include/toad/figure.hh
figuretool.o: include/toad/figuremodel.hh include/toad/model.hh
figuretool.o: include/toad/wordprocessor.hh
matrix2d.o: include/toad/matrix2d.hh include/toad/types.hh
matrix2d.o: include/toad/io/serializable.hh include/toad/io/atvparser.hh
simpletimer.o: include/toad/simpletimer.hh include/toad/types.hh
simpletimer.o: include/toad/io/serializable.hh include/toad/io/atvparser.hh
region.o: include/toad/region.hh include/toad/types.hh
region.o: include/toad/io/serializable.hh include/toad/io/atvparser.hh
polygon.o: include/toad/types.hh include/toad/io/serializable.hh
polygon.o: include/toad/io/atvparser.hh include/toad/matrix2d.hh
springlayout.o: include/toad/springlayout.hh include/toad/layout.hh
springlayout.o: include/toad/io/serializable.hh include/toad/io/atvparser.hh
springlayout.o: include/toad/eventfilter.hh include/toad/window.hh
springlayout.o: include/toad/interactor.hh include/toad/types.hh
springlayout.o: include/toad/connect.hh include/toad/cursor.hh
springlayout.o: include/toad/color.hh include/toad/region.hh
combobox.o: include/toad/combobox.hh include/toad/table.hh
combobox.o: include/toad/pen.hh include/toad/penbase.hh include/toad/color.hh
combobox.o: include/toad/types.hh include/toad/io/serializable.hh
combobox.o: include/toad/io/atvparser.hh include/toad/font.hh
combobox.o: include/toad/pointer.hh include/toad/matrix2d.hh
combobox.o: include/toad/region.hh include/toad/scrollpane.hh
combobox.o: include/toad/window.hh include/toad/interactor.hh
combobox.o: include/toad/connect.hh include/toad/cursor.hh
combobox.o: include/toad/model.hh include/toad/dragndrop.hh
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
treemodel.o: include/toad/interactor.hh include/toad/connect.hh
treemodel.o: include/toad/cursor.hh include/toad/model.hh
treemodel.o: include/toad/dragndrop.hh
treeadapter.o: include/toad/treeadapter.hh include/toad/treemodel.hh
treeadapter.o: include/toad/table.hh include/toad/pen.hh
treeadapter.o: include/toad/penbase.hh include/toad/color.hh
treeadapter.o: include/toad/types.hh include/toad/io/serializable.hh
treeadapter.o: include/toad/io/atvparser.hh include/toad/font.hh
treeadapter.o: include/toad/pointer.hh include/toad/matrix2d.hh
treeadapter.o: include/toad/region.hh include/toad/scrollpane.hh
treeadapter.o: include/toad/window.hh include/toad/interactor.hh
treeadapter.o: include/toad/connect.hh include/toad/cursor.hh
treeadapter.o: include/toad/model.hh include/toad/dragndrop.hh
htmlview.o: include/toad/htmlview.hh include/toad/scrollpane.hh
htmlview.o: include/toad/window.hh include/toad/interactor.hh
htmlview.o: include/toad/types.hh include/toad/io/serializable.hh
htmlview.o: include/toad/io/atvparser.hh include/toad/connect.hh
htmlview.o: include/toad/cursor.hh include/toad/color.hh
htmlview.o: include/toad/region.hh include/toad/penbase.hh
htmlview.o: include/toad/font.hh include/toad/pointer.hh
htmlview.o: include/toad/matrix2d.hh include/toad/scrollbar.hh
htmlview.o: include/toad/control.hh include/toad/integermodel.hh
htmlview.o: include/toad/numbermodel.hh include/toad/model.hh
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
messagebox.o: include/toad/connect.hh include/toad/cursor.hh
messagebox.o: include/toad/region.hh include/toad/pushbutton.hh
messagebox.o: include/toad/buttonbase.hh include/toad/labelowner.hh
messagebox.o: include/toad/control.hh include/toad/messagebox.hh
layout.o: include/toad/layout.hh include/toad/io/serializable.hh
layout.o: include/toad/io/atvparser.hh include/toad/eventfilter.hh
layout.o: include/toad/window.hh include/toad/interactor.hh
layout.o: include/toad/types.hh include/toad/connect.hh
layout.o: include/toad/cursor.hh include/toad/color.hh include/toad/region.hh
layout.o: include/toad/textarea.hh include/toad/core.hh include/toad/pen.hh
layout.o: include/toad/penbase.hh include/toad/font.hh
layout.o: include/toad/pointer.hh include/toad/matrix2d.hh
layout.o: include/toad/bitmap.hh include/toad/control.hh
layout.o: include/toad/textmodel.hh include/toad/model.hh
layout.o: include/toad/undo.hh include/toad/scrollbar.hh
layout.o: include/toad/integermodel.hh include/toad/numbermodel.hh
layout.o: include/toad/pushbutton.hh include/toad/buttonbase.hh
layout.o: include/toad/labelowner.hh include/toad/io/urlstream.hh
layout.o: include/toad/layouteditor.hh
pointer.o: include/toad/pointer.hh
connect.o: include/toad/connect.hh
rectangle.o: include/toad/types.hh include/toad/io/serializable.hh
rectangle.o: include/toad/io/atvparser.hh
font.o: include/toad/font.hh include/toad/pointer.hh include/toad/types.hh
font.o: include/toad/io/serializable.hh include/toad/io/atvparser.hh
eventfilter.o: include/toad/eventfilter.hh
arrowbutton.o: include/toad/arrowbutton.hh include/toad/pushbutton.hh
arrowbutton.o: include/toad/buttonbase.hh include/toad/connect.hh
arrowbutton.o: include/toad/labelowner.hh include/toad/control.hh
arrowbutton.o: include/toad/window.hh include/toad/interactor.hh
arrowbutton.o: include/toad/types.hh include/toad/io/serializable.hh
arrowbutton.o: include/toad/io/atvparser.hh include/toad/cursor.hh
arrowbutton.o: include/toad/color.hh include/toad/region.hh
arrowbutton.o: include/toad/simpletimer.hh include/toad/pen.hh
arrowbutton.o: include/toad/penbase.hh include/toad/font.hh
arrowbutton.o: include/toad/pointer.hh include/toad/matrix2d.hh
scrollbar.o: include/toad/scrollbar.hh include/toad/control.hh
scrollbar.o: include/toad/window.hh include/toad/interactor.hh
scrollbar.o: include/toad/types.hh include/toad/io/serializable.hh
scrollbar.o: include/toad/io/atvparser.hh include/toad/connect.hh
scrollbar.o: include/toad/cursor.hh include/toad/color.hh
scrollbar.o: include/toad/region.hh include/toad/integermodel.hh
scrollbar.o: include/toad/numbermodel.hh include/toad/model.hh
scrollbar.o: include/toad/pointer.hh include/toad/textmodel.hh
scrollbar.o: include/toad/undo.hh include/toad/pen.hh include/toad/penbase.hh
scrollbar.o: include/toad/font.hh include/toad/matrix2d.hh
scrollbar.o: include/toad/arrowbutton.hh include/toad/pushbutton.hh
scrollbar.o: include/toad/buttonbase.hh include/toad/labelowner.hh
scrollbar.o: include/toad/simpletimer.hh
utf8.o: include/toad/utf8.hh
undo.o: include/toad/undo.hh
undomanager.o: include/toad/undomanager.hh include/toad/interactor.hh
undomanager.o: include/toad/types.hh include/toad/io/serializable.hh
undomanager.o: include/toad/io/atvparser.hh include/toad/connect.hh
undomanager.o: include/toad/action.hh include/toad/window.hh
undomanager.o: include/toad/cursor.hh include/toad/color.hh
undomanager.o: include/toad/region.hh include/toad/model.hh
undomanager.o: include/toad/pointer.hh include/toad/undo.hh
undomanager.o: include/toad/exception.hh
model.o: include/toad/model.hh include/toad/pointer.hh
model.o: include/toad/connect.hh include/toad/undomanager.hh
model.o: include/toad/interactor.hh include/toad/types.hh
model.o: include/toad/io/serializable.hh include/toad/io/atvparser.hh
model.o: include/toad/action.hh include/toad/window.hh include/toad/cursor.hh
model.o: include/toad/color.hh include/toad/region.hh include/toad/undo.hh
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
action.o: include/toad/io/serializable.hh include/toad/io/atvparser.hh
action.o: include/toad/connect.hh include/toad/cursor.hh
action.o: include/toad/color.hh include/toad/region.hh include/toad/model.hh
action.o: include/toad/pointer.hh
bitmap.o: include/toad/bitmap.hh include/toad/penbase.hh
bitmap.o: include/toad/color.hh include/toad/types.hh
bitmap.o: include/toad/io/serializable.hh include/toad/io/atvparser.hh
bitmap.o: include/toad/font.hh include/toad/pointer.hh
bitmap.o: include/toad/matrix2d.hh
io/atvparser.o: io/atvparser.hh
io/binstream.o: include/toad/io/binstream.hh include/toad/types.hh
io/binstream.o: include/toad/io/serializable.hh include/toad/io/atvparser.hh
io/serializable.o: io/serializable.hh include/toad/io/atvparser.hh
io/urlstream.o: include/toad/io/urlstream.hh
gauge.o: include/toad/gauge.hh include/toad/arrowbutton.hh
gauge.o: include/toad/pushbutton.hh include/toad/buttonbase.hh
gauge.o: include/toad/connect.hh include/toad/labelowner.hh
gauge.o: include/toad/control.hh include/toad/window.hh
gauge.o: include/toad/interactor.hh include/toad/types.hh
gauge.o: include/toad/io/serializable.hh include/toad/io/atvparser.hh
gauge.o: include/toad/cursor.hh include/toad/color.hh include/toad/region.hh
gauge.o: include/toad/simpletimer.hh include/toad/integermodel.hh
gauge.o: include/toad/numbermodel.hh include/toad/model.hh
gauge.o: include/toad/pointer.hh include/toad/textmodel.hh
gauge.o: include/toad/undo.hh
colordialog.o: include/toad/colordialog.hh include/toad/dialog.hh
colordialog.o: include/toad/window.hh include/toad/interactor.hh
colordialog.o: include/toad/types.hh include/toad/io/serializable.hh
colordialog.o: include/toad/io/atvparser.hh include/toad/connect.hh
colordialog.o: include/toad/cursor.hh include/toad/color.hh
colordialog.o: include/toad/region.hh include/toad/layout.hh
colordialog.o: include/toad/eventfilter.hh include/toad/figuremodel.hh
colordialog.o: include/toad/model.hh include/toad/pointer.hh
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
dragndrop.o: include/toad/io/serializable.hh include/toad/io/atvparser.hh
dragndrop.o: include/toad/pointer.hh include/toad/connect.hh
dragndrop.o: include/toad/window.hh include/toad/interactor.hh
dragndrop.o: include/toad/cursor.hh include/toad/color.hh
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
types.o: include/toad/types.hh include/toad/io/serializable.hh
types.o: include/toad/io/atvparser.hh
dnd/dropobject.o: include/toad/dnd/dropobject.hh include/toad/dragndrop.hh
dnd/dropobject.o: include/toad/types.hh include/toad/io/serializable.hh
dnd/dropobject.o: include/toad/io/atvparser.hh include/toad/pointer.hh
dnd/dropobject.o: include/toad/connect.hh
dnd/color.o: include/toad/dnd/color.hh include/toad/dragndrop.hh
dnd/color.o: include/toad/types.hh include/toad/io/serializable.hh
dnd/color.o: include/toad/io/atvparser.hh include/toad/pointer.hh
dnd/color.o: include/toad/connect.hh include/toad/color.hh
dnd/textplain.o: include/toad/dnd/textplain.hh include/toad/connect.hh
dnd/textplain.o: include/toad/dragndrop.hh include/toad/types.hh
dnd/textplain.o: include/toad/io/serializable.hh include/toad/io/atvparser.hh
dnd/textplain.o: include/toad/pointer.hh
dnd/image.o: include/toad/dnd/image.hh include/toad/dragndrop.hh
dnd/image.o: include/toad/types.hh include/toad/io/serializable.hh
dnd/image.o: include/toad/io/atvparser.hh include/toad/pointer.hh
dnd/image.o: include/toad/connect.hh include/toad/bitmap.hh
dnd/image.o: include/toad/penbase.hh include/toad/color.hh
dnd/image.o: include/toad/font.hh include/toad/matrix2d.hh
figure/toolbox.o: include/toad/figure/toolbox.hh include/toad/window.hh
figure/toolbox.o: include/toad/interactor.hh include/toad/types.hh
figure/toolbox.o: include/toad/io/serializable.hh
figure/toolbox.o: include/toad/io/atvparser.hh include/toad/connect.hh
figure/toolbox.o: include/toad/cursor.hh include/toad/color.hh
figure/toolbox.o: include/toad/region.hh include/toad/action.hh
figure/toolbox.o: include/toad/model.hh include/toad/pointer.hh
figure/toolbox.o: include/toad/eventfilter.hh include/toad/focusmanager.hh
figure/toolpanel.o: include/toad/figure/toolpanel.hh
figure/toolpanel.o: include/toad/figureeditor.hh include/toad/figure.hh
figure/toolpanel.o: include/toad/penbase.hh include/toad/color.hh
figure/toolpanel.o: include/toad/types.hh include/toad/io/serializable.hh
figure/toolpanel.o: include/toad/io/atvparser.hh include/toad/font.hh
figure/toolpanel.o: include/toad/pointer.hh include/toad/matrix2d.hh
figure/toolpanel.o: include/toad/window.hh include/toad/interactor.hh
figure/toolpanel.o: include/toad/connect.hh include/toad/cursor.hh
figure/toolpanel.o: include/toad/region.hh include/toad/bitmap.hh
figure/toolpanel.o: include/toad/figuremodel.hh include/toad/model.hh
figure/toolpanel.o: include/toad/wordprocessor.hh include/toad/pen.hh
figure/toolpanel.o: include/toad/scrollpane.hh include/toad/undo.hh
figure/toolpanel.o: include/toad/boolmodel.hh include/toad/textmodel.hh
figure/toolpanel.o: include/toad/integermodel.hh include/toad/numbermodel.hh
figure/toolpanel.o: include/toad/floatmodel.hh include/toad/figure/toolbox.hh
figure/toolpanel.o: include/toad/action.hh include/toad/undomanager.hh
figure/toolpanel.o: include/toad/figure/toolbutton.hh
figure/toolpanel.o: include/toad/buttonbase.hh include/toad/labelowner.hh
figure/toolpanel.o: include/toad/control.hh
figure/toolbutton.o: include/toad/figure/toolbutton.hh
figure/toolbutton.o: include/toad/buttonbase.hh include/toad/connect.hh
figure/toolbutton.o: include/toad/labelowner.hh include/toad/control.hh
figure/toolbutton.o: include/toad/window.hh include/toad/interactor.hh
figure/toolbutton.o: include/toad/types.hh include/toad/io/serializable.hh
figure/toolbutton.o: include/toad/io/atvparser.hh include/toad/cursor.hh
figure/toolbutton.o: include/toad/color.hh include/toad/region.hh
figure/toolbutton.o: include/toad/action.hh include/toad/model.hh
figure/toolbutton.o: include/toad/pointer.hh include/toad/pen.hh
figure/toolbutton.o: include/toad/penbase.hh include/toad/font.hh
figure/toolbutton.o: include/toad/matrix2d.hh
figure/frame.o: include/toad/figure.hh include/toad/penbase.hh
figure/frame.o: include/toad/color.hh include/toad/types.hh
figure/frame.o: include/toad/io/serializable.hh include/toad/io/atvparser.hh
figure/frame.o: include/toad/font.hh include/toad/pointer.hh
figure/frame.o: include/toad/matrix2d.hh include/toad/window.hh
figure/frame.o: include/toad/interactor.hh include/toad/connect.hh
figure/frame.o: include/toad/cursor.hh include/toad/region.hh
figure/frame.o: include/toad/bitmap.hh include/toad/figuremodel.hh
figure/frame.o: include/toad/model.hh include/toad/wordprocessor.hh
figure/frame.o: include/toad/pen.hh include/toad/figureeditor.hh
figure/frame.o: include/toad/scrollpane.hh include/toad/undo.hh
figure/frame.o: include/toad/boolmodel.hh include/toad/textmodel.hh
figure/frame.o: include/toad/integermodel.hh include/toad/numbermodel.hh
figure/frame.o: include/toad/floatmodel.hh include/toad/figure/toolbox.hh
figure/frame.o: include/toad/action.hh
figure/image.o: include/toad/figure.hh include/toad/penbase.hh
figure/image.o: include/toad/color.hh include/toad/types.hh
figure/image.o: include/toad/io/serializable.hh include/toad/io/atvparser.hh
figure/image.o: include/toad/font.hh include/toad/pointer.hh
figure/image.o: include/toad/matrix2d.hh include/toad/window.hh
figure/image.o: include/toad/interactor.hh include/toad/connect.hh
figure/image.o: include/toad/cursor.hh include/toad/region.hh
figure/image.o: include/toad/bitmap.hh include/toad/figuremodel.hh
figure/image.o: include/toad/model.hh include/toad/wordprocessor.hh
figure/image.o: include/toad/pen.hh include/toad/figureeditor.hh
figure/image.o: include/toad/scrollpane.hh include/toad/undo.hh
figure/image.o: include/toad/boolmodel.hh include/toad/textmodel.hh
figure/image.o: include/toad/integermodel.hh include/toad/numbermodel.hh
figure/image.o: include/toad/floatmodel.hh include/toad/figure/toolbox.hh
figure/image.o: include/toad/action.hh include/toad/filedialog.hh
figure/image.o: include/toad/dialog.hh include/toad/layout.hh
figure/image.o: include/toad/eventfilter.hh include/toad/table.hh
figure/image.o: include/toad/dragndrop.hh include/toad/pushbutton.hh
figure/image.o: include/toad/buttonbase.hh include/toad/labelowner.hh
figure/image.o: include/toad/control.hh include/toad/stl/vector.hh
figure/arrow.o: include/toad/figure.hh include/toad/penbase.hh
figure/arrow.o: include/toad/color.hh include/toad/types.hh
figure/arrow.o: include/toad/io/serializable.hh include/toad/io/atvparser.hh
figure/arrow.o: include/toad/font.hh include/toad/pointer.hh
figure/arrow.o: include/toad/matrix2d.hh include/toad/window.hh
figure/arrow.o: include/toad/interactor.hh include/toad/connect.hh
figure/arrow.o: include/toad/cursor.hh include/toad/region.hh
figure/arrow.o: include/toad/bitmap.hh include/toad/figuremodel.hh
figure/arrow.o: include/toad/model.hh include/toad/wordprocessor.hh
figure/arrow.o: include/toad/pen.hh include/toad/figureeditor.hh
figure/arrow.o: include/toad/scrollpane.hh include/toad/undo.hh
figure/arrow.o: include/toad/boolmodel.hh include/toad/textmodel.hh
figure/arrow.o: include/toad/integermodel.hh include/toad/numbermodel.hh
figure/arrow.o: include/toad/floatmodel.hh include/toad/figure/toolbox.hh
figure/arrow.o: include/toad/action.hh
figure/text.o: include/toad/figure.hh include/toad/penbase.hh
figure/text.o: include/toad/color.hh include/toad/types.hh
figure/text.o: include/toad/io/serializable.hh include/toad/io/atvparser.hh
figure/text.o: include/toad/font.hh include/toad/pointer.hh
figure/text.o: include/toad/matrix2d.hh include/toad/window.hh
figure/text.o: include/toad/interactor.hh include/toad/connect.hh
figure/text.o: include/toad/cursor.hh include/toad/region.hh
figure/text.o: include/toad/bitmap.hh include/toad/figuremodel.hh
figure/text.o: include/toad/model.hh include/toad/wordprocessor.hh
figure/text.o: include/toad/pen.hh include/toad/figureeditor.hh
figure/text.o: include/toad/scrollpane.hh include/toad/undo.hh
figure/text.o: include/toad/boolmodel.hh include/toad/textmodel.hh
figure/text.o: include/toad/integermodel.hh include/toad/numbermodel.hh
figure/text.o: include/toad/floatmodel.hh include/toad/figure/toolbox.hh
figure/text.o: include/toad/action.hh
figure/circle.o: include/toad/figure.hh include/toad/penbase.hh
figure/circle.o: include/toad/color.hh include/toad/types.hh
figure/circle.o: include/toad/io/serializable.hh include/toad/io/atvparser.hh
figure/circle.o: include/toad/font.hh include/toad/pointer.hh
figure/circle.o: include/toad/matrix2d.hh include/toad/window.hh
figure/circle.o: include/toad/interactor.hh include/toad/connect.hh
figure/circle.o: include/toad/cursor.hh include/toad/region.hh
figure/circle.o: include/toad/bitmap.hh include/toad/figuremodel.hh
figure/circle.o: include/toad/model.hh include/toad/wordprocessor.hh
figure/circle.o: include/toad/pen.hh include/toad/figureeditor.hh
figure/circle.o: include/toad/scrollpane.hh include/toad/undo.hh
figure/circle.o: include/toad/boolmodel.hh include/toad/textmodel.hh
figure/circle.o: include/toad/integermodel.hh include/toad/numbermodel.hh
figure/circle.o: include/toad/floatmodel.hh include/toad/figure/toolbox.hh
figure/circle.o: include/toad/action.hh include/toad/vector.hh
figure/group.o: include/toad/figure.hh include/toad/penbase.hh
figure/group.o: include/toad/color.hh include/toad/types.hh
figure/group.o: include/toad/io/serializable.hh include/toad/io/atvparser.hh
figure/group.o: include/toad/font.hh include/toad/pointer.hh
figure/group.o: include/toad/matrix2d.hh include/toad/window.hh
figure/group.o: include/toad/interactor.hh include/toad/connect.hh
figure/group.o: include/toad/cursor.hh include/toad/region.hh
figure/group.o: include/toad/bitmap.hh include/toad/figuremodel.hh
figure/group.o: include/toad/model.hh include/toad/wordprocessor.hh
figure/group.o: include/toad/pen.hh include/toad/figureeditor.hh
figure/group.o: include/toad/scrollpane.hh include/toad/undo.hh
figure/group.o: include/toad/boolmodel.hh include/toad/textmodel.hh
figure/group.o: include/toad/integermodel.hh include/toad/numbermodel.hh
figure/group.o: include/toad/floatmodel.hh include/toad/figure/toolbox.hh
figure/group.o: include/toad/action.hh
figure/transform.o: include/toad/vector.hh include/toad/penbase.hh
figure/transform.o: include/toad/color.hh include/toad/types.hh
figure/transform.o: include/toad/io/serializable.hh
figure/transform.o: include/toad/io/atvparser.hh include/toad/font.hh
figure/transform.o: include/toad/pointer.hh include/toad/matrix2d.hh
figure/transform.o: include/toad/window.hh include/toad/interactor.hh
figure/transform.o: include/toad/connect.hh include/toad/cursor.hh
figure/transform.o: include/toad/region.hh include/toad/bitmap.hh
figure/transform.o: include/toad/figuremodel.hh include/toad/model.hh
figure/transform.o: include/toad/figure.hh include/toad/wordprocessor.hh
figure/transform.o: include/toad/pen.hh include/toad/figureeditor.hh
figure/transform.o: include/toad/scrollpane.hh include/toad/undo.hh
figure/transform.o: include/toad/boolmodel.hh include/toad/textmodel.hh
figure/transform.o: include/toad/integermodel.hh include/toad/numbermodel.hh
figure/transform.o: include/toad/floatmodel.hh include/toad/figure/toolbox.hh
figure/transform.o: include/toad/action.hh
figure/perspectivetransform.o: include/toad/vector.hh include/toad/penbase.hh
figure/perspectivetransform.o: include/toad/color.hh include/toad/types.hh
figure/perspectivetransform.o: include/toad/io/serializable.hh
figure/perspectivetransform.o: include/toad/io/atvparser.hh
figure/perspectivetransform.o: include/toad/font.hh include/toad/pointer.hh
figure/perspectivetransform.o: include/toad/matrix2d.hh
figure/perspectivetransform.o: include/toad/window.hh
figure/perspectivetransform.o: include/toad/interactor.hh
figure/perspectivetransform.o: include/toad/connect.hh include/toad/cursor.hh
figure/perspectivetransform.o: include/toad/region.hh include/toad/bitmap.hh
figure/perspectivetransform.o: include/toad/figuremodel.hh
figure/perspectivetransform.o: include/toad/model.hh include/toad/figure.hh
figure/perspectivetransform.o: include/toad/wordprocessor.hh
figure/perspectivetransform.o: include/toad/pen.hh
figure/perspectivetransform.o: include/toad/figureeditor.hh
figure/perspectivetransform.o: include/toad/scrollpane.hh
figure/perspectivetransform.o: include/toad/undo.hh include/toad/boolmodel.hh
figure/perspectivetransform.o: include/toad/textmodel.hh
figure/perspectivetransform.o: include/toad/integermodel.hh
figure/perspectivetransform.o: include/toad/numbermodel.hh
figure/perspectivetransform.o: include/toad/floatmodel.hh
figure/perspectivetransform.o: include/toad/figure/toolbox.hh
figure/perspectivetransform.o: include/toad/action.hh
figure/rectangle.o: include/toad/vector.hh include/toad/penbase.hh
figure/rectangle.o: include/toad/color.hh include/toad/types.hh
figure/rectangle.o: include/toad/io/serializable.hh
figure/rectangle.o: include/toad/io/atvparser.hh include/toad/font.hh
figure/rectangle.o: include/toad/pointer.hh include/toad/matrix2d.hh
figure/rectangle.o: include/toad/window.hh include/toad/interactor.hh
figure/rectangle.o: include/toad/connect.hh include/toad/cursor.hh
figure/rectangle.o: include/toad/region.hh include/toad/bitmap.hh
figure/rectangle.o: include/toad/figuremodel.hh include/toad/model.hh
figure/rectangle.o: include/toad/figure.hh include/toad/wordprocessor.hh
figure/rectangle.o: include/toad/pen.hh include/toad/figureeditor.hh
figure/rectangle.o: include/toad/scrollpane.hh include/toad/undo.hh
figure/rectangle.o: include/toad/boolmodel.hh include/toad/textmodel.hh
figure/rectangle.o: include/toad/integermodel.hh include/toad/numbermodel.hh
figure/rectangle.o: include/toad/floatmodel.hh include/toad/figure/toolbox.hh
figure/rectangle.o: include/toad/action.hh include/toad/figure/shapetool.hh
figure/rectangle.o: include/toad/figuretool.hh include/toad/core.hh
figure/window.o: include/toad/figure.hh include/toad/penbase.hh
figure/window.o: include/toad/color.hh include/toad/types.hh
figure/window.o: include/toad/io/serializable.hh include/toad/io/atvparser.hh
figure/window.o: include/toad/font.hh include/toad/pointer.hh
figure/window.o: include/toad/matrix2d.hh include/toad/window.hh
figure/window.o: include/toad/interactor.hh include/toad/connect.hh
figure/window.o: include/toad/cursor.hh include/toad/region.hh
figure/window.o: include/toad/bitmap.hh include/toad/figuremodel.hh
figure/window.o: include/toad/model.hh include/toad/wordprocessor.hh
figure/window.o: include/toad/pen.hh include/toad/figureeditor.hh
figure/window.o: include/toad/scrollpane.hh include/toad/undo.hh
figure/window.o: include/toad/boolmodel.hh include/toad/textmodel.hh
figure/window.o: include/toad/integermodel.hh include/toad/numbermodel.hh
figure/window.o: include/toad/floatmodel.hh include/toad/figure/toolbox.hh
figure/window.o: include/toad/action.hh include/toad/labelowner.hh
figure/window.o: include/toad/control.hh
figure/selectiontool.o: include/toad/figure/selectiontool.hh
figure/selectiontool.o: include/toad/figureeditor.hh include/toad/figure.hh
figure/selectiontool.o: include/toad/penbase.hh include/toad/color.hh
figure/selectiontool.o: include/toad/types.hh include/toad/io/serializable.hh
figure/selectiontool.o: include/toad/io/atvparser.hh include/toad/font.hh
figure/selectiontool.o: include/toad/pointer.hh include/toad/matrix2d.hh
figure/selectiontool.o: include/toad/window.hh include/toad/interactor.hh
figure/selectiontool.o: include/toad/connect.hh include/toad/cursor.hh
figure/selectiontool.o: include/toad/region.hh include/toad/bitmap.hh
figure/selectiontool.o: include/toad/figuremodel.hh include/toad/model.hh
figure/selectiontool.o: include/toad/wordprocessor.hh include/toad/pen.hh
figure/selectiontool.o: include/toad/scrollpane.hh include/toad/undo.hh
figure/selectiontool.o: include/toad/boolmodel.hh include/toad/textmodel.hh
figure/selectiontool.o: include/toad/integermodel.hh
figure/selectiontool.o: include/toad/numbermodel.hh
figure/selectiontool.o: include/toad/floatmodel.hh
figure/selectiontool.o: include/toad/figure/toolbox.hh include/toad/action.hh
figure/selectiontool.o: include/toad/figuretool.hh include/toad/core.hh
figure/selectiontool.o: include/toad/undomanager.hh include/toad/vector.hh
figure/nodetool.o: include/toad/figure/nodetool.hh include/toad/figuretool.hh
figure/nodetool.o: include/toad/core.hh include/toad/window.hh
figure/nodetool.o: include/toad/interactor.hh include/toad/types.hh
figure/nodetool.o: include/toad/io/serializable.hh
figure/nodetool.o: include/toad/io/atvparser.hh include/toad/connect.hh
figure/nodetool.o: include/toad/cursor.hh include/toad/color.hh
figure/nodetool.o: include/toad/region.hh include/toad/pen.hh
figure/nodetool.o: include/toad/penbase.hh include/toad/font.hh
figure/nodetool.o: include/toad/pointer.hh include/toad/matrix2d.hh
figure/nodetool.o: include/toad/bitmap.hh include/toad/figure.hh
figure/nodetool.o: include/toad/figuremodel.hh include/toad/model.hh
figure/nodetool.o: include/toad/wordprocessor.hh include/toad/figureeditor.hh
figure/nodetool.o: include/toad/scrollpane.hh include/toad/undo.hh
figure/nodetool.o: include/toad/boolmodel.hh include/toad/textmodel.hh
figure/nodetool.o: include/toad/integermodel.hh include/toad/numbermodel.hh
figure/nodetool.o: include/toad/floatmodel.hh include/toad/figure/toolbox.hh
figure/nodetool.o: include/toad/action.hh include/toad/undomanager.hh
figure/shapetool.o: include/toad/figure/shapetool.hh
figure/shapetool.o: include/toad/figuretool.hh include/toad/core.hh
figure/shapetool.o: include/toad/window.hh include/toad/interactor.hh
figure/shapetool.o: include/toad/types.hh include/toad/io/serializable.hh
figure/shapetool.o: include/toad/io/atvparser.hh include/toad/connect.hh
figure/shapetool.o: include/toad/cursor.hh include/toad/color.hh
figure/shapetool.o: include/toad/region.hh include/toad/pen.hh
figure/shapetool.o: include/toad/penbase.hh include/toad/font.hh
figure/shapetool.o: include/toad/pointer.hh include/toad/matrix2d.hh
figure/shapetool.o: include/toad/bitmap.hh include/toad/figure.hh
figure/shapetool.o: include/toad/figuremodel.hh include/toad/model.hh
figure/shapetool.o: include/toad/wordprocessor.hh
figure/shapetool.o: include/toad/figureeditor.hh include/toad/scrollpane.hh
figure/shapetool.o: include/toad/undo.hh include/toad/boolmodel.hh
figure/shapetool.o: include/toad/textmodel.hh include/toad/integermodel.hh
figure/shapetool.o: include/toad/numbermodel.hh include/toad/floatmodel.hh
figure/shapetool.o: include/toad/figure/toolbox.hh include/toad/action.hh
figure/texttool.o: include/toad/figure/texttool.hh include/toad/figuretool.hh
figure/texttool.o: include/toad/core.hh include/toad/window.hh
figure/texttool.o: include/toad/interactor.hh include/toad/types.hh
figure/texttool.o: include/toad/io/serializable.hh
figure/texttool.o: include/toad/io/atvparser.hh include/toad/connect.hh
figure/texttool.o: include/toad/cursor.hh include/toad/color.hh
figure/texttool.o: include/toad/region.hh include/toad/pen.hh
figure/texttool.o: include/toad/penbase.hh include/toad/font.hh
figure/texttool.o: include/toad/pointer.hh include/toad/matrix2d.hh
figure/texttool.o: include/toad/bitmap.hh include/toad/figure.hh
figure/texttool.o: include/toad/figuremodel.hh include/toad/model.hh
figure/texttool.o: include/toad/wordprocessor.hh include/toad/figureeditor.hh
figure/texttool.o: include/toad/scrollpane.hh include/toad/undo.hh
figure/texttool.o: include/toad/boolmodel.hh include/toad/textmodel.hh
figure/texttool.o: include/toad/integermodel.hh include/toad/numbermodel.hh
figure/texttool.o: include/toad/floatmodel.hh include/toad/figure/toolbox.hh
figure/texttool.o: include/toad/action.hh
figure/connecttool.o: include/toad/figure/connecttool.hh
figure/connecttool.o: include/toad/figuretool.hh include/toad/core.hh
figure/connecttool.o: include/toad/window.hh include/toad/interactor.hh
figure/connecttool.o: include/toad/types.hh include/toad/io/serializable.hh
figure/connecttool.o: include/toad/io/atvparser.hh include/toad/connect.hh
figure/connecttool.o: include/toad/cursor.hh include/toad/color.hh
figure/connecttool.o: include/toad/region.hh include/toad/pen.hh
figure/connecttool.o: include/toad/penbase.hh include/toad/font.hh
figure/connecttool.o: include/toad/pointer.hh include/toad/matrix2d.hh
figure/connecttool.o: include/toad/bitmap.hh include/toad/figure.hh
figure/connecttool.o: include/toad/figuremodel.hh include/toad/model.hh
figure/connecttool.o: include/toad/wordprocessor.hh
figure/connecttool.o: include/toad/figure/connectfigure.hh
figure/connecttool.o: include/toad/figureeditor.hh include/toad/scrollpane.hh
figure/connecttool.o: include/toad/undo.hh include/toad/boolmodel.hh
figure/connecttool.o: include/toad/textmodel.hh include/toad/integermodel.hh
figure/connecttool.o: include/toad/numbermodel.hh include/toad/floatmodel.hh
figure/connecttool.o: include/toad/figure/toolbox.hh include/toad/action.hh
figure/connecttool.o: include/toad/vector.hh
figure/connectfigure.o: include/toad/figure/connectfigure.hh
figure/connectfigure.o: include/toad/figure.hh include/toad/penbase.hh
figure/connectfigure.o: include/toad/color.hh include/toad/types.hh
figure/connectfigure.o: include/toad/io/serializable.hh
figure/connectfigure.o: include/toad/io/atvparser.hh include/toad/font.hh
figure/connectfigure.o: include/toad/pointer.hh include/toad/matrix2d.hh
figure/connectfigure.o: include/toad/window.hh include/toad/interactor.hh
figure/connectfigure.o: include/toad/connect.hh include/toad/cursor.hh
figure/connectfigure.o: include/toad/region.hh include/toad/bitmap.hh
figure/connectfigure.o: include/toad/figuremodel.hh include/toad/model.hh
figure/connectfigure.o: include/toad/wordprocessor.hh include/toad/pen.hh
figure/connectfigure.o: include/toad/figureeditor.hh
figure/connectfigure.o: include/toad/scrollpane.hh include/toad/undo.hh
figure/connectfigure.o: include/toad/boolmodel.hh include/toad/textmodel.hh
figure/connectfigure.o: include/toad/integermodel.hh
figure/connectfigure.o: include/toad/numbermodel.hh
figure/connectfigure.o: include/toad/floatmodel.hh
figure/connectfigure.o: include/toad/figure/toolbox.hh include/toad/action.hh
figure/connectfigure.o: include/toad/vector.hh include/toad/geometry.hh
vector.o: include/toad/vector.hh include/toad/penbase.hh
vector.o: include/toad/color.hh include/toad/types.hh
vector.o: include/toad/io/serializable.hh include/toad/io/atvparser.hh
vector.o: include/toad/font.hh include/toad/pointer.hh
vector.o: include/toad/matrix2d.hh include/toad/window.hh
vector.o: include/toad/interactor.hh include/toad/connect.hh
vector.o: include/toad/cursor.hh include/toad/region.hh
vector.o: include/toad/bitmap.hh include/toad/figuremodel.hh
vector.o: include/toad/model.hh include/toad/geometry.hh include/toad/pen.hh
vector.o: include/toad/figure.hh include/toad/wordprocessor.hh
geometry.o: include/toad/geometry.hh include/toad/types.hh
geometry.o: include/toad/io/serializable.hh include/toad/io/atvparser.hh
geometry.o: include/toad/vector.hh include/toad/penbase.hh
geometry.o: include/toad/color.hh include/toad/font.hh
geometry.o: include/toad/pointer.hh include/toad/matrix2d.hh
geometry.o: include/toad/window.hh include/toad/interactor.hh
geometry.o: include/toad/connect.hh include/toad/cursor.hh
geometry.o: include/toad/region.hh include/toad/bitmap.hh
geometry.o: include/toad/figuremodel.hh include/toad/model.hh
wordprocessor.o: include/toad/utf8.hh include/toad/wordprocessor.hh
wordprocessor.o: include/toad/types.hh include/toad/io/serializable.hh
wordprocessor.o: include/toad/io/atvparser.hh include/toad/color.hh
wordprocessor.o: include/toad/font.hh include/toad/pointer.hh
wordprocessor.o: include/toad/pen.hh include/toad/penbase.hh
wordprocessor.o: include/toad/matrix2d.hh include/toad/window.hh
wordprocessor.o: include/toad/interactor.hh include/toad/connect.hh
wordprocessor.o: include/toad/cursor.hh include/toad/region.hh
stacktrace.o: include/toad/stacktrace.hh
test_table.o: include/toad/table.hh include/toad/pen.hh
test_table.o: include/toad/penbase.hh include/toad/color.hh
test_table.o: include/toad/types.hh include/toad/io/serializable.hh
test_table.o: include/toad/io/atvparser.hh include/toad/font.hh
test_table.o: include/toad/pointer.hh include/toad/matrix2d.hh
test_table.o: include/toad/region.hh include/toad/scrollpane.hh
test_table.o: include/toad/window.hh include/toad/interactor.hh
test_table.o: include/toad/connect.hh include/toad/cursor.hh
test_table.o: include/toad/model.hh include/toad/dragndrop.hh
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
test_scroll.o: include/toad/connect.hh include/toad/cursor.hh
test_scroll.o: include/toad/model.hh include/toad/dragndrop.hh
test_scroll.o: include/toad/checkbox.hh include/toad/labelowner.hh
test_scroll.o: include/toad/control.hh include/toad/boolmodel.hh
test_scroll.o: include/toad/radiobutton.hh include/toad/radiobuttonbase.hh
test_scroll.o: include/toad/buttonbase.hh include/toad/dialog.hh
test_scroll.o: include/toad/layout.hh include/toad/eventfilter.hh
test_scroll.o: include/toad/figuremodel.hh include/toad/stl/vector.hh
test_scroll.o: include/toad/utf8.hh
test_dialog.o: include/toad/dialog.hh include/toad/window.hh
test_dialog.o: include/toad/interactor.hh include/toad/types.hh
test_dialog.o: include/toad/io/serializable.hh include/toad/io/atvparser.hh
test_dialog.o: include/toad/connect.hh include/toad/cursor.hh
test_dialog.o: include/toad/color.hh include/toad/region.hh
test_dialog.o: include/toad/layout.hh include/toad/eventfilter.hh
test_dialog.o: include/toad/figuremodel.hh include/toad/model.hh
test_dialog.o: include/toad/pointer.hh include/toad/textarea.hh
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
test_timer.o: include/toad/types.hh include/toad/io/serializable.hh
test_timer.o: include/toad/io/atvparser.hh include/toad/connect.hh
test_timer.o: include/toad/cursor.hh include/toad/color.hh
test_timer.o: include/toad/region.hh include/toad/simpletimer.hh
test_combobox.o: include/toad/combobox.hh include/toad/table.hh
test_combobox.o: include/toad/pen.hh include/toad/penbase.hh
test_combobox.o: include/toad/color.hh include/toad/types.hh
test_combobox.o: include/toad/io/serializable.hh include/toad/io/atvparser.hh
test_combobox.o: include/toad/font.hh include/toad/pointer.hh
test_combobox.o: include/toad/matrix2d.hh include/toad/region.hh
test_combobox.o: include/toad/scrollpane.hh include/toad/window.hh
test_combobox.o: include/toad/interactor.hh include/toad/connect.hh
test_combobox.o: include/toad/cursor.hh include/toad/model.hh
test_combobox.o: include/toad/dragndrop.hh
test_cursor.o: include/toad/window.hh include/toad/interactor.hh
test_cursor.o: include/toad/types.hh include/toad/io/serializable.hh
test_cursor.o: include/toad/io/atvparser.hh include/toad/connect.hh
test_cursor.o: include/toad/cursor.hh include/toad/color.hh
test_cursor.o: include/toad/region.hh include/toad/pen.hh
test_cursor.o: include/toad/penbase.hh include/toad/font.hh
test_cursor.o: include/toad/pointer.hh include/toad/matrix2d.hh
test_colordialog.o: include/toad/colordialog.hh include/toad/dialog.hh
test_colordialog.o: include/toad/window.hh include/toad/interactor.hh
test_colordialog.o: include/toad/types.hh include/toad/io/serializable.hh
test_colordialog.o: include/toad/io/atvparser.hh include/toad/connect.hh
test_colordialog.o: include/toad/cursor.hh include/toad/color.hh
test_colordialog.o: include/toad/region.hh include/toad/layout.hh
test_colordialog.o: include/toad/eventfilter.hh include/toad/figuremodel.hh
test_colordialog.o: include/toad/model.hh include/toad/pointer.hh
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
test_grab.o: include/toad/types.hh include/toad/io/serializable.hh
test_grab.o: include/toad/io/atvparser.hh include/toad/connect.hh
test_grab.o: include/toad/cursor.hh include/toad/color.hh
test_grab.o: include/toad/region.hh include/toad/pen.hh
test_grab.o: include/toad/penbase.hh include/toad/font.hh
test_grab.o: include/toad/pointer.hh include/toad/matrix2d.hh
test_path.o: include/toad/window.hh include/toad/interactor.hh
test_path.o: include/toad/types.hh include/toad/io/serializable.hh
test_path.o: include/toad/io/atvparser.hh include/toad/connect.hh
test_path.o: include/toad/cursor.hh include/toad/color.hh
test_path.o: include/toad/region.hh include/toad/pen.hh
test_path.o: include/toad/penbase.hh include/toad/font.hh
test_path.o: include/toad/pointer.hh include/toad/matrix2d.hh
test_path.o: include/toad/figure.hh include/toad/bitmap.hh
test_path.o: include/toad/figuremodel.hh include/toad/model.hh
test_path.o: include/toad/wordprocessor.hh include/toad/vector.hh
test_path.o: include/toad/geometry.hh
test_image.o: include/toad/window.hh include/toad/interactor.hh
test_image.o: include/toad/types.hh include/toad/io/serializable.hh
test_image.o: include/toad/io/atvparser.hh include/toad/connect.hh
test_image.o: include/toad/cursor.hh include/toad/color.hh
test_image.o: include/toad/region.hh include/toad/pen.hh
test_image.o: include/toad/penbase.hh include/toad/font.hh
test_image.o: include/toad/pointer.hh include/toad/matrix2d.hh
test_image.o: include/toad/bitmap.hh
test_curve.o: include/toad/window.hh include/toad/interactor.hh
test_curve.o: include/toad/types.hh include/toad/io/serializable.hh
test_curve.o: include/toad/io/atvparser.hh include/toad/connect.hh
test_curve.o: include/toad/cursor.hh include/toad/color.hh
test_curve.o: include/toad/region.hh include/toad/pen.hh
test_curve.o: include/toad/penbase.hh include/toad/font.hh
test_curve.o: include/toad/pointer.hh include/toad/matrix2d.hh
test_curve.o: include/toad/bitmap.hh
test_text.o: include/toad/wordprocessor.hh include/toad/types.hh
test_text.o: include/toad/io/serializable.hh include/toad/io/atvparser.hh
test_text.o: include/toad/color.hh include/toad/font.hh
test_text.o: include/toad/pointer.hh include/toad/pen.hh
test_text.o: include/toad/penbase.hh include/toad/matrix2d.hh
test_text.o: include/toad/window.hh include/toad/interactor.hh
test_text.o: include/toad/connect.hh include/toad/cursor.hh
test_text.o: include/toad/region.hh include/toad/utf8.hh
test_tablet.o: include/toad/window.hh include/toad/interactor.hh
test_tablet.o: include/toad/types.hh include/toad/io/serializable.hh
test_tablet.o: include/toad/io/atvparser.hh include/toad/connect.hh
test_tablet.o: include/toad/cursor.hh include/toad/color.hh
test_tablet.o: include/toad/region.hh include/toad/pen.hh
test_tablet.o: include/toad/penbase.hh include/toad/font.hh
test_tablet.o: include/toad/pointer.hh include/toad/matrix2d.hh
test_tablet.o: include/toad/figure.hh include/toad/bitmap.hh
test_tablet.o: include/toad/figuremodel.hh include/toad/model.hh
test_tablet.o: include/toad/wordprocessor.hh include/toad/vector.hh
test_tablet.o: include/toad/geometry.hh include/toad/dialog.hh
test_tablet.o: include/toad/layout.hh include/toad/eventfilter.hh
test_tablet.o: include/toad/springlayout.hh include/toad/textfield.hh
test_tablet.o: include/toad/textarea.hh include/toad/core.hh
test_tablet.o: include/toad/control.hh include/toad/textmodel.hh
test_tablet.o: include/toad/undo.hh include/toad/scrollbar.hh
test_tablet.o: include/toad/integermodel.hh include/toad/numbermodel.hh
test_tablet.o: include/toad/fatradiobutton.hh include/toad/radiobuttonbase.hh
test_tablet.o: include/toad/buttonbase.hh include/toad/labelowner.hh
test_tablet.o: include/toad/scrollpane.hh include/toad/floatmodel.hh
test_tablet.o: include/toad/simpletimer.hh booleanop.hh
test_path_bool.o: include/toad/window.hh include/toad/interactor.hh
test_path_bool.o: include/toad/types.hh include/toad/io/serializable.hh
test_path_bool.o: include/toad/io/atvparser.hh include/toad/connect.hh
test_path_bool.o: include/toad/cursor.hh include/toad/color.hh
test_path_bool.o: include/toad/region.hh include/toad/pen.hh
test_path_bool.o: include/toad/penbase.hh include/toad/font.hh
test_path_bool.o: include/toad/pointer.hh include/toad/matrix2d.hh
test_path_bool.o: include/toad/figure.hh include/toad/bitmap.hh
test_path_bool.o: include/toad/figuremodel.hh include/toad/model.hh
test_path_bool.o: include/toad/wordprocessor.hh include/toad/vector.hh
test_path_bool.o: include/toad/geometry.hh include/toad/booleanop.hh
test_guitar.o: include/toad/window.hh include/toad/interactor.hh
test_guitar.o: include/toad/types.hh include/toad/io/serializable.hh
test_guitar.o: include/toad/io/atvparser.hh include/toad/connect.hh
test_guitar.o: include/toad/cursor.hh include/toad/color.hh
test_guitar.o: include/toad/region.hh include/toad/dialog.hh
test_guitar.o: include/toad/layout.hh include/toad/eventfilter.hh
test_guitar.o: include/toad/figuremodel.hh include/toad/model.hh
test_guitar.o: include/toad/pointer.hh include/toad/pen.hh
test_guitar.o: include/toad/penbase.hh include/toad/font.hh
test_guitar.o: include/toad/matrix2d.hh include/toad/textarea.hh
test_guitar.o: include/toad/core.hh include/toad/bitmap.hh
test_guitar.o: include/toad/control.hh include/toad/textmodel.hh
test_guitar.o: include/toad/undo.hh include/toad/scrollbar.hh
test_guitar.o: include/toad/integermodel.hh include/toad/numbermodel.hh
test_guitar.o: include/toad/pushbutton.hh include/toad/buttonbase.hh
test_guitar.o: include/toad/labelowner.hh include/toad/table.hh
test_guitar.o: include/toad/scrollpane.hh include/toad/dragndrop.hh
test_vector_buffer.o: include/toad/vector.hh include/toad/penbase.hh
test_vector_buffer.o: include/toad/color.hh include/toad/types.hh
test_vector_buffer.o: include/toad/io/serializable.hh
test_vector_buffer.o: include/toad/io/atvparser.hh include/toad/font.hh
test_vector_buffer.o: include/toad/pointer.hh include/toad/matrix2d.hh
test_vector_buffer.o: include/toad/window.hh include/toad/interactor.hh
test_vector_buffer.o: include/toad/connect.hh include/toad/cursor.hh
test_vector_buffer.o: include/toad/region.hh include/toad/bitmap.hh
test_vector_buffer.o: include/toad/figuremodel.hh include/toad/model.hh
test_vector_buffer.o: include/toad/figure.hh include/toad/wordprocessor.hh
test_vector_buffer.o: include/toad/pen.hh
test_path_offset.o: include/toad/window.hh include/toad/interactor.hh
test_path_offset.o: include/toad/types.hh include/toad/io/serializable.hh
test_path_offset.o: include/toad/io/atvparser.hh include/toad/connect.hh
test_path_offset.o: include/toad/cursor.hh include/toad/color.hh
test_path_offset.o: include/toad/region.hh include/toad/pen.hh
test_path_offset.o: include/toad/penbase.hh include/toad/font.hh
test_path_offset.o: include/toad/pointer.hh include/toad/matrix2d.hh
test_path_offset.o: include/toad/figure.hh include/toad/bitmap.hh
test_path_offset.o: include/toad/figuremodel.hh include/toad/model.hh
test_path_offset.o: include/toad/wordprocessor.hh include/toad/vector.hh
test_path_offset.o: include/toad/geometry.hh
test_toolbar.o: include/toad/window.hh include/toad/interactor.hh
test_toolbar.o: include/toad/types.hh include/toad/io/serializable.hh
test_toolbar.o: include/toad/io/atvparser.hh include/toad/connect.hh
test_toolbar.o: include/toad/cursor.hh include/toad/color.hh
test_toolbar.o: include/toad/region.hh include/toad/pen.hh
test_toolbar.o: include/toad/penbase.hh include/toad/font.hh
test_toolbar.o: include/toad/pointer.hh include/toad/matrix2d.hh
test_toolbar.o: include/toad/springlayout.hh include/toad/layout.hh
test_toolbar.o: include/toad/eventfilter.hh include/toad/menubar.hh
test_toolbar.o: include/toad/menuhelper.hh include/toad/action.hh
test_toolbar.o: include/toad/model.hh include/toad/command.hh
test_toolbar.o: include/toad/pushbutton.hh include/toad/buttonbase.hh
test_toolbar.o: include/toad/labelowner.hh include/toad/control.hh
test_toolbar.o: include/toad/focusmanager.hh include/toad/figure/toolbox.hh
test_toolbar.o: include/toad/figure/toolpanel.hh include/toad/figureeditor.hh
test_toolbar.o: include/toad/figure.hh include/toad/bitmap.hh
test_toolbar.o: include/toad/figuremodel.hh include/toad/wordprocessor.hh
test_toolbar.o: include/toad/scrollpane.hh include/toad/undo.hh
test_toolbar.o: include/toad/boolmodel.hh include/toad/textmodel.hh
test_toolbar.o: include/toad/integermodel.hh include/toad/numbermodel.hh
test_toolbar.o: include/toad/floatmodel.hh include/toad/undomanager.hh
test_toolbar.o: include/toad/figure/selectiontool.hh
test_toolbar.o: include/toad/figuretool.hh include/toad/core.hh
test_toolbar.o: include/toad/figure/nodetool.hh
test_toolbar.o: include/toad/figure/shapetool.hh
test_toolbar.o: include/toad/figure/texttool.hh
booleanop.o: include/toad/booleanop.hh include/toad/types.hh
booleanop.o: include/toad/io/serializable.hh include/toad/io/atvparser.hh
booleanop.o: include/toad/pen.hh include/toad/penbase.hh
booleanop.o: include/toad/color.hh include/toad/font.hh
booleanop.o: include/toad/pointer.hh include/toad/matrix2d.hh
booleanop.o: include/toad/geometry.hh include/toad/vector.hh
booleanop.o: include/toad/window.hh include/toad/interactor.hh
booleanop.o: include/toad/connect.hh include/toad/cursor.hh
booleanop.o: include/toad/region.hh include/toad/bitmap.hh
booleanop.o: include/toad/figuremodel.hh include/toad/model.hh
bop12/booleanop.o: bop12/booleanop.h bop12/polygon.h bop12/utilities.h
bop12/booleanop.o: bop12/point_2.h bop12/bbox_2.h bop12/segment_2.h
bop12/polygon.o: bop12/polygon.h bop12/utilities.h bop12/point_2.h
bop12/polygon.o: bop12/bbox_2.h bop12/segment_2.h
bop12/utilities.o: bop12/utilities.h bop12/point_2.h bop12/bbox_2.h
bop12/utilities.o: bop12/segment_2.h
window.o: include/toad/core.hh include/toad/window.hh
window.o: include/toad/interactor.hh include/toad/types.hh
window.o: include/toad/io/serializable.hh include/toad/io/atvparser.hh
window.o: include/toad/connect.hh include/toad/cursor.hh
window.o: include/toad/color.hh include/toad/region.hh include/toad/pen.hh
window.o: include/toad/penbase.hh include/toad/font.hh
window.o: include/toad/pointer.hh include/toad/matrix2d.hh
window.o: include/toad/bitmap.hh include/toad/layout.hh
window.o: include/toad/eventfilter.hh include/toad/focusmanager.hh
window.o: include/toad/io/urlstream.hh include/toad/command.hh
window.o: include/toad/stacktrace.hh cocoa/toadwindow.h cocoa/toadview.h
window.o: cocoa/toadview.impl
mouseevent.o: include/toad/window.hh include/toad/interactor.hh
mouseevent.o: include/toad/types.hh include/toad/io/serializable.hh
mouseevent.o: include/toad/io/atvparser.hh include/toad/connect.hh
mouseevent.o: include/toad/cursor.hh include/toad/color.hh
mouseevent.o: include/toad/region.hh include/toad/layout.hh
mouseevent.o: include/toad/eventfilter.hh cocoa/toadwindow.h cocoa/toadview.h
pen.o: include/toad/core.hh include/toad/window.hh include/toad/interactor.hh
pen.o: include/toad/types.hh include/toad/io/serializable.hh
pen.o: include/toad/io/atvparser.hh include/toad/connect.hh
pen.o: include/toad/cursor.hh include/toad/color.hh include/toad/region.hh
pen.o: include/toad/pen.hh include/toad/penbase.hh include/toad/font.hh
pen.o: include/toad/pointer.hh include/toad/matrix2d.hh
pen.o: include/toad/bitmap.hh
fischland/draw.o: fischland/fischland.hh include/toad/cursor.hh
fischland/draw.o: fischland/fischeditor.hh include/toad/figureeditor.hh
fischland/draw.o: include/toad/figure.hh include/toad/penbase.hh
fischland/draw.o: include/toad/color.hh include/toad/types.hh
fischland/draw.o: include/toad/io/serializable.hh
fischland/draw.o: include/toad/io/atvparser.hh include/toad/font.hh
fischland/draw.o: include/toad/pointer.hh include/toad/matrix2d.hh
fischland/draw.o: include/toad/window.hh include/toad/interactor.hh
fischland/draw.o: include/toad/connect.hh include/toad/region.hh
fischland/draw.o: include/toad/bitmap.hh include/toad/figuremodel.hh
fischland/draw.o: include/toad/model.hh include/toad/wordprocessor.hh
fischland/draw.o: include/toad/pen.hh include/toad/scrollpane.hh
fischland/draw.o: include/toad/undo.hh include/toad/boolmodel.hh
fischland/draw.o: include/toad/textmodel.hh include/toad/integermodel.hh
fischland/draw.o: include/toad/numbermodel.hh include/toad/floatmodel.hh
fischland/draw.o: include/toad/figure/toolbox.hh include/toad/action.hh
fischland/draw.o: fischland/page.hh include/toad/core.hh
fischland/draw.o: include/toad/dialog.hh include/toad/layout.hh
fischland/draw.o: include/toad/eventfilter.hh include/toad/treeadapter.hh
fischland/draw.o: include/toad/treemodel.hh include/toad/table.hh
fischland/draw.o: include/toad/dragndrop.hh fischland/fishbox.hh
fischland/draw.o: fischland/colorpalette.hh include/toad/combobox.hh
fischland/draw.o: include/toad/textfield.hh include/toad/textarea.hh
fischland/draw.o: include/toad/control.hh include/toad/scrollbar.hh
fischland/draw.o: include/toad/dnd/color.hh fischland/lineal.hh
fischland/draw.o: fischland/config.h include/toad/figure/connectfigure.hh
fischland/draw.o: include/toad/menubar.hh include/toad/menuhelper.hh
fischland/draw.o: include/toad/undomanager.hh include/toad/command.hh
fischland/draw.o: include/toad/fatradiobutton.hh
fischland/draw.o: include/toad/radiobuttonbase.hh include/toad/buttonbase.hh
fischland/draw.o: include/toad/labelowner.hh include/toad/pushbutton.hh
fischland/draw.o: include/toad/colorselector.hh include/toad/filedialog.hh
fischland/draw.o: include/toad/stl/vector.hh include/toad/arrowbutton.hh
fischland/draw.o: include/toad/simpletimer.hh include/toad/exception.hh
fischland/draw.o: include/toad/fatcheckbutton.hh include/toad/htmlview.hh
fischland/draw.o: include/toad/popupmenu.hh include/toad/messagebox.hh
fischland/draw.o: include/toad/springlayout.hh fischland/rotatetool.hh
fischland/draw.o: include/toad/figuretool.hh
fischland/colorpalette.o: fischland/colorpalette.hh include/toad/window.hh
fischland/colorpalette.o: include/toad/interactor.hh include/toad/types.hh
fischland/colorpalette.o: include/toad/io/serializable.hh
fischland/colorpalette.o: include/toad/io/atvparser.hh
fischland/colorpalette.o: include/toad/connect.hh include/toad/cursor.hh
fischland/colorpalette.o: include/toad/color.hh include/toad/region.hh
fischland/colorpalette.o: include/toad/combobox.hh include/toad/table.hh
fischland/colorpalette.o: include/toad/pen.hh include/toad/penbase.hh
fischland/colorpalette.o: include/toad/font.hh include/toad/pointer.hh
fischland/colorpalette.o: include/toad/matrix2d.hh include/toad/scrollpane.hh
fischland/colorpalette.o: include/toad/model.hh include/toad/dragndrop.hh
fischland/colorpalette.o: include/toad/textfield.hh include/toad/textarea.hh
fischland/colorpalette.o: include/toad/core.hh include/toad/bitmap.hh
fischland/colorpalette.o: include/toad/control.hh include/toad/textmodel.hh
fischland/colorpalette.o: include/toad/undo.hh include/toad/scrollbar.hh
fischland/colorpalette.o: include/toad/integermodel.hh
fischland/colorpalette.o: include/toad/numbermodel.hh
fischland/colorpalette.o: include/toad/dnd/color.hh
fischland/colorpalette.o: include/toad/figureeditor.hh include/toad/figure.hh
fischland/colorpalette.o: include/toad/figuremodel.hh
fischland/colorpalette.o: include/toad/wordprocessor.hh
fischland/colorpalette.o: include/toad/boolmodel.hh
fischland/colorpalette.o: include/toad/floatmodel.hh
fischland/colorpalette.o: include/toad/figure/toolbox.hh
fischland/colorpalette.o: include/toad/action.hh
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
fischland/fitcurve.o: include/toad/connect.hh include/toad/cursor.hh
fischland/fitcurve.o: include/toad/region.hh include/toad/bitmap.hh
fischland/fitcurve.o: include/toad/figuremodel.hh include/toad/model.hh
fischland/fitcurve.o: include/toad/wordprocessor.hh include/toad/pen.hh
fischland/fpath.o: fischland/fpath.hh include/toad/figure.hh
fischland/fpath.o: include/toad/penbase.hh include/toad/color.hh
fischland/fpath.o: include/toad/types.hh include/toad/io/serializable.hh
fischland/fpath.o: include/toad/io/atvparser.hh include/toad/font.hh
fischland/fpath.o: include/toad/pointer.hh include/toad/matrix2d.hh
fischland/fpath.o: include/toad/window.hh include/toad/interactor.hh
fischland/fpath.o: include/toad/connect.hh include/toad/cursor.hh
fischland/fpath.o: include/toad/region.hh include/toad/bitmap.hh
fischland/fpath.o: include/toad/figuremodel.hh include/toad/model.hh
fischland/fpath.o: include/toad/wordprocessor.hh include/toad/pen.hh
fischland/fpath.o: include/toad/action.hh include/toad/popupmenu.hh
fischland/fpath.o: include/toad/menuhelper.hh include/toad/layout.hh
fischland/fpath.o: include/toad/eventfilter.hh include/toad/figureeditor.hh
fischland/fpath.o: include/toad/scrollpane.hh include/toad/undo.hh
fischland/fpath.o: include/toad/boolmodel.hh include/toad/textmodel.hh
fischland/fpath.o: include/toad/integermodel.hh include/toad/numbermodel.hh
fischland/fpath.o: include/toad/floatmodel.hh include/toad/figure/toolbox.hh
fischland/fpath.o: include/toad/geometry.hh include/toad/vector.hh
fischland/lineal.o: fischland/lineal.hh include/toad/figureeditor.hh
fischland/lineal.o: include/toad/figure.hh include/toad/penbase.hh
fischland/lineal.o: include/toad/color.hh include/toad/types.hh
fischland/lineal.o: include/toad/io/serializable.hh
fischland/lineal.o: include/toad/io/atvparser.hh include/toad/font.hh
fischland/lineal.o: include/toad/pointer.hh include/toad/matrix2d.hh
fischland/lineal.o: include/toad/window.hh include/toad/interactor.hh
fischland/lineal.o: include/toad/connect.hh include/toad/cursor.hh
fischland/lineal.o: include/toad/region.hh include/toad/bitmap.hh
fischland/lineal.o: include/toad/figuremodel.hh include/toad/model.hh
fischland/lineal.o: include/toad/wordprocessor.hh include/toad/pen.hh
fischland/lineal.o: include/toad/scrollpane.hh include/toad/undo.hh
fischland/lineal.o: include/toad/boolmodel.hh include/toad/textmodel.hh
fischland/lineal.o: include/toad/integermodel.hh include/toad/numbermodel.hh
fischland/lineal.o: include/toad/floatmodel.hh include/toad/figure/toolbox.hh
fischland/lineal.o: include/toad/action.hh
fischland/page.o: fischland/page.hh include/toad/core.hh
fischland/page.o: include/toad/window.hh include/toad/interactor.hh
fischland/page.o: include/toad/types.hh include/toad/io/serializable.hh
fischland/page.o: include/toad/io/atvparser.hh include/toad/connect.hh
fischland/page.o: include/toad/cursor.hh include/toad/color.hh
fischland/page.o: include/toad/region.hh include/toad/pen.hh
fischland/page.o: include/toad/penbase.hh include/toad/font.hh
fischland/page.o: include/toad/pointer.hh include/toad/matrix2d.hh
fischland/page.o: include/toad/bitmap.hh include/toad/dialog.hh
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
fischland/fishbox.o: fischland/fishbox.hh include/toad/dialog.hh
fischland/fishbox.o: include/toad/window.hh include/toad/interactor.hh
fischland/fishbox.o: include/toad/types.hh include/toad/io/serializable.hh
fischland/fishbox.o: include/toad/io/atvparser.hh include/toad/connect.hh
fischland/fishbox.o: include/toad/cursor.hh include/toad/color.hh
fischland/fishbox.o: include/toad/region.hh include/toad/layout.hh
fischland/fishbox.o: include/toad/eventfilter.hh include/toad/figuremodel.hh
fischland/fishbox.o: include/toad/model.hh include/toad/pointer.hh
fischland/fishbox.o: include/toad/figureeditor.hh include/toad/figure.hh
fischland/fishbox.o: include/toad/penbase.hh include/toad/font.hh
fischland/fishbox.o: include/toad/matrix2d.hh include/toad/bitmap.hh
fischland/fishbox.o: include/toad/wordprocessor.hh include/toad/pen.hh
fischland/fishbox.o: include/toad/scrollpane.hh include/toad/undo.hh
fischland/fishbox.o: include/toad/boolmodel.hh include/toad/textmodel.hh
fischland/fishbox.o: include/toad/integermodel.hh include/toad/numbermodel.hh
fischland/fishbox.o: include/toad/floatmodel.hh
fischland/fishbox.o: include/toad/figure/toolbox.hh include/toad/action.hh
fischland/fishbox.o: include/toad/table.hh include/toad/dragndrop.hh
fischland/fishbox.o: include/toad/figure/selectiontool.hh
fischland/fishbox.o: include/toad/figuretool.hh include/toad/core.hh
fischland/fishbox.o: include/toad/undomanager.hh
fischland/fishbox.o: include/toad/figure/nodetool.hh
fischland/fishbox.o: include/toad/figure/shapetool.hh
fischland/fishbox.o: include/toad/figure/texttool.hh
fischland/fishbox.o: include/toad/figure/connecttool.hh fischland/filltool.hh
fischland/fishbox.o: fischland/fpath.hh fischland/pentool.hh
fischland/fishbox.o: fischland/penciltool.hh fischland/rotatetool.hh
fischland/fishbox.o: fischland/colorpicker.hh fischland/colorpalette.hh
fischland/fishbox.o: include/toad/combobox.hh include/toad/textfield.hh
fischland/fishbox.o: include/toad/textarea.hh include/toad/control.hh
fischland/fishbox.o: include/toad/scrollbar.hh include/toad/dnd/color.hh
fischland/fishbox.o: fischland/fischland.hh fischland/cairo.hh
fischland/fishbox.o: fischland/config.h include/toad/pushbutton.hh
fischland/fishbox.o: include/toad/buttonbase.hh include/toad/labelowner.hh
fischland/fishbox.o: include/toad/fatradiobutton.hh
fischland/fishbox.o: include/toad/radiobuttonbase.hh
fischland/fishbox.o: include/toad/fatcheckbutton.hh
fischland/fishbox.o: include/toad/colordialog.hh include/toad/rgbmodel.hh
fischland/fishbox.o: include/toad/gauge.hh include/toad/arrowbutton.hh
fischland/fishbox.o: include/toad/simpletimer.hh include/toad/messagebox.hh
fischland/fishbox.o: include/toad/figure/toolpanel.hh fischland/fontdialog.hh
fischland/colorpicker.o: fischland/colorpicker.hh
fischland/colorpicker.o: include/toad/figureeditor.hh include/toad/figure.hh
fischland/colorpicker.o: include/toad/penbase.hh include/toad/color.hh
fischland/colorpicker.o: include/toad/types.hh
fischland/colorpicker.o: include/toad/io/serializable.hh
fischland/colorpicker.o: include/toad/io/atvparser.hh include/toad/font.hh
fischland/colorpicker.o: include/toad/pointer.hh include/toad/matrix2d.hh
fischland/colorpicker.o: include/toad/window.hh include/toad/interactor.hh
fischland/colorpicker.o: include/toad/connect.hh include/toad/cursor.hh
fischland/colorpicker.o: include/toad/region.hh include/toad/bitmap.hh
fischland/colorpicker.o: include/toad/figuremodel.hh include/toad/model.hh
fischland/colorpicker.o: include/toad/wordprocessor.hh include/toad/pen.hh
fischland/colorpicker.o: include/toad/scrollpane.hh include/toad/undo.hh
fischland/colorpicker.o: include/toad/boolmodel.hh include/toad/textmodel.hh
fischland/colorpicker.o: include/toad/integermodel.hh
fischland/colorpicker.o: include/toad/numbermodel.hh
fischland/colorpicker.o: include/toad/floatmodel.hh
fischland/colorpicker.o: include/toad/figure/toolbox.hh
fischland/colorpicker.o: include/toad/action.hh fischland/colorpalette.hh
fischland/colorpicker.o: include/toad/combobox.hh include/toad/table.hh
fischland/colorpicker.o: include/toad/dragndrop.hh include/toad/textfield.hh
fischland/colorpicker.o: include/toad/textarea.hh include/toad/core.hh
fischland/colorpicker.o: include/toad/control.hh include/toad/scrollbar.hh
fischland/colorpicker.o: include/toad/dnd/color.hh fischland/fishbox.hh
fischland/colorpicker.o: include/toad/dialog.hh include/toad/layout.hh
fischland/colorpicker.o: include/toad/eventfilter.hh
fischland/rotatetool.o: fischland/rotatetool.hh include/toad/figureeditor.hh
fischland/rotatetool.o: include/toad/figure.hh include/toad/penbase.hh
fischland/rotatetool.o: include/toad/color.hh include/toad/types.hh
fischland/rotatetool.o: include/toad/io/serializable.hh
fischland/rotatetool.o: include/toad/io/atvparser.hh include/toad/font.hh
fischland/rotatetool.o: include/toad/pointer.hh include/toad/matrix2d.hh
fischland/rotatetool.o: include/toad/window.hh include/toad/interactor.hh
fischland/rotatetool.o: include/toad/connect.hh include/toad/cursor.hh
fischland/rotatetool.o: include/toad/region.hh include/toad/bitmap.hh
fischland/rotatetool.o: include/toad/figuremodel.hh include/toad/model.hh
fischland/rotatetool.o: include/toad/wordprocessor.hh include/toad/pen.hh
fischland/rotatetool.o: include/toad/scrollpane.hh include/toad/undo.hh
fischland/rotatetool.o: include/toad/boolmodel.hh include/toad/textmodel.hh
fischland/rotatetool.o: include/toad/integermodel.hh
fischland/rotatetool.o: include/toad/numbermodel.hh
fischland/rotatetool.o: include/toad/floatmodel.hh
fischland/rotatetool.o: include/toad/figure/toolbox.hh include/toad/action.hh
fischland/rotatetool.o: include/toad/figuretool.hh include/toad/core.hh
fischland/rotatetool.o: fischland/fischland.hh
fischland/pentool.o: fischland/pentool.hh fischland/fpath.hh
fischland/pentool.o: include/toad/figure.hh include/toad/penbase.hh
fischland/pentool.o: include/toad/color.hh include/toad/types.hh
fischland/pentool.o: include/toad/io/serializable.hh
fischland/pentool.o: include/toad/io/atvparser.hh include/toad/font.hh
fischland/pentool.o: include/toad/pointer.hh include/toad/matrix2d.hh
fischland/pentool.o: include/toad/window.hh include/toad/interactor.hh
fischland/pentool.o: include/toad/connect.hh include/toad/cursor.hh
fischland/pentool.o: include/toad/region.hh include/toad/bitmap.hh
fischland/pentool.o: include/toad/figuremodel.hh include/toad/model.hh
fischland/pentool.o: include/toad/wordprocessor.hh include/toad/pen.hh
fischland/pentool.o: include/toad/figuretool.hh include/toad/core.hh
fischland/pentool.o: fischland/fischland.hh include/toad/figureeditor.hh
fischland/pentool.o: include/toad/scrollpane.hh include/toad/undo.hh
fischland/pentool.o: include/toad/boolmodel.hh include/toad/textmodel.hh
fischland/pentool.o: include/toad/integermodel.hh include/toad/numbermodel.hh
fischland/pentool.o: include/toad/floatmodel.hh
fischland/pentool.o: include/toad/figure/toolbox.hh include/toad/action.hh
fischland/penciltool.o: fischland/penciltool.hh fischland/fpath.hh
fischland/penciltool.o: include/toad/figure.hh include/toad/penbase.hh
fischland/penciltool.o: include/toad/color.hh include/toad/types.hh
fischland/penciltool.o: include/toad/io/serializable.hh
fischland/penciltool.o: include/toad/io/atvparser.hh include/toad/font.hh
fischland/penciltool.o: include/toad/pointer.hh include/toad/matrix2d.hh
fischland/penciltool.o: include/toad/window.hh include/toad/interactor.hh
fischland/penciltool.o: include/toad/connect.hh include/toad/cursor.hh
fischland/penciltool.o: include/toad/region.hh include/toad/bitmap.hh
fischland/penciltool.o: include/toad/figuremodel.hh include/toad/model.hh
fischland/penciltool.o: include/toad/wordprocessor.hh include/toad/pen.hh
fischland/penciltool.o: include/toad/figuretool.hh include/toad/core.hh
fischland/penciltool.o: include/toad/integermodel.hh
fischland/penciltool.o: include/toad/numbermodel.hh include/toad/textmodel.hh
fischland/penciltool.o: include/toad/undo.hh include/toad/boolmodel.hh
fischland/penciltool.o: fischland/fischland.hh include/toad/figureeditor.hh
fischland/penciltool.o: include/toad/scrollpane.hh include/toad/floatmodel.hh
fischland/penciltool.o: include/toad/figure/toolbox.hh include/toad/action.hh
fischland/penciltool.o: include/toad/undomanager.hh include/toad/textfield.hh
fischland/penciltool.o: include/toad/textarea.hh include/toad/control.hh
fischland/penciltool.o: include/toad/scrollbar.hh include/toad/checkbox.hh
fischland/penciltool.o: include/toad/labelowner.hh
fischland/filltool.o: fischland/filltool.hh include/toad/figuretool.hh
fischland/filltool.o: include/toad/core.hh include/toad/window.hh
fischland/filltool.o: include/toad/interactor.hh include/toad/types.hh
fischland/filltool.o: include/toad/io/serializable.hh
fischland/filltool.o: include/toad/io/atvparser.hh include/toad/connect.hh
fischland/filltool.o: include/toad/cursor.hh include/toad/color.hh
fischland/filltool.o: include/toad/region.hh include/toad/pen.hh
fischland/filltool.o: include/toad/penbase.hh include/toad/font.hh
fischland/filltool.o: include/toad/pointer.hh include/toad/matrix2d.hh
fischland/filltool.o: include/toad/bitmap.hh include/toad/figure.hh
fischland/filltool.o: include/toad/figuremodel.hh include/toad/model.hh
fischland/filltool.o: include/toad/wordprocessor.hh fischland/fpath.hh
fischland/filltool.o: include/toad/figureeditor.hh include/toad/scrollpane.hh
fischland/filltool.o: include/toad/undo.hh include/toad/boolmodel.hh
fischland/filltool.o: include/toad/textmodel.hh include/toad/integermodel.hh
fischland/filltool.o: include/toad/numbermodel.hh include/toad/floatmodel.hh
fischland/filltool.o: include/toad/figure/toolbox.hh include/toad/action.hh
fischland/filltoolutil.o: fischland/filltool.hh include/toad/figuretool.hh
fischland/filltoolutil.o: include/toad/core.hh include/toad/window.hh
fischland/filltoolutil.o: include/toad/interactor.hh include/toad/types.hh
fischland/filltoolutil.o: include/toad/io/serializable.hh
fischland/filltoolutil.o: include/toad/io/atvparser.hh
fischland/filltoolutil.o: include/toad/connect.hh include/toad/cursor.hh
fischland/filltoolutil.o: include/toad/color.hh include/toad/region.hh
fischland/filltoolutil.o: include/toad/pen.hh include/toad/penbase.hh
fischland/filltoolutil.o: include/toad/font.hh include/toad/pointer.hh
fischland/filltoolutil.o: include/toad/matrix2d.hh include/toad/bitmap.hh
fischland/filltoolutil.o: include/toad/figure.hh include/toad/figuremodel.hh
fischland/filltoolutil.o: include/toad/model.hh include/toad/wordprocessor.hh
fischland/filltoolutil.o: fischland/fpath.hh
fischland/fischeditor.o: fischland/fischeditor.hh
fischland/fischeditor.o: include/toad/figureeditor.hh include/toad/figure.hh
fischland/fischeditor.o: include/toad/penbase.hh include/toad/color.hh
fischland/fischeditor.o: include/toad/types.hh
fischland/fischeditor.o: include/toad/io/serializable.hh
fischland/fischeditor.o: include/toad/io/atvparser.hh include/toad/font.hh
fischland/fischeditor.o: include/toad/pointer.hh include/toad/matrix2d.hh
fischland/fischeditor.o: include/toad/window.hh include/toad/interactor.hh
fischland/fischeditor.o: include/toad/connect.hh include/toad/cursor.hh
fischland/fischeditor.o: include/toad/region.hh include/toad/bitmap.hh
fischland/fischeditor.o: include/toad/figuremodel.hh include/toad/model.hh
fischland/fischeditor.o: include/toad/wordprocessor.hh include/toad/pen.hh
fischland/fischeditor.o: include/toad/scrollpane.hh include/toad/undo.hh
fischland/fischeditor.o: include/toad/boolmodel.hh include/toad/textmodel.hh
fischland/fischeditor.o: include/toad/integermodel.hh
fischland/fischeditor.o: include/toad/numbermodel.hh
fischland/fischeditor.o: include/toad/floatmodel.hh
fischland/fischeditor.o: include/toad/figure/toolbox.hh
fischland/fischeditor.o: include/toad/action.hh fischland/page.hh
fischland/fischeditor.o: include/toad/core.hh include/toad/dialog.hh
fischland/fischeditor.o: include/toad/layout.hh include/toad/eventfilter.hh
fischland/fischeditor.o: include/toad/treeadapter.hh
fischland/fischeditor.o: include/toad/treemodel.hh include/toad/table.hh
fischland/fischeditor.o: include/toad/dragndrop.hh
test/main.o: test/gtest.h
test/util.o: test/util.hh include/toad/window.hh include/toad/interactor.hh
test/util.o: include/toad/types.hh include/toad/io/serializable.hh
test/util.o: include/toad/io/atvparser.hh include/toad/connect.hh
test/util.o: include/toad/cursor.hh include/toad/color.hh
test/util.o: include/toad/region.hh test/gtest.h include/toad/core.hh
test/util.o: include/toad/pen.hh include/toad/penbase.hh include/toad/font.hh
test/util.o: include/toad/pointer.hh include/toad/matrix2d.hh
test/util.o: include/toad/bitmap.hh
test/gtest-all.o: test/gtest.h
test/display.o: test/util.hh include/toad/window.hh
test/display.o: include/toad/interactor.hh include/toad/types.hh
test/display.o: include/toad/io/serializable.hh include/toad/io/atvparser.hh
test/display.o: include/toad/connect.hh include/toad/cursor.hh
test/display.o: include/toad/color.hh include/toad/region.hh test/gtest.h
test/display.o: include/toad/core.hh include/toad/pen.hh
test/display.o: include/toad/penbase.hh include/toad/font.hh
test/display.o: include/toad/pointer.hh include/toad/matrix2d.hh
test/display.o: include/toad/bitmap.hh include/toad/simpletimer.hh
test/display.o: include/toad/scrollpane.hh
test/signal.o: include/toad/connect.hh test/gtest.h
test/figureeditor.o: test/util.hh include/toad/window.hh
test/figureeditor.o: include/toad/interactor.hh include/toad/types.hh
test/figureeditor.o: include/toad/io/serializable.hh
test/figureeditor.o: include/toad/io/atvparser.hh include/toad/connect.hh
test/figureeditor.o: include/toad/cursor.hh include/toad/color.hh
test/figureeditor.o: include/toad/region.hh test/gtest.h
test/figureeditor.o: include/toad/figureeditor.hh include/toad/figure.hh
test/figureeditor.o: include/toad/penbase.hh include/toad/font.hh
test/figureeditor.o: include/toad/pointer.hh include/toad/matrix2d.hh
test/figureeditor.o: include/toad/bitmap.hh include/toad/figuremodel.hh
test/figureeditor.o: include/toad/model.hh include/toad/wordprocessor.hh
test/figureeditor.o: include/toad/pen.hh include/toad/scrollpane.hh
test/figureeditor.o: include/toad/undo.hh include/toad/boolmodel.hh
test/figureeditor.o: include/toad/textmodel.hh include/toad/integermodel.hh
test/figureeditor.o: include/toad/numbermodel.hh include/toad/floatmodel.hh
test/figureeditor.o: include/toad/figure/toolbox.hh include/toad/action.hh
test/figureeditor.o: include/toad/figure/selectiontool.hh
test/figureeditor.o: include/toad/figuretool.hh include/toad/core.hh
test/figureeditor.o: include/toad/undomanager.hh
test/figureeditor.o: include/toad/figure/nodetool.hh
test/figureeditor.o: include/toad/figure/shapetool.hh
test/figureeditor-render.o: test/util.hh include/toad/window.hh
test/figureeditor-render.o: include/toad/interactor.hh include/toad/types.hh
test/figureeditor-render.o: include/toad/io/serializable.hh
test/figureeditor-render.o: include/toad/io/atvparser.hh
test/figureeditor-render.o: include/toad/connect.hh include/toad/cursor.hh
test/figureeditor-render.o: include/toad/color.hh include/toad/region.hh
test/figureeditor-render.o: test/gtest.h include/toad/core.hh
test/figureeditor-render.o: include/toad/pen.hh include/toad/penbase.hh
test/figureeditor-render.o: include/toad/font.hh include/toad/pointer.hh
test/figureeditor-render.o: include/toad/matrix2d.hh include/toad/bitmap.hh
test/figureeditor-render.o: include/toad/simpletimer.hh
test/figureeditor-render.o: include/toad/figureeditor.hh
test/figureeditor-render.o: include/toad/figure.hh
test/figureeditor-render.o: include/toad/figuremodel.hh include/toad/model.hh
test/figureeditor-render.o: include/toad/wordprocessor.hh
test/figureeditor-render.o: include/toad/scrollpane.hh include/toad/undo.hh
test/figureeditor-render.o: include/toad/boolmodel.hh
test/figureeditor-render.o: include/toad/textmodel.hh
test/figureeditor-render.o: include/toad/integermodel.hh
test/figureeditor-render.o: include/toad/numbermodel.hh
test/figureeditor-render.o: include/toad/floatmodel.hh
test/figureeditor-render.o: include/toad/figure/toolbox.hh
test/figureeditor-render.o: include/toad/action.hh
test/figureeditor-render.o: include/toad/figure/connectfigure.hh
test/wordprocessor.o: include/toad/wordprocessor.hh include/toad/types.hh
test/wordprocessor.o: include/toad/io/serializable.hh
test/wordprocessor.o: include/toad/io/atvparser.hh include/toad/color.hh
test/wordprocessor.o: include/toad/font.hh include/toad/pointer.hh
test/wordprocessor.o: include/toad/pen.hh include/toad/penbase.hh
test/wordprocessor.o: include/toad/matrix2d.hh include/toad/window.hh
test/wordprocessor.o: include/toad/interactor.hh include/toad/connect.hh
test/wordprocessor.o: include/toad/cursor.hh include/toad/region.hh
test/wordprocessor.o: include/toad/utf8.hh test/gtest.h
test/serializable.o: include/toad/io/serializable.hh
test/serializable.o: include/toad/io/atvparser.hh include/toad/types.hh
test/serializable.o: test/gtest.h
test/rectangle.o: include/toad/types.hh include/toad/io/serializable.hh
test/rectangle.o: include/toad/io/atvparser.hh test/gtest.h
test/booleanop.o: include/toad/geometry.hh include/toad/types.hh
test/booleanop.o: include/toad/io/serializable.hh
test/booleanop.o: include/toad/io/atvparser.hh include/toad/vector.hh
test/booleanop.o: include/toad/penbase.hh include/toad/color.hh
test/booleanop.o: include/toad/font.hh include/toad/pointer.hh
test/booleanop.o: include/toad/matrix2d.hh include/toad/window.hh
test/booleanop.o: include/toad/interactor.hh include/toad/connect.hh
test/booleanop.o: include/toad/cursor.hh include/toad/region.hh
test/booleanop.o: include/toad/bitmap.hh include/toad/figuremodel.hh
test/booleanop.o: include/toad/model.hh include/toad/booleanop.hh
test/booleanop.o: test/gtest.h
test/lineintersection.o: include/toad/geometry.hh include/toad/types.hh
test/lineintersection.o: include/toad/io/serializable.hh
test/lineintersection.o: include/toad/io/atvparser.hh include/toad/vector.hh
test/lineintersection.o: include/toad/penbase.hh include/toad/color.hh
test/lineintersection.o: include/toad/font.hh include/toad/pointer.hh
test/lineintersection.o: include/toad/matrix2d.hh include/toad/window.hh
test/lineintersection.o: include/toad/interactor.hh include/toad/connect.hh
test/lineintersection.o: include/toad/cursor.hh include/toad/region.hh
test/lineintersection.o: include/toad/bitmap.hh include/toad/figuremodel.hh
test/lineintersection.o: include/toad/model.hh test/gtest.h
test/fitcurve.o: include/toad/geometry.hh include/toad/types.hh
test/fitcurve.o: include/toad/io/serializable.hh include/toad/io/atvparser.hh
test/fitcurve.o: include/toad/vector.hh include/toad/penbase.hh
test/fitcurve.o: include/toad/color.hh include/toad/font.hh
test/fitcurve.o: include/toad/pointer.hh include/toad/matrix2d.hh
test/fitcurve.o: include/toad/window.hh include/toad/interactor.hh
test/fitcurve.o: include/toad/connect.hh include/toad/cursor.hh
test/fitcurve.o: include/toad/region.hh include/toad/bitmap.hh
test/fitcurve.o: include/toad/figuremodel.hh include/toad/model.hh
test/fitcurve.o: test/gtest.h
interactor.o: include/toad/interactor.hh include/toad/types.hh
interactor.o: include/toad/io/serializable.hh include/toad/io/atvparser.hh
interactor.o: include/toad/connect.hh
control.o: include/toad/control.hh include/toad/window.hh
control.o: include/toad/interactor.hh include/toad/types.hh
control.o: include/toad/io/serializable.hh include/toad/io/atvparser.hh
control.o: include/toad/connect.hh include/toad/cursor.hh
control.o: include/toad/color.hh include/toad/region.hh
labelowner.o: include/toad/labelowner.hh include/toad/control.hh
labelowner.o: include/toad/window.hh include/toad/interactor.hh
labelowner.o: include/toad/types.hh include/toad/io/serializable.hh
labelowner.o: include/toad/io/atvparser.hh include/toad/connect.hh
labelowner.o: include/toad/cursor.hh include/toad/color.hh
labelowner.o: include/toad/region.hh
buttonbase.o: include/toad/core.hh include/toad/window.hh
buttonbase.o: include/toad/interactor.hh include/toad/types.hh
buttonbase.o: include/toad/io/serializable.hh include/toad/io/atvparser.hh
buttonbase.o: include/toad/connect.hh include/toad/cursor.hh
buttonbase.o: include/toad/color.hh include/toad/region.hh
buttonbase.o: include/toad/pen.hh include/toad/penbase.hh
buttonbase.o: include/toad/font.hh include/toad/pointer.hh
buttonbase.o: include/toad/matrix2d.hh include/toad/bitmap.hh
buttonbase.o: include/toad/buttonbase.hh include/toad/labelowner.hh
buttonbase.o: include/toad/control.hh
pushbutton.o: include/toad/core.hh include/toad/window.hh
pushbutton.o: include/toad/interactor.hh include/toad/types.hh
pushbutton.o: include/toad/io/serializable.hh include/toad/io/atvparser.hh
pushbutton.o: include/toad/connect.hh include/toad/cursor.hh
pushbutton.o: include/toad/color.hh include/toad/region.hh
pushbutton.o: include/toad/pen.hh include/toad/penbase.hh
pushbutton.o: include/toad/font.hh include/toad/pointer.hh
pushbutton.o: include/toad/matrix2d.hh include/toad/bitmap.hh
pushbutton.o: include/toad/buttonbase.hh include/toad/labelowner.hh
pushbutton.o: include/toad/control.hh include/toad/pushbutton.hh
checkbox.o: include/toad/pen.hh include/toad/penbase.hh include/toad/color.hh
checkbox.o: include/toad/types.hh include/toad/io/serializable.hh
checkbox.o: include/toad/io/atvparser.hh include/toad/font.hh
checkbox.o: include/toad/pointer.hh include/toad/matrix2d.hh
checkbox.o: include/toad/window.hh include/toad/interactor.hh
checkbox.o: include/toad/connect.hh include/toad/cursor.hh
checkbox.o: include/toad/region.hh include/toad/checkbox.hh
checkbox.o: include/toad/labelowner.hh include/toad/control.hh
checkbox.o: include/toad/boolmodel.hh include/toad/model.hh
penbase.o: include/toad/penbase.hh include/toad/color.hh
penbase.o: include/toad/types.hh include/toad/io/serializable.hh
penbase.o: include/toad/io/atvparser.hh include/toad/font.hh
penbase.o: include/toad/pointer.hh include/toad/matrix2d.hh
focusmanager.o: include/toad/focusmanager.hh include/toad/window.hh
focusmanager.o: include/toad/interactor.hh include/toad/types.hh
focusmanager.o: include/toad/io/serializable.hh include/toad/io/atvparser.hh
focusmanager.o: include/toad/connect.hh include/toad/cursor.hh
focusmanager.o: include/toad/color.hh include/toad/region.hh
focusmanager.o: include/toad/eventfilter.hh
textarea.o: include/toad/textarea.hh include/toad/core.hh
textarea.o: include/toad/window.hh include/toad/interactor.hh
textarea.o: include/toad/types.hh include/toad/io/serializable.hh
textarea.o: include/toad/io/atvparser.hh include/toad/connect.hh
textarea.o: include/toad/cursor.hh include/toad/color.hh
textarea.o: include/toad/region.hh include/toad/pen.hh
textarea.o: include/toad/penbase.hh include/toad/font.hh
textarea.o: include/toad/pointer.hh include/toad/matrix2d.hh
textarea.o: include/toad/bitmap.hh include/toad/control.hh
textarea.o: include/toad/textmodel.hh include/toad/model.hh
textarea.o: include/toad/undo.hh include/toad/scrollbar.hh
textarea.o: include/toad/integermodel.hh include/toad/numbermodel.hh
textarea.o: include/toad/simpletimer.hh include/toad/undomanager.hh
textarea.o: include/toad/action.hh include/toad/utf8.hh
textfield.o: include/toad/textfield.hh include/toad/textarea.hh
textfield.o: include/toad/core.hh include/toad/window.hh
textfield.o: include/toad/interactor.hh include/toad/types.hh
textfield.o: include/toad/io/serializable.hh include/toad/io/atvparser.hh
textfield.o: include/toad/connect.hh include/toad/cursor.hh
textfield.o: include/toad/color.hh include/toad/region.hh include/toad/pen.hh
textfield.o: include/toad/penbase.hh include/toad/font.hh
textfield.o: include/toad/pointer.hh include/toad/matrix2d.hh
textfield.o: include/toad/bitmap.hh include/toad/control.hh
textfield.o: include/toad/textmodel.hh include/toad/model.hh
textfield.o: include/toad/undo.hh include/toad/scrollbar.hh
textfield.o: include/toad/integermodel.hh include/toad/numbermodel.hh
scrollpane.o: include/toad/scrollpane.hh include/toad/window.hh
scrollpane.o: include/toad/interactor.hh include/toad/types.hh
scrollpane.o: include/toad/io/serializable.hh include/toad/io/atvparser.hh
scrollpane.o: include/toad/connect.hh include/toad/cursor.hh
scrollpane.o: include/toad/color.hh include/toad/region.hh
scrollpane.o: include/toad/penbase.hh include/toad/font.hh
scrollpane.o: include/toad/pointer.hh include/toad/matrix2d.hh
scrollpane.o: include/toad/scrollbar.hh include/toad/control.hh
scrollpane.o: include/toad/integermodel.hh include/toad/numbermodel.hh
scrollpane.o: include/toad/model.hh include/toad/textmodel.hh
scrollpane.o: include/toad/undo.hh
table.o: include/toad/table.hh include/toad/pen.hh include/toad/penbase.hh
table.o: include/toad/color.hh include/toad/types.hh
table.o: include/toad/io/serializable.hh include/toad/io/atvparser.hh
table.o: include/toad/font.hh include/toad/pointer.hh
table.o: include/toad/matrix2d.hh include/toad/region.hh
table.o: include/toad/scrollpane.hh include/toad/window.hh
table.o: include/toad/interactor.hh include/toad/connect.hh
table.o: include/toad/cursor.hh include/toad/model.hh
table.o: include/toad/dragndrop.hh include/toad/figure.hh
table.o: include/toad/bitmap.hh include/toad/figuremodel.hh
table.o: include/toad/wordprocessor.hh
menuhelper.o: include/toad/menuhelper.hh include/toad/window.hh
menuhelper.o: include/toad/interactor.hh include/toad/types.hh
menuhelper.o: include/toad/io/serializable.hh include/toad/io/atvparser.hh
menuhelper.o: include/toad/connect.hh include/toad/cursor.hh
menuhelper.o: include/toad/color.hh include/toad/region.hh
menuhelper.o: include/toad/layout.hh include/toad/eventfilter.hh
menuhelper.o: include/toad/menubutton.hh include/toad/io/urlstream.hh
menuhelper.o: include/toad/action.hh include/toad/model.hh
menuhelper.o: include/toad/pointer.hh
menubutton.o: include/toad/menubutton.hh include/toad/menuhelper.hh
menubutton.o: include/toad/window.hh include/toad/interactor.hh
menubutton.o: include/toad/types.hh include/toad/io/serializable.hh
menubutton.o: include/toad/io/atvparser.hh include/toad/connect.hh
menubutton.o: include/toad/cursor.hh include/toad/color.hh
menubutton.o: include/toad/region.hh include/toad/layout.hh
menubutton.o: include/toad/eventfilter.hh include/toad/pen.hh
menubutton.o: include/toad/penbase.hh include/toad/font.hh
menubutton.o: include/toad/pointer.hh include/toad/matrix2d.hh
menubutton.o: include/toad/popup.hh include/toad/action.hh
menubutton.o: include/toad/model.hh include/toad/command.hh
menubutton.o: include/toad/bitmap.hh include/toad/focusmanager.hh
menubar.o: include/toad/menubar.hh include/toad/menuhelper.hh
menubar.o: include/toad/window.hh include/toad/interactor.hh
menubar.o: include/toad/types.hh include/toad/io/serializable.hh
menubar.o: include/toad/io/atvparser.hh include/toad/connect.hh
menubar.o: include/toad/cursor.hh include/toad/color.hh
menubar.o: include/toad/region.hh include/toad/layout.hh
menubar.o: include/toad/eventfilter.hh include/toad/focusmanager.hh
menubar.o: include/toad/pushbutton.hh include/toad/buttonbase.hh
menubar.o: include/toad/labelowner.hh include/toad/control.hh
menubar.o: include/toad/textfield.hh include/toad/textarea.hh
menubar.o: include/toad/core.hh include/toad/pen.hh include/toad/penbase.hh
menubar.o: include/toad/font.hh include/toad/pointer.hh
menubar.o: include/toad/matrix2d.hh include/toad/bitmap.hh
menubar.o: include/toad/textmodel.hh include/toad/model.hh
menubar.o: include/toad/undo.hh include/toad/scrollbar.hh
menubar.o: include/toad/integermodel.hh include/toad/numbermodel.hh
menubar.o: include/toad/action.hh include/toad/utf8.hh
popup.o: include/toad/popup.hh include/toad/menuhelper.hh
popup.o: include/toad/window.hh include/toad/interactor.hh
popup.o: include/toad/types.hh include/toad/io/serializable.hh
popup.o: include/toad/io/atvparser.hh include/toad/connect.hh
popup.o: include/toad/cursor.hh include/toad/color.hh include/toad/region.hh
popup.o: include/toad/layout.hh include/toad/eventfilter.hh
popupmenu.o: include/toad/popupmenu.hh include/toad/menuhelper.hh
popupmenu.o: include/toad/window.hh include/toad/interactor.hh
popupmenu.o: include/toad/types.hh include/toad/io/serializable.hh
popupmenu.o: include/toad/io/atvparser.hh include/toad/connect.hh
popupmenu.o: include/toad/cursor.hh include/toad/color.hh
popupmenu.o: include/toad/region.hh include/toad/layout.hh
popupmenu.o: include/toad/eventfilter.hh include/toad/focusmanager.hh
command.o: include/toad/command.hh include/toad/pointer.hh
command.o: include/toad/window.hh include/toad/interactor.hh
command.o: include/toad/types.hh include/toad/io/serializable.hh
command.o: include/toad/io/atvparser.hh include/toad/connect.hh
command.o: include/toad/cursor.hh include/toad/color.hh
command.o: include/toad/region.hh
dialog.o: include/toad/pen.hh include/toad/penbase.hh include/toad/color.hh
dialog.o: include/toad/types.hh include/toad/io/serializable.hh
dialog.o: include/toad/io/atvparser.hh include/toad/font.hh
dialog.o: include/toad/pointer.hh include/toad/matrix2d.hh
dialog.o: include/toad/window.hh include/toad/interactor.hh
dialog.o: include/toad/connect.hh include/toad/cursor.hh
dialog.o: include/toad/region.hh include/toad/dialog.hh
dialog.o: include/toad/layout.hh include/toad/eventfilter.hh
dialog.o: include/toad/figuremodel.hh include/toad/model.hh
dialog.o: include/toad/layouteditor.hh include/toad/control.hh
dialog.o: include/toad/dialogeditor.hh include/toad/figure.hh
dialog.o: include/toad/bitmap.hh include/toad/wordprocessor.hh
dialog.o: include/toad/labelowner.hh include/toad/io/urlstream.hh
dialog.o: include/toad/focusmanager.hh include/toad/undomanager.hh
dialog.o: include/toad/action.hh include/toad/undo.hh
dialog.o: include/toad/scrollbar.hh include/toad/integermodel.hh
dialog.o: include/toad/numbermodel.hh include/toad/textmodel.hh
dialog.o: include/toad/pushbutton.hh include/toad/buttonbase.hh
dialog.o: include/toad/fatradiobutton.hh include/toad/radiobuttonbase.hh
dialog.o: include/toad/textfield.hh include/toad/textarea.hh
dialog.o: include/toad/core.hh include/toad/figureeditor.hh
dialog.o: include/toad/scrollpane.hh include/toad/boolmodel.hh
dialog.o: include/toad/floatmodel.hh include/toad/figure/toolbox.hh
dialog.o: include/toad/colorselector.hh include/toad/dnd/color.hh
dialog.o: include/toad/dragndrop.hh include/toad/figure/nodetool.hh
dialog.o: include/toad/figuretool.hh
dialogeditor.o: include/toad/dialogeditor.hh include/toad/dialog.hh
dialogeditor.o: include/toad/window.hh include/toad/interactor.hh
dialogeditor.o: include/toad/types.hh include/toad/io/serializable.hh
dialogeditor.o: include/toad/io/atvparser.hh include/toad/connect.hh
dialogeditor.o: include/toad/cursor.hh include/toad/color.hh
dialogeditor.o: include/toad/region.hh include/toad/layout.hh
dialogeditor.o: include/toad/eventfilter.hh include/toad/figuremodel.hh
dialogeditor.o: include/toad/model.hh include/toad/pointer.hh
dialogeditor.o: include/toad/pushbutton.hh include/toad/buttonbase.hh
dialogeditor.o: include/toad/labelowner.hh include/toad/control.hh
dialogeditor.o: include/toad/fatcheckbutton.hh include/toad/boolmodel.hh
dialogeditor.o: include/toad/layouteditor.hh include/toad/focusmanager.hh
colorselector.o: include/toad/colorselector.hh include/toad/dnd/color.hh
colorselector.o: include/toad/dragndrop.hh include/toad/types.hh
colorselector.o: include/toad/io/serializable.hh include/toad/io/atvparser.hh
colorselector.o: include/toad/pointer.hh include/toad/connect.hh
colorselector.o: include/toad/color.hh include/toad/figureeditor.hh
colorselector.o: include/toad/figure.hh include/toad/penbase.hh
colorselector.o: include/toad/font.hh include/toad/matrix2d.hh
colorselector.o: include/toad/window.hh include/toad/interactor.hh
colorselector.o: include/toad/cursor.hh include/toad/region.hh
colorselector.o: include/toad/bitmap.hh include/toad/figuremodel.hh
colorselector.o: include/toad/model.hh include/toad/wordprocessor.hh
colorselector.o: include/toad/pen.hh include/toad/scrollpane.hh
colorselector.o: include/toad/undo.hh include/toad/boolmodel.hh
colorselector.o: include/toad/textmodel.hh include/toad/integermodel.hh
colorselector.o: include/toad/numbermodel.hh include/toad/floatmodel.hh
colorselector.o: include/toad/figure/toolbox.hh include/toad/action.hh
colorselector.o: include/toad/colordialog.hh include/toad/dialog.hh
colorselector.o: include/toad/layout.hh include/toad/eventfilter.hh
colorselector.o: include/toad/rgbmodel.hh include/toad/textarea.hh
colorselector.o: include/toad/core.hh include/toad/control.hh
colorselector.o: include/toad/scrollbar.hh include/toad/pushbutton.hh
colorselector.o: include/toad/buttonbase.hh include/toad/labelowner.hh
colorselector.o: include/toad/undomanager.hh
fatradiobutton.o: include/toad/pen.hh include/toad/penbase.hh
fatradiobutton.o: include/toad/color.hh include/toad/types.hh
fatradiobutton.o: include/toad/io/serializable.hh
fatradiobutton.o: include/toad/io/atvparser.hh include/toad/font.hh
fatradiobutton.o: include/toad/pointer.hh include/toad/matrix2d.hh
fatradiobutton.o: include/toad/window.hh include/toad/interactor.hh
fatradiobutton.o: include/toad/connect.hh include/toad/cursor.hh
fatradiobutton.o: include/toad/region.hh include/toad/buttonbase.hh
fatradiobutton.o: include/toad/labelowner.hh include/toad/control.hh
fatradiobutton.o: include/toad/fatradiobutton.hh
fatradiobutton.o: include/toad/radiobuttonbase.hh include/toad/model.hh
radiobuttonbase.o: include/toad/pen.hh include/toad/penbase.hh
radiobuttonbase.o: include/toad/color.hh include/toad/types.hh
radiobuttonbase.o: include/toad/io/serializable.hh
radiobuttonbase.o: include/toad/io/atvparser.hh include/toad/font.hh
radiobuttonbase.o: include/toad/pointer.hh include/toad/matrix2d.hh
radiobuttonbase.o: include/toad/window.hh include/toad/interactor.hh
radiobuttonbase.o: include/toad/connect.hh include/toad/cursor.hh
radiobuttonbase.o: include/toad/region.hh include/toad/radiobuttonbase.hh
radiobuttonbase.o: include/toad/buttonbase.hh include/toad/labelowner.hh
radiobuttonbase.o: include/toad/control.hh include/toad/model.hh
radiobutton.o: include/toad/pen.hh include/toad/penbase.hh
radiobutton.o: include/toad/color.hh include/toad/types.hh
radiobutton.o: include/toad/io/serializable.hh include/toad/io/atvparser.hh
radiobutton.o: include/toad/font.hh include/toad/pointer.hh
radiobutton.o: include/toad/matrix2d.hh include/toad/window.hh
radiobutton.o: include/toad/interactor.hh include/toad/connect.hh
radiobutton.o: include/toad/cursor.hh include/toad/region.hh
radiobutton.o: include/toad/radiobutton.hh include/toad/radiobuttonbase.hh
radiobutton.o: include/toad/buttonbase.hh include/toad/labelowner.hh
radiobutton.o: include/toad/control.hh include/toad/model.hh
fatcheckbutton.o: include/toad/fatcheckbutton.hh include/toad/buttonbase.hh
fatcheckbutton.o: include/toad/connect.hh include/toad/labelowner.hh
fatcheckbutton.o: include/toad/control.hh include/toad/window.hh
fatcheckbutton.o: include/toad/interactor.hh include/toad/types.hh
fatcheckbutton.o: include/toad/io/serializable.hh
fatcheckbutton.o: include/toad/io/atvparser.hh include/toad/cursor.hh
fatcheckbutton.o: include/toad/color.hh include/toad/region.hh
fatcheckbutton.o: include/toad/boolmodel.hh include/toad/model.hh
fatcheckbutton.o: include/toad/pointer.hh include/toad/pen.hh
fatcheckbutton.o: include/toad/penbase.hh include/toad/font.hh
fatcheckbutton.o: include/toad/matrix2d.hh
layouteditor.o: include/toad/layouteditor.hh include/toad/control.hh
layouteditor.o: include/toad/window.hh include/toad/interactor.hh
layouteditor.o: include/toad/types.hh include/toad/io/serializable.hh
layouteditor.o: include/toad/io/atvparser.hh include/toad/connect.hh
layouteditor.o: include/toad/cursor.hh include/toad/color.hh
layouteditor.o: include/toad/region.hh
color.o: include/toad/color.hh include/toad/types.hh
color.o: include/toad/io/serializable.hh include/toad/io/atvparser.hh
filedialog.o: include/toad/filedialog.hh include/toad/dialog.hh
filedialog.o: include/toad/window.hh include/toad/interactor.hh
filedialog.o: include/toad/types.hh include/toad/io/serializable.hh
filedialog.o: include/toad/io/atvparser.hh include/toad/connect.hh
filedialog.o: include/toad/cursor.hh include/toad/color.hh
filedialog.o: include/toad/region.hh include/toad/layout.hh
filedialog.o: include/toad/eventfilter.hh include/toad/figuremodel.hh
filedialog.o: include/toad/model.hh include/toad/pointer.hh
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
cursor.o: include/toad/connect.hh include/toad/cursor.hh
cursor.o: include/toad/region.hh include/toad/bitmap.hh
tableadapter.o: include/toad/table.hh include/toad/pen.hh
tableadapter.o: include/toad/penbase.hh include/toad/color.hh
tableadapter.o: include/toad/types.hh include/toad/io/serializable.hh
tableadapter.o: include/toad/io/atvparser.hh include/toad/font.hh
tableadapter.o: include/toad/pointer.hh include/toad/matrix2d.hh
tableadapter.o: include/toad/region.hh include/toad/scrollpane.hh
tableadapter.o: include/toad/window.hh include/toad/interactor.hh
tableadapter.o: include/toad/connect.hh include/toad/cursor.hh
tableadapter.o: include/toad/model.hh include/toad/dragndrop.hh
tableadapter.o: include/toad/utf8.hh
core.o: include/toad/core.hh include/toad/window.hh
core.o: include/toad/interactor.hh include/toad/types.hh
core.o: include/toad/io/serializable.hh include/toad/io/atvparser.hh
core.o: include/toad/connect.hh include/toad/cursor.hh include/toad/color.hh
core.o: include/toad/region.hh include/toad/pen.hh include/toad/penbase.hh
core.o: include/toad/font.hh include/toad/pointer.hh include/toad/matrix2d.hh
core.o: include/toad/bitmap.hh include/toad/figure.hh
core.o: include/toad/figuremodel.hh include/toad/model.hh
core.o: include/toad/wordprocessor.hh include/toad/command.hh
core.o: include/toad/dialogeditor.hh include/toad/dialog.hh
core.o: include/toad/layout.hh include/toad/eventfilter.hh
core.o: fischland/fontdialog.hh include/toad/textfield.hh
core.o: include/toad/textarea.hh include/toad/control.hh
core.o: include/toad/textmodel.hh include/toad/undo.hh
core.o: include/toad/scrollbar.hh include/toad/integermodel.hh
core.o: include/toad/numbermodel.hh include/toad/table.hh
core.o: include/toad/scrollpane.hh include/toad/dragndrop.hh
figure.o: include/toad/figure.hh include/toad/penbase.hh
figure.o: include/toad/color.hh include/toad/types.hh
figure.o: include/toad/io/serializable.hh include/toad/io/atvparser.hh
figure.o: include/toad/font.hh include/toad/pointer.hh
figure.o: include/toad/matrix2d.hh include/toad/window.hh
figure.o: include/toad/interactor.hh include/toad/connect.hh
figure.o: include/toad/cursor.hh include/toad/region.hh
figure.o: include/toad/bitmap.hh include/toad/figuremodel.hh
figure.o: include/toad/model.hh include/toad/wordprocessor.hh
figure.o: include/toad/pen.hh include/toad/figureeditor.hh
figure.o: include/toad/scrollpane.hh include/toad/undo.hh
figure.o: include/toad/boolmodel.hh include/toad/textmodel.hh
figure.o: include/toad/integermodel.hh include/toad/numbermodel.hh
figure.o: include/toad/floatmodel.hh include/toad/figure/toolbox.hh
figure.o: include/toad/action.hh include/toad/dialog.hh
figure.o: include/toad/layout.hh include/toad/eventfilter.hh
figure.o: include/toad/menuhelper.hh include/toad/springlayout.hh
figuremodel.o: include/toad/figure.hh include/toad/penbase.hh
figuremodel.o: include/toad/color.hh include/toad/types.hh
figuremodel.o: include/toad/io/serializable.hh include/toad/io/atvparser.hh
figuremodel.o: include/toad/font.hh include/toad/pointer.hh
figuremodel.o: include/toad/matrix2d.hh include/toad/window.hh
figuremodel.o: include/toad/interactor.hh include/toad/connect.hh
figuremodel.o: include/toad/cursor.hh include/toad/region.hh
figuremodel.o: include/toad/bitmap.hh include/toad/figuremodel.hh
figuremodel.o: include/toad/model.hh include/toad/wordprocessor.hh
figuremodel.o: include/toad/pen.hh include/toad/figureeditor.hh
figuremodel.o: include/toad/scrollpane.hh include/toad/undo.hh
figuremodel.o: include/toad/boolmodel.hh include/toad/textmodel.hh
figuremodel.o: include/toad/integermodel.hh include/toad/numbermodel.hh
figuremodel.o: include/toad/floatmodel.hh include/toad/figure/toolbox.hh
figuremodel.o: include/toad/action.hh include/toad/undomanager.hh
figuremodel.o: include/toad/io/binstream.hh
figureeditor.o: include/toad/figureeditor.hh include/toad/figure.hh
figureeditor.o: include/toad/penbase.hh include/toad/color.hh
figureeditor.o: include/toad/types.hh include/toad/io/serializable.hh
figureeditor.o: include/toad/io/atvparser.hh include/toad/font.hh
figureeditor.o: include/toad/pointer.hh include/toad/matrix2d.hh
figureeditor.o: include/toad/window.hh include/toad/interactor.hh
figureeditor.o: include/toad/connect.hh include/toad/cursor.hh
figureeditor.o: include/toad/region.hh include/toad/bitmap.hh
figureeditor.o: include/toad/figuremodel.hh include/toad/model.hh
figureeditor.o: include/toad/wordprocessor.hh include/toad/pen.hh
figureeditor.o: include/toad/scrollpane.hh include/toad/undo.hh
figureeditor.o: include/toad/boolmodel.hh include/toad/textmodel.hh
figureeditor.o: include/toad/integermodel.hh include/toad/numbermodel.hh
figureeditor.o: include/toad/floatmodel.hh include/toad/figure/toolbox.hh
figureeditor.o: include/toad/action.hh include/toad/figuretool.hh
figureeditor.o: include/toad/core.hh include/toad/colordialog.hh
figureeditor.o: include/toad/dialog.hh include/toad/layout.hh
figureeditor.o: include/toad/eventfilter.hh include/toad/rgbmodel.hh
figureeditor.o: include/toad/textarea.hh include/toad/control.hh
figureeditor.o: include/toad/scrollbar.hh include/toad/dnd/color.hh
figureeditor.o: include/toad/dragndrop.hh include/toad/checkbox.hh
figureeditor.o: include/toad/labelowner.hh include/toad/undomanager.hh
figureeditor.o: include/toad/stacktrace.hh
figuretool.o: include/toad/figuretool.hh include/toad/core.hh
figuretool.o: include/toad/window.hh include/toad/interactor.hh
figuretool.o: include/toad/types.hh include/toad/io/serializable.hh
figuretool.o: include/toad/io/atvparser.hh include/toad/connect.hh
figuretool.o: include/toad/cursor.hh include/toad/color.hh
figuretool.o: include/toad/region.hh include/toad/pen.hh
figuretool.o: include/toad/penbase.hh include/toad/font.hh
figuretool.o: include/toad/pointer.hh include/toad/matrix2d.hh
figuretool.o: include/toad/bitmap.hh include/toad/figure.hh
figuretool.o: include/toad/figuremodel.hh include/toad/model.hh
figuretool.o: include/toad/wordprocessor.hh
matrix2d.o: include/toad/matrix2d.hh include/toad/types.hh
matrix2d.o: include/toad/io/serializable.hh include/toad/io/atvparser.hh
simpletimer.o: include/toad/simpletimer.hh include/toad/types.hh
simpletimer.o: include/toad/io/serializable.hh include/toad/io/atvparser.hh
region.o: include/toad/region.hh include/toad/types.hh
region.o: include/toad/io/serializable.hh include/toad/io/atvparser.hh
polygon.o: include/toad/types.hh include/toad/io/serializable.hh
polygon.o: include/toad/io/atvparser.hh include/toad/matrix2d.hh
springlayout.o: include/toad/springlayout.hh include/toad/layout.hh
springlayout.o: include/toad/io/serializable.hh include/toad/io/atvparser.hh
springlayout.o: include/toad/eventfilter.hh include/toad/window.hh
springlayout.o: include/toad/interactor.hh include/toad/types.hh
springlayout.o: include/toad/connect.hh include/toad/cursor.hh
springlayout.o: include/toad/color.hh include/toad/region.hh
combobox.o: include/toad/combobox.hh include/toad/table.hh
combobox.o: include/toad/pen.hh include/toad/penbase.hh include/toad/color.hh
combobox.o: include/toad/types.hh include/toad/io/serializable.hh
combobox.o: include/toad/io/atvparser.hh include/toad/font.hh
combobox.o: include/toad/pointer.hh include/toad/matrix2d.hh
combobox.o: include/toad/region.hh include/toad/scrollpane.hh
combobox.o: include/toad/window.hh include/toad/interactor.hh
combobox.o: include/toad/connect.hh include/toad/cursor.hh
combobox.o: include/toad/model.hh include/toad/dragndrop.hh
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
treemodel.o: include/toad/interactor.hh include/toad/connect.hh
treemodel.o: include/toad/cursor.hh include/toad/model.hh
treemodel.o: include/toad/dragndrop.hh
treeadapter.o: include/toad/treeadapter.hh include/toad/treemodel.hh
treeadapter.o: include/toad/table.hh include/toad/pen.hh
treeadapter.o: include/toad/penbase.hh include/toad/color.hh
treeadapter.o: include/toad/types.hh include/toad/io/serializable.hh
treeadapter.o: include/toad/io/atvparser.hh include/toad/font.hh
treeadapter.o: include/toad/pointer.hh include/toad/matrix2d.hh
treeadapter.o: include/toad/region.hh include/toad/scrollpane.hh
treeadapter.o: include/toad/window.hh include/toad/interactor.hh
treeadapter.o: include/toad/connect.hh include/toad/cursor.hh
treeadapter.o: include/toad/model.hh include/toad/dragndrop.hh
htmlview.o: include/toad/htmlview.hh include/toad/scrollpane.hh
htmlview.o: include/toad/window.hh include/toad/interactor.hh
htmlview.o: include/toad/types.hh include/toad/io/serializable.hh
htmlview.o: include/toad/io/atvparser.hh include/toad/connect.hh
htmlview.o: include/toad/cursor.hh include/toad/color.hh
htmlview.o: include/toad/region.hh include/toad/penbase.hh
htmlview.o: include/toad/font.hh include/toad/pointer.hh
htmlview.o: include/toad/matrix2d.hh include/toad/scrollbar.hh
htmlview.o: include/toad/control.hh include/toad/integermodel.hh
htmlview.o: include/toad/numbermodel.hh include/toad/model.hh
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
messagebox.o: include/toad/connect.hh include/toad/cursor.hh
messagebox.o: include/toad/region.hh include/toad/pushbutton.hh
messagebox.o: include/toad/buttonbase.hh include/toad/labelowner.hh
messagebox.o: include/toad/control.hh include/toad/messagebox.hh
layout.o: include/toad/layout.hh include/toad/io/serializable.hh
layout.o: include/toad/io/atvparser.hh include/toad/eventfilter.hh
layout.o: include/toad/window.hh include/toad/interactor.hh
layout.o: include/toad/types.hh include/toad/connect.hh
layout.o: include/toad/cursor.hh include/toad/color.hh include/toad/region.hh
layout.o: include/toad/textarea.hh include/toad/core.hh include/toad/pen.hh
layout.o: include/toad/penbase.hh include/toad/font.hh
layout.o: include/toad/pointer.hh include/toad/matrix2d.hh
layout.o: include/toad/bitmap.hh include/toad/control.hh
layout.o: include/toad/textmodel.hh include/toad/model.hh
layout.o: include/toad/undo.hh include/toad/scrollbar.hh
layout.o: include/toad/integermodel.hh include/toad/numbermodel.hh
layout.o: include/toad/pushbutton.hh include/toad/buttonbase.hh
layout.o: include/toad/labelowner.hh include/toad/io/urlstream.hh
layout.o: include/toad/layouteditor.hh
pointer.o: include/toad/pointer.hh
connect.o: include/toad/connect.hh
rectangle.o: include/toad/types.hh include/toad/io/serializable.hh
rectangle.o: include/toad/io/atvparser.hh
font.o: include/toad/font.hh include/toad/pointer.hh include/toad/types.hh
font.o: include/toad/io/serializable.hh include/toad/io/atvparser.hh
eventfilter.o: include/toad/eventfilter.hh
arrowbutton.o: include/toad/arrowbutton.hh include/toad/pushbutton.hh
arrowbutton.o: include/toad/buttonbase.hh include/toad/connect.hh
arrowbutton.o: include/toad/labelowner.hh include/toad/control.hh
arrowbutton.o: include/toad/window.hh include/toad/interactor.hh
arrowbutton.o: include/toad/types.hh include/toad/io/serializable.hh
arrowbutton.o: include/toad/io/atvparser.hh include/toad/cursor.hh
arrowbutton.o: include/toad/color.hh include/toad/region.hh
arrowbutton.o: include/toad/simpletimer.hh include/toad/pen.hh
arrowbutton.o: include/toad/penbase.hh include/toad/font.hh
arrowbutton.o: include/toad/pointer.hh include/toad/matrix2d.hh
scrollbar.o: include/toad/scrollbar.hh include/toad/control.hh
scrollbar.o: include/toad/window.hh include/toad/interactor.hh
scrollbar.o: include/toad/types.hh include/toad/io/serializable.hh
scrollbar.o: include/toad/io/atvparser.hh include/toad/connect.hh
scrollbar.o: include/toad/cursor.hh include/toad/color.hh
scrollbar.o: include/toad/region.hh include/toad/integermodel.hh
scrollbar.o: include/toad/numbermodel.hh include/toad/model.hh
scrollbar.o: include/toad/pointer.hh include/toad/textmodel.hh
scrollbar.o: include/toad/undo.hh include/toad/pen.hh include/toad/penbase.hh
scrollbar.o: include/toad/font.hh include/toad/matrix2d.hh
scrollbar.o: include/toad/arrowbutton.hh include/toad/pushbutton.hh
scrollbar.o: include/toad/buttonbase.hh include/toad/labelowner.hh
scrollbar.o: include/toad/simpletimer.hh
utf8.o: include/toad/utf8.hh
undo.o: include/toad/undo.hh
undomanager.o: include/toad/undomanager.hh include/toad/interactor.hh
undomanager.o: include/toad/types.hh include/toad/io/serializable.hh
undomanager.o: include/toad/io/atvparser.hh include/toad/connect.hh
undomanager.o: include/toad/action.hh include/toad/window.hh
undomanager.o: include/toad/cursor.hh include/toad/color.hh
undomanager.o: include/toad/region.hh include/toad/model.hh
undomanager.o: include/toad/pointer.hh include/toad/undo.hh
undomanager.o: include/toad/exception.hh
model.o: include/toad/model.hh include/toad/pointer.hh
model.o: include/toad/connect.hh include/toad/undomanager.hh
model.o: include/toad/interactor.hh include/toad/types.hh
model.o: include/toad/io/serializable.hh include/toad/io/atvparser.hh
model.o: include/toad/action.hh include/toad/window.hh include/toad/cursor.hh
model.o: include/toad/color.hh include/toad/region.hh include/toad/undo.hh
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
action.o: include/toad/io/serializable.hh include/toad/io/atvparser.hh
action.o: include/toad/connect.hh include/toad/cursor.hh
action.o: include/toad/color.hh include/toad/region.hh include/toad/model.hh
action.o: include/toad/pointer.hh
bitmap.o: include/toad/bitmap.hh include/toad/penbase.hh
bitmap.o: include/toad/color.hh include/toad/types.hh
bitmap.o: include/toad/io/serializable.hh include/toad/io/atvparser.hh
bitmap.o: include/toad/font.hh include/toad/pointer.hh
bitmap.o: include/toad/matrix2d.hh
io/atvparser.o: io/atvparser.hh
io/binstream.o: include/toad/io/binstream.hh include/toad/types.hh
io/binstream.o: include/toad/io/serializable.hh include/toad/io/atvparser.hh
io/serializable.o: io/serializable.hh include/toad/io/atvparser.hh
io/urlstream.o: include/toad/io/urlstream.hh
gauge.o: include/toad/gauge.hh include/toad/arrowbutton.hh
gauge.o: include/toad/pushbutton.hh include/toad/buttonbase.hh
gauge.o: include/toad/connect.hh include/toad/labelowner.hh
gauge.o: include/toad/control.hh include/toad/window.hh
gauge.o: include/toad/interactor.hh include/toad/types.hh
gauge.o: include/toad/io/serializable.hh include/toad/io/atvparser.hh
gauge.o: include/toad/cursor.hh include/toad/color.hh include/toad/region.hh
gauge.o: include/toad/simpletimer.hh include/toad/integermodel.hh
gauge.o: include/toad/numbermodel.hh include/toad/model.hh
gauge.o: include/toad/pointer.hh include/toad/textmodel.hh
gauge.o: include/toad/undo.hh
colordialog.o: include/toad/colordialog.hh include/toad/dialog.hh
colordialog.o: include/toad/window.hh include/toad/interactor.hh
colordialog.o: include/toad/types.hh include/toad/io/serializable.hh
colordialog.o: include/toad/io/atvparser.hh include/toad/connect.hh
colordialog.o: include/toad/cursor.hh include/toad/color.hh
colordialog.o: include/toad/region.hh include/toad/layout.hh
colordialog.o: include/toad/eventfilter.hh include/toad/figuremodel.hh
colordialog.o: include/toad/model.hh include/toad/pointer.hh
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
dragndrop.o: include/toad/io/serializable.hh include/toad/io/atvparser.hh
dragndrop.o: include/toad/pointer.hh include/toad/connect.hh
dragndrop.o: include/toad/window.hh include/toad/interactor.hh
dragndrop.o: include/toad/cursor.hh include/toad/color.hh
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
types.o: include/toad/types.hh include/toad/io/serializable.hh
types.o: include/toad/io/atvparser.hh
dnd/dropobject.o: include/toad/dnd/dropobject.hh include/toad/dragndrop.hh
dnd/dropobject.o: include/toad/types.hh include/toad/io/serializable.hh
dnd/dropobject.o: include/toad/io/atvparser.hh include/toad/pointer.hh
dnd/dropobject.o: include/toad/connect.hh
dnd/color.o: include/toad/dnd/color.hh include/toad/dragndrop.hh
dnd/color.o: include/toad/types.hh include/toad/io/serializable.hh
dnd/color.o: include/toad/io/atvparser.hh include/toad/pointer.hh
dnd/color.o: include/toad/connect.hh include/toad/color.hh
dnd/textplain.o: include/toad/dnd/textplain.hh include/toad/connect.hh
dnd/textplain.o: include/toad/dragndrop.hh include/toad/types.hh
dnd/textplain.o: include/toad/io/serializable.hh include/toad/io/atvparser.hh
dnd/textplain.o: include/toad/pointer.hh
dnd/image.o: include/toad/dnd/image.hh include/toad/dragndrop.hh
dnd/image.o: include/toad/types.hh include/toad/io/serializable.hh
dnd/image.o: include/toad/io/atvparser.hh include/toad/pointer.hh
dnd/image.o: include/toad/connect.hh include/toad/bitmap.hh
dnd/image.o: include/toad/penbase.hh include/toad/color.hh
dnd/image.o: include/toad/font.hh include/toad/matrix2d.hh
figure/toolbox.o: include/toad/figure/toolbox.hh include/toad/window.hh
figure/toolbox.o: include/toad/interactor.hh include/toad/types.hh
figure/toolbox.o: include/toad/io/serializable.hh
figure/toolbox.o: include/toad/io/atvparser.hh include/toad/connect.hh
figure/toolbox.o: include/toad/cursor.hh include/toad/color.hh
figure/toolbox.o: include/toad/region.hh include/toad/action.hh
figure/toolbox.o: include/toad/model.hh include/toad/pointer.hh
figure/toolbox.o: include/toad/eventfilter.hh include/toad/focusmanager.hh
figure/toolpanel.o: include/toad/figure/toolpanel.hh
figure/toolpanel.o: include/toad/figureeditor.hh include/toad/figure.hh
figure/toolpanel.o: include/toad/penbase.hh include/toad/color.hh
figure/toolpanel.o: include/toad/types.hh include/toad/io/serializable.hh
figure/toolpanel.o: include/toad/io/atvparser.hh include/toad/font.hh
figure/toolpanel.o: include/toad/pointer.hh include/toad/matrix2d.hh
figure/toolpanel.o: include/toad/window.hh include/toad/interactor.hh
figure/toolpanel.o: include/toad/connect.hh include/toad/cursor.hh
figure/toolpanel.o: include/toad/region.hh include/toad/bitmap.hh
figure/toolpanel.o: include/toad/figuremodel.hh include/toad/model.hh
figure/toolpanel.o: include/toad/wordprocessor.hh include/toad/pen.hh
figure/toolpanel.o: include/toad/scrollpane.hh include/toad/undo.hh
figure/toolpanel.o: include/toad/boolmodel.hh include/toad/textmodel.hh
figure/toolpanel.o: include/toad/integermodel.hh include/toad/numbermodel.hh
figure/toolpanel.o: include/toad/floatmodel.hh include/toad/figure/toolbox.hh
figure/toolpanel.o: include/toad/action.hh include/toad/undomanager.hh
figure/toolpanel.o: include/toad/figure/toolbutton.hh
figure/toolpanel.o: include/toad/buttonbase.hh include/toad/labelowner.hh
figure/toolpanel.o: include/toad/control.hh
figure/toolbutton.o: include/toad/figure/toolbutton.hh
figure/toolbutton.o: include/toad/buttonbase.hh include/toad/connect.hh
figure/toolbutton.o: include/toad/labelowner.hh include/toad/control.hh
figure/toolbutton.o: include/toad/window.hh include/toad/interactor.hh
figure/toolbutton.o: include/toad/types.hh include/toad/io/serializable.hh
figure/toolbutton.o: include/toad/io/atvparser.hh include/toad/cursor.hh
figure/toolbutton.o: include/toad/color.hh include/toad/region.hh
figure/toolbutton.o: include/toad/action.hh include/toad/model.hh
figure/toolbutton.o: include/toad/pointer.hh include/toad/pen.hh
figure/toolbutton.o: include/toad/penbase.hh include/toad/font.hh
figure/toolbutton.o: include/toad/matrix2d.hh
figure/frame.o: include/toad/figure.hh include/toad/penbase.hh
figure/frame.o: include/toad/color.hh include/toad/types.hh
figure/frame.o: include/toad/io/serializable.hh include/toad/io/atvparser.hh
figure/frame.o: include/toad/font.hh include/toad/pointer.hh
figure/frame.o: include/toad/matrix2d.hh include/toad/window.hh
figure/frame.o: include/toad/interactor.hh include/toad/connect.hh
figure/frame.o: include/toad/cursor.hh include/toad/region.hh
figure/frame.o: include/toad/bitmap.hh include/toad/figuremodel.hh
figure/frame.o: include/toad/model.hh include/toad/wordprocessor.hh
figure/frame.o: include/toad/pen.hh include/toad/figureeditor.hh
figure/frame.o: include/toad/scrollpane.hh include/toad/undo.hh
figure/frame.o: include/toad/boolmodel.hh include/toad/textmodel.hh
figure/frame.o: include/toad/integermodel.hh include/toad/numbermodel.hh
figure/frame.o: include/toad/floatmodel.hh include/toad/figure/toolbox.hh
figure/frame.o: include/toad/action.hh
figure/image.o: include/toad/figure.hh include/toad/penbase.hh
figure/image.o: include/toad/color.hh include/toad/types.hh
figure/image.o: include/toad/io/serializable.hh include/toad/io/atvparser.hh
figure/image.o: include/toad/font.hh include/toad/pointer.hh
figure/image.o: include/toad/matrix2d.hh include/toad/window.hh
figure/image.o: include/toad/interactor.hh include/toad/connect.hh
figure/image.o: include/toad/cursor.hh include/toad/region.hh
figure/image.o: include/toad/bitmap.hh include/toad/figuremodel.hh
figure/image.o: include/toad/model.hh include/toad/wordprocessor.hh
figure/image.o: include/toad/pen.hh include/toad/figureeditor.hh
figure/image.o: include/toad/scrollpane.hh include/toad/undo.hh
figure/image.o: include/toad/boolmodel.hh include/toad/textmodel.hh
figure/image.o: include/toad/integermodel.hh include/toad/numbermodel.hh
figure/image.o: include/toad/floatmodel.hh include/toad/figure/toolbox.hh
figure/image.o: include/toad/action.hh include/toad/filedialog.hh
figure/image.o: include/toad/dialog.hh include/toad/layout.hh
figure/image.o: include/toad/eventfilter.hh include/toad/table.hh
figure/image.o: include/toad/dragndrop.hh include/toad/pushbutton.hh
figure/image.o: include/toad/buttonbase.hh include/toad/labelowner.hh
figure/image.o: include/toad/control.hh include/toad/stl/vector.hh
figure/arrow.o: include/toad/figure.hh include/toad/penbase.hh
figure/arrow.o: include/toad/color.hh include/toad/types.hh
figure/arrow.o: include/toad/io/serializable.hh include/toad/io/atvparser.hh
figure/arrow.o: include/toad/font.hh include/toad/pointer.hh
figure/arrow.o: include/toad/matrix2d.hh include/toad/window.hh
figure/arrow.o: include/toad/interactor.hh include/toad/connect.hh
figure/arrow.o: include/toad/cursor.hh include/toad/region.hh
figure/arrow.o: include/toad/bitmap.hh include/toad/figuremodel.hh
figure/arrow.o: include/toad/model.hh include/toad/wordprocessor.hh
figure/arrow.o: include/toad/pen.hh include/toad/figureeditor.hh
figure/arrow.o: include/toad/scrollpane.hh include/toad/undo.hh
figure/arrow.o: include/toad/boolmodel.hh include/toad/textmodel.hh
figure/arrow.o: include/toad/integermodel.hh include/toad/numbermodel.hh
figure/arrow.o: include/toad/floatmodel.hh include/toad/figure/toolbox.hh
figure/arrow.o: include/toad/action.hh
figure/text.o: include/toad/figure.hh include/toad/penbase.hh
figure/text.o: include/toad/color.hh include/toad/types.hh
figure/text.o: include/toad/io/serializable.hh include/toad/io/atvparser.hh
figure/text.o: include/toad/font.hh include/toad/pointer.hh
figure/text.o: include/toad/matrix2d.hh include/toad/window.hh
figure/text.o: include/toad/interactor.hh include/toad/connect.hh
figure/text.o: include/toad/cursor.hh include/toad/region.hh
figure/text.o: include/toad/bitmap.hh include/toad/figuremodel.hh
figure/text.o: include/toad/model.hh include/toad/wordprocessor.hh
figure/text.o: include/toad/pen.hh include/toad/figureeditor.hh
figure/text.o: include/toad/scrollpane.hh include/toad/undo.hh
figure/text.o: include/toad/boolmodel.hh include/toad/textmodel.hh
figure/text.o: include/toad/integermodel.hh include/toad/numbermodel.hh
figure/text.o: include/toad/floatmodel.hh include/toad/figure/toolbox.hh
figure/text.o: include/toad/action.hh
figure/circle.o: include/toad/figure.hh include/toad/penbase.hh
figure/circle.o: include/toad/color.hh include/toad/types.hh
figure/circle.o: include/toad/io/serializable.hh include/toad/io/atvparser.hh
figure/circle.o: include/toad/font.hh include/toad/pointer.hh
figure/circle.o: include/toad/matrix2d.hh include/toad/window.hh
figure/circle.o: include/toad/interactor.hh include/toad/connect.hh
figure/circle.o: include/toad/cursor.hh include/toad/region.hh
figure/circle.o: include/toad/bitmap.hh include/toad/figuremodel.hh
figure/circle.o: include/toad/model.hh include/toad/wordprocessor.hh
figure/circle.o: include/toad/pen.hh include/toad/figureeditor.hh
figure/circle.o: include/toad/scrollpane.hh include/toad/undo.hh
figure/circle.o: include/toad/boolmodel.hh include/toad/textmodel.hh
figure/circle.o: include/toad/integermodel.hh include/toad/numbermodel.hh
figure/circle.o: include/toad/floatmodel.hh include/toad/figure/toolbox.hh
figure/circle.o: include/toad/action.hh include/toad/vector.hh
figure/group.o: include/toad/figure.hh include/toad/penbase.hh
figure/group.o: include/toad/color.hh include/toad/types.hh
figure/group.o: include/toad/io/serializable.hh include/toad/io/atvparser.hh
figure/group.o: include/toad/font.hh include/toad/pointer.hh
figure/group.o: include/toad/matrix2d.hh include/toad/window.hh
figure/group.o: include/toad/interactor.hh include/toad/connect.hh
figure/group.o: include/toad/cursor.hh include/toad/region.hh
figure/group.o: include/toad/bitmap.hh include/toad/figuremodel.hh
figure/group.o: include/toad/model.hh include/toad/wordprocessor.hh
figure/group.o: include/toad/pen.hh include/toad/figureeditor.hh
figure/group.o: include/toad/scrollpane.hh include/toad/undo.hh
figure/group.o: include/toad/boolmodel.hh include/toad/textmodel.hh
figure/group.o: include/toad/integermodel.hh include/toad/numbermodel.hh
figure/group.o: include/toad/floatmodel.hh include/toad/figure/toolbox.hh
figure/group.o: include/toad/action.hh
figure/transform.o: include/toad/vector.hh include/toad/penbase.hh
figure/transform.o: include/toad/color.hh include/toad/types.hh
figure/transform.o: include/toad/io/serializable.hh
figure/transform.o: include/toad/io/atvparser.hh include/toad/font.hh
figure/transform.o: include/toad/pointer.hh include/toad/matrix2d.hh
figure/transform.o: include/toad/window.hh include/toad/interactor.hh
figure/transform.o: include/toad/connect.hh include/toad/cursor.hh
figure/transform.o: include/toad/region.hh include/toad/bitmap.hh
figure/transform.o: include/toad/figuremodel.hh include/toad/model.hh
figure/transform.o: include/toad/figure.hh include/toad/wordprocessor.hh
figure/transform.o: include/toad/pen.hh include/toad/figureeditor.hh
figure/transform.o: include/toad/scrollpane.hh include/toad/undo.hh
figure/transform.o: include/toad/boolmodel.hh include/toad/textmodel.hh
figure/transform.o: include/toad/integermodel.hh include/toad/numbermodel.hh
figure/transform.o: include/toad/floatmodel.hh include/toad/figure/toolbox.hh
figure/transform.o: include/toad/action.hh
figure/perspectivetransform.o: include/toad/vector.hh include/toad/penbase.hh
figure/perspectivetransform.o: include/toad/color.hh include/toad/types.hh
figure/perspectivetransform.o: include/toad/io/serializable.hh
figure/perspectivetransform.o: include/toad/io/atvparser.hh
figure/perspectivetransform.o: include/toad/font.hh include/toad/pointer.hh
figure/perspectivetransform.o: include/toad/matrix2d.hh
figure/perspectivetransform.o: include/toad/window.hh
figure/perspectivetransform.o: include/toad/interactor.hh
figure/perspectivetransform.o: include/toad/connect.hh include/toad/cursor.hh
figure/perspectivetransform.o: include/toad/region.hh include/toad/bitmap.hh
figure/perspectivetransform.o: include/toad/figuremodel.hh
figure/perspectivetransform.o: include/toad/model.hh include/toad/figure.hh
figure/perspectivetransform.o: include/toad/wordprocessor.hh
figure/perspectivetransform.o: include/toad/pen.hh
figure/perspectivetransform.o: include/toad/figureeditor.hh
figure/perspectivetransform.o: include/toad/scrollpane.hh
figure/perspectivetransform.o: include/toad/undo.hh include/toad/boolmodel.hh
figure/perspectivetransform.o: include/toad/textmodel.hh
figure/perspectivetransform.o: include/toad/integermodel.hh
figure/perspectivetransform.o: include/toad/numbermodel.hh
figure/perspectivetransform.o: include/toad/floatmodel.hh
figure/perspectivetransform.o: include/toad/figure/toolbox.hh
figure/perspectivetransform.o: include/toad/action.hh
figure/rectangle.o: include/toad/vector.hh include/toad/penbase.hh
figure/rectangle.o: include/toad/color.hh include/toad/types.hh
figure/rectangle.o: include/toad/io/serializable.hh
figure/rectangle.o: include/toad/io/atvparser.hh include/toad/font.hh
figure/rectangle.o: include/toad/pointer.hh include/toad/matrix2d.hh
figure/rectangle.o: include/toad/window.hh include/toad/interactor.hh
figure/rectangle.o: include/toad/connect.hh include/toad/cursor.hh
figure/rectangle.o: include/toad/region.hh include/toad/bitmap.hh
figure/rectangle.o: include/toad/figuremodel.hh include/toad/model.hh
figure/rectangle.o: include/toad/figure.hh include/toad/wordprocessor.hh
figure/rectangle.o: include/toad/pen.hh include/toad/figureeditor.hh
figure/rectangle.o: include/toad/scrollpane.hh include/toad/undo.hh
figure/rectangle.o: include/toad/boolmodel.hh include/toad/textmodel.hh
figure/rectangle.o: include/toad/integermodel.hh include/toad/numbermodel.hh
figure/rectangle.o: include/toad/floatmodel.hh include/toad/figure/toolbox.hh
figure/rectangle.o: include/toad/action.hh include/toad/figure/shapetool.hh
figure/rectangle.o: include/toad/figuretool.hh include/toad/core.hh
figure/window.o: include/toad/figure.hh include/toad/penbase.hh
figure/window.o: include/toad/color.hh include/toad/types.hh
figure/window.o: include/toad/io/serializable.hh include/toad/io/atvparser.hh
figure/window.o: include/toad/font.hh include/toad/pointer.hh
figure/window.o: include/toad/matrix2d.hh include/toad/window.hh
figure/window.o: include/toad/interactor.hh include/toad/connect.hh
figure/window.o: include/toad/cursor.hh include/toad/region.hh
figure/window.o: include/toad/bitmap.hh include/toad/figuremodel.hh
figure/window.o: include/toad/model.hh include/toad/wordprocessor.hh
figure/window.o: include/toad/pen.hh include/toad/figureeditor.hh
figure/window.o: include/toad/scrollpane.hh include/toad/undo.hh
figure/window.o: include/toad/boolmodel.hh include/toad/textmodel.hh
figure/window.o: include/toad/integermodel.hh include/toad/numbermodel.hh
figure/window.o: include/toad/floatmodel.hh include/toad/figure/toolbox.hh
figure/window.o: include/toad/action.hh include/toad/labelowner.hh
figure/window.o: include/toad/control.hh
figure/selectiontool.o: include/toad/figure/selectiontool.hh
figure/selectiontool.o: include/toad/figureeditor.hh include/toad/figure.hh
figure/selectiontool.o: include/toad/penbase.hh include/toad/color.hh
figure/selectiontool.o: include/toad/types.hh include/toad/io/serializable.hh
figure/selectiontool.o: include/toad/io/atvparser.hh include/toad/font.hh
figure/selectiontool.o: include/toad/pointer.hh include/toad/matrix2d.hh
figure/selectiontool.o: include/toad/window.hh include/toad/interactor.hh
figure/selectiontool.o: include/toad/connect.hh include/toad/cursor.hh
figure/selectiontool.o: include/toad/region.hh include/toad/bitmap.hh
figure/selectiontool.o: include/toad/figuremodel.hh include/toad/model.hh
figure/selectiontool.o: include/toad/wordprocessor.hh include/toad/pen.hh
figure/selectiontool.o: include/toad/scrollpane.hh include/toad/undo.hh
figure/selectiontool.o: include/toad/boolmodel.hh include/toad/textmodel.hh
figure/selectiontool.o: include/toad/integermodel.hh
figure/selectiontool.o: include/toad/numbermodel.hh
figure/selectiontool.o: include/toad/floatmodel.hh
figure/selectiontool.o: include/toad/figure/toolbox.hh include/toad/action.hh
figure/selectiontool.o: include/toad/figuretool.hh include/toad/core.hh
figure/selectiontool.o: include/toad/undomanager.hh include/toad/vector.hh
figure/nodetool.o: include/toad/figure/nodetool.hh include/toad/figuretool.hh
figure/nodetool.o: include/toad/core.hh include/toad/window.hh
figure/nodetool.o: include/toad/interactor.hh include/toad/types.hh
figure/nodetool.o: include/toad/io/serializable.hh
figure/nodetool.o: include/toad/io/atvparser.hh include/toad/connect.hh
figure/nodetool.o: include/toad/cursor.hh include/toad/color.hh
figure/nodetool.o: include/toad/region.hh include/toad/pen.hh
figure/nodetool.o: include/toad/penbase.hh include/toad/font.hh
figure/nodetool.o: include/toad/pointer.hh include/toad/matrix2d.hh
figure/nodetool.o: include/toad/bitmap.hh include/toad/figure.hh
figure/nodetool.o: include/toad/figuremodel.hh include/toad/model.hh
figure/nodetool.o: include/toad/wordprocessor.hh include/toad/figureeditor.hh
figure/nodetool.o: include/toad/scrollpane.hh include/toad/undo.hh
figure/nodetool.o: include/toad/boolmodel.hh include/toad/textmodel.hh
figure/nodetool.o: include/toad/integermodel.hh include/toad/numbermodel.hh
figure/nodetool.o: include/toad/floatmodel.hh include/toad/figure/toolbox.hh
figure/nodetool.o: include/toad/action.hh include/toad/undomanager.hh
figure/shapetool.o: include/toad/figure/shapetool.hh
figure/shapetool.o: include/toad/figuretool.hh include/toad/core.hh
figure/shapetool.o: include/toad/window.hh include/toad/interactor.hh
figure/shapetool.o: include/toad/types.hh include/toad/io/serializable.hh
figure/shapetool.o: include/toad/io/atvparser.hh include/toad/connect.hh
figure/shapetool.o: include/toad/cursor.hh include/toad/color.hh
figure/shapetool.o: include/toad/region.hh include/toad/pen.hh
figure/shapetool.o: include/toad/penbase.hh include/toad/font.hh
figure/shapetool.o: include/toad/pointer.hh include/toad/matrix2d.hh
figure/shapetool.o: include/toad/bitmap.hh include/toad/figure.hh
figure/shapetool.o: include/toad/figuremodel.hh include/toad/model.hh
figure/shapetool.o: include/toad/wordprocessor.hh
figure/shapetool.o: include/toad/figureeditor.hh include/toad/scrollpane.hh
figure/shapetool.o: include/toad/undo.hh include/toad/boolmodel.hh
figure/shapetool.o: include/toad/textmodel.hh include/toad/integermodel.hh
figure/shapetool.o: include/toad/numbermodel.hh include/toad/floatmodel.hh
figure/shapetool.o: include/toad/figure/toolbox.hh include/toad/action.hh
figure/texttool.o: include/toad/figure/texttool.hh include/toad/figuretool.hh
figure/texttool.o: include/toad/core.hh include/toad/window.hh
figure/texttool.o: include/toad/interactor.hh include/toad/types.hh
figure/texttool.o: include/toad/io/serializable.hh
figure/texttool.o: include/toad/io/atvparser.hh include/toad/connect.hh
figure/texttool.o: include/toad/cursor.hh include/toad/color.hh
figure/texttool.o: include/toad/region.hh include/toad/pen.hh
figure/texttool.o: include/toad/penbase.hh include/toad/font.hh
figure/texttool.o: include/toad/pointer.hh include/toad/matrix2d.hh
figure/texttool.o: include/toad/bitmap.hh include/toad/figure.hh
figure/texttool.o: include/toad/figuremodel.hh include/toad/model.hh
figure/texttool.o: include/toad/wordprocessor.hh include/toad/figureeditor.hh
figure/texttool.o: include/toad/scrollpane.hh include/toad/undo.hh
figure/texttool.o: include/toad/boolmodel.hh include/toad/textmodel.hh
figure/texttool.o: include/toad/integermodel.hh include/toad/numbermodel.hh
figure/texttool.o: include/toad/floatmodel.hh include/toad/figure/toolbox.hh
figure/texttool.o: include/toad/action.hh
figure/connecttool.o: include/toad/figure/connecttool.hh
figure/connecttool.o: include/toad/figuretool.hh include/toad/core.hh
figure/connecttool.o: include/toad/window.hh include/toad/interactor.hh
figure/connecttool.o: include/toad/types.hh include/toad/io/serializable.hh
figure/connecttool.o: include/toad/io/atvparser.hh include/toad/connect.hh
figure/connecttool.o: include/toad/cursor.hh include/toad/color.hh
figure/connecttool.o: include/toad/region.hh include/toad/pen.hh
figure/connecttool.o: include/toad/penbase.hh include/toad/font.hh
figure/connecttool.o: include/toad/pointer.hh include/toad/matrix2d.hh
figure/connecttool.o: include/toad/bitmap.hh include/toad/figure.hh
figure/connecttool.o: include/toad/figuremodel.hh include/toad/model.hh
figure/connecttool.o: include/toad/wordprocessor.hh
figure/connecttool.o: include/toad/figure/connectfigure.hh
figure/connecttool.o: include/toad/figureeditor.hh include/toad/scrollpane.hh
figure/connecttool.o: include/toad/undo.hh include/toad/boolmodel.hh
figure/connecttool.o: include/toad/textmodel.hh include/toad/integermodel.hh
figure/connecttool.o: include/toad/numbermodel.hh include/toad/floatmodel.hh
figure/connecttool.o: include/toad/figure/toolbox.hh include/toad/action.hh
figure/connecttool.o: include/toad/vector.hh
figure/connectfigure.o: include/toad/figure/connectfigure.hh
figure/connectfigure.o: include/toad/figure.hh include/toad/penbase.hh
figure/connectfigure.o: include/toad/color.hh include/toad/types.hh
figure/connectfigure.o: include/toad/io/serializable.hh
figure/connectfigure.o: include/toad/io/atvparser.hh include/toad/font.hh
figure/connectfigure.o: include/toad/pointer.hh include/toad/matrix2d.hh
figure/connectfigure.o: include/toad/window.hh include/toad/interactor.hh
figure/connectfigure.o: include/toad/connect.hh include/toad/cursor.hh
figure/connectfigure.o: include/toad/region.hh include/toad/bitmap.hh
figure/connectfigure.o: include/toad/figuremodel.hh include/toad/model.hh
figure/connectfigure.o: include/toad/wordprocessor.hh include/toad/pen.hh
figure/connectfigure.o: include/toad/figureeditor.hh
figure/connectfigure.o: include/toad/scrollpane.hh include/toad/undo.hh
figure/connectfigure.o: include/toad/boolmodel.hh include/toad/textmodel.hh
figure/connectfigure.o: include/toad/integermodel.hh
figure/connectfigure.o: include/toad/numbermodel.hh
figure/connectfigure.o: include/toad/floatmodel.hh
figure/connectfigure.o: include/toad/figure/toolbox.hh include/toad/action.hh
figure/connectfigure.o: include/toad/vector.hh include/toad/geometry.hh
vector.o: include/toad/vector.hh include/toad/penbase.hh
vector.o: include/toad/color.hh include/toad/types.hh
vector.o: include/toad/io/serializable.hh include/toad/io/atvparser.hh
vector.o: include/toad/font.hh include/toad/pointer.hh
vector.o: include/toad/matrix2d.hh include/toad/window.hh
vector.o: include/toad/interactor.hh include/toad/connect.hh
vector.o: include/toad/cursor.hh include/toad/region.hh
vector.o: include/toad/bitmap.hh include/toad/figuremodel.hh
vector.o: include/toad/model.hh include/toad/geometry.hh include/toad/pen.hh
vector.o: include/toad/figure.hh include/toad/wordprocessor.hh
geometry.o: include/toad/geometry.hh include/toad/types.hh
geometry.o: include/toad/io/serializable.hh include/toad/io/atvparser.hh
geometry.o: include/toad/vector.hh include/toad/penbase.hh
geometry.o: include/toad/color.hh include/toad/font.hh
geometry.o: include/toad/pointer.hh include/toad/matrix2d.hh
geometry.o: include/toad/window.hh include/toad/interactor.hh
geometry.o: include/toad/connect.hh include/toad/cursor.hh
geometry.o: include/toad/region.hh include/toad/bitmap.hh
geometry.o: include/toad/figuremodel.hh include/toad/model.hh
wordprocessor.o: include/toad/utf8.hh include/toad/wordprocessor.hh
wordprocessor.o: include/toad/types.hh include/toad/io/serializable.hh
wordprocessor.o: include/toad/io/atvparser.hh include/toad/color.hh
wordprocessor.o: include/toad/font.hh include/toad/pointer.hh
wordprocessor.o: include/toad/pen.hh include/toad/penbase.hh
wordprocessor.o: include/toad/matrix2d.hh include/toad/window.hh
wordprocessor.o: include/toad/interactor.hh include/toad/connect.hh
wordprocessor.o: include/toad/cursor.hh include/toad/region.hh
stacktrace.o: include/toad/stacktrace.hh
test_table.o: include/toad/table.hh include/toad/pen.hh
test_table.o: include/toad/penbase.hh include/toad/color.hh
test_table.o: include/toad/types.hh include/toad/io/serializable.hh
test_table.o: include/toad/io/atvparser.hh include/toad/font.hh
test_table.o: include/toad/pointer.hh include/toad/matrix2d.hh
test_table.o: include/toad/region.hh include/toad/scrollpane.hh
test_table.o: include/toad/window.hh include/toad/interactor.hh
test_table.o: include/toad/connect.hh include/toad/cursor.hh
test_table.o: include/toad/model.hh include/toad/dragndrop.hh
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
test_scroll.o: include/toad/connect.hh include/toad/cursor.hh
test_scroll.o: include/toad/model.hh include/toad/dragndrop.hh
test_scroll.o: include/toad/checkbox.hh include/toad/labelowner.hh
test_scroll.o: include/toad/control.hh include/toad/boolmodel.hh
test_scroll.o: include/toad/radiobutton.hh include/toad/radiobuttonbase.hh
test_scroll.o: include/toad/buttonbase.hh include/toad/dialog.hh
test_scroll.o: include/toad/layout.hh include/toad/eventfilter.hh
test_scroll.o: include/toad/figuremodel.hh include/toad/stl/vector.hh
test_scroll.o: include/toad/utf8.hh
test_dialog.o: include/toad/dialog.hh include/toad/window.hh
test_dialog.o: include/toad/interactor.hh include/toad/types.hh
test_dialog.o: include/toad/io/serializable.hh include/toad/io/atvparser.hh
test_dialog.o: include/toad/connect.hh include/toad/cursor.hh
test_dialog.o: include/toad/color.hh include/toad/region.hh
test_dialog.o: include/toad/layout.hh include/toad/eventfilter.hh
test_dialog.o: include/toad/figuremodel.hh include/toad/model.hh
test_dialog.o: include/toad/pointer.hh include/toad/textarea.hh
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
test_timer.o: include/toad/types.hh include/toad/io/serializable.hh
test_timer.o: include/toad/io/atvparser.hh include/toad/connect.hh
test_timer.o: include/toad/cursor.hh include/toad/color.hh
test_timer.o: include/toad/region.hh include/toad/simpletimer.hh
test_combobox.o: include/toad/combobox.hh include/toad/table.hh
test_combobox.o: include/toad/pen.hh include/toad/penbase.hh
test_combobox.o: include/toad/color.hh include/toad/types.hh
test_combobox.o: include/toad/io/serializable.hh include/toad/io/atvparser.hh
test_combobox.o: include/toad/font.hh include/toad/pointer.hh
test_combobox.o: include/toad/matrix2d.hh include/toad/region.hh
test_combobox.o: include/toad/scrollpane.hh include/toad/window.hh
test_combobox.o: include/toad/interactor.hh include/toad/connect.hh
test_combobox.o: include/toad/cursor.hh include/toad/model.hh
test_combobox.o: include/toad/dragndrop.hh
test_cursor.o: include/toad/window.hh include/toad/interactor.hh
test_cursor.o: include/toad/types.hh include/toad/io/serializable.hh
test_cursor.o: include/toad/io/atvparser.hh include/toad/connect.hh
test_cursor.o: include/toad/cursor.hh include/toad/color.hh
test_cursor.o: include/toad/region.hh include/toad/pen.hh
test_cursor.o: include/toad/penbase.hh include/toad/font.hh
test_cursor.o: include/toad/pointer.hh include/toad/matrix2d.hh
test_colordialog.o: include/toad/colordialog.hh include/toad/dialog.hh
test_colordialog.o: include/toad/window.hh include/toad/interactor.hh
test_colordialog.o: include/toad/types.hh include/toad/io/serializable.hh
test_colordialog.o: include/toad/io/atvparser.hh include/toad/connect.hh
test_colordialog.o: include/toad/cursor.hh include/toad/color.hh
test_colordialog.o: include/toad/region.hh include/toad/layout.hh
test_colordialog.o: include/toad/eventfilter.hh include/toad/figuremodel.hh
test_colordialog.o: include/toad/model.hh include/toad/pointer.hh
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
test_grab.o: include/toad/types.hh include/toad/io/serializable.hh
test_grab.o: include/toad/io/atvparser.hh include/toad/connect.hh
test_grab.o: include/toad/cursor.hh include/toad/color.hh
test_grab.o: include/toad/region.hh include/toad/pen.hh
test_grab.o: include/toad/penbase.hh include/toad/font.hh
test_grab.o: include/toad/pointer.hh include/toad/matrix2d.hh
test_path.o: include/toad/window.hh include/toad/interactor.hh
test_path.o: include/toad/types.hh include/toad/io/serializable.hh
test_path.o: include/toad/io/atvparser.hh include/toad/connect.hh
test_path.o: include/toad/cursor.hh include/toad/color.hh
test_path.o: include/toad/region.hh include/toad/pen.hh
test_path.o: include/toad/penbase.hh include/toad/font.hh
test_path.o: include/toad/pointer.hh include/toad/matrix2d.hh
test_path.o: include/toad/figure.hh include/toad/bitmap.hh
test_path.o: include/toad/figuremodel.hh include/toad/model.hh
test_path.o: include/toad/wordprocessor.hh include/toad/vector.hh
test_path.o: include/toad/geometry.hh
test_image.o: include/toad/window.hh include/toad/interactor.hh
test_image.o: include/toad/types.hh include/toad/io/serializable.hh
test_image.o: include/toad/io/atvparser.hh include/toad/connect.hh
test_image.o: include/toad/cursor.hh include/toad/color.hh
test_image.o: include/toad/region.hh include/toad/pen.hh
test_image.o: include/toad/penbase.hh include/toad/font.hh
test_image.o: include/toad/pointer.hh include/toad/matrix2d.hh
test_image.o: include/toad/bitmap.hh
test_curve.o: include/toad/window.hh include/toad/interactor.hh
test_curve.o: include/toad/types.hh include/toad/io/serializable.hh
test_curve.o: include/toad/io/atvparser.hh include/toad/connect.hh
test_curve.o: include/toad/cursor.hh include/toad/color.hh
test_curve.o: include/toad/region.hh include/toad/pen.hh
test_curve.o: include/toad/penbase.hh include/toad/font.hh
test_curve.o: include/toad/pointer.hh include/toad/matrix2d.hh
test_curve.o: include/toad/bitmap.hh
test_text.o: include/toad/wordprocessor.hh include/toad/types.hh
test_text.o: include/toad/io/serializable.hh include/toad/io/atvparser.hh
test_text.o: include/toad/color.hh include/toad/font.hh
test_text.o: include/toad/pointer.hh include/toad/pen.hh
test_text.o: include/toad/penbase.hh include/toad/matrix2d.hh
test_text.o: include/toad/window.hh include/toad/interactor.hh
test_text.o: include/toad/connect.hh include/toad/cursor.hh
test_text.o: include/toad/region.hh include/toad/utf8.hh
test_tablet.o: include/toad/window.hh include/toad/interactor.hh
test_tablet.o: include/toad/types.hh include/toad/io/serializable.hh
test_tablet.o: include/toad/io/atvparser.hh include/toad/connect.hh
test_tablet.o: include/toad/cursor.hh include/toad/color.hh
test_tablet.o: include/toad/region.hh include/toad/pen.hh
test_tablet.o: include/toad/penbase.hh include/toad/font.hh
test_tablet.o: include/toad/pointer.hh include/toad/matrix2d.hh
test_tablet.o: include/toad/figure.hh include/toad/bitmap.hh
test_tablet.o: include/toad/figuremodel.hh include/toad/model.hh
test_tablet.o: include/toad/wordprocessor.hh include/toad/vector.hh
test_tablet.o: include/toad/geometry.hh include/toad/dialog.hh
test_tablet.o: include/toad/layout.hh include/toad/eventfilter.hh
test_tablet.o: include/toad/springlayout.hh include/toad/textfield.hh
test_tablet.o: include/toad/textarea.hh include/toad/core.hh
test_tablet.o: include/toad/control.hh include/toad/textmodel.hh
test_tablet.o: include/toad/undo.hh include/toad/scrollbar.hh
test_tablet.o: include/toad/integermodel.hh include/toad/numbermodel.hh
test_tablet.o: include/toad/fatradiobutton.hh include/toad/radiobuttonbase.hh
test_tablet.o: include/toad/buttonbase.hh include/toad/labelowner.hh
test_tablet.o: include/toad/scrollpane.hh include/toad/floatmodel.hh
test_tablet.o: include/toad/simpletimer.hh booleanop.hh
test_path_bool.o: include/toad/window.hh include/toad/interactor.hh
test_path_bool.o: include/toad/types.hh include/toad/io/serializable.hh
test_path_bool.o: include/toad/io/atvparser.hh include/toad/connect.hh
test_path_bool.o: include/toad/cursor.hh include/toad/color.hh
test_path_bool.o: include/toad/region.hh include/toad/pen.hh
test_path_bool.o: include/toad/penbase.hh include/toad/font.hh
test_path_bool.o: include/toad/pointer.hh include/toad/matrix2d.hh
test_path_bool.o: include/toad/figure.hh include/toad/bitmap.hh
test_path_bool.o: include/toad/figuremodel.hh include/toad/model.hh
test_path_bool.o: include/toad/wordprocessor.hh include/toad/vector.hh
test_path_bool.o: include/toad/geometry.hh include/toad/booleanop.hh
test_guitar.o: include/toad/window.hh include/toad/interactor.hh
test_guitar.o: include/toad/types.hh include/toad/io/serializable.hh
test_guitar.o: include/toad/io/atvparser.hh include/toad/connect.hh
test_guitar.o: include/toad/cursor.hh include/toad/color.hh
test_guitar.o: include/toad/region.hh include/toad/dialog.hh
test_guitar.o: include/toad/layout.hh include/toad/eventfilter.hh
test_guitar.o: include/toad/figuremodel.hh include/toad/model.hh
test_guitar.o: include/toad/pointer.hh include/toad/pen.hh
test_guitar.o: include/toad/penbase.hh include/toad/font.hh
test_guitar.o: include/toad/matrix2d.hh include/toad/textarea.hh
test_guitar.o: include/toad/core.hh include/toad/bitmap.hh
test_guitar.o: include/toad/control.hh include/toad/textmodel.hh
test_guitar.o: include/toad/undo.hh include/toad/scrollbar.hh
test_guitar.o: include/toad/integermodel.hh include/toad/numbermodel.hh
test_guitar.o: include/toad/pushbutton.hh include/toad/buttonbase.hh
test_guitar.o: include/toad/labelowner.hh include/toad/table.hh
test_guitar.o: include/toad/scrollpane.hh include/toad/dragndrop.hh
test_vector_buffer.o: include/toad/vector.hh include/toad/penbase.hh
test_vector_buffer.o: include/toad/color.hh include/toad/types.hh
test_vector_buffer.o: include/toad/io/serializable.hh
test_vector_buffer.o: include/toad/io/atvparser.hh include/toad/font.hh
test_vector_buffer.o: include/toad/pointer.hh include/toad/matrix2d.hh
test_vector_buffer.o: include/toad/window.hh include/toad/interactor.hh
test_vector_buffer.o: include/toad/connect.hh include/toad/cursor.hh
test_vector_buffer.o: include/toad/region.hh include/toad/bitmap.hh
test_vector_buffer.o: include/toad/figuremodel.hh include/toad/model.hh
test_vector_buffer.o: include/toad/figure.hh include/toad/wordprocessor.hh
test_vector_buffer.o: include/toad/pen.hh
test_path_offset.o: include/toad/window.hh include/toad/interactor.hh
test_path_offset.o: include/toad/types.hh include/toad/io/serializable.hh
test_path_offset.o: include/toad/io/atvparser.hh include/toad/connect.hh
test_path_offset.o: include/toad/cursor.hh include/toad/color.hh
test_path_offset.o: include/toad/region.hh include/toad/pen.hh
test_path_offset.o: include/toad/penbase.hh include/toad/font.hh
test_path_offset.o: include/toad/pointer.hh include/toad/matrix2d.hh
test_path_offset.o: include/toad/figure.hh include/toad/bitmap.hh
test_path_offset.o: include/toad/figuremodel.hh include/toad/model.hh
test_path_offset.o: include/toad/wordprocessor.hh include/toad/vector.hh
test_path_offset.o: include/toad/geometry.hh
test_toolbar.o: include/toad/window.hh include/toad/interactor.hh
test_toolbar.o: include/toad/types.hh include/toad/io/serializable.hh
test_toolbar.o: include/toad/io/atvparser.hh include/toad/connect.hh
test_toolbar.o: include/toad/cursor.hh include/toad/color.hh
test_toolbar.o: include/toad/region.hh include/toad/pen.hh
test_toolbar.o: include/toad/penbase.hh include/toad/font.hh
test_toolbar.o: include/toad/pointer.hh include/toad/matrix2d.hh
test_toolbar.o: include/toad/springlayout.hh include/toad/layout.hh
test_toolbar.o: include/toad/eventfilter.hh include/toad/menubar.hh
test_toolbar.o: include/toad/menuhelper.hh include/toad/action.hh
test_toolbar.o: include/toad/model.hh include/toad/command.hh
test_toolbar.o: include/toad/pushbutton.hh include/toad/buttonbase.hh
test_toolbar.o: include/toad/labelowner.hh include/toad/control.hh
test_toolbar.o: include/toad/focusmanager.hh include/toad/figure/toolbox.hh
test_toolbar.o: include/toad/figure/toolpanel.hh include/toad/figureeditor.hh
test_toolbar.o: include/toad/figure.hh include/toad/bitmap.hh
test_toolbar.o: include/toad/figuremodel.hh include/toad/wordprocessor.hh
test_toolbar.o: include/toad/scrollpane.hh include/toad/undo.hh
test_toolbar.o: include/toad/boolmodel.hh include/toad/textmodel.hh
test_toolbar.o: include/toad/integermodel.hh include/toad/numbermodel.hh
test_toolbar.o: include/toad/floatmodel.hh include/toad/undomanager.hh
test_toolbar.o: include/toad/figure/selectiontool.hh
test_toolbar.o: include/toad/figuretool.hh include/toad/core.hh
test_toolbar.o: include/toad/figure/nodetool.hh
test_toolbar.o: include/toad/figure/shapetool.hh
test_toolbar.o: include/toad/figure/texttool.hh
booleanop.o: include/toad/booleanop.hh include/toad/types.hh
booleanop.o: include/toad/io/serializable.hh include/toad/io/atvparser.hh
booleanop.o: include/toad/pen.hh include/toad/penbase.hh
booleanop.o: include/toad/color.hh include/toad/font.hh
booleanop.o: include/toad/pointer.hh include/toad/matrix2d.hh
booleanop.o: include/toad/geometry.hh include/toad/vector.hh
booleanop.o: include/toad/window.hh include/toad/interactor.hh
booleanop.o: include/toad/connect.hh include/toad/cursor.hh
booleanop.o: include/toad/region.hh include/toad/bitmap.hh
booleanop.o: include/toad/figuremodel.hh include/toad/model.hh
bop12/booleanop.o: bop12/booleanop.h bop12/polygon.h bop12/utilities.h
bop12/booleanop.o: bop12/point_2.h bop12/bbox_2.h bop12/segment_2.h
bop12/polygon.o: bop12/polygon.h bop12/utilities.h bop12/point_2.h
bop12/polygon.o: bop12/bbox_2.h bop12/segment_2.h
bop12/utilities.o: bop12/utilities.h bop12/point_2.h bop12/bbox_2.h
bop12/utilities.o: bop12/segment_2.h
window.o: include/toad/core.hh include/toad/window.hh
window.o: include/toad/interactor.hh include/toad/types.hh
window.o: include/toad/io/serializable.hh include/toad/io/atvparser.hh
window.o: include/toad/connect.hh include/toad/cursor.hh
window.o: include/toad/color.hh include/toad/region.hh include/toad/pen.hh
window.o: include/toad/penbase.hh include/toad/font.hh
window.o: include/toad/pointer.hh include/toad/matrix2d.hh
window.o: include/toad/bitmap.hh include/toad/layout.hh
window.o: include/toad/eventfilter.hh include/toad/focusmanager.hh
window.o: include/toad/io/urlstream.hh include/toad/command.hh
window.o: include/toad/stacktrace.hh cocoa/toadwindow.h cocoa/toadview.h
window.o: cocoa/toadview.impl
mouseevent.o: include/toad/window.hh include/toad/interactor.hh
mouseevent.o: include/toad/types.hh include/toad/io/serializable.hh
mouseevent.o: include/toad/io/atvparser.hh include/toad/connect.hh
mouseevent.o: include/toad/cursor.hh include/toad/color.hh
mouseevent.o: include/toad/region.hh include/toad/layout.hh
mouseevent.o: include/toad/eventfilter.hh cocoa/toadwindow.h cocoa/toadview.h
pen.o: include/toad/core.hh include/toad/window.hh include/toad/interactor.hh
pen.o: include/toad/types.hh include/toad/io/serializable.hh
pen.o: include/toad/io/atvparser.hh include/toad/connect.hh
pen.o: include/toad/cursor.hh include/toad/color.hh include/toad/region.hh
pen.o: include/toad/pen.hh include/toad/penbase.hh include/toad/font.hh
pen.o: include/toad/pointer.hh include/toad/matrix2d.hh
pen.o: include/toad/bitmap.hh
