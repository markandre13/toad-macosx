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

TEST(WordProcessor, prepareHTMLText)
{
  struct fragtest {
    size_t offset, length;
    bool bold, italics, eol;
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
  };

  for(auto &t: test) {
    vector<size_t> xpos;
    TPreparedDocument document;
  
    xpos.assign(3, 0);
    prepareHTMLText(t.in, xpos, &document);

    cout << "----------------------------------------------------------------------" << endl
         << t.in << endl;
     
    for(auto &line: document.lines) {
      cout << "line:" << endl;
      for(auto &fragment: line->fragments) {
        cout << "  fragment: " << fragment->offset << ", " << fragment->length
             << (fragment->attr.bold?", bold":"")
             << (fragment->attr.italic?", italics":"") << endl;
      }
    }

// FIXME: do check if either data set is too small/too big
    auto line = document.lines.begin();
    if (line==document.lines.end()) {
cout << "FIXME: need to check for empty test test" << endl;
      continue;
    }
    auto fragment = (*line)->fragments.begin();
//cout << "first line, first fragment" << endl;
    for(auto &f: t.frags) {
//      cout << "  expect: " << f.offset << ", " << f.length << (f.eol?", eol":"") << endl;
//      cout << "  got   : " << (*fragment)->offset << ", " << (*fragment)->length << endl;
      
      ASSERT_EQ((*fragment)->offset, f.offset);
      ASSERT_EQ((*fragment)->length, f.length);
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
  }
}
