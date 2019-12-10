#include "GPixel.h"
#include "GPaint.h"

GPixel blend(const GPaint&, const GPixel&);
GPixel colorToPixel(const GColor&);
void raw_blit(GPixel*, int, const GPaint&);
void switch_blit(GPixel*, int, int, int, const GPaint&);
GPixel mulTwo(GPixel, GPixel);
GColor fabs(const GColor&);

const int opt1[] = {
  0, 1, 2, 1, 4, 5, 2, 7, 0, 5, 4, 7
};

const int opt0[] = {
  0, 0, 2, 2, 2, 0, 0, 0, 2, 2, 0, 2
};
