/*
 * Fischland -- A 2D vector graphics editor
 * Copyright (C) 1999-2015 by Mark-André Hopf <mhopf@mark13.org>
 * Visit http://www.mark13.org/fischland/.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "fischland.hh"
#include "fischeditor.hh"
#include "fishbox.hh"
#include "colorpalette.hh"
#include "lineal.hh"
#include "page.hh"
#include "config.h"
#include "connectfigure.hh"


#ifdef HAVE_LIBCAIRO
#include "cairo.hh"
#endif

#include <toad/core.hh>
#include <toad/menubar.hh>
#include <toad/undomanager.hh>
#include <toad/action.hh>
#include <toad/command.hh>
#include <toad/fatradiobutton.hh>
#include <toad/pushbutton.hh>
#include <toad/figure.hh>
#include <toad/figureeditor.hh>
#include <toad/colorselector.hh>
#include <toad/filedialog.hh>
#include <toad/combobox.hh>
#include <toad/arrowbutton.hh>
#include <toad/exception.hh>
#include <toad/dnd/color.hh>
#include <toad/dialog.hh>
#include <toad/textfield.hh>
#include <toad/fatcheckbutton.hh>
#include <toad/htmlview.hh>
#include <toad/popupmenu.hh>
#include <toad/messagebox.hh>
#include <toad/springlayout.hh>

#include <fstream>
#include <vector>
#include <stdlib.h>
#include <time.h>
#include <fcntl.h>

#include "rotatetool.hh"

// basename on MacOS X
#include <libgen.h>

using namespace toad;
using namespace fischland;

string programname("Fischland");

string datadir("/usr/local/share");
string resourcename("fischland");
string version("snapshot");

/**
 * 
 *
 */
string
resource(const string &filename)
{
  char *home = getenv("HOME");
  char cwd[4096];
  getcwd(cwd, sizeof(cwd)-1);
  string path;

  for(int i=0; i<3; ++i) {
    switch(i) {
      case 0:
        if (home) {
          path = home;
          path += "/.";
          path += resourcename;
        }
        break;
      case 1:
        path = datadir;
        path += "/";
        path += resourcename;
        break;
      case 2:
        path = cwd;
        path += "/resource";
        break;
      default:
        path.clear();
    }
    path += "/";
    path += filename;
    if (!path.empty()) {
      int fd = open(path.c_str(), O_RDONLY);
      if (fd>=0) {
        close(fd);
        return path;
      }
    }
  }
  return path;
}

// i've put TCollection into it's own namespace so it can be
// included in other programs too
namespace fischland {

class TPage:
  public TSerializable
{
  SERIALIZABLE_INTERFACE(fischland::, TPage);
  
  public:
    TPage() {}
    TPage(const string &name, TFigureModel *model) {
      this->name = name;
      this->model = model;
    }
  
//  protected:
    PFigureModel model;
    TTextModel name;
    TTextModel description;
};

/**
 * This models holds the various figure models which ____ Fischlands
 * image. (Layer's aren't part of this yet, sorry.)
 */
class TCollection:
  public TModel, public TSerializable
{
  SERIALIZABLE_INTERFACE(fischland::, TCollection);

  public:
    void append(TFigureModel *model) {
      storage.push_back(new TPage("unnamed page", model));
    }

//  protected:
    typedef vector<TPage*> TStorage;
    TStorage storage;
    string author;
    string date;
    string description;
};

typedef GSmartPointer<TCollection> PCollection;

} // namespace fischland

using namespace fischland;

void
TPage::store(TOutObjectStream &out) const
{
  ::store(out, "name", name);
  ::store(out, "description", description);
  ::store(out, model);
}

bool
TPage::restore(TInObjectStream &in)
{
  if (in.what == ATV_START) {
    model = 0;
//    name.clear();
//    description.clear();
    return true;
  }
  if (in.what == ATV_GROUP && in.type == "toad::TFigureModel") {
    model = new TFigureModel();
    in.setInterpreter(model);
    return true;
  }
  if (
    ::restore(in, "name", &name) ||
    ::restore(in, "description", &description) ||
    finished(in)
  ) return true;
  ATV_FAILED(in)
  return false;
}

void
TCollection::store(TOutObjectStream &out) const
{
  ::store(out, "author", author);
  ::store(out, "date", date);
  ::store(out, "description", description);
  for(TStorage::const_iterator p = storage.begin();
      p != storage.end();
      ++p)
  {
    ::store(out, *p);
  }
}

bool
TCollection::restore(TInObjectStream &in)
{
  if (in.what == ATV_GROUP && in.type == "fischland::TPage") {
    TPage *page = new TPage();
    storage.push_back(page);
    in.setInterpreter(page);
    return true;
  }
  if (
    ::restore(in, "author", &author) ||
    ::restore(in, "date", &date) ||
    ::restore(in, "description", &description) ||
    finished(in)
  ) return true;
  ATV_FAILED(in)
  return false;
}

