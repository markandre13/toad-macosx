/*
 * Fischland -- A 2D vector graphics editor
 * Copyright (C) 1999-2006 by Mark-André Hopf <mhopf@mark13.org>
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
#include "toolbox.hh"
#include "colorpalette.hh"
#include "lineal.hh"
#include "page.hh"
#include "config.h"

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
    TColoredFigure *cf = dynamic_cast<TColoredFigure*>(*p1);
    if (cf) {
      TFigureAttributes a;
      cf->getAttributes(&a);
      if (a.linewidth<15) {
        if (a.linewidth==0.0)
          a.linewidth=0.5;
        a.reason = TFigureAttributes::LINEWIDTH;
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
        if ((*p)->mat) {
          pen.push();   
          pen.multiply( (*p)->mat );
          (*p)->paint(pen, TFigure::NORMAL);
          pen.pop();
        } else {
          (*p)->paint(pen, TFigure::NORMAL);
        }
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

void
TMainWindow::menuPrint()
{
/*
  class W: 
    public TWindow
  {
  public:
    PEditModel editmodel;
    W(TWindow *p, const string &t, TEditModel *m): TWindow(p, t) {editmodel=m;}
    void paint() {
  // current page
*/
#if 1
  TPen pen("output.pdf");

  // all pages
  printSlide(pen, editmodel->document->content.getRoot());
#endif
/*
    }
  };
  
  TWindow *w = new W(0, "print", editmodel);
  w->createWindow();
*/
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
    "Fischland v0.6 -- A 2D vector graphics editor\n"
    "\n"
    "Copyright (C) 2003-2005 by Mark-André Hopf <mhopf@mark13.org>\n"
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
  me->setAttributes(TToolBox::preferences);
  me->setBackground(1,1,1);
  me->setRowHeaderRenderer(new TLineal(true));
  me->setColHeaderRenderer(new TLineal(false));
  me->setFont("arial,helvetica,sans-serif:size=12");

  TMenuBar *mb = new TMenuBar(this, "menubar");
  mb->loadLayout(RESOURCE("menubar.atv"));

  TAction *a;
  a = new TAction(this, "file|new", TAbstractChoice::DOMAIN_FOCUS);
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
  a = new TAction(this, "file|close");
  CONNECT(a->sigClicked, this, menuClose);
  a = new TAction(this, "file|quit");
  CONNECT(a->sigClicked, this, closeRequest);
  
  a = new TAction(this, "edit|insert_image");
  TCLOSURE1(
    a->sigClicked,
    gw, me,
    TFImage *img = new TFImage();
    img->startInPlace();
    gw->addFigure(img);
  )

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

void
foo(){exit(0);}

#define TEST06 1

#ifdef TEST01

class TMyWindow:
  public TWindow
{
    TPushButton *btn;
    TWindow *wnd;
    bool open;
  public:
    TMyWindow(TWindow *p, const string &t): TWindow(p, t) {
      // setBackground(0,0,255);
      btn = new TPushButton(this, "Open");
      btn->setShape(100,15,100,25);
      CONNECT(btn->sigClicked, this, button);

      open = false;
      wnd = 0;
    }
    
    void paint() {
      cout << "TMyWindow::paint()" << endl;
      TPen pen(this);
      pen.drawLine(0,0,100,100);
    }
    
    void button() {
      cout << "button pressed" << endl;
      open = !open;
      btn->setTitle(open ? "Close" : "Open");
      if (open) {
        if (!wnd) {
          wnd = new TWindow(this, "popup");
          wnd->setBackground(255,0,0);
          wnd->flagPopup = true;
          wnd->placeWindow(PLACE_PULLDOWN, btn);
          
          TWindow *a = new TWindow(wnd, "button");
          a->setBackground(0,0,255);
          a->setShape(10, 40, 40, 70);
          
          wnd->createWindow();
          a->setPosition(5,5);
        } else {
          wnd->setMapped(true);
        }
      } else {
#if 1        
        delete wnd;
        wnd = 0;
#else
        wnd->setMapped(false);
#endif
      }
    }
    
};
#endif

#ifdef TEST06

