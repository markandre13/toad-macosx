static double
perpendicularDistance(const TPoint &pt, const TPoint &lineStart, const TPoint &lineEnd)
{
  double dx = lineEnd.x - lineStart.x;
  double dy = lineEnd.y - lineStart.y;

  //Normalise
  double mag = pow(pow(dx,2.0)+pow(dy,2.0),0.5);
  if(mag > 0.0)	{
    dx /= mag; dy /= mag;
  }

  double pvx = pt.x - lineStart.x;
  double pvy = pt.y - lineStart.y;

  //Get dot product (project pv onto normalized direction)
  double pvdot = dx * pvx + dy * pvy;

  //Scale line direction vector
  double dsx = pvdot * dx;
  double dsy = pvdot * dy;

  //Subtract this from pv
  double ax = pvx - dsx;
  double ay = pvy - dsy;

  return pow(pow(ax,2.0)+pow(ay,2.0),0.5);
}

void
ramerDouglasPeucker(const vector<TPoint> &in, double epsilon, vector<TPoint> *out)
{
  if(in.size()<2)
    throw invalid_argument("Not enough points to simplify");

  // find the point with the maximum distance from line between start and end
  double dmax = 0.0;
  size_t index = 0;
  size_t end = in.size()-1;
  for(size_t i = 1; i < end; ++i) {
    double d = perpendicularDistance(in[i], in[0], in[end]);
    if (d > dmax) {
      index = i;
      dmax = d;
    }
  }

  // if max distance is greater than epsilon, recursively simplify
  if(dmax > epsilon) {
    // Recursive call
    vector<TPoint> recResults1;
    vector<TPoint> recResults2;
    vector<TPoint> firstLine(in.begin(), in.begin()+index+1);
    vector<TPoint> lastLine(in.begin()+index, in.end());
    ramerDouglasPeucker(firstLine, epsilon, recResults1);
    ramerDouglasPeucker(lastLine, epsilon, recResults2);

    // build the result list
    out->assign(recResults1.begin(), recResults1.end()-1);
    out->insert(out.end(), recResults2.begin(), recResults2.end());
    if(out->size()<2)
      throw runtime_error("Problem assembling output");
  } else {
    // just return start and end points
    out->clear();
    out->push_back(in[0]);
    out->push_back(in[end]);
  }
}