class TMainWindow:
  public TWindow
{
    typedef TWindow super;
    typedef TMainWindow This;
    string filename;
    PEditModel editmodel;
    TFischEditor *editor;
    TSingleSelectionModel currentPage;
    
    bool _check();
    bool _save(const string &title);

  public:
    TMainWindow(TWindow *parent, const string &title, TEditModel *m=0);
    ~TMainWindow();
    
    void load(const string &filename);
    
    void menuNew();
    void menuNewView();
    void menuClose();
    
    void menuOpen();
    bool menuSave();
    bool menuSaveAs();
    void menuPrint();
    void menuPrint2Clipboard();
    void menuAbout();
    void menuCopyright();
    
    void menuSlides();
    void menuLayers();

    void closeRequest();
    
    void changeZoom(TFigureEditor *gw, TComboBox*);
    
    void setEditModel(TEditModel*);
    TEditModel *getEditModel() const {
      return editmodel;
    }
    TSignal sigEditModel;
    
    TPushButton *page_add, *page_del, *page_up, *page_down, *page_edit;

    void pageAdd();
    void pageDelete();
    void pageUp();
    void pageDown();
    void pageEdit();
    void updatePageButtons();
    
    void _gotoPage(int page);

    TEditModel* newEditModel();
    void _fixLineWidth();
    void editorModified();
};

/**
 * kludge for old files using a line width of 0 to 12 instead of 96-12*96
 */
static void
_fixLineWidth(TFigureModel::iterator b, TFigureModel::iterator e)
{
  for(TFigureModel::iterator p1 = b; p1 != e; ++p1) {
    TFGroup *g = dynamic_cast<TFGroup*>(*p1);
    if (g) {
      _fixLineWidth(g->gadgets.begin(), g->gadgets.end());
      continue;
    }
    TAttributedFigure *cf = dynamic_cast<TAttributedFigure*>(*p1);
    if (cf) {
      TFigureAttributes a;
      cf->getAttributes(&a);
      if (a.linewidth<15) {
        if (a.linewidth==0.0)
          a.linewidth=0.5;
        a.reason.linewidth = true;
        cf->setAttributes(&a);
      }
    }
  }
}

void
TMainWindow::_fixLineWidth()
{
#if 1
  cout << __PRETTY_FUNCTION__ << " is disabled\n";
#else
  if (!collection)
    return;
    
  for(vector<TPage*>::iterator p0 = collection->storage.begin();
      p0 != collection->storage.end();
      ++p0)
  {
    ::_fixLineWidth((*p0)->model->begin(), (*p0)->model->end());
  }    
#endif
}

void
TMainWindow::_gotoPage(int page)
{
#if 1
  cout << "TMainWindow::_gotoPage(" << page << ") is disabled\n";
#else
  TFigureModel *m = 0;
  if (!collection->storage.empty())
    m = collection->storage[/*page*/currentPage.getRow()]->model;
  editor->setModel(m);
  updatePageButtons();
#endif
}

void
TMainWindow::pageAdd()
{
#if 1
  cout << "TMainWindow::pageAdd() is disabled\n";
#else
  TPage *page = new TPage("new page", new TFigureModel());
  int n = currentPage.getRow() + 1;
  TCollection::TStorage::iterator p = collection->storage.begin() + n;
  collection->storage.insert(p, page);
  sigCollection();
  currentPage.setSelection(0, n);
  updatePageButtons();
  pageEdit();
#endif
}

void
TMainWindow::pageDelete()
{
#if 1
  cout << "TMainWindow::pageDelete() is disabled\n";
#else
  if (collection->storage.size()<=1)
    return;
  int n = currentPage.getRow();
  TCollection::TStorage::iterator p = collection->storage.begin() + n;
  collection->storage.erase(p);
  sigCollection();
  if (n>=collection->storage.size())
    n = collection->storage.size()-1;
  currentPage.setSelection(0, n);
  updatePageButtons();
#endif
}

void
TMainWindow::pageUp()
{
#if 1
  cout << "TMainWindow::pageUp() is disabled\n";
#else
  if (collection->storage.size()<=1)
    return;
  int n = currentPage.getRow();
  if (n<=0)
    return;
  TPage *memo = collection->storage[n];
  collection->storage[n] = collection->storage[n-1];
  collection->storage[n-1] = memo;
  sigCollection();
  currentPage.setSelection(0, n-1);
  updatePageButtons();
#endif
}

void
TMainWindow::pageDown()
{
#if 1
  cout << "TMainWindow::pageDown() is disabled\n";
#else
  if (collection->storage.size()<=1)
    return;
  int n = currentPage.getRow();
  if (n>=collection->storage.size()-1)
    return;
  TPage *memo = collection->storage[n];
  collection->storage[n] = collection->storage[n+1];
  collection->storage[n+1] = memo;
  sigCollection();
  currentPage.setSelection(0, n+1);
  updatePageButtons();
#endif
}

