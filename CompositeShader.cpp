#include "GMatrix.h"
#include "GShader.h"
#include "GBitmap.h"
#include "GMath.h"
#include "alib.h"
#include "ColorOps.h"

class CompositeShader : public GShader {
private:
  GShader *shader1, *shader2;
public:
  CompositeShader(GShader* shader1, GShader* shader2)
    : shader1(shader1), shader2(shader2) {}

  bool isOpaque() override {
    return false;
  }

  bool setContext(const GMatrix& ctm) override {
    return shader1 -> setContext(ctm) &&
    shader2 -> setContext(ctm);
  }

  void shadeRow(int x, int y, int count, GPixel row[]) override {
    shader1 -> shadeRow(x, y, count, row);
    GPixel *temp;
    temp = new GPixel[count];
    shader2 -> shadeRow(x, y, count, temp);
    for (int i = 0; i < count; ++ i) {
      row[i] = mulTwo(row[i], temp[i]);
    }
    delete[] temp;
  }
};


std::unique_ptr<GShader> GCreateCompositeShader(GShader *s, GShader *t, GShader::TileMode mode = GShader::kClamp) {
  return std::unique_ptr<GShader>(new CompositeShader(s, t));
}


