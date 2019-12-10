#include "GPixel.h"
#include "GColor.h"
#include "GPaint.h"
#include "GShader.h"

GColor fabs(const GColor &c) {
  return {
    fabs(c.fA),
    fabs(c.fR),
    fabs(c.fG),
    fabs(c.fB)
  };
}

inline unsigned int norm(unsigned int x) {
  return (x * 65793 + (1 << 23)) >> 24;
}

inline unsigned long long expand(int x) {
  unsigned long long hi = x & 0xff00ff00;
  unsigned long long lo = x & 0x00ff00ff;

  return (hi << 24) | lo;
}

inline GPixel compact(unsigned long long x) {
  return ((x >> 24) & 0xff00ff00) | (x & 0x00ff00ff);
}

inline unsigned long long repeat(unsigned long long x) {
  return (x << 48) | (x << 32) | (x << 16) | x;
}

inline GPixel mul(GPixel c, GPixel x) {
  // ((prod + 128) + (prod + 128) >> 8) >> 8
  /*
  unsigned long long prod = c * expand(x);
  prod += repeat(128);
  prod += (prod >> 8) & repeat(0xff);
  prod >>= 8;
  return compact(prod);
  */
  return GPixel_PackARGB(
      norm(GPixel_GetA(x) * c) & 0xff,
      norm(GPixel_GetR(x) * c) & 0xff,
      norm(GPixel_GetG(x) * c) & 0xff,
      norm(GPixel_GetB(x) * c) & 0xff
  );
}

inline GPixel smul(GPixel c, GPixel x, GPixel c2, GPixel x2) {
  // ((prod + 128) + (prod + 128) >> 8) >> 8
  /*
  unsigned long long prod = c * expand(x) + c2 * expand(x2);
  prod += repeat(128);
  prod += (prod >> 8) & repeat(0xff);
  prod >>= 8;
  return compact(prod);
  */
  return GPixel_PackARGB(
      norm(GPixel_GetA(x) * c + c2 * GPixel_GetA(x2)) & 0xff,
      norm(GPixel_GetR(x) * c + c2 * GPixel_GetR(x2)) & 0xff,
      norm(GPixel_GetG(x) * c + c2 * GPixel_GetG(x2)) & 0xff,
      norm(GPixel_GetB(x) * c + c2 * GPixel_GetB(x2)) & 0xff
  );
}

GPixel colorToPixel(const GColor& c) {
  GColor cl = c.pinToUnit();

  cl.fR *= cl.fA;
  cl.fG *= cl.fA;
  cl.fB *= cl.fA;

  unsigned int a = GRoundToInt(cl.fA * 255.0);
  unsigned int r = GRoundToInt(cl.fR * 255.0);
  unsigned int g = GRoundToInt(cl.fG * 255.0);
  unsigned int b = GRoundToInt(cl.fB * 255.0);

  return GPixel_PackARGB(a, r, g, b);
}

typedef GPixel (*blend_mode)(GPixel, GPixel);

inline GPixel ps(GPixel src, GPixel dest) {
  // Da * S
  return mul(GPixel_GetA(dest), src);
}

inline GPixel pd(GPixel src, GPixel dest) {
  // Sa * D
  return mul(GPixel_GetA(src), dest);
}

inline GPixel cs(GPixel src, GPixel dest) {
  // (1 - Da) * S
  return mul(0xff ^ GPixel_GetA(dest), src);
}

inline GPixel cd(GPixel src, GPixel dest) {
  // (1 - Sa) * D
  return mul(0xff ^ GPixel_GetA(src), dest);
}

inline GPixel blend_clear(GPixel src, GPixel dest) {
  return 0;
}

inline GPixel blend_source(GPixel src, GPixel dest) {
  return src;
}

inline GPixel blend_dest(GPixel src, GPixel dest) {
  return dest;
}

inline GPixel blend_source_over(GPixel src, GPixel dest) {
  return src + cd(src, dest);
}

inline GPixel blend_dest_over(GPixel src, GPixel dest) {
  return dest + cs(src, dest);
}

inline GPixel blend_source_in(GPixel src, GPixel dest) {
  return ps(src, dest);
}

inline GPixel blend_dest_in(GPixel src, GPixel dest) {
  return pd(src, dest);
}

inline GPixel blend_source_out(GPixel src, GPixel dest) {
  return cs(src, dest);
}

inline GPixel blend_dest_out(GPixel src, GPixel dest) {
  return cd(src, dest);
}

inline GPixel blend_source_atop(GPixel src, GPixel dest) {
  return smul(GPixel_GetA(dest), src, 0xff ^ GPixel_GetA(src), dest);
}

inline GPixel blend_dest_atop(GPixel src, GPixel dest) {
  return smul(GPixel_GetA(src), dest, 0xff ^ GPixel_GetA(dest), src);
}

inline GPixel blend_xor(GPixel src, GPixel dest) {
  return smul(0xff ^ GPixel_GetA(dest), src, 0xff ^ GPixel_GetA(src), dest);
}

const blend_mode modes[] = {
  blend_clear,
  blend_source,
  blend_dest,
  blend_source_over,
  blend_dest_over,
  blend_source_in,
  blend_dest_in,
  blend_source_out,
  blend_dest_out,
  blend_source_atop,
  blend_dest_atop,
  blend_xor
};

