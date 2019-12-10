#include "Midpoint.h"

GPoint quad_midpoint(const GPoint *pts, float w) {
  GPoint v1 = pts[0] * (1 - w) + (pts[1] * w);
  GPoint v2 = pts[1] * (1 - w) + (pts[2] * w);
  return v1 * (1 - w)  + (v2 * w);
}

GPoint cubic_midpoint(const GPoint *pts, float w) {
  GPoint v1 = pts[0] * (1 - w) + (pts[1] * w);
  GPoint v2 = pts[1] * (1 - w) + (pts[2] * w);
  GPoint v3 = pts[2] * (1 - w) + (pts[3] * w);
  GPoint vd1 = v1 * (1 - w) + (v2 * w);
  GPoint vd2 = v2 * (1 - w) + (v3 * w);
  return vd1 * (1 - w)  + (vd2 * w);
}
