#include <toad/wordprocessor.hh>
#include <toad/utf8.hh>

#include "gtest.h"

using namespace toad;
using namespace toad::wordprocessor;

TEST(WordProcessor, xmlinc)
{
  size_t x;
  //            0         1         2         3
  //            0123456789012345678901234567890
  x= 5; xmlinc("Move forward folks.", &x);
  ASSERT_EQ(6, x);
  
  // entity
  x= 4; xmlinc("Move &times; folks.", &x);
  ASSERT_EQ(5, x);

  x= 5; xmlinc("Move &times; folks.", &x);
  ASSERT_EQ(12, x);

  // self-closing tag
  x= 4; xmlinc("Move <br/> folks.", &x);
  ASSERT_EQ(5, x);

  x= 5; xmlinc("Move <br/> folks.", &x);
  ASSERT_EQ(10, x);

  // opening tag
  x= 4; xmlinc("Move <i>forward</i> folks.", &x);
  ASSERT_EQ(5, x);

  x= 5; xmlinc("Move <i>forward</i> folks.", &x);
  ASSERT_EQ(9, x);

  // closing tag
  x=14; xmlinc("Move <i>forward</i> folks.", &x);
  ASSERT_EQ(15, x);

  x=15; xmlinc("Move <i>forward</i> folks.", &x);
  ASSERT_EQ(20, x);
  
  // nested opening tags
  x= 5; xmlinc("Move <i><u>forward</u></i> folks.", &x);
  ASSERT_EQ(12, x);

  x= 5; xmlinc("Move <i><u><b>forward</b></u></i> folks.", &x);
  ASSERT_EQ(15, x);
  
  // combinations
  x= 5; xmlinc("Move <br/><i>forward</i> folks.", &x);
  ASSERT_EQ(10, x);
  
  x= 5; xmlinc("Move <i><br/></i> folks.", &x);
  ASSERT_EQ(13, x);
  
  x= 5; xmlinc("Move &times;<i>to</i> it.", &x);
  ASSERT_EQ(12, x);
}

TEST(WordProcessor, xmldec)
{
  size_t x;
  //            0         1         2         3
  //            0123456789012345678901234567890
  x= 6; xmldec("Move forward folks.", &x);
  ASSERT_EQ(5, x);
  
  // entity
  x=13; xmldec("Move &times; folks.", &x);
  ASSERT_EQ(12, x);

  x=12; xmldec("Move &times; folks.", &x);
  ASSERT_EQ(5, x);

  // self-closing tag
  x=11; xmldec("Move <br/> folks.", &x);
  ASSERT_EQ(10, x);

  x=10; xmldec("Move <br/> folks.", &x);
  ASSERT_EQ(5, x);

  // opening tag
  x=10; xmldec("Move <i>forward</i> folks.", &x);
  ASSERT_EQ(9, x);

  x= 9; xmldec("Move <i>forward</i> folks.", &x);
  ASSERT_EQ(5, x);

  // closing tag
  x=21; xmldec("Move <i>forward</i> folks.", &x);
  ASSERT_EQ(20, x);

  x=20; xmldec("Move <i>forward</i> folks.", &x);
  ASSERT_EQ(15, x);

  // nested opening tags
  x=12; xmldec("Move <i><u>forward</u></i> folks.", &x);
  ASSERT_EQ(5, x);

  x=15; xmldec("Move <i><u><b>forward</b></u></i> folks.", &x);
  ASSERT_EQ(5, x);
  
  // combinations
  x=10; xmldec("Move <br/><i>forward</i> folks.", &x);
  ASSERT_EQ(5, x);
  
  x=13; xmldec("Move <i><br/></i> folks.", &x);
  ASSERT_EQ(5, x);
  
  x=12; xmldec("Move &times;<i>to</i> it.", &x);
  ASSERT_EQ(5, x);
  
  // semicolon not belonging to entity
  x=10; xmldec("a &amp; b;", &x);
  ASSERT_EQ(9, x);
  
  x=5; xmldec("what;", &x);
  ASSERT_EQ(4, x);
  
  x=11; xmldec("<br/> when;", &x);
  ASSERT_EQ(10, x);
}

TEST(WordProcessor, isadd)
{
  ASSERT_EQ(isadd("<b>abc</b>defg", "b", 0,  4), false);
  //               ^   <
  ASSERT_EQ(isadd("a<b>bcd</b>efg", "b", 0,  5), true );
  //               ^    <
  ASSERT_EQ(isadd("<b>abc</b>defg", "b", 0,  6), false);
  //               ^     <
  ASSERT_EQ(isadd("<b>abc</b>defg", "b", 0,  9), false);
  //               ^        <
  ASSERT_EQ(isadd("<b>abc</b>defg", "b", 0, 10), false);
  //               ^         <
  ASSERT_EQ(isadd("<b>abc</b>defg", "b", 0, 11), true );
  //               ^          <
  ASSERT_EQ(isadd("abcdefg", "b", 4,6), true);
  //                   ^ <
  ASSERT_EQ(isadd("abcdefg", "b", 5,6), true);
  //                    ^<
  ASSERT_EQ(isadd("ab<b>cde</b>fg", "b", 2,7), false);
  //                 ^   <
  ASSERT_EQ(isadd("ab<b>cde</b>fg", "b", 2,9), false);
  //                 ^     <
  ASSERT_EQ(isadd("ab<b>cde</b>fg", "b", 2,10), false);
  //                 ^      <
  ASSERT_EQ(isadd("ab<b>cde</b>fg", "b", 2,12), false);
  //                 ^         <
  ASSERT_EQ(isadd("ab<b>cde</b>fg", "b", 2,13), true);
  //                 ^          <
  ASSERT_EQ(isadd("ab<b>cde</b>fg", "b", 2,14), true);
  //                 ^           <
  ASSERT_EQ(isadd("ab&times;cd", "b", 2,9), true);
  //                 ^      <
}

