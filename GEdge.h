#include "GPoint.h"

struct GEdge {
  int top, bottom, dir;
  float slope, curr_x;

  GEdge(const GPoint& a, const GPoint &b, int dir) : dir(dir) {
    top = GRoundToInt(a.y());
    bottom = GRoundToInt(b.y());

    GVector v = b - a;
    slope = v.x() / v.y();

    curr_x = slope * ((float) top + 0.5 - a.y()) + a.x();
  }

  bool valid() {
    return top < bottom;
  }

  void inc() {
    curr_x += slope;
  }

  void print() {
    printf("top: %d\n", top);
    printf("bottom: %d\n", bottom);
    printf("slope: %f\n", slope);
    printf("curr_x: %f\n", curr_x);
  }

  bool operator < (const GEdge &b) const {
    if (top != b.top) return top < b.top;
    return curr_x < b.curr_x;
  }
};

