#include "Canvas.h"
#include "Midpoint.h"

bool comp(const GEdge &a, const GEdge &b) {
  return a.curr_x < b.curr_x;
}

void Canvas::drawPath(const GPath& path, const GPaint& paint) {
  GPath p = path;
  p.transform(curr_m);
  GPath::Edger iter(p);
  GPoint pts[4];

  std::vector<GEdge> clipped;

  if (paint.getShader() != nullptr) {
    paint.getShader() -> setContext(curr_m);
  }

  GPath::Verb verb;

  while ((verb = iter.next(pts)) != GPath::kDone)
    if (verb == GPath::kLine)
      clip_edge(pts[0], pts[1], clipped, pts[0].y() > pts[1].y() ? 1 : -1);
    else if (verb == GPath::kQuad) {
      GVector dV = (pts[0] + pts[2] - (pts[1] * 2));
      float tolerance = sqrtf(dV.length());
      int N = GRoundToInt(tolerance);
      float step = 1.0f / N;
      float now = step;
      GPoint last = pts[0], midpoint;
      for (int i = 0; i < N; ++ i, now += step) {
        midpoint = quad_midpoint(pts, now);

        clip_edge(last, midpoint, clipped, last.y() > midpoint.y() ? 1 : -1);

        last = midpoint;
      }
    } else if (verb == GPath::kCubic) {
      //max(p0-2p1+p2, p1-2p2+p3)
      GVector dV1 = (pts[0] + pts[2] - (pts[1] * 2));
      GVector dV2 = (pts[1] + pts[3] - (pts[2] * 2));
      //sqrt(D/tol * 3/4)
      float tolerance = std::max(dV1.length(), dV2.length());
      tolerance = sqrtf(tolerance * 3.0);
      int N = GRoundToInt(tolerance);
      float step = 1.0f / N;
      float now = step;
      GPoint last = pts[0], midpoint;
      for (int i = 0; i < N; ++ i, now += step) {
        midpoint = cubic_midpoint(pts, now);

        clip_edge(last, midpoint, clipped, last.y() > midpoint.y() ? 1 : -1);

        last = midpoint;
      }
    }

  std::sort(clipped.begin(), clipped.end());

  if ((int) clipped.size() < 2) return;

  int y = clipped.begin() -> top;
  int maxy = clipped.rbegin() -> bottom;

  for (GEdge e : clipped)
    maxy = std::max(e.bottom, maxy);

  // walk
  
  std::vector<GEdge>::iterator it = clipped.begin();
  std::vector<GEdge> active, temp;

  for (int curr_y = y; curr_y < maxy;) {
    // add edge
    while (curr_y >= it -> top && it != clipped.end()) {
      if (it -> valid()) active.push_back(*it);
      ++ it;
    }
    // sort edge
    sort(active.begin(), active.end(), comp);
    // blit area
    int wind = 0;
    int left = -1;
    for (GEdge e : active) {
      wind += e.dir;
      if (left == -1 && wind - e.dir == 0) {
        left = GRoundToInt(e.curr_x);
        continue;
      }
      if (!wind) {
        int right = GRoundToInt(e.curr_x);
        if (right - left > 0)
          switch_blit(getPixel(left, curr_y), left, curr_y, right - left, paint);
        left = -1;
      }
    }
    // delete edge
    ++ curr_y;

    temp.clear();

    for (GEdge e : active)
      if (curr_y < e.bottom)
        temp.push_back(e);

    active.clear();

    for (GEdge e : temp)
      active.push_back(e);

    for (GEdge &e : active)
      e.inc();
  }
}

