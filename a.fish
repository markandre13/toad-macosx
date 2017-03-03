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
        x = 160
        y = 140
        w = 91
        h = 51
      }
      toad::TFRectangle {
        id = 2
        linecolor ={ 0 0 0 }
        linewidth = 1
        x = 232
        y = 292
        w = 91
        h = 61
      }
      toad::TFConnection {
        linecolor ={ 0 0 0 }
        linewidth = 1
        start = 1
        end = 2
      }
      toad::TFCircle {
        id = 3
        linecolor ={ 0 0 0 }
        linewidth = 1
        x = 360.5
        y = 356.5
        w = 109
        h = 101
      }
      toad::TFConnection {
        linecolor ={ 0 0 0 }
        linewidth = 1
        start = 3
        end = 3
      }
      toad::TFConnection {
        linecolor ={ 0 0 0 }
        linewidth = 1
        start = 3
        end = 1
      }
    }
  }
}