TEST(WordProcessor, tagtoggle)
{
  struct test {
    const char *in;	// input
    size_t bgn, end;	// selection
    const char *out;	// input after adding <b> to selection
    string marker_in;
    string marker_out;
  };

  static vector<test> test = {
    // tail remove
    { // 0         1         2         3         4         5
      // 012345678901234567890123456789012345678901234567890
        "<b><i>TFigure<br/></i>asd</b>",
      //                        ^ <
                               23,25,
        "<b><i>TFigure<br/></i>a</b>sd"
    },
    { // 0         1         2         3         4         5
      // 012345678901234567890123456789012345678901234567890
        "<b><i>TFigure<br/></i>asd</b>",
      //                       ^  <
                              22,25,
        "<b><i>TFigure<br/></i></b>asd"
    },

    // head remove
    { // 0         1         2         3         4         5
      // 012345678901234567890123456789012345678901234567890
        "<b>asd<i>TFigure<br/></i></b>",
      // ^    <
         0,   5,
        "as<b>d<i>TFigure<br/></i></b>"
    },
    { // 0         1         2         3         4         5
      // 012345678901234567890123456789012345678901234567890
        "<b>asd<i>TFigure<br/></i></b>",
      // ^     <
         0,    6,
        "asd<b><i>TFigure<br/></i></b>"
    },

    // touch at head
    { // 0         1         2         3         4
      // 0123456789012345678901234567890123456789012
        "Hello <b>this totally</b> really awesome.",
      // ^    <
         0,   5,
        "<b>Hello</b> <b>this totally</b> really awesome."
    },
    { // 0         1         2         3         4
      // 0123456789012345678901234567890123456789012
        "Hello <b>this totally</b> really awesome.",
      // ^     <
         0,    6,
        "<b>Hello this totally</b> really awesome."
    },
#if 0
    // ignore: end of selection is inside a tag
    { // 0         1         2         3         4
      // 0123456789012345678901234567890123456789012
        "Hello <b>this totally</b> really awesome.",
      // ^       <
         0,      8,
        "<b>Hello this totally</b> really awesome."
    },
#endif
    { // 0         1         2         3         4
      // 0123456789012345678901234567890123456789012
        "Hello <b>this totally</b> really awesome.",
      // ^        <
         0,       9,
        "<b>Hello this totally</b> really awesome."
    },
    { // 0         1         2         3         4
      // 0123456789012345678901234567890123456789012
        "Hello <b>this totally</b> really awesome.",
      // ^         <
         0,        10,
        "<b>Hello this totally</b> really awesome."
    },
    // touch at tail
    { // 0         1         2         3         4
      // 0123456789012345678901234567890123456789012
        "Hello <b>this totally</b> really awesome.",
      //                    ^<
                            19,20,
        "Hello <b>this total</b>l<b>y</b> really awesome."
    },
    { // 0         1         2         3         4
      // 0123456789012345678901234567890123456789012
        "Hello <b>this totally</b> really awesome.",
      //                    ^     <
                            19,   25,
        "Hello <b>this total</b>ly really awesome."
    },
    { // 0         1         2         3         4
      // 0123456789012345678901234567890123456789012
        "Hello <b>this totally</b> really awesome.",
      //                     ^    <
                            20,   25,
        "Hello <b>this totall</b>y really awesome."
    },
    { // 0         1         2         3         4
      // 0123456789012345678901234567890123456789012
        "Hello <b>this totally</b> really awesome.",
      //                      ^    <
                              21,  26,
        "Hello <b>this totally </b>really awesome."
    },
    
    // ...
    { // 0         1         2         3         4
      // 0123456789012345678901234567890123456789012
        "Hello <b>this totally</b> really awesome.",
      //               ^                 <
                       14,               32,
        "Hello <b>this totally really</b> awesome."
    },
    { // 0         1         2         3         4
      // 0123456789012345678901234567890123456789012
        "Hello <b>this totally really</b> awesome.",
      //               ^      <
                       14,    21,
        "Hello <b>this </b>totally<b> really</b> awesome."
    },

    // interlace

// FIXME: needs more tests, also: how does it behave when we throw <b> into the mix?

    // enter tag
    { // 0         1         2         3         4         5
      // 012345678901234567890123456789012345678901234567890
        "Hello <i>this totally really</i> awesome.",			// +1
      // ^            <
         0,           13,
        "<b>Hello </b><i><b>this</b> totally really</i> awesome."
    },
    { // 0         1         2         3         4         5
      // 012345678901234567890123456789012345678901234567890
        "Hello <i>this <u>is totally</u> really</i> awesome.",		// +2
      // ^                  <
         0,                 19,
        "<b>Hello </b><i><b>this </b><u><b>is</b> totally</u> really</i> awesome."
    },
    { // 0         1         2         3         4         5
      // 012345678901234567890123456789012345678901234567890
        "Hello <i>this <u>is</u> totally really</i> awesome.",		// +1
      // ^                              <
         0,                             31,
        "<b>Hello </b><i><b>this <u>is</u> totally</b> really</i> awesome."
    },
    { // 0         1         2         3         4         5
      // 012345678901234567890123456789012345678901234567890
        "<b>Hello <i>this is <u>not totally</u> really</i> awesome.</b>",
      //                  ^         <
                          17,       27,
        "<b>Hello </b><i><b>this </b>is <u>not <b>totally</b></u><b> really</b></i><b> awesome.</b>"
    },
    { // 0         1         2         3         4         5         6         7         8         9
      // 0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890
        "<b>Hello </b><i><b>this </b>is <u>not <b>totally</b></u><b> really</b></i><b> awesome.</b>",
      // ^                                                                                         <
         0,                                                                                        90,
        "<b>Hello <i>this is <u>not totally</u> really</i> awesome.</b>"
    },
    { // 0         1         2         3         4         5         6         7         8         9
      // 0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789
        "<b>Hello </b><i><b>this </b>is <u>not <b>totally</b></u><b> really</b></i><b> awesome.</b>",
      //                     ^                       <
                             20,                     44,
        "<b>Hello <i>this is <u>not totally</u> really</i> awesome.</b>"
    },

    // leave tag
    { // 0         1         2         3         4         5
      // 012345678901234567890123456789012345678901234567890
        "Hello <i>this is totally really</i> awesome.",
      //                          ^                 <
                                  25,               43,
        "Hello <i>this is totally <b>really</b></i><b> awesome</b>."	// -1
    },
    { // 0         1         2         3         4         5
      // 012345678901234567890123456789012345678901234567890
        "Hello <i>this <u>is totally</u> really</i> awesome.",		// -2
      //                     ^                             <
                             20,                           50,
        "Hello <i>this <u>is <b>totally</b></u><b> really</b></i><b> awesome</b>."
    },
    { // 0         1         2         3         4         5
      // 012345678901234567890123456789012345678901234567890
        "Hello <i>this is <u>totally</u> really</i> awesome.",		// -1
      //               ^                                   <
                       14,                                 50,
        "Hello <i>this <b>is <u>totally</u> really</b></i><b> awesome</b>."
    },
    { // 0         1         2         3         4         5
      // 012345678901234567890123456789012345678901234567890
        "<b>Hello <i>this <u>is not</u> totally really</i> awesome.</b>",
      //                        ^              <
                                23,            38,
        "<b>Hello </b><i><b>this </b><u><b>is </b>not</u> totally<b> really</b></i><b> awesome.</b>"
    },
    // add tag
    { // 0         1         2         3         4         5
      // 012345678901234567890123456789012345678901234567890
        "This is a bold move.",
      //           ^   <
                   10, 14,
        "This is a <b>bold</b> move.",
        "abcdefghijklmnopqrst",
        "abcdefghijk   lmno    pqrst",
    },
    { // 0         1         2         3         4         5
      // 012345678901234567890123456789012345678901234567890
        "This <i>is</i> a bold move.",
      //                  ^   <
                          17, 21,
        "This <i>is</i> a <b>bold</b> move.",
        "abcdef   gh    ijklmnopqrst",
        "abcdef   gh    ijk   lmno    pqrst",
        
    },
    { // 0         1         2         3         4         5
      // 012345678901234567890123456789012345678901234567890
        "This is<br/>a bold move.",
      //               ^   <
                       14, 18,
        "This is<br/>a <b>bold</b> move.",
        "abcdefgh    ijklmnopqrst",
        "abcdefgh    ijk   lmno    pqrst",
    },

    // drop tag
    { // 0         1         2         3         4         5
      // 012345678901234567890123456789012345678901234567890
        "This was a <b>bold</b> move.",
      //            ^      <
                    11,    18,
        "This was a bold move."
    },
    { // 0         1         2         3         4         5
      // 012345678901234567890123456789012345678901234567890
        "This <i>was</i> a <b>bold</b> move.",
      //                   ^      <
                           18,    25,
        "This <i>was</i> a bold move."
    },

    { // 0         1         2         3         4         5
      // 012345678901234567890123456789012345678901234567890
        "hello<b>.....you</b> there.",
      //              ^  <
                      13,16,
        "hello<b>.....</b>you there."
    },
    { // 0         1         2         3         4         5
      // 012345678901234567890123456789012345678901234567890
        "hello<b><br/>you</b> there.",
      //              ^  <
                      13,16,
        "hello<b><br/></b>you there."
    },

    // with entities
    { // 0         1         2         3         4         5
      // 012345678901234567890123456789012345678901234567890
        "hello &times; you there.",
      //               ^  <
                       14,17,
        "hello &times; <b>you</b> there."
    },
    { // 0         1         2         3         4         5
      // 012345678901234567890123456789012345678901234567890
        "hello &times; you there.",
      //       ^      <
               6,     13,
        "hello <b>&times;</b> you there."
    },
    { // 0         1         2         3         4         5
      // 012345678901234567890123456789012345678901234567890
        "abc<br/><i>edf</i>",
      // ^       <
         0,      8,
        "<b>abc<br/></b><i>edf</i>"
    },
    { // 0         1         2         3         4         5
      // 012345678901234567890123456789012345678901234567890
        "abc<br/>&lt;<i>edf</i>",
      // ^       <
         0,      8,
        "<b>abc<br/></b>&lt;<i>edf</i>"
    },
    { // 0         1         2         3         4         5
      // 012345678901234567890123456789012345678901234567890
        "abc<br/>&lt;<i>edf</i>",
      // ^           <
         0,         12,
        "<b>abc<br/>&lt;</b><i>edf</i>"
    },
  };

  for(auto &t: test) {
    cout << "----------------------------------- " << t.bgn << ", " << t.end << endl;

    vector<size_t> xpos;
    xpos.assign(3, 0);
    xpos[SELECTION_BGN]=t.bgn;
    xpos[SELECTION_END]=t.end;
    
    map<char, size_t> sl;
    map<size_t, char> c;
cout << "marker in :";
    for(size_t i=0; i<t.marker_in.size(); ++i) {
      if (t.marker_in[i]!=' ') {
cout << " " << i;
        c[xpos.size()] = t.marker_in[i];
        sl[t.marker_in[i]] = xpos.size();
        xpos.push_back(i);
      }
    }
cout << endl;

    string out = tagtoggle(t.in, xpos, "b");
    ASSERT_EQ(t.out, out);
    
    string s;
cout << "marker out:";
    for(size_t i=3; i<xpos.size(); ++i) {
cout << " " << xpos[i];
      while(s.size()<=xpos[i])
        s+=' ';
//cout << "["<<xpos[i]<<"] = '" << c[i] << "'\n";
      s[xpos[i]] = c[i];
//cout << "'" << s << "'\n";
    }
cout << endl;

    ASSERT_EQ(s, t.marker_out);
    
/*
    for(size_t i=0; i<t.marker_out.size(); ++i) {
      if (t.marker_in[i]!=' ') {
        ASSERT_EQ(i, xpos[sl[t.marker_in[i]]]);
      }
    }
*/   
    
  }
}

