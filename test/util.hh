#import <Cocoa/Cocoa.h>
#include <toad/window.hh>

std::string testname();
CGImageRef grabImage(toad::TWindow *window);
void saveImage(CGImageRef image, const std::string &filename);
void compareImageFile(const std::string &file0, const std::string &file1);