void
TMainWindow::pageEdit()
{
#if 1
  cout << "TMainWindow::pageEdit() is disabled\n";
#else
  TDialog *dlg = new TDialog(this, "Edit Page");
  new TTextField(dlg, "page name", 
                 &collection->storage[currentPage.getRow()]->name);
  new TTextArea(dlg, "page description",
                &collection->storage[currentPage.getRow()]->description);
  connect((new TPushButton(dlg, "ok"))->sigClicked, dlg, &TDialog::closeRequest);
  dlg->loadLayout(RESOURCE("editpage.atv"));
  dlg->doModalLoop();
  delete dlg;
#endif
}

void
TMainWindow::updatePageButtons()
{
#if 1
  cout << "TMainWindow::updatePageButtons() is disabled\n";
#else
  page_del->setEnabled(collection->storage.size()>1);
  page_up->setEnabled(currentPage.getRow()>0);
// cerr << "current page = " << currentPage.getRow() << endl;
  page_down->setEnabled(currentPage.getRow()<collection->storage.size()-1);
#endif
}

#if 0
class TCollectionRenderer:
  public TSimpleTableAdapter
{
    int width, height;
    TMainWindow *wnd;
  public:
    TCollectionRenderer(TMainWindow *w) {
      wnd = w;
      TFont &font(TOADBase::getDefaultFont());
      height = font.getHeight();
      width = 0;
      adjust();
      connect(wnd->sigCollection, this, &TCollectionRenderer::adjust);
    }
    
    ~TCollectionRenderer() {
      disconnect(wnd->sigCollection, this);
    }
    
    void adjust()
    {
      TFont &font(TOADBase::getDefaultFont());
      width = 0;
      if (!wnd->getCollection()) {
        sigChanged();
        return;
      }
      for(TCollection::TStorage::iterator p = wnd->getCollection()->storage.begin();
          p != wnd->getCollection()->storage.end();
          ++p)
      {
        int w = font.getTextWidth( (*p)->name );
        if (w>width)
          width = w;
      }
      sigChanged();
    }
    size_t getRows() { 
      return wnd->getCollection() ?
        wnd->getCollection()->storage.size() :
        0; 
    }
    int getRowHeight(size_t row) { return height; }
    int getColWidth(size_t col) { return width; }
    void renderItem(TPen &pen, const TTableEvent &te)
    {
      if (!wnd->getCollection()) {
        cerr << "hey? TTable/TComboBox tried to render non-existent collection in row " << te.row << endl;
        return;
      }
      if (wnd->getCollection()->storage.size()<=te.row) {
        cerr << "hey? TTable/TComboBox tried to render non-existent row " << te.row << endl;
        return;
      }
      pen.drawString(0,0,wnd->getCollection()->storage[te.row]->name);
    }
};
#endif

void
TMainWindow::menuNew()
{
  if (!_check())
    return;
    
  filename.erase();
  setTitle(programname);
  setEditModel(newEditModel());
}

void
TMainWindow::menuNewView()
{
  TMainWindow *mw = new TMainWindow(NULL, programname, editmodel);
  mw->placeWindow(PLACE_PARENT_RANDOM, this);
  mw->setTitle(programname+ ": " + basename((char*)filename.c_str()));
  mw->filename = filename;
  mw->createWindow();
}

bool
TMainWindow::_check()
{
//  editor->getTool()->stop(editor); // FIXME: editor should do that on it's own when it's loosing focus.
  if (editor->isModified() && editmodel->_toad_ref_cntr<=2) {
    unsigned r = messageBox(NULL,
      "Figure is modified",
      "Do you want to save the current file?",
      TMessageBox::ICON_QUESTION |
      TMessageBox::YES | TMessageBox::NO );
    if (r==TMessageBox::YES) {
      if (!menuSave())
        return false;
    } else if (r!=TMessageBox::NO) {
      return false;
    }
  }
  return true;
}

void
TMainWindow::menuClose()
{
  
}

void
TMainWindow::menuOpen()
{
  if (!_check())
    return;

  TFileDialog dlg(this, "Open..");
  dlg.addFileFilter("Fischland (*.atv, *.vec, *.fish)");
  // dlg.addFileFilter("Scaleable Vector Graphics (*.svg)");
cerr << "start modal loop" << endl;
  dlg.doModalLoop();
cerr << "left modal loop" << endl;
  if (dlg.getResult()!=TMessageBox::OK)
    return;
  load(dlg.getFilename());
}