TEST(WordProcessor, expandselection)
{
  size_t sb=0, se=8;
  expandselection("abc<br/><i>edf</i>", &sb, &se, "b");
  ASSERT_EQ(0, sb);
  ASSERT_EQ(8, se);
}


TEST(WordProcessor, prepareHTMLText)
{
  struct fragtest {
    size_t offset, length;
    TCoord y;
    bool bold, italics, size, eol;
  };
  struct test {
    const char *in;
    vector<fragtest> frags;
  };
  
  vector<test> test = {
    //       0         1         2         3         4         5
    //       012345678901234567890123456789012345678901234567890
    { .in = "Let me<br/>this to you.",
      .frags = {
        { .offset=0, .length=6, .eol=true },
        { .offset=11, .length=12,},
      }
    },
    //       0         1         2         3         4         5
    //       012345678901234567890123456789012345678901234567890
    { .in = "Let me<br/><br/>this to you.",
      .frags = {
        { .offset=0, .length=6, .eol=true },
        { .offset=11, .length=0, .eol=true },
        { .offset=16, .length=12,},
      }
    },
    //       0         1         2         3         4         5
    //       012345678901234567890123456789012345678901234567890
    { .in = "Break<br/><br/><br/>Dance",
      .frags = {
        { .offset= 0, .length=5, .eol=true },
        { .offset=10, .length=0, .eol=true },
        { .offset=15, .length=0, .eol=true },
        { .offset=20, .length=5 },
      },
    },
    //       0         1         2         3         4         5
    //       012345678901234567890123456789012345678901234567890
    { .in = "Let <b>me<br/>this</b> to you.",
      .frags = {
        { .offset= 0, .length=4 },
        { .offset= 7, .length=2, .bold=true, .eol=true },
        { .offset=14, .length=4, .bold=true },
        { .offset=22, .length=8 }
      }
    },
    //       0         1         2         3         4         5
    //       012345678901234567890123456789012345678901234567890
    { .in = "This w<i>as a </i><b><i>bo</i>ld</b> move.",
      .frags = {
        { .offset=0, .length=6 },
        { .offset=9, .length=5, .italics=true },
        { .offset=24, .length=2, .italics=true, .bold=true },
        { .offset=30, .length=2, .bold=true },
        { .offset=36, .length=6 },
      }
    },
    //       0         1         2         3         4         5
    //       012345678901234567890123456789012345678901234567890
    { .in = "Break<br/>",
      .frags = {
        { .offset= 0, .length=5, .eol=true },
        { .offset=10, .length=0 },
      },
    },
    //       0         1         2         3         4         5
    //       012345678901234567890123456789012345678901234567890
    { .in = "Hello<b><br/></b> there.",
      .frags = {
        { .offset= 0, .length=5, },
        { .offset= 8, .length=0, .bold=true, .eol=true },
        { .offset=13, .length=0, .bold=true },
        { .offset=17, .length=7 },
      },
    },
    //       0         1         2         3         4         5
    //       012345678901234567890123456789012345678901234567890
    { .in = "Say<br/><b>no</b> more.",
      .frags = {
        { .offset= 0, .length=3, .eol=true },
        { .offset= 8, .length=0, },
        { .offset=11, .length=2, .bold=true },
        { .offset=17, .length=6 },
      },
    },
    //       0         1         2         3         4         5
    //       012345678901234567890123456789012345678901234567890
    { .in = "",
      .frags = {
        { .offset= 0, .length=0 }
      },
    },
    //       0         1         2         3         4         5
    //       012345678901234567890123456789012345678901234567890
    { .in = "x<sup>2</sup>",
      .frags = {
        { .offset= 0, .length=1 },
        { .offset= 6, .length=1, .y=-2 },
        { .offset=13, .length=0 },
      },
    },
    //       0         1         2         3         4         5
    //       012345678901234567890123456789012345678901234567890
    { .in = "<br/>F",
      .frags = {
        { .offset= 0, .length=0, .eol=true },
        { .offset= 5, .length=1, },
      },
    },
    //       0         1         2         3         4         5
    //       012345678901234567890123456789012345678901234567890
    { .in = "prepare<br/>&lt;tag&gt;",
      .frags = {
        { .offset= 0, .length=7, .eol=true },
        { .offset=12, .length=4, },
        { .offset=16, .length=3, },
        { .offset=19, .length=4, },
        { .offset=23, .length=0, },
      },
    },
    //       0         1         2         3         4         5
    //       012345678901234567890123456789012345678901234567890
    { .in = "prepare&lt;<br/>tag&gt;",
      .frags = {
        { .offset= 0, .length=7,  },
        { .offset= 7, .length=4, },
        { .offset=11, .length=0, .eol=true },
        { .offset=16, .length=3, },
        { .offset=19, .length=4, },
        { .offset=23, .length=0, },
      },
    },
  };

  for(auto &t: test) {
    vector<size_t> xpos;
    TPreparedDocument document;
  
    xpos.assign(3, 0);

    prepareHTMLText(t.in, xpos, &document);

    cout << "----------------------------------------------------------------------" << endl
         << t.in << endl;
     
    auto f = t.frags.begin();
    for(auto &line: document.lines) {
      cout << "line:" << endl;
      ASSERT_NE(f, t.frags.end());
      for(auto &&fragment: line->fragments) {
        cout << "  fragment: offset=" << fragment->offset
             << ", length=" << fragment->length
             << ", y=" << fragment->origin.y
             << (fragment->attr.bold?", bold":"")
             << (fragment->attr.italic?", italics":"") << endl;
        ASSERT_EQ(fragment->offset,      f->offset);
        ASSERT_EQ(fragment->length,      f->length);
        ASSERT_EQ(fragment->attr.bold,   f->bold);
        ASSERT_EQ(fragment->attr.italic, f->italics);
        ASSERT_EQ(fragment->origin.y,    f->y);
        if ( fragment == line->fragments.back() && // end of line but...
             line != document.lines.back() )       // ...not end of document
        {
          ASSERT_TRUE(f->eol);
        }
        ++f;
      }
    }
    ASSERT_EQ(f, t.frags.end());
  }
}

