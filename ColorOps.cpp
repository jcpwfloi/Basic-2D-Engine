#include "GColor.h"

GColor operator - (const GColor &a, const GColor &b) {
  return {
    a.fA - b.fA,
    a.fR - b.fR,
    a.fG - b.fG,
    a.fB - b.fB
  };
}

GColor operator + (const GColor &a, const GColor &b) {
  return {
    a.fA + b.fA,
    a.fR + b.fR,
    a.fG + b.fG,
    a.fB + b.fB
  };
}

GColor operator * (const GColor &a, float x) {
  return {
    a.fA * x,
    a.fR * x,
    a.fG * x,
    a.fB * x
  };
}