void
TMainWindow::load(const string &filename)
{    
  ifstream fin(filename.c_str());
  TInObjectStream in(&fin);
//  in.setVerbose(true);
//  in.setDebug(true);

  TSerializable *s = in.restore();
  if (!in || !s) {
    string msg =
      programname + " failed to load '" + filename + "'\n\n" +
      in.getErrorText();
      messageBox(0, 
               "Failed to load file",
               msg,
               TMessageBox::ICON_STOP | TMessageBox::OK);
    return;
  }
  in.close();
  TFigureEditor::restoreRelations(); // FIXME: TFigureEditor should be able to register this in TInObjectStream

  TFigureModel *figuremodel;
  TCollection *collection;
  TDocument *document;
  string msg;

  figuremodel = dynamic_cast<TFigureModel *>(s);
  if (figuremodel) {
    cout << "found old TFigureModel object" << endl;
    goto done;
  }

  collection = dynamic_cast<TCollection*>(s);
  if (collection) {
    cout << "found old TCollection object" << endl;
    document = new TDocument();
    TSlide *slide = 0;
    for(TCollection::TStorage::iterator p = collection->storage.begin();
        p != collection->storage.end();
        ++p)
    {
      if (!slide) {
        slide = new TSlide;
        document->content.setRoot(slide);
      } else {
        slide->next = new TSlide;
        slide = slide->next;
      }
      TLayer *layer = new TLayer;
      slide->content.setRoot(layer);
      slide->name = (*p)->name;
      slide->comment = (*p)->description;
      for(TFigureModel::iterator q = (*p)->model->begin();
          q != (*p)->model->end();
          ++q)
      {
        layer->content.add(*q);
      }
      (*p)->model->drop();
      slide->content.update();
    }
    document->content.update();
    editmodel->setDocument(document);
    goto done;
  }
  
  document = dynamic_cast<TDocument*>(s);
  if (document) {
    cout << "found document!" << endl;
    editmodel->setDocument(document);
    editor->clearFischModified();
    goto done;
  }

  msg = programname + " can't load objects of type '" +
        s->getClassName() + "'.\n\n"
        "I've expected either 'fischland::TDocument', 'fischland::TCollection' or "
        "'toad::TFigureModel'.";
  delete s;
  messageBox(0, "Failed to load file", msg,
             TMessageBox::ICON_STOP | TMessageBox::OK);
  return;

done:
  _fixLineWidth();

  this->filename = filename;
  setTitle(programname+ ": " + basename((char*)filename.c_str()));
}

void
TMainWindow::setEditModel(TEditModel *e)
{
//cerr << "enter TMainWindow::setCollection" << endl;
//cerr << "  remove model from editor" << endl;
//  editor->setModel(0);
//cerr << "  set collection object" << endl;
  editmodel = e;
  editor->setEditModel(e);
//cerr << "  goto page 0" << endl;
//  currentPage.setSelection(0,0);
//  _gotoPage(0);
//cerr << "  trigger sigCollection" << endl;
//  sigCollection();
//  updatePageButtons();
//cerr << "leave TMainWindow::setCollection" << endl;
}

bool
TMainWindow::menuSave()
{
  if (filename.empty()) {
    return menuSaveAs();
  }
  _save("Save");
  return true;
}

bool
TMainWindow::menuSaveAs()
{
  TFileDialog dlg(this, "Save As..", TFileDialog::MODE_SAVE);
  dlg.addFileFilter("Fischland (*.atv, *.vec, *.fish)");
  dlg.setFilename(filename);
  dlg.doModalLoop();
  if (dlg.getResult()==TMessageBox::OK) {
    filename = dlg.getFilename();
    if (_save("Save As..")) {
      setTitle(programname+ ": " + basename((char*)filename.c_str()));
      return true;
    }
  }
  return false;
}

bool
TMainWindow::_save(const string &title)
{
  ofstream out(filename.c_str());
  if (!out) {
    messageBox(NULL,
               title,
               "Damn! I've failed to create the file.",
               TMessageBox::ICON_EXCLAMATION | TMessageBox::OK);
    return false;
  }
  out << "// fish -- a Fischland 2D Vector Graphics file" << endl
      << "// Please see http://www.mark13.org/fischland/ for more details." << endl;
  TOutObjectStream oout(&out);
  oout.store(editmodel->document);
  editor->clearFischModified();
  return true;
}

void
printLayer(TPenBase &pen, TLayer *layer)
{
  while(layer) {
    if (layer->print) {
      for(TFigureModel::iterator p = layer->content.begin();
          p != layer->content.end();
          ++p)
      {
        (*p)->paint(pen, TFigure::NORMAL);
      }
    }
    printLayer(pen, layer->down);
    layer = layer->next;
  }
}

void
printSlide(TPenBase &pen, TSlide *slide)
{
  while(slide) {
    if (slide->print) {
      printLayer(pen, slide->content.getRoot());
      pen.showPage();
    }
    printSlide(pen, slide->down);
    slide = slide->next;
  }
}

TBoundary
boundsOfLayer(TLayer *layer)
{
  TBoundary b;
  while(layer) {
    if (layer->print) {
      for(TFigureModel::iterator p = layer->content.begin();
          p != layer->content.end();
          ++p)
      {
        b.expand((*p)->bounds());
      }
    }
    b.expand(boundsOfLayer(layer->down));
    layer = layer->next;
  }
  return b;
}

TBoundary
boundsOfSlide(TSlide *slide)
{
  TBoundary b;
  while(slide) {
    if (slide->print) {
      b.expand(boundsOfLayer(slide->content.getRoot()));
    }
    b.expand(boundsOfSlide(slide->down));
    slide = slide->next;
  }
  return b;
}

void
TMainWindow::menuPrint2Clipboard()
{
  // either selection or current page?
  TPen pen;
cout << "initClipboard" << endl;
  pen.initClipboard(boundsOfSlide(editmodel->document->content.getRoot()));
cout << "inited ClipBoad" << endl;

  // paint all active layers
  for(auto figuremodel: editmodel->modelpath) {
      for(auto figure: *figuremodel) {
          figure->paint(pen, TFigure::NORMAL);
      }
  }


//    print(pen, p, true);
  // printSlide(pen, editmodel->document->content.getRoot());
}

