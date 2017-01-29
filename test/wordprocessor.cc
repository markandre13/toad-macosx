#include <toad/wordprocessor.hh>
#include <toad/utf8.hh>

#include "gtest.h"

using namespace toad;
using namespace toad::wordprocessor;

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
  };

  static test test[] = {
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

    // drop tag
    { // 0         1         2         3         4         5
      // 012345678901234567890123456789012345678901234567890
        "This was a <b>bold</b> move.",
      //            ^      <
                    11,    18,
        "This was a bold move."
    },
  };

  for(size_t idx=0; idx<(sizeof(test)/sizeof(struct test)); ++idx) {
    cout << "----------------------------------- " << test[idx].bgn << ", " << test[idx].end << endl;
    string text = test[idx].in;
  
    vector<size_t> xpos;
    xpos.assign(3,0);
    xpos[SELECTION_BGN]=test[idx].bgn;
    xpos[SELECTION_END]=test[idx].end;

    string out = tagtoggle(test[idx].in, xpos, "b");
    
    if (out != test[idx].out) {
      cout << "in  : " << text << endl;
      cout << "want: " << test[idx].out << endl;
      cout << "got : " << out  << endl;
    }
    ASSERT_EQ(out, test[idx].out);
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
    { .in = "This w<i>as a </i><b><i>bo</i>ld</b> move.",
      .frags = {
        { .offset=0, .length=6 },
        { .offset=9, .length=5, .italics=true },
        { .offset=24, .length=2, .italics=true, .bold=true },
        { .offset=30, .length=2, .bold=true },
        { .offset=36, .length=6, .eol=true },
      }
    },
  };

  for(auto &t: test) {
    vector<size_t> xpos;
    TPreparedDocument document;
  
    xpos.assign(3, 0);
    prepareHTMLText(t.in, xpos, &document);
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
