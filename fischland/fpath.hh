#include <toad/figure.hh>
#include <toad/boolmodel.hh>
#include <toad/integermodel.hh>

#include <toad/figureeditor.hh>

using namespace toad;

class TFPath:
  public TColoredFigure
{
    SERIALIZABLE_INTERFACE(toad::, TFPath);
  public:
    TFPath() {
      closed = false;
    }
    void paint(TPenBase&, EPaintType);
    void paintSelection(TPenBase &pen, int handle);
    void getShape(toad::TRectangle *r);

    void translate(int dx, int dy);
    bool getHandle(unsigned handle, TPoint *p);
    void translateHandle(unsigned handle, TCoord x, TCoord y, unsigned modifier);
    double _distance(TFigureEditor *fe, int x, int y);
    unsigned mouseRDown(TFigureEditor*, TMouseEvent &);
    
    void addPoint(const TPoint &p) { polygon.addPoint(p); }
    void addPoint(int x, int y) { polygon.addPoint(x,y); }
    void insertPointNear(int x, int y);
    void deletePoint(unsigned i);

    bool closed;
    TPolygon polygon;
    vector<byte> corner;
};

class TPenTool:
  public TFigureTool
{
    TFPath *path;
    bool down;
  public:
    TPenTool() {
      down = false;
      path = 0;
    }
    static TPenTool* getTool();
    void cursor(TFigureEditor *fe, int x, int y);
    void mouseEvent(TFigureEditor *fe, TMouseEvent &me);
    void keyEvent(TFigureEditor *fe, TKeyEvent &ke);
    void paintSelection(TFigureEditor *fe, TPenBase &pen);
    void stop(TFigureEditor*);
};

class TPencilTool:
  public TFigureTool
{
    TPolygon polygon;
    bool closed;
    
    TIntegerModel fidelity;
    TIntegerModel smoothness;
    TBoolModel fillNewStrokes;
    TBoolModel keepSelected; // true by default!
    TBoolModel editSelectedPaths;
    TIntegerModel withinPixels;
    
  public:
    static TPencilTool* getTool();
    void mouseEvent(TFigureEditor *fe, TMouseEvent &me);
    void keyEvent(TFigureEditor *fe, TKeyEvent &ke);
    void stop(TFigureEditor*);
};