void
TMainWindow::menuPrint()
{
  TPen pen("output.pdf"/*, boundsOfSlide(editmodel->document->content.getRoot())*/);
  // all pages
  printSlide(pen, editmodel->document->content.getRoot());
}

void
TMainWindow::closeRequest()
{
  if (_check())
    sendMessageDeleteWindow(this);
}

static PBitmap bmp_vlogo;

void
TMainWindow::menuAbout()
{
  THTMLView *htmlview = new THTMLView(0, "Fischland -- About");
  TPopupMenu *menu = new TPopupMenu(htmlview, "popupmenu");
  menu->addFilter();
  htmlview->open(RESOURCE("index.html"));
  htmlview->createWindow();
}

void
TMainWindow::menuCopyright()
{
  messageBox(
    this, 
    "Fischland -- Copyright",
    "Fischland alpha -- A 2D vector graphics editor\n"
    "\n"
    "Copyright (C) 2003-2017 by Mark-André Hopf <mhopf@mark13.org>\n"
    "Visit http://www.mark13.org/fischland/.\n"
    "\n"
    "Pencil curve smoothing code was taken from Xara LX,\n"
    "Copyright (C) 1993-2006 Xara Group Ltd. and written by Rik Heywood.\n"
    "\n"
    "This program is free software; you can redistribute it and/or modify "
    "it under the terms of the GNU General Public License as published by "
    "the Free Software Foundation; either version 2 of the License, or "
    "(at your option) any later version.\n"
    "\n"
    "This program is distributed in the hope that it will be useful, "
    "but WITHOUT ANY WARRANTY; without even the implied warranty of "
    "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the "
    "GNU General Public License for more details.\n"
    "\n"
    "You should have received a copy of the GNU General Public License "
    "along with this program; if not, write to the Free Software "
    "Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA",
    TMessageBox::OK,
    bmp_vlogo);
}

// extern void createMemoryFiles();
struct TZoom
{
  const char *text;
  double factor;
  const char * toText(unsigned) const { return text; }
};

TZoom zoom[13] = {
  { "12.5%", 0.125 },
  { "25%",  0.25 },
  { "50%",  0.50 },
  { "75%",  0.75 },
  { "100%", 1.0 },
  { "150%", 1.5 },
  { "200%", 2.0 },
  { "300%", 3.0 },
  { "600%", 6.0 },
  { "1200%", 12.0 },
  { "2400%", 24.0 },
  { "4800%", 48.0 },
  { "9600%", 1.0 }
};

class TZoomAdapter:
  public TSimpleTableAdapter
{
  public:
    size_t getRows() { return 13; }
    void tableEvent(TTableEvent &te) {
      switch(te.type) {
        case TTableEvent::GET_COL_SIZE:
          te.w = getDefaultFont().getTextWidth("9600%");
          break;
        case TTableEvent::GET_ROW_SIZE:
          te.h = getDefaultFont().getHeight();
          break;
        case TTableEvent::PAINT:
          renderBackground(te);
          te.pen->drawString(0,0,zoom[te.row].text);
          renderCursor(te);
          break;
        default:;
      }
    }
};

static void
gotoPage(TMainWindow *wnd, TComboBox *cb)
{
  // wnd->_gotoPage(cb->getLastSelectionRow());
}

void
TMainWindow::editorModified()
{
  if (editor->isFischModified()) {
    setTitle(programname+ ": *" + basename((char*)filename.c_str()));
  } else {
    setTitle(programname+ ": " + basename((char*)filename.c_str()));
  }
}