TEST(WordProcessor, textDelete)
{
  struct fragtest {
    size_t offset;
    const char *txt;
    bool bold, italics, eol;
  };
  struct test {
    const char *in, *out;
    size_t pos, bgn, end;
    vector<fragtest> frags;
  };
  
  vector<test> test = {
    //       0         1         2         3         4         5
    //       012345678901234567890123456789012345678901234567890
    { .in = "What is seven &times; six?",
      .pos = 14,
      .frags = {
        { .offset=0,  .txt="What is seven  six?", },
      }
    },
    //       0         1         2         3         4         5
    //       012345678901234567890123456789012345678901234567890
    { .in = "This is a <i><b>bold</b></i> move.",
      .pos = 2,
      .frags = {
        { .offset=0,  .txt="Ths is a ", },
        { .offset=15, .txt="bold", .italics=true, .bold=true },
        { .offset=27, .txt=" move." },
      }
    },
    //       0         1         2         3         4         5
    //       012345678901234567890123456789012345678901234567890
    { .in = "This is a <i><b>bold</b></i> move.",
      .pos = 10,
      .frags = {
        { .offset=0,  .txt="This is a ", },
        { .offset=16, .txt="old", .italics=true, .bold=true },
        { .offset=27, .txt=" move." },
      }
    },
    //       0         1         2         3         4         5
    //       012345678901234567890123456789012345678901234567890
    { .in = "This is a <i><b>b</b></i> move.",
      .pos = 10,
      .frags = {
        { .offset=0,  .txt="This is a  move.", },
      }
    },
    //       0         1         2         3         4         5
    //       012345678901234567890123456789012345678901234567890
    { .in = "This is a <i><b>b</b>old</i> move.",
      .pos = 10,
      .frags = {
        { .offset=0,  .txt="This is a ", },
        { .offset=13, .txt="old", .italics=true},
        { .offset=20, .txt=" move.", },
      }
    },
    //       0         1         2         3         4         5
    //       012345678901234567890123456789012345678901234567890
    { .in = "This is a<br/> break dance.",
      .pos = 9,
      .frags = {
        { .offset=0,  .txt="This is a break dance." },
      }
    },
    //       0         1         2         3         4         5
    //       012345678901234567890123456789012345678901234567890
    { .in = "Hello<b><br/>u</b> there.",
      .pos = 13,
      .frags = {
        { .offset=0,  .txt="Hello",  },
        { .offset=8,  .txt="", .bold=true, .eol=true },
        { .offset=13, .txt="", .bold=true, },
        { .offset=17, .txt=" there." },
      }
    },
    //       0         1         2         3         4         5
    //       012345678901234567890123456789012345678901234567890
    { .in = "<i><b>b</b></i>",
      .pos = 6,
      .frags = {
        { .offset=0,  .txt="",  },
      }
    },
    //       0         1         2         3         4         5
    //       012345678901234567890123456789012345678901234567890
    { .in = "<b>was<br/>ich<br/>s</b><i><b>ag</b>e</i>",
      .out ="<b>was<br/>ich<br/>s</b><i><b>ag</b></i>",
      .pos = 36,
      .frags = {
        { .offset = 3, .txt="was", .bold=true, .eol=true },
        { .offset =11, .txt="ich", .bold=true, .eol=true },
        { .offset =19, .txt="s", .bold=true, },
        { .offset =30, .txt="ag", .bold=true, .italics=true },
        { .offset =36, .txt="", .italics=true },
        { .offset =40, .txt="" },
      }
    },
    //       0         1         2         3         4         5
    //       012345678901234567890123456789012345678901234567890
    { .in = "<b>was<br/>ich<br/>s</b><i><b>ag</b>e<br/></i>",
      .out ="<b>was<br/>ich<br/>s</b><i><b>ag</b><br/></i>",
      .pos = 36,
      .frags = {
        { .offset = 3, .txt="was", .bold=true, .eol=true },
        { .offset =11, .txt="ich", .bold=true, .eol=true },
        { .offset =19, .txt="s", .bold=true, },
        { .offset =30, .txt="ag", .bold=true, .italics=true },
        { .offset =36, .txt="", .italics=true, .eol=true },
        { .offset =41, .txt="", .italics=true, },
        { .offset =45, .txt="" },
      }
    },
    //       0         1         2         3         4         5
    //       012345678901234567890123456789012345678901234567890
    { .in = "ab<i><b>c&amp;de</b></i>fg",
      .out ="ab<i><b>&amp;de</b></i>fg",
      .pos = 8,
      .frags = {
        { .offset = 0, .txt="ab" },
        { .offset = 8, .txt="&amp;",  .bold=true, .italics=true },
        { .offset =13, .txt="de", .bold=true, .italics=true },
        { .offset =23, .txt="fg" },
      }
    },
  };
  
  for(auto &t: test) {
    string text(t.in);
    vector<size_t> xpos;
    xpos.assign(3, 0);
    TPreparedDocument document;
  
    prepareHTMLText(t.in, xpos, &document);

    cout << "----------------------------------------------------------------------" << endl
         << t.in << endl;
/*     
    for(auto &line: document.lines) {
      cout << "line:" << endl;
      for(auto &fragment: line->fragments) {
        cout << "  fragment: " << fragment->offset << ", " << fragment->length
             << (fragment->attr.bold?", bold":"")
             << (fragment->attr.italic?", italics":"") << endl;
      }
    }
*/  
    xpos[CURSOR]=t.pos;
    textDelete(text, document, xpos);

    for(auto &line: document.lines) {
      cout << "line:" << endl;
      for(auto &fragment: line->fragments) {
        cout << "  fragment: " << fragment->offset << ", " << fragment->length
             << ", \"" << text.substr(fragment->offset, fragment->length) << "\" "
             << (fragment->attr.bold?", bold":"")
             << (fragment->attr.italic?", italics":"") << endl;
      }
    }
    
    if (t.out)
      ASSERT_STREQ(t.out, text.c_str());

    auto line = document.lines.begin();
    if (line==document.lines.end()) {
      ASSERT_EQ(0, t.frags.size());
      continue;
    }
    auto fragment = (*line)->fragments.begin();
//cout << "first line, first fragment" << endl;
    for(auto &f: t.frags) {
//      cout << "  expect: " << f.offset << ", " << f.length << (f.eol?", eol":"") << endl;
//      cout << "  got   : " << (*fragment)->offset << ", " << (*fragment)->length << endl;
      
      ASSERT_EQ((*fragment)->offset, f.offset);
//      ASSERT_STREQ(text.substr((*fragment)->offset, (*fragment)->length).c_str(), f.txt);
      ASSERT_STREQ(f.txt, text.substr((*fragment)->offset, (*fragment)->length).c_str());
      ASSERT_EQ((*fragment)->attr.bold, f.bold);
      ASSERT_EQ((*fragment)->attr.italic, f.italics);
      
//cout << "next fragment" << endl;
      ++fragment;
      
      if (fragment == (*line)->fragments.end()) {
//cout << "next line" << endl;
        ++line;
        if (line==document.lines.end()) {
//cout << "last line" << endl;
          break;
        }
        fragment = (*line)->fragments.begin();
        ASSERT_EQ(true, f.eol);
      } else {
        ASSERT_EQ(false, f.eol);
      }
    }
    // check that the result is not larger than the expected test set
    if (line!=document.lines.end())
      ASSERT_EQ(fragment, (*line)->fragments.end());
    else
      ASSERT_EQ(line, document.lines.end());

  }
}

