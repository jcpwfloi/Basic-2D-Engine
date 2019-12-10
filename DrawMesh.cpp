#include "Canvas.h"
#include "ColorOps.h"

std::unique_ptr<GShader> GCreateTriShader(const GColor c[3], const GPoint p[3], GShader::TileMode mode = GShader::kClamp);
std::unique_ptr<GShader> GCreateProxyShader(GShader*, const GMatrix&, GShader::TileMode mode = GShader::kClamp);
std::unique_ptr<GShader> GCreateCompositeShader(GShader *s, GShader *t, GShader::TileMode mode = GShader::kClamp);

GShader* CTriShader(const GColor c[3], const GPoint p[3], GShader::TileMode mode = GShader::kClamp);
GShader* CProxyShader(GShader*, const GMatrix&, GShader::TileMode mode = GShader::kClamp);

GMatrix compute_basis(const GPoint &p0, const GPoint &p1, const GPoint &p2) {
  GVector u = p1 - p0;
  GVector v = p2 - p0;
  return {
    u.x(), v.x(), p0.x(),
    u.y(), v.y(), p0.y()
  };
}

void Canvas::drawTriangle(const GPoint pts[3], const GColor colors[3], const GPoint texs[3], const GPaint& paint) {
  std::unique_ptr<GShader> s;

  if (colors != nullptr && texs != nullptr) {
    GMatrix P, T, invT;
    P = compute_basis(pts[0], pts[1], pts[2]);
    T = compute_basis(texs[0], texs[1], texs[2]);
    T.invert(&invT);
    T.setConcat(P, invT);
    s = GCreateCompositeShader(
        CTriShader(colors, pts),
        CProxyShader(paint.getShader(), T)
    );
  }
  else if (colors != nullptr) {
    s = GCreateTriShader(colors, pts);
  }
  else if (texs != nullptr) {
    GMatrix P, T, invT;
    P = compute_basis(pts[0], pts[1], pts[2]);
    T = compute_basis(texs[0], texs[1], texs[2]);
    T.invert(&invT);
    T.setConcat(P, invT);
    s = GCreateProxyShader(paint.getShader(), T);
  }

  GPaint p;
  p.setShader(s.get());

  drawConvexPolygon(pts, 3, p);
}

void Canvas::drawMesh(const GPoint verts[], const GColor colors[], const GPoint texs[],
                              int count, const int indices[], const GPaint& paint) {
  GPoint pts[3], *tx = nullptr;
  GColor *co = nullptr;
  for (int i = 0, n = 0; i < count; ++ i, n += 3) {
    pts[0] = verts[indices[n + 0]];
    pts[1] = verts[indices[n + 1]];
    pts[2] = verts[indices[n + 2]];

    if (colors != nullptr) {
      co = new GColor[3];
      co[0] = colors[indices[n + 0]];
      co[1] = colors[indices[n + 1]];
      co[2] = colors[indices[n + 2]];
    }

    if (texs != nullptr) {
      tx = new GPoint[3];
      tx[0] = texs[indices[n + 0]];
      tx[1] = texs[indices[n + 1]];
      tx[2] = texs[indices[n + 2]];
    }

    drawTriangle(pts, co, tx, paint);
    delete[] co;
    delete[] tx;
  }
}

GPoint mid_quad(const GPoint p[4], float u, float v) {
  return p[0] * ((1 - u) * (1 - v)) + p[1] * ((1 - v) * u) + p[3] * ((1 - u) * v) + p[2] * (u * v);
}

GColor mid_quad(const GColor p[4], float u, float v) {
  return p[0] * ((1 - u) * (1 - v)) + p[1] * ((1 - v) * u) + p[3] * ((1 - u) * v) + p[2] * (u * v);
}

void Canvas::drawQuad(const GPoint verts[4], const GColor colors[4], const GPoint texs[4],
                              int level, const GPaint& paint) {
  int row_count = level + 2;
  GPoint *v, *t = nullptr;
  GColor *c = nullptr;

  int *indices = new int[(row_count - 1) * (row_count - 1) * 2 * 3];

  v = new GPoint[row_count * row_count];
  if (colors != nullptr) c = new GColor[row_count * row_count];
  if (texs != nullptr) t = new GPoint[row_count * row_count];

  for (int x = 0; x < row_count; ++ x)
    for (int y = 0; y < row_count; ++ y) {
      v[x * row_count + y] = mid_quad(verts, 1.0f * x / (row_count - 1), 1.0f * y / (row_count - 1));
      if (colors != nullptr) c[x * row_count + y] = mid_quad(colors, 1.0f * x / (row_count - 1), 1.0f * y / (row_count - 1));
      if (texs != nullptr) t[x * row_count + y] = mid_quad(texs, 1.0f * x / (row_count - 1), 1.0f * y / (row_count - 1));
    }

  int cnt = 0;

  for (int x = 0; x < row_count - 1; ++ x)
    for (int y = 0; y < row_count - 1; ++ y) {
      indices[cnt ++] = x * row_count + y;
      indices[cnt ++] = x * row_count + y + 1;
      indices[cnt ++] = (x + 1) * row_count + y;
      indices[cnt ++] = x * row_count + y + 1;
      indices[cnt ++] = (x + 1) * row_count + y;
      indices[cnt ++] = (x + 1) * row_count + y + 1;
    }

  drawMesh(v, c, t, (row_count - 1) * (row_count - 1) * 2, indices, paint);

  delete[] v;
  delete[] c;
  delete[] t;
  delete[] indices;
}