TMainWindow::TMainWindow(TWindow *p, const string &t, TEditModel *e):
  super(p, t)
{
  new TUndoManager(this, "undomanager");

  TFischEditor *me = new TFischEditor(this, "figureeditor");


  CONNECT(me->sigFischModified, this, editorModified);
  editor = me;
  me->setAttributes(TFishBox::preferences);
  me->setBackground(1,1,1);
  me->setRowHeaderRenderer(new TLineal(true));
  me->setColHeaderRenderer(new TLineal(false));
  me->setFont("arial,helvetica,sans-serif:size=12");

  TMenuBar *mb = new TMenuBar(this, "menubar");
  mb->loadLayout(RESOURCE("menubar.atv"));

  TAction *a;
  a = new TAction(this, "file|new");
  CONNECT(a->sigClicked, this, menuNew);
  a = new TAction(this, "file|newview");
  CONNECT(a->sigClicked, this, menuNewView);

  a = new TAction(this, "file|open");
  CONNECT(a->sigClicked, this, menuOpen);
  a = new TAction(this, "file|save");
  CONNECT(a->sigClicked, this, menuSave);
  a = new TAction(this, "file|save_as");
  CONNECT(a->sigClicked, this, menuSaveAs);
  a = new TAction(this, "file|print");
  CONNECT(a->sigClicked, this, menuPrint);
  a = new TAction(this, "file|print2clipboard");
  CONNECT(a->sigClicked, this, menuPrint2Clipboard);
  a = new TAction(this, "file|close");
  CONNECT(a->sigClicked, this, menuClose);
  a = new TAction(this, "file|quit");
  CONNECT(a->sigClicked, this, closeRequest);
  
  a = new TAction(this, "edit|insert_image");
  connect(a->sigClicked, [me] {
    TFImage *img = new TFImage();
    img->startInPlace();
    me->addFigure(img);
  });
  
  a = new TAction(this, "object|transform|perspective");
  connect(a->sigClicked, [me] {
    if (me->getModel() && !me->selection.empty()) {
      cout << "perspective" << endl;
      me->getModel()->transform(me->selection, new TFPerspectiveTransform);
    }
  });

  a = new TAction(this, "slide|slides");
  CONNECT(a->sigClicked, this, menuSlides);

  a = new TAction(this, "slide|layers");
  CONNECT(a->sigClicked, this, menuLayers);

  a = new TAction(this, "help|manual");
  CONNECT(a->sigClicked, this, menuAbout);
  a = new TAction(this, "help|about");
  CONNECT(a->sigClicked, this, menuCopyright);
  
  setBackground(TColor::DIALOG);
  setSize(640,480);

  TComboBox *zoom;
  zoom = new TComboBox(this, "zoom");
  zoom->setSize(64, -1);
  zoom->setAdapter(new TZoomAdapter);
  connect(zoom->sigSelection, this, &This::changeZoom, me, zoom);
  zoom->setCursor(0,4);
//  zoom->setCursor(0,12);
  zoom->clickAtCursor();

  TComboBox *page;
  page = new TComboBox(this, "page");
  page->setSelectionModel(&currentPage);
  connect(currentPage.sigChanged, gotoPage, this, page);
  page->setSize(150, -1);
#if 0
  page->setAdapter(new TCollectionRenderer(this));
#endif
  me->setGrid(4);
  
  int h = page->getHeight();

  page_add  = new TPushButton(this, "page add");
  page_add->setLabel("+");
  page_add->setToolTip("insert a new page after this one");
  page_add->setSize(h,h);
  CONNECT(page_add->sigClicked, this, pageAdd);

  page_del  = new TPushButton(this, "page delete");
  page_del->setLabel("-");
  page_del->setToolTip("delete this page");
  page_del->setSize(h,h);
  CONNECT(page_del->sigClicked, this, pageDelete);

  page_up   = new TPushButton(this, "page up");
  page_up->setLabel("u");
  page_up->setToolTip("move this page up");
  page_up->setSize(h,h);
  CONNECT(page_up->sigClicked, this, pageUp);

  page_down = new TPushButton(this, "page down");
  page_down->setLabel("d");
  page_down->setToolTip("move this page down");
  page_down->setSize(h,h);
  CONNECT(page_down->sigClicked, this, pageDown);

  page_edit = new TPushButton(this, "page edit");
  page_edit->setLabel("e");
  page_edit->setToolTip("edit page name");
  page_edit->setSize(h,h);
  CONNECT(page_edit->sigClicked, this, pageEdit);

  if (!e)
    e = newEditModel();
    
  setEditModel(e);
  
  loadLayout(RESOURCE("TMainWindow.atv"));
}

TMainWindow::~TMainWindow()
{
}

TEditModel*
TMainWindow::newEditModel()
{
  TEditModel *e = new TEditModel;
  TDocument *doc = new TDocument();
  doc->content.addBelow(0);            // add a slide
  doc->content[0].content.addBelow(0); // add a layer
  e->setDocument(doc);
  e->slide.select(0,0);
  e->layer.select(0,0);
  e->figuremodel = &doc->content[0].content[0].content;
  return e;
}

void
TMainWindow::changeZoom(TFigureEditor *fe, TComboBox *cb)
{
  unsigned i = cb->getSelectionModel()->getRow();
  fe->identity();
  fe->scale(zoom[i].factor, zoom[i].factor);
}

void
TMainWindow::menuSlides()
{
  if (!editmodel) {
    cout << "no edit model" << endl;
    return;
  }
  TPageDialog *dlg = new TPageDialog(this, "Slides", editmodel, true);
  dlg->createWindow();
}

void
TMainWindow::menuLayers()
{
  if (!editmodel) {
    cout << "no edit model" << endl;
    return;
  }
  TPageDialog *dlg = new TPageDialog(this, "Layers", editmodel, false);
  dlg->createWindow();
}

TCursor *fischland::cursor[16];

void test_combobox();
void test_timer();
void test_table();
void test_scroll();
void test_dialog();
void test_cursor();
void test_colordialog();
void test_grab();
void test_path();
void test_image();
void test_curve();
void test_text();
void test_toolbar();
void test_tablet();
void test_path_bool();
void test_guitar();
void test_vector_buffer();
void test_path_offset();