class TMyWindow:
  public TWindow
{
  public:
    TMyWindow(TWindow *p, const string &t): TWindow(p, t) {
//      setBackground(0,0,0);
        setSize(640,480);
    }
    void paint() {
      TPen pen(this);
      pen.setColor(1,0,0);
      pen.drawRectangle(100.5,100.5,49,99);
      
//      CGContextClipToRect(pen.ctx, CGRectMake(101,101,48,98));
      pen.setClipRect(TRectangle(101,101,48,98));
      
      pen.setColor(0,0,1);
      pen.fillRectangle(50,50,100,200);
      
//      pen.drawString(20,20, "Ärgerlich");
#if 0      
      pen.identity();
      
      pen.setColor(1.0, 0.5, 0);
      pen.drawRectanglePC(0,0,100,50);

      pen.translate(100,50);

      pen.setColor(1,1,0);
      pen.drawRectanglePC(0,0,100,50);

      pen.rotate(2.0*M_PI / 360.0 * 20.0);

      pen.setColor(0.5,1,0.5);
      pen.drawRectanglePC(0,0,100,50);

      pen.rotate(2.0*M_PI / 360.0 * 20.0);

      pen.setColor(0,1,1);
      pen.drawRectanglePC(0,0,100,50);

      pen.push();

      pen.translate(100,50);

      pen.setColor(0,0.5,1);
      pen.drawRectanglePC(0,0,100,50);

      pen.rotate(2.0*M_PI / 360.0 * 20.0);

      pen.setColor(0,0,1);
      pen.drawRectanglePC(0,0,100,50);
      
      pen.pop();
      
      pen.setColor(1,1,1);
      pen.drawRectanglePC(5,5,90,40);
      
      const TMatrix2D *m = pen.getMatrix();
      pen.setMatrix(*m);
      
      pen.drawRectanglePC(10,10,80,30);
      
      pen.identity();
      
      pen.translate(0.5, 0.5);
      
      pen.setLineStyle(TPen::DASH);
      
      pen.drawRectanglePC(5,5, 90, 40);
      
      pen.setLineStyle(TPen::SOLID);
      pen.drawRectangle(10,10,80,30);

      pen.setFillColor(1,0.5,0);

      pen.fillRectangle(10,60,20,20);
      pen.drawRectangle(10,60,20,20);

      pen.fillCircle(35,60,20,20);
      pen.drawCircle(35,60,20,20);
      
      TPolygon p;
      p.addPoint(10,145);
      p.addPoint(10,85);
      p.addPoint(70,85);
      p.addPoint(80,145);
      
      pen.fillPolyBezier(p);
      pen.drawLines(p);
      
      pen.drawString(10,150, "Hä");
#endif
    }
};
#endif

void test_combobox();
void test_timer();
void test_table();
void test_scroll();
void test_dialog();
void test_cursor();
void test_colordialog();

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
  }

#if 0

#ifdef TEST01
  toad::initialize(argc, argv);
  TWindow *w0 = new TMyWindow(0, "TEST01");
  toad::mainLoop();
  toad::terminate();
#endif

#ifdef TEST02
  toad::initialize(argc, argv);
  TWindow *w = new TWindow(0, "combobox test");
  w->setBackground(0,0,255);
  
  TComboBox *cb = new TComboBox(w, "combobox");
  cb->setShape(10,10,300,20);
#if 0
  TTable *cb = new TTable(w, "table");
  cb->setShape(10,10,300,180);
#else
  cb->setAdapter(new TZoomAdapter);
  cb->getSelectionModel()->select(0,0);
#endif
  toad::mainLoop();
  toad::terminate();
#endif