TEST(WordProcessor, textInsert)
{
  struct fragtest {
    size_t offset;
    const char *txt;
    bool bold, italics, eol;
  };
  struct test {
    const char *in, *out;
    size_t pos, bgn, end;
    vector<fragtest> frags;
  };
  
  vector<test> test = {
    //       0         1         2         3         4         5
    //       012345678901234567890123456789012345678901234567890
    { .in = "",
      .pos = 0,
      .frags = {
        { .offset=0,  .txt="T" },
      }
    },
    //       0         1         2         3         4         5
    //       012345678901234567890123456789012345678901234567890
    { .in = "a",
      .pos = 1,
      .frags = {
        { .offset=0,  .txt="aT" },
      }
    },
    //       0         1         2         3         4         5
    //       012345678901234567890123456789012345678901234567890
    { .in = "<br/>",
      .pos = 0,
      .frags = {
        { .offset=0,  .txt="T", .eol=true },
        { .offset=6,  .txt="", },
      }
    },
    //       0         1         2         3         4         5
    //       012345678901234567890123456789012345678901234567890
    { .in = "<br/>Hi",
      .pos = 0,
      .frags = {
        { .offset=0,  .txt="T", .eol=true },
        { .offset=6,  .txt="Hi", },
      }
    },
    //       0         1         2         3         4         5
    //       012345678901234567890123456789012345678901234567890
    { .in = "<b>Hi</b>",
      .pos = 0,
      .frags = {
        { .offset=3,  .txt="THi", .bold=true },
        { .offset=10, .txt="" },
      }
    },
    //       0         1         2         3         4         5
    //       012345678901234567890123456789012345678901234567890
    { .in = "abc&amp;",
      .pos = 8,
      .frags = {
        { .offset=0, .txt="abc" },
        { .offset=3, .txt="&amp;" },
        { .offset=8, .txt="T" },
      }
    },
    //       0         1         2         3         4         5
    //       012345678901234567890123456789012345678901234567890
    { .in = "abc&amp;<br/>def",
      .pos = 8,
      .frags = {
        { .offset= 0, .txt="abc" },
        { .offset= 3, .txt="&amp;" },
        { .offset= 8, .txt="T", .eol=true },
        { .offset=14, .txt="def" }
      }
    },
    //       0         1         2         3         4         5
    //       012345678901234567890123456789012345678901234567890
    { .in = "Fine with<i><b>&lt;&gt;&amp;</b></i> you.",
      .pos = 23,
      .frags = {
        { .offset= 0, .txt="Fine with" },
        { .offset=15, .txt="&lt;",  .italics=true, .bold=true },
        { .offset=19, .txt="&gt;",  .italics=true, .bold=true },
        { .offset=23, .txt="T",     .italics=true, .bold=true },
        { .offset=24, .txt="&amp;", .italics=true, .bold=true },
        { .offset=37, .txt=" you." }
      }
    },
  };
  
  for(auto &t: test) {
    string text(t.in);
    vector<size_t> xpos;
    xpos.assign(3, 0);
    TPreparedDocument document;
  
    prepareHTMLText(t.in, xpos, &document);
    updateMarker(text, &document, xpos);

    cout << "----------------------------------------------------------------------" << endl
         << t.in << endl;

    for(auto &line: document.lines) {
      cout << "line:" << endl;
      for(auto &fragment: line->fragments) {
        cout << "  fragment: " << fragment->offset << ", " << fragment->length
             << ", \"" << text.substr(fragment->offset, fragment->length) << "\" "
             << (fragment->attr.bold?", bold":"")
             << (fragment->attr.italic?", italics":"") << endl;
      }
    }


    xpos[CURSOR]=t.pos;
    textInsert(text, document, xpos, "T");

    cout << "- - - - - - - - - - - -" << endl;

    for(auto &line: document.lines) {
      cout << "line:" << endl;
      for(auto &fragment: line->fragments) {
        cout << "  fragment: " << fragment->offset << ", " << fragment->length
             << ", \"" << text.substr(fragment->offset, fragment->length) << "\" "
             << (fragment->attr.bold?", bold":"")
             << (fragment->attr.italic?", italics":"") << endl;
      }
    }
    
    if (t.out)
      ASSERT_STREQ(t.out, text.c_str());

    auto line = document.lines.begin();
    if (line==document.lines.end()) {
      ASSERT_EQ(0, t.frags.size());
      continue;
    }
    auto fragment = (*line)->fragments.begin();
//cout << "first line, first fragment" << endl;
    for(auto &f: t.frags) {
//      cout << "  expect: " << f.offset << ", " << f.length << (f.eol?", eol":"") << endl;
//      cout << "  got   : " << (*fragment)->offset << ", " << (*fragment)->length << endl;
      
      ASSERT_EQ((*fragment)->offset, f.offset);
//      ASSERT_STREQ(text.substr((*fragment)->offset, (*fragment)->length).c_str(), f.txt);
      ASSERT_STREQ(f.txt, text.substr((*fragment)->offset, (*fragment)->length).c_str());
      ASSERT_EQ((*fragment)->attr.bold, f.bold);
      ASSERT_EQ((*fragment)->attr.italic, f.italics);
      
//cout << "next fragment" << endl;
      ++fragment;
      
      if (fragment == (*line)->fragments.end()) {
//cout << "next line" << endl;
        ++line;
        if (line==document.lines.end()) {
//cout << "last line" << endl;
          break;
        }
        fragment = (*line)->fragments.begin();
        ASSERT_EQ(true, f.eol);
      } else {
        ASSERT_EQ(false, f.eol);
      }
    }
    // check that the result is not larger than the expected test set
    if (line!=document.lines.end())
      ASSERT_EQ(fragment, (*line)->fragments.end());
    else
      ASSERT_EQ(line, document.lines.end());
  }
}

