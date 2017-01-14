namespace toad {
  class TWindow;
}

@interface toadView : NSView <NSTextInputClient>
{
  @public
    toad::TWindow *twindow;
}
@end
