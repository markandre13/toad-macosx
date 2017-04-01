// fish -- a Fischland 2D Vector Graphics file
// Please see http://www.mark13.org/fischland/ for more details.

fischland::TDocument {
  author = ""
  date = ""
  description = ""
  fischland::TSlide {
    name = "unnamed #3"
    comment = ""
    lock = false
    show = true
    print = true
    fischland::TLayer {
      name = "unnamed #4"
      comment = ""
      lock = false
      show = true
      print = true
      toad::TFRectangle {
        id = 2
        linecolor ={ 0 0 0 }
        linewidth = 1
        x = 304
        y = 124
        w = 97
        h = 57
      }
      toad::TFRectangle {
        id = 1
        linecolor ={ 0 0 0 }
        linewidth = 1
        x = 69.996
        y = 90.6602
        w = 97
        h = 67
      }
      toad::TFCircle {
        linecolor ={ 0 0 0 }
        linewidth = 1
        x = 156.738
        y = 232.617
        w = 115
        h = 107
      }
      toad::TFConnection {
        linecolor ={ 0 0 0 }
        linewidth = 1
        arrowmode = head
        arrowtype = empty
        arrowheight = 8
        arrowwidth = 4
        start = 1
        end = 2
        p ={ 165.996 143.749 227.496 169.488 242.078 104.613 263.211 171.785 276.566 112.676 304 126.977 }
      }
      toad::TFText {
        linecolor ={ 0 0 0 }
        linewidth = 1
        x = 152.5
        y = 32.5
        w = 156.053
        h = 29
        fontname = "arial,helvetica,sans-serif:size=12"
        text = "In <i>Italy</i> I <u>underlined</u><br/>something <b>bold</b> like E=m&times;c<sup>2</<sup>."
        relation = null
      }
      toad::TFText {
        linecolor ={ 0 0 0 }
        linewidth = 1
        x = 72.996
        y = 91.6602
        w = 98.3633
        h = 3
        fontname = "arial,helvetica,sans-serif:size=12"
        text = "hello folk!!!"
        relation = 1
      }
    }
  }
}