int 
main(int argc, char **argv, char **envv)
{
  if (argc>=2) {
    if (strcmp(argv[1], "--test-combobox")==0) {
      toad::initialize(argc, argv);
      test_combobox();
      toad::terminate();
      return 0;
    } else
    if (strcmp(argv[1], "--test-timer")==0) {
      toad::initialize(argc, argv);
      test_timer();
      toad::terminate();
      return 0;
    } else
    if (strcmp(argv[1], "--test-table")==0) {
      toad::initialize(argc, argv);
      test_table();
      toad::terminate();
      return 0;
    } else
    if (strcmp(argv[1], "--test-scroll")==0) {
      toad::initialize(argc, argv);
      test_scroll();
      toad::terminate();
      return 0;
    } else
    if (strcmp(argv[1], "--test-dialog")==0) {
      toad::initialize(argc, argv);
      test_dialog();
      toad::terminate();
      return 0;
    }
    if (strcmp(argv[1], "--test-cursor")==0) {
      toad::initialize(argc, argv);
      test_cursor();
      toad::terminate();
      return 0;
    }
    if (strcmp(argv[1], "--test-colordialog")==0) {
      toad::initialize(argc, argv);
      test_colordialog();
      toad::terminate();
      return 0;
    }
    if (strcmp(argv[1], "--test-grab")==0) {
      toad::initialize(argc, argv);
      test_grab();
      toad::terminate();
      return 0;
    }
    if (strcmp(argv[1], "--test-path")==0) {
      toad::initialize(argc, argv);
      test_path();
      toad::terminate();
      return 0;
    }
    if (strcmp(argv[1], "--test-image")==0) {
      toad::initialize(argc, argv);
      test_image();
      toad::terminate();
      return 0;
    }
    if (strcmp(argv[1], "--test-curve")==0) {
      toad::initialize(argc, argv);
      test_curve();
      toad::terminate();
      return 0;
    }
    if (strcmp(argv[1], "--test-text")==0) {
      toad::initialize(1, argv);
      test_text();
      toad::terminate();
      return 0;
    }
    if (strcmp(argv[1], "--test-toolbar")==0) {
      toad::initialize(1, argv);
      test_toolbar();
      toad::terminate();
      return 0;
    }
    if (strcmp(argv[1], "--test-tablet")==0) {
      toad::initialize(argc, argv);
      test_tablet();
      toad::terminate();
      return 0;
    }
    if (strcmp(argv[1], "--test-path-bool")==0) {
      toad::initialize(1, argv);
      test_path_bool();
      toad::terminate();
      return 0;
    }
    if (strcmp(argv[1], "--test-guitar")==0) {
      toad::initialize(argc, argv);
      test_guitar();
      toad::terminate();
      return 0;
    }
    if (strcmp(argv[1], "--test-vector-buffer")==0) {
      toad::initialize(argc, argv);
      test_vector_buffer();
      toad::terminate();
      return 0;
    }
    if (strcmp(argv[1], "--test-path-offset")==0) {
      toad::initialize(argc, argv);
      test_path_offset();
      toad::terminate();
      return 0;
    }
  }

  toad::initialize(argc, argv);

//    createMemoryFiles();
  toad::getDefaultStore().registerObject(new TPage());
  toad::getDefaultStore().registerObject(new TCollection());

  toad::getDefaultStore().registerObject(new TDocument());
  toad::getDefaultStore().registerObject(new TSlide());
  toad::getDefaultStore().registerObject(new TLayer());

  toad::getDefaultStore().registerObject(new TFConnection());

  bmp_vlogo = new TBitmap();
  bmp_vlogo->load(RESOURCE("logo_vertical.jpg"));

  static const char bm[10][32][32+1] = {
    {
    // CURSOR_PEN
    // 0        1         2         3
    // 12345678901234567890123456789012
      "    .                           ",
      "    .                           ",
      "   ...                          ",
      "   ...                          ",
      "  .#.#.                         ",
      "  .#.#.                         ",
      " .##.##.                        ",
      " .##.##.                        ",
      ".###.###.                       ",
      ".#######.                       ",
      ".###.###.                       ",
      ".#######.                       ",
      " .#####.                        ",
      ".........                       ",
      ".........                       ",
      "#.......#                       ",
      "#.......#                       ",
      "#.......#                       ",
      "#.......#                       ",
      "#.......#                       ",
      "#.......#                       ",
      " #######                        ",
      "                                ",
      "                                ",
      "                                ",
      "                                ",
      "                                ",
      "                                ",
      "                                ",
      "                                ",
      "                                ",
      "                                "
    },{
    // CURSOR_PEN_CLOSE
    // 0        1         2         3
    // 12345678901234567890123456789012
      "    .                           ",
      "    .                           ",
      "   ...                          ",
      "   ...                          ",
      "  .#.#.                         ",
      "  .#.#.                         ",
      " .##.##.                        ",
      " .##.##.                        ",
      ".###.###.                       ",
      ".#######.                       ",
      ".###.###.                       ",
      ".#######.                       ",
      " .#####.                        ",
      ".........                       ",
      ".........                       ",
      "#.......#                       ",
      "#.......#                       ",
      "#.......#   ...                 ",
      "#.......#  .###.                ",
      "#.......# .#   #.               ",
      "#.......# .#   #.               ",
      " #######  .#   #.               ",
      "           .###.                ",
      "            ...                 ",
      "                                ",
      "                                ",
      "                                ",
      "                                ",
      "                                ",
      "                                ",
      "                                ",
      "                                "
    },{
    // CURSOR_PEN_EDGE
    // 0        1         2         3
    // 12345678901234567890123456789012
      "    .                           ",
      "    .                           ",
      "   ...                          ",
      "   ...                          ",
      "  .#.#.                         ",
      "  .#.#.                         ",
      " .##.##.                        ",
      " .##.##.                        ",
      ".###.###.                       ",
      ".#######.                       ",
      ".###.###.                       ",
      ".#######.                       ",
      " .#####.                        ",
      ".........                       ",
      ".........                       ",
      "#.......#                       ",
      "#.......# #                     ",
      "#.......##.#                    ",
      "#.......##..#                   ",
      "#.......##.#.#                  ",
      "#.......##.##.#                 ",
      "##########.# #.#                ",
      "         #.#  #                 ",
      "         #.#                    ",
      "          #                     ",
      "                                ",
      "                                ",
      "                                ",
      "                                ",
      "                                ",
      "                                ",
      "                                "
    },{
    // CURSOR_PEN_DRAG
    // 0        1         2         3
    // 12345678901234567890123456789012
      " #                              ",
      "#.#                             ",
      "#..#                            ",
      "#...#                           ",
      "#....#                          ",
      "#.....#                         ",
      "#......#                        ",
      "#.......#                       ",
      "#........#                      ",
      "#.........#                     ",
      "#....#######                    ",
      "#...#                           ",
      "#..#                            ",
      "#.#                             ",
      "##                              ",
      "#                               ",
      "                                ",
      "                                ",
      "                                ",
      "                                ",
      "                                ",
      "                                ",
      "                                ",
      "                                ",
      "                                ",
      "                                ",
      "                                ",
      "                                ",
      "                                ",
      "                                ",
      "                                ",
      "                                "
    },{
    // CURSOR_PENCIL_CLOSE
    // 0        1         2         3
    // 12345678901234567890123456789012
      "                ####            ",
      "               #....#           ",
      "              #.####.#          ",
      "             #..#####.#         ",
      "            #....####.#         ",
      "           #.##......#          ",
      "          #.####....#           ",
      "         #.#######.#            ",
      "        #.#######.#             ",
      "       #.#######.#              ",
      "      #.#######.#               ",
      "     #.#######.#                ",
      "    #.#######.#                 ",
      "   #.#######.#                  ",
      "  #.#######.#    ...            ",
      " #.#######.#    .###.           ",
      "#..######.#    .#   #.          ",
      "#.#..###.#     .#   #.          ",
      "#.###...#      .#   #.          ",
      "#..###.#        .###.           ",
      "#.....#          ...            ",
      "######                          ",
      "                                ",
      "                                ",
      "                                ",
      "                                ",
      "                                ",
      "                                ",
      "                                ",
      "                                ",
      "                                ",
      "                                "
    },{
    // CURSOR_PENCIL
    // 0        1         2         3
    // 12345678901234567890123456789012
      "                ####            ",
      "               #....#           ",
      "              #.####.#          ",
      "             #..#####.#         ",
      "            #....####.#         ",
      "           #.##......#          ",
      "          #.####....#           ",
      "         #.#######.#            ",
      "        #.#######.#             ",
      "       #.#######.#              ",
      "      #.#######.#               ",
      "     #.#######.#                ",
      "    #.#######.#                 ",
      "   #.#######.#                  ",
      "  #.#######.#                   ",
      " #.#######.#                    ",
      "#..######.#                     ",
      "#.#..###.#                      ",
      "#.###...#                       ",
      "#..###.#                        ",
      "#.....#                         ",
      "######                          ",
      "                                ",
      "                                ",
      "                                ",
      "                                ",
      "                                ",
      "                                ",
      "                                ",
      "                                ",
      "                                ",
      "                                "
    }
  };
  cursor[0] = new TCursor(bm[0], 4, 0);
  cursor[1] = new TCursor(bm[1], 4, 0);
  cursor[2] = new TCursor(bm[2], 4, 0);
  cursor[3] = new TCursor(bm[3], 0, 0);
  cursor[4] = new TCursor(bm[4], 0, 21);
  cursor[5] = new TCursor(bm[5], 0, 21);

  new TFishBox(0, "Tools");
//  if (argc==1) {
  TMainWindow *wnd =  new TMainWindow(0, programname);
  wnd->load("a.fish");
  TFigureEditor::restoreRelations(); // FIXME: TFigureEditor should be able to register this in TInObjectStream
/*
  } else {
    for(int i=1; i<argc; ++i) {
      TMainWindow *wnd = new TMainWindow(0, programname);
      wnd->load(argv[i]);
    }
  }
*/
  toad::mainLoop();
  bmp_vlogo = 0;
  toad::terminate();
  return 0;
}