TEST(WordProcessor, variousEntityTests)
{
  string text="abc";
  vector<size_t> xpos;
  xpos.assign(3, 0);
  TPreparedDocument document;
  prepareHTMLText(text, xpos, &document);

  xpos[CURSOR]=3;

  text.insert(xpos[CURSOR], "&amp;");
  xmlinc(text, &xpos[CURSOR]);
  prepareHTMLText(text, xpos, &document);
  updateMarker(text, &document, xpos);
  
cout << "=====================================================================" << endl;
  cout << "0         1         2         3         4         5" << endl;
  cout << "012345678901234567890123456789012345678901234567890" << endl;
  cout << text << endl;
  cout << xpos[CURSOR] << endl;
  cout << document.marker[CURSOR].pos << endl;
  dump(text, document);
cout << "textInsert ----------------------------------------------------------" << endl;

  textInsert(text, document, xpos, "d");

  cout << "0         1         2         3         4         5" << endl;
  cout << "012345678901234567890123456789012345678901234567890" << endl;
  cout << text << endl;
  cout << xpos[CURSOR] << endl;
  cout << document.marker[CURSOR].pos << endl;
  dump(text, document);

cout << "lineBreak -----------------------------------------------------------" << endl;

  text.insert(xpos[CURSOR], "<br/>");
  xmlinc(text, &xpos[CURSOR]);
  prepareHTMLText(text, xpos, &document);
  updateMarker(text, &document, xpos);

  cout << "0         1         2         3         4         5" << endl;
  cout << "012345678901234567890123456789012345678901234567890" << endl;
  cout << text << endl;
  cout << xpos[CURSOR] << endl;
  cout << document.marker[CURSOR].pos << endl;
  dump(text, document);
  
  ASSERT_EQ(0, document.lines.back()->fragments.back()->origin.x);
}

