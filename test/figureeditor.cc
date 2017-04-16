/*
 *
 */

#include "util.hh"
#include "gtest.h"

#include <toad/core.hh>
#include <toad/window.hh>
#include <toad/pen.hh>
#include <toad/simpletimer.hh>
#include <toad/figureeditor.hh>

#include <iostream>

using namespace toad;
using namespace std;

namespace {

class FigureEditor:
  public ::testing::Test
{
  protected:
    static void SetUpTestCase() {
      toad::initialize(0, NULL);
    }
    
    static void TearDownTestCase() {
      toad::terminate();
    }
};

ostringstream out;

TEST_F(FigureEditor, SwitchingToANewToolStopsTheCurrentTool)
{
  class TMyFigureTool:
    public TFigureTool
  {
      string name;
    public:
      TMyFigureTool(const string &aName): name(aName) {}
    protected:
      void stop(TFigureEditor *fe) override {
cout << "tool '" << name << "' stopped for editor " << fe << " '" << fe->getTitle() << "'" << endl;
        out << "tool '" << name << "' stopped" << endl;
      }
  };

  TFigureEditor *fe0 = new TFigureEditor(nullptr, "fe0");
    
  TToolBox *tb = new TToolBox();
  
  TFigureTool
    *tool1 = new TMyFigureTool("tool1"),
    *tool2 = new TMyFigureTool("tool2");
    
  tb->add("t1", tool1);
  tb->add("t2", tool2);
  
  PFigureModel m0 = new TFigureModel();
    
  fe0->setToolBox(tb);
  fe0->setModel(m0);

  out.str("");
cout << "++++++++++++++ switch to tool2" << endl;
  tb->setValue(tool2);
cout << "-------------- switched to tool2" << endl;
  EXPECT_EQ("tool 'tool1' stopped\n", out.str());

//  out.str("");
//  tb->setValue(tool1);
//  EXPECT_EQ("tool 'tool2' stopped\n", out.str());
}

#if 0
TEST_F(FigureEditor, SwitchingToANewModelStopsTheCurrentTool)
{
  class TMyFigureTool:
    public TFigureTool
  {
      string name;
    public:
      TMyFigureTool(const string &aName): name(aName) {}
    protected:
      void stop(TFigureEditor *fe) override {
        out << "tool '" << name << "' stopped" << endl;
      }
  };

  TFigureEditor *fe0 = new TFigureEditor(nullptr, "fe0");
    
  TToolBox *tb = new TToolBox();
  
  TFigureTool
    *tool1 = new TMyFigureTool("tool1"),
    *tool2 = new TMyFigureTool("tool2");
    
  tb->add("t1", tool1);
  tb->add("t2", tool2);
  
  PFigureModel
    m0 = new TFigureModel(),
    m1 = new TFigureModel();
    
  fe0->setToolBox(tb);
  fe0->setModel(m0);

  out.str("");
  fe0->setModel(m1);
  EXPECT_EQ("tool 'tool1' stopped\n", out.str());

  out.str("");
  fe0->setModel(m0);
  EXPECT_EQ("tool 'tool1' stopped\n", out.str());
}
#endif

// TFigureTool stop must be called when: a new tool is choosen or the model
// changes (either in different or same editor)
//

// a figure being edited is locked for other users. we could extend this
// condition to 'locked for other tools' and represent other users as tools
// within the figure editor?

} // namespace