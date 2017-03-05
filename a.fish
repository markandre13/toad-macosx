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
        id = 1
        linecolor ={ 0 0 0 }
        linewidth = 1
        x = 304
        y = 124
        w = 92
        h = 52
      }
      toad::TFRectangle {
        id = 2
        linecolor ={ 0 0 0 }
        linewidth = 1
        x = 109.996
        y = 74.6602
        w = 92
        h = 62
      }
      toad::TFConnection {
        linecolor ={ 0 0 0 }
        fillcolor ={ 255 0 0 }
        linewidth = 1
        arrowmode = tail
        arrowtype = filled-concave
        arrowheight = 8
        arrowwidth = 4
        start = 1
        end = 2
      }
      toad::TFCircle {
        id = 3
        linecolor ={ 0 0 0 }
        linewidth = 1
        x = 156.738
        y = 232.617
        w = 110
        h = 102
      }
      toad::TFConnection {
        linecolor ={ 0 0 0 }
        fillcolor ={ 255 255 255 }
        linewidth = 1
        arrowmode = tail
        arrowtype = filled
        arrowheight = 8
        arrowwidth = 4
        start = 3
        end = 1
      }
      toad::TFConnection {
        linecolor ={ 0 0 0 }
        fillcolor ={ 255 0 0 }
        linewidth = 1
        arrowmode = head
        arrowtype = filled-concave
        arrowheight = 8
        arrowwidth = 4
        start = 3
        end = 2
      }
    }
  }
}