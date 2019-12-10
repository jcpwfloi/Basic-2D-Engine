#include "GMatrix.h"
#include "GShader.h"
#include "GBitmap.h"
#include "GMath.h"
#include "alib.h"

class BitmapShader : public GShader {
  public:
    BitmapShader(const GBitmap& b, const GMatrix& m, TileMode mode) : bitmap(b), local(m), opaque(b.isOpaque()), mode(mode) {
      bool ret = setContext(GMatrix());
      assert(ret);

      GMatrix scale;
      scale.setScale(bitmap.width(), bitmap.height());

      local.setConcat(local, scale);
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
      GPoint pt = fInv.mapXY(x + 0.5, y + 0.5), t;
      GVector v = { fInv[0], fInv[3] };
      for (int i = 0; i < count; ++ i) {
        t = pin(pt);
        row[i] = *bitmap.getAddr(GFloorToInt(t.x() * (float) bitmap.width()), GFloorToInt(t.y() * (float) bitmap.height()));
        pt += v;
      }
    }
  private:
    GPoint pin(GPoint x) {
      if (mode == kClamp)
        return {
          std::min(std::max(0.0f, x.x()), 1.0f),
          std::min(std::max(0.0f, x.y()), 1.0f)
        };
      else if (mode == kRepeat) {
        return {
          x.x() - GFloorToInt(x.x()),
          x.y() - GFloorToInt(x.y())
        };
      } else {
        float xx = (x.x() / 2 - GFloorToInt(x.x() / 2)) * 2;
        xx = xx > 1.0f ? 2.0f - xx : xx;
        float yy = (x.y() / 2 - GFloorToInt(x.y() / 2)) * 2;
        yy = yy > 1.0f ? 2.0f - yy : yy;
        return {
          xx, yy
        };
      }
    }

    GBitmap bitmap;
    GMatrix local;
    GMatrix fInv;

    TileMode mode;

    bool opaque;
};

std::unique_ptr<GShader> GCreateBitmapShader(const GBitmap& b, const GMatrix& localM, GShader::TileMode mode) {
  return std::unique_ptr<GShader>(new BitmapShader(b, localM, mode));
}