TEST(WordProcessor, updateMarker)
{
  string text="ab&lt;cd";
  vector<size_t> xpos;
  xpos.assign(3, 0);
  TPreparedDocument document;
  prepareHTMLText(text, xpos, &document);

  // before 'a'
  xpos[CURSOR]=0;
  updateMarker(text, &document, xpos);
  dump(text, document);
  cout << xpos[CURSOR] << " -> " << document.marker[CURSOR].pos << ", height " << document.marker[CURSOR].fragment->size.height << endl;
  
  // before 'b'
  xmlinc(text, &xpos[CURSOR]);
  updateMarker(text, &document, xpos);
  dump(text, document);
  cout << xpos[CURSOR] << " -> " << document.marker[CURSOR].pos << ", height " << document.marker[CURSOR].fragment->size.height << endl;

  // before '&amp;'
  xmlinc(text, &xpos[CURSOR]);
  updateMarker(text, &document, xpos);
  dump(text, document);
  cout << xpos[CURSOR] << " -> " << document.marker[CURSOR].pos << ", height " << document.marker[CURSOR].fragment->size.height << endl;

  // before 'c'
  xmlinc(text, &xpos[CURSOR]);
  updateMarker(text, &document, xpos);
  dump(text, document);
  cout << xpos[CURSOR] << " -> " << document.marker[CURSOR].pos << ", height " << document.marker[CURSOR].fragment->size.height << endl;

  // before 'd'
  xmlinc(text, &xpos[CURSOR]);
  updateMarker(text, &document, xpos);
  dump(text, document);
  cout << xpos[CURSOR] << " -> " << document.marker[CURSOR].pos << ", height " << document.marker[CURSOR].fragment->size.height << endl;

  // behind 'd'
  xmlinc(text, &xpos[CURSOR]);
  updateMarker(text, &document, xpos);
  dump(text, document);
  cout << xpos[CURSOR] << " -> " << document.marker[CURSOR].pos << ", height " << document.marker[CURSOR].fragment->size.height << endl;
}

