#include "GMatrix.h"
#include "GShader.h"
#include "GBitmap.h"
#include "GMath.h"
#include "alib.h"
#include "ColorOps.h"

class TriShader : public GShader {
  public:
    TriShader(const GColor c[3], const GPoint p[3], TileMode mode) : mode(mode) {
      for (int i = 0; i < 3; ++ i) this -> c[i] = c[i];

      GVector u = p[1] - p[0], v = p[2] - p[0];

      local.set6(u.x(), v.x(), p[0].x(), u.y(), v.y(), p[0].y());
    }

    bool isOpaque() override {
      return opaque;
    }

    bool setContext(const GMatrix& ctm) override {
      fInv.setConcat(ctm, local);
      bool ret = fInv.invert(&fInv);
      return ret;
    }

    void shadeRow(int x, int y, int count, GPixel row[]) override {
      GPoint pt = fInv.mapXY(x + 0.5, y + 0.5);
      GColor DC1 = c[1] - c[0],
             DC2 = c[2] - c[0];
      GColor c0 = DC1 * pt.x() + DC2 * pt.y() + c[0];

      GColor DC = DC1 * fInv[0] + DC2 * fInv[3];

      row[0] = colorToPixel(c0);
      for (int i = 1; i < count; ++ i) {
        c0 = c0 + DC;
        row[i] = colorToPixel(c0);
      }
    }
  private:
    GColor c[3];

    GMatrix local;
    GMatrix fInv;

    TileMode mode;

    bool opaque;
};

std::unique_ptr<GShader> GCreateTriShader(const GColor c[3], const GPoint p[3], GShader::TileMode mode = GShader::kClamp) {
  return std::unique_ptr<GShader>(new TriShader(c, p, mode));
}

GShader* CTriShader(const GColor c[3], const GPoint p[3], GShader::TileMode mode = GShader::kClamp) {
  return new TriShader(c, p, mode);
}