const int opt1[] = {
  0, 1, 2, 1, 4, 5, 2, 7, 0, 5, 4, 7
};

const int opt0[] = {
  0, 0, 2, 2, 2, 0, 0, 0, 2, 2, 0, 2
};

GPixel blend(const GPaint& src, const GPixel& dest) {
  blend_mode mode = modes[(int) src.getBlendMode()];

  return mode(colorToPixel(src.getColor()), dest);
}

GPixel full_blend(const GPaint& src, const GPixel& dest) {
  return colorToPixel(src.getColor());
}

void raw_blit(GPixel* start, int count, const GPaint& color) {
  int mode = (int) color.getBlendMode();
  blend_mode func = modes[mode];
  GPixel src = colorToPixel(color.getColor());

  int alpha = GPixel_GetA(src);

  if (alpha == 0xff) {
    func = modes[opt1[mode]];
    mode = opt1[mode];
  }
  if (!alpha) {
    func = modes[opt0[mode]];
    mode = opt0[mode];
  }

  if (func == blend_clear)
    memset(start, 0, count * sizeof(GPixel));
  else if (func == blend_source)
    std::fill(start, start + count, src);
  else
    switch (mode) {
      case 0:
        for (int i = 0; i < count; ++ i)
          start[i] = blend_clear(src, start[i]);
        break;
      case 1:
        for (int i = 0; i < count; ++ i)
          start[i] = blend_source(src, start[i]);
        break;
      case 2:
        for (int i = 0; i < count; ++ i)
          start[i] = blend_dest(src, start[i]);
        break;
      case 3:
        for (int i = 0; i < count; ++ i)
          start[i] = blend_source_over(src, start[i]);
        break;
      case 4:
        for (int i = 0; i < count; ++ i)
          start[i] = blend_dest_over(src, start[i]);
        break;
      case 5:
        for (int i = 0; i < count; ++ i)
          start[i] = blend_source_in(src, start[i]);
        break;
      case 6:
        for (int i = 0; i < count; ++ i)
          start[i] = blend_dest_in(src, start[i]);
        break;
      case 7:
        for (int i = 0; i < count; ++ i)
          start[i] = blend_source_out(src, start[i]);
        break;
      case 8:
        for (int i = 0; i < count; ++ i)
          start[i] = blend_dest_out(src, start[i]);
        break;
      case 9:
        for (int i = 0; i < count; ++ i)
          start[i] = blend_source_atop(src, start[i]);
        break;
      case 10:
        for (int i = 0; i < count; ++ i)
          start[i] = blend_dest_atop(src, start[i]);
        break;
      case 11:
        for (int i = 0; i < count; ++ i)
          start[i] = blend_xor(src, start[i]);
        break;
    }
}

void switch_blit(GPixel* start, int x, int y, int count, const GPaint& color) {
  if (color.getShader() == nullptr)
    raw_blit(start, count, color);
  else {
    int mode = (int) color.getBlendMode();
    blend_mode func = modes[mode];
    GShader* shader = color.getShader();
    GPixel* temp = new GPixel[count];
    shader -> shadeRow(x, y, count, temp);
    switch (mode) {
      case 0:
        for (int i = 0; i < count; ++ i)
          start[i] = blend_clear(temp[i], start[i]);
        break;
      case 1:
        for (int i = 0; i < count; ++ i)
          start[i] = blend_source(temp[i], start[i]);
        break;
      case 2:
        for (int i = 0; i < count; ++ i)
          start[i] = blend_dest(temp[i], start[i]);
        break;
      case 3:
        for (int i = 0; i < count; ++ i)
          start[i] = blend_source_over(temp[i], start[i]);
        break;
      case 4:
        for (int i = 0; i < count; ++ i)
          start[i] = blend_dest_over(temp[i], start[i]);
        break;
      case 5:
        for (int i = 0; i < count; ++ i)
          start[i] = blend_source_in(temp[i], start[i]);
        break;
      case 6:
        for (int i = 0; i < count; ++ i)
          start[i] = blend_dest_in(temp[i], start[i]);
        break;
      case 7:
        for (int i = 0; i < count; ++ i)
          start[i] = blend_source_out(temp[i], start[i]);
        break;
      case 8:
        for (int i = 0; i < count; ++ i)
          start[i] = blend_dest_out(temp[i], start[i]);
        break;
      case 9:
        for (int i = 0; i < count; ++ i)
          start[i] = blend_source_atop(temp[i], start[i]);
        break;
      case 10:
        for (int i = 0; i < count; ++ i)
          start[i] = blend_dest_atop(temp[i], start[i]);
        break;
      case 11:
        for (int i = 0; i < count; ++ i)
          start[i] = blend_xor(temp[i], start[i]);
        break;
    }
    delete[] temp;
  }
}

GPixel mulTwo(GPixel a, GPixel b) {
  return GPixel_PackARGB(
      norm(GPixel_GetA(a) * GPixel_GetA(b)),
      norm(GPixel_GetR(a) * GPixel_GetR(b)),
      norm(GPixel_GetG(a) * GPixel_GetG(b)),
      norm(GPixel_GetB(a) * GPixel_GetB(b))
  );
}
