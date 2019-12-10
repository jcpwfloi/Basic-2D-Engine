#include "GMatrix.h"
#include "GShader.h"
#include "GBitmap.h"
#include "GMath.h"
#include "alib.h"
#include "ColorOps.h"

class ProxyShader : public GShader {
private:
  GShader* fRealShader;
  GMatrix  fExtraTransform;
public:
  ProxyShader(GShader* shader, const GMatrix& extraTransform)
    : fRealShader(shader), fExtraTransform(extraTransform) {}

  bool isOpaque() override {
    return fRealShader -> isOpaque();
  }

  bool setContext(const GMatrix& ctm) override {
    GMatrix m;
    m.setConcat(ctm, fExtraTransform);
    return fRealShader -> setContext(m);
  }

  void shadeRow(int x, int y, int count, GPixel row[]) override {
    fRealShader -> shadeRow(x, y, count, row);
  }
};


std::unique_ptr<GShader> GCreateProxyShader(GShader *s, const GMatrix& m, GShader::TileMode mode = GShader::kClamp) {
  return std::unique_ptr<GShader>(new ProxyShader(s, m));
}

GShader* CProxyShader(GShader *s, const GMatrix& m, GShader::TileMode mode = GShader::kClamp) {
  return new ProxyShader(s, m);
}