TEST(WordProcessor, selection)
{ //             0         1         2
  //             012345678901234567890
  string text = "a<br/>b<br/>c<br/>d";
  vector<size_t> xpos;
  xpos.assign(3, 0);
  TPreparedDocument document;
  prepareHTMLText(text, xpos, &document);

  // before 'a'
  xpos[CURSOR]=0;
  xpos[SELECTION_BGN]=6;
  xpos[SELECTION_END]=12;
  updateMarker(text, &document, xpos);
}

// <b>hello</b>
// ^ insert a 0 doesn't work

// cursor movement is as required:
// aaaa<br/>bb<b>bbb</b><br/>cccccc
//                  <        <
//
// but inserting a break between bbb and gives other results
// aaa<b>bbb</b>ccc
// aaa<b>bbb<br/></b>ccc
//          <    <
// and inserting at the head of the new line will now be bold
// which is the same behaviour as Apple Pages. Good enough.
//
// 1. in this case the cursor won't be rendered within <b>
// aaa<b><br/>bbb</b>ccc
// 
// 2. this code can go here:
// <i><b>aaa</b></i>
//                  <
// but shouldn't (usually we are always one character behind the closing
// tag, and here is no such position)
//
// 3. inserting here:
// <b>aaaaaaa</b>
// <
// doesn't work (updatePrepated error)
// and if we were to mimic Apple Pages, we would insert within <b>
