#include "Canvas.h"

class RadialGradientShader : public GShader {
  public:
    RadialGradientShader(const GPoint &center, float radius, const GColor* arr, int count, TileMode mode)
      : center(center), radius(radius), count(count), mode(mode) {
        colors = new GColor[count];
        for (int i = 0; i < count; ++ i)
          colors[i] = arr[i];

        bool ret = setContext(GMatrix());
    }

    ~RadialGradientShader() {
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
      GPoint pt = fInv.mapXY(x, y);
      GVector v = { fInv[0], fInv[3] };
      for (int i = 0; i < count; ++ i) {
        row[i] = getPixel(pt.x(), pt.y());
        pt += v;
      }
    }
  private:
    int count;
    GPoint center;
    float radius;
    GColor *colors;

    GMatrix local, fInv;

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
      GPoint p = { x, y };
      float px = (p - center).length() / radius;
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

std::unique_ptr<GShader> Canvas::final_createRadialGradient(GPoint center, float radius, const GColor c[], int count, GShader::TileMode mode) {
  return std::unique_ptr<GShader>(new RadialGradientShader(center, radius, c, count, mode));
}