#if TEST03
  // springlayout
  toad::initialize(argc, argv); {
    TWindow *wnd = new TWindow(NULL,"form example");
    TWindow *btn;
    btn = new TMenuBar(wnd, "menubar");
    TAction *a;
    a = new TAction(wnd, "file|open");
    a = new TAction(wnd, "file|save");
    a = new TAction(wnd, "file|save as..");
    a = new TAction(wnd, "file|quit");
    connect(a->sigClicked, foo);
    btn = new TPushButton(wnd, "toolbar");
    btn = new TPushButton(wnd, "clientarea");
    btn = new TPushButton(wnd, "statusbar");
    TScrollBar *sb;
    sb = new TScrollBar(wnd, "horizontal");
    sb = new TScrollBar(wnd, "vertical");
    TSpringLayout *form = new TSpringLayout;
    form->attach("menubar", TSpringLayout::TOP|TSpringLayout::LEFT|TSpringLayout::RIGHT);
    form->attach("toolbar", TSpringLayout::LEFT|TSpringLayout::RIGHT);
    form->attach("toolbar", TSpringLayout::TOP, "menubar");

    form->attach("vertical", TSpringLayout::LEFT);
    form->attach("vertical", TSpringLayout::TOP, "menubar");
    form->attach("vertical", TSpringLayout::BOTTOM, "horizontal");

    form->attach("horizontal", TSpringLayout::LEFT, "vertical");
    form->attach("horizontal", TSpringLayout::RIGHT);
    form->attach("horizontal", TSpringLayout::BOTTOM, "statusbar");

    form->attach("clientarea", TSpringLayout::LEFT, "vertical");
    form->attach("clientarea", TSpringLayout::RIGHT);
    form->attach("clientarea", TSpringLayout::TOP, "menubar");
    form->attach("clientarea", TSpringLayout::BOTTOM, "horizontal");

    form->attach("statusbar", TSpringLayout::LEFT|TSpringLayout::RIGHT|TSpringLayout::BOTTOM);

    wnd->setLayout(form);

    toad::mainLoop();
  } toad::terminate();
  return 0;
#endif

#ifdef TEST04
  // mouse test
  toad::initialize(argc, argv);
  class TMyWindow: public TWindow {
    public:
      TMyWindow(TWindow *w, const string &t):TWindow(w,t){
        // setAllMouseMoveEvents(true);
      }
      void paint(){}
      void mouseEvent(const TMouseEvent &me) {
        switch(me.type) {
          case TMouseEvent::MOVE:
            cout << "MOVE" << endl;
            break;
          case TMouseEvent::ENTER:
            cout << "ENTER" << endl;
            break;
          case TMouseEvent::LEAVE:
            cout << "LEAVE" << endl;
            break;
          case TMouseEvent::LDOWN:
            cout << "LDOWN" << endl;
            break;
          case TMouseEvent::MDOWN:
            cout << "MDOWN" << endl;
            break;
          case TMouseEvent::RDOWN:
            cout << "RDOWN" << endl;
            break;
          case TMouseEvent::LUP:
            cout << "LUP" << endl;
            break;
          case TMouseEvent::MUP:
            cout << "MUP" << endl;
            break;
          case TMouseEvent::RUP:
            cout << "RUP" << endl;
            break;
          case TMouseEvent::ROLL_UP:
            cout << "ROLL UP" << endl;
            break;
          case TMouseEvent::ROLL_DOWN:
            cout << "ROLL DOWN" << endl;
            break;
        }
        cout <<" "<<x<<", "<<y;
        unsigned m = me.modifier();
        if (m&MK_SHIFT)
          cout<<" MK_SHIFT";
        if (m&MK_CONTROL)
          cout<<" MK_CONTROL";
        if (m&MK_ALT)
          cout<<" MK_ALT";
        if (m&MK_ALTGR)
          cout<<" MK_ALTGR";
        if (m&MK_LBUTTON)
          cout<<" MK_LBUTTON";
        if (m&MK_MBUTTON)
          cout<<" MK_MBUTTON";
        if (m&MK_RBUTTON)
          cout<<" MK_RBUTTON";
        if (m&MK_DOUBLE)
          cout<<" MK_DOUBLE";
        cout << endl;
      }
  };
  TMyWindow wnd(0, "test 0");
  toad::mainLoop();
#endif

#ifdef TEST05
  toad::initialize(argc, argv);
  TWindow *w0 = new TTextArea(0, "TEST05");
  toad::mainLoop();
  toad::terminate();
#endif

#ifdef TEST06
  toad::initialize(argc, argv);
  TWindow *w0 = new TMyWindow(0, "TEST06");
  toad::mainLoop();
  toad::terminate();
#endif

#else
  toad::initialize(argc, argv);

//    createMemoryFiles();
  toad::getDefaultStore().registerObject(new TPage());
  toad::getDefaultStore().registerObject(new TCollection());

  toad::getDefaultStore().registerObject(new TDocument());
  toad::getDefaultStore().registerObject(new TSlide());
  toad::getDefaultStore().registerObject(new TLayer());

  bmp_vlogo = new TBitmap();
  bmp_vlogo->load(RESOURCE("logo_vertical.jpg"));

  static const char bm[6][32][32+1] = {
    {
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

  new TToolBox(0, programname);
//  if (argc==1) {
    new TMainWindow(0, programname);
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
#endif
}
