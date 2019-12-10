#include "GPoint.h"
#include "GMatrix.h"
#include "math.h"

void print(const GMatrix &x) {
  for (int i = 0; i < 6; ++ i)
    printf("%10f", x[i]);
  puts("");
}

void GMatrix::setIdentity() {
  set6(1, 0, 0, 0, 1, 0);
}

void GMatrix::setTranslate(float tx, float ty) {
  set6(1, 0, tx, 0, 1, ty);
}

void GMatrix::setScale(float sx, float sy) {
  set6(sx, 0, 0, 0, sy, 0);
}

void GMatrix::setRotate(float radians) {
  float cosx = cos(radians), sinx = sin(radians);
  set6(cosx, -sinx, 0, sinx, cosx, 0);
}

void GMatrix::setConcat(const GMatrix& secundo, const GMatrix& primo) {
  GMatrix x;
  x.set6(1.0 * secundo[0] * primo[0] + 1.0 * secundo[1] * primo[3],
      1.0 * secundo[0] * primo[1] + 1.0 * secundo[1] * primo[4],
      1.0 * secundo[0] * primo[2] + 1.0 * secundo[1] * primo[5] + secundo[2],
      1.0 * secundo[3] * primo[0] + 1.0 * secundo[4] * primo[3],
      1.0 * secundo[3] * primo[1] + 1.0 * secundo[4] * primo[4],
      1.0 * secundo[3] * primo[2] + 1.0 * secundo[4] * primo[5] + secundo[5]);
  for (int i = 0; i < 6; ++ i)
    fMat[i] = x[i];
}

bool GMatrix::invert(GMatrix* inverse) const {
  const GMatrix x = *this;
  double det = 1.0 * x[0] * x[4] - 1.0 * x[1] * x[3];
  if (fabs(det) == 0) return false;

  double inv = 1 / det;

  GMatrix t;
  t.set6(x[4] * inv, -x[1] * inv, (x[1] * x[5] - x[2] * x[4]) * inv,
      -x[3] * inv, x[0] * inv, (x[2] * x[3] - x[0] * x[5]) * inv);

  inverse -> set6(t[0], t[1], t[2], t[3], t[4], t[5]);

  return true;
}

void GMatrix::mapPoints(GPoint dst[], const GPoint src[], int count) const {
  const GMatrix x = *this;
  for (int i = 0; i < count; ++ i) {
    dst[i] = { x[0] * src[i].x() + x[1] * src[i].y() + x[2],
               x[3] * src[i].x() + x[4] * src[i].y() + x[5]
    };
  }
}
