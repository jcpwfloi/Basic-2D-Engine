#include "GPath.h"
#include "GMatrix.h"
#include "Midpoint.h"
#include <math.h>

GPath& GPath::addRect(const GRect& r, Direction dir) {
  moveTo(r.left(), r.top());
  switch (dir) {
    case kCW_Direction:
      lineTo(r.right(), r.top());
      lineTo(r.right(), r.bottom());
      lineTo(r.left(), r.bottom());
      break;
    case kCCW_Direction:
      lineTo(r.left(), r.bottom());
      lineTo(r.right(), r.bottom());
      lineTo(r.right(), r.top());
      break;
    default:
      assert(false);
  }
  return *this;
}

GPath& GPath::addPolygon(const GPoint pts[], int count) {
  assert(count > 0);
  moveTo(pts[0]);
  for (int i = 1; i < count; ++ i)
    lineTo(pts[i]);
  return *this;
}

GRect GPath::bounds() const {
  float top = 1e9, bottom = 0.0, left = 1e9, right = 0.0;
  for (auto i : fPts) {
    top = std::min(top, i.y());
    bottom = std::max(bottom, i.y());
    left = std::min(left, i.x());
    right = std::max(right, i.x());
  }

  if (countPoints() == 0)
    top = left = 0.0;

  return GRect::MakeLTRB(left, top, right, bottom);
}

void GPath::transform(const GMatrix& m) {
  for (auto &i : fPts) {
    GPoint x = m.mapXY(i.x(), i.y());
    i.set(x.x(), x.y());
  }
}

GPath& GPath::addCircle(GPoint center, float radius, GPath::Direction dir) {
  GMatrix m;
  if (dir == kCW_Direction) m.setRotate(M_PI / 4.0);
  else m.setRotate(-M_PI / 4.0);
  GPoint first = { 1.0f, tanf(M_PI / 8.0) };
  GPoint second = { sqrtf(2.0) / 2.0f, sqrtf(2.0) / 2.0f };
  if (dir == kCCW_Direction) {
    first = m.mapPt(first);
    second = m.mapPt(second);
    second = m.mapPt(second);
  }

  GVector v = { radius, 0 };
  moveTo(center + v);

  for (int i = 0; i < 8; ++ i) {
    quadTo(center + first * radius, center + second * radius);
    first = m.mapPt(first);
    second = m.mapPt(second);
  }
  return *this;
}

void GPath::ChopQuadAt(const GPoint src[3], GPoint dst[5], float t) {
  dst[0] = src[0];
  dst[4] = src[2];
  dst[1] = src[0] * (1 - t) + (src[1] * t);
  dst[3] = src[1] * (1 - t) + (src[2] * t);
  dst[2] = dst[1] * (1 - t) + (dst[3] * t);
}

void GPath::ChopCubicAt(const GPoint src[4], GPoint dst[7], float t) {
  dst[0] = src[0];
  dst[6] = src[3];
  dst[1] = src[0] * (1 - t) + (src[1] * t);
  GPoint temp = src[1] * (1 - t) + (src[2] * t);
  dst[2] = dst[1] * (1 - t) + (temp * t);
  dst[5] = src[2] * (1 - t) + (src[3] * t);
  dst[4] = temp * (1 - t) + (dst[5] * t);
  dst[3] = dst[2] * (1 - t) + (dst[4] * t);
}
