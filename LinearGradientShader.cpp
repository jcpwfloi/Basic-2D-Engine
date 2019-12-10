#include "GShader.h"
#include "GMatrix.h"
#include "alib.h"

class LinearGradientShader : public GShader {
  public:
    LinearGradientShader(const GPoint &p0, const GPoint &p1, const GColor* arr, int count, TileMode mode)
      : p0(p0), p1(p1), count(count), mode(mode) {
        colors = new GColor[count];
        for (int i = 0; i < count; ++ i)
          colors[i] = arr[i];

        GVector v = p1 - p0;
        trans.set6(v.x(), -v.y(), p0.x(), v.y(), v.x(), p0.y());
        trans.invert(&trans);
    }

    ~LinearGradientShader() {
      delete[] colors;
    }

    bool isOpaque() {
      return false;
    }

    bool setContext(const GMatrix& ctm) {
      fInv.setConcat(ctm, local);
      bool ret = fInv.invert(&fInv);
      return ret;
    }

    void shadeRow(int x, int y, int count, GPixel row[]) {
      GPoint pt = fInv.mapXY(x + 0.5, y + 0.5);
      GVector v = { fInv[0], fInv[3] };
      for (int i = 0; i < count; ++ i) {
        row[i] = getPixel(pt.x(), pt.y());
        pt += v;
      }
    }
  private:
    int count;
    GPoint p0, p1;
    GColor *colors;

    GMatrix local, fInv;
    GMatrix trans;

    TileMode mode;

    float pin(float x) {
      if (mode == kClamp)
        return std::min(std::max(0.0f, x), 1.0f);
      else if (mode == kRepeat) {
        return x - GFloorToInt(x);
      } else {
        float xx = (x / 2 - GFloorToInt(x / 2)) * 2;
        xx = xx > 1.0f ? 2.0f - xx : xx;
        return xx;
      }
    }

    GPixel getPixel(float x, float y) {
      GPoint p = trans.mapXY(x, y);
      float px = p.x();
      px = pin(px);
      //px = std::max(0.0f, std::min(px, 1.0f));
      px *= count - 1;
      int left = (int) px;
      int right = left + 1;
      if (right >= count) right = count - 1;
      float w = px - left;
      return colorToPixel(getColor(w, colors[left], colors[right]));
    }

    GColor getColor(float w, GColor left, GColor right) {
      return {
        w * right.fA + (1 - w) * left.fA,
        w * right.fR + (1 - w) * left.fR,
        w * right.fG + (1 - w) * left.fG,
        w * right.fB + (1 - w) * left.fB
      };
    }
};

std::unique_ptr<GShader> GCreateLinearGradient(GPoint p0, GPoint p1, const GColor* c, int count, GShader::TileMode mode) {
  return std::unique_ptr<GShader>(new LinearGradientShader(p0, p1, c, count, mode));
}
