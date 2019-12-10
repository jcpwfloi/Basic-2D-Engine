#include "GCanvas.h"
#include "GShader.h"
#include "GMatrix.h"
#include "GEdge.h"
#include "GPath.h"
#include "GBitmap.h"
#include "alib.h"
#include <vector>
#include <stack>

class Canvas : public GCanvas {
  public:
    Canvas(const GBitmap& device) : fDevice(device) {
      curr_m = GMatrix();
      matrix_stack.push(curr_m);
    }

    void save() override {
      matrix_stack.push(curr_m);
    }
    void restore() override {
      if (matrix_stack.size() == 1) return;
      curr_m = matrix_stack.top();
      matrix_stack.pop();
    }

    void concat(const GMatrix& m) override {
      curr_m.setConcat(curr_m, m);
    }

    void drawPaint(const GPaint& color) override {
      GShader *shader = color.getShader();
      if (shader != nullptr) {
        shader -> setContext(curr_m);
      }
      for (int y = 0; y < fDevice.height(); ++ y)
        if (shader != nullptr) {
          shader -> shadeRow(0, y, fDevice.width() - 1, getPixel(0, y));
        } else for (int x = 0; x < fDevice.width(); ++ x) 
          paint(x, y, color);
    }

    void drawRect(const GRect& rect, const GPaint& color) override {
      float top = rect.top();
      float bottom = rect.bottom();
      float left = rect.left();
      float right = rect.right();

      GPoint pts[4];
      pts[0] = { left, top };
      pts[1] = { right, top };
      pts[2] = { right, bottom };
      pts[3] = { left, bottom };
      drawConvexPolygon(pts, 4, color);
    }

    static void ppoint(const GPoint& a) {
      printf("p (%f %f)\n", a.x(), a.y());
    }

    void drawPath(const GPath&, const GPaint&) override;

    void clip_edge(const GPoint& a, const GPoint& b, std::vector<GEdge>& ret, int dir = 1) {
      GPoint top, bottom;
      if (a.y() - b.y() < 0) top.set(a.x(), a.y()), bottom.set(b.x(), b.y());
      else top.set(b.x(), b.y()), bottom.set(a.x(), a.y());

      if (bottom.y() < 0 || top.y() > 0.0 + fDevice.height()) return;

      GEdge ans = GEdge(top, bottom, dir);

      if (ans.bottom <= ans.top) return;

      if (top.y() < 0) {
        top.set(top.x() + (0 - top.y()) * ans.slope, 0);
      }

      if (bottom.y() > fDevice.height()) {
        bottom.set(bottom.x() + (fDevice.height() - bottom.y()) * ans.slope, fDevice.height());
      }

      if (top.x() <= 0 && bottom.x() <= 0) {
        top.set(0, top.y());
        bottom.set(0, bottom.y());
        ret.push_back(GEdge(top, bottom, dir));
        return;
      }

      if (top.x() >= fDevice.width() && bottom.x() >= fDevice.width()) {
        top.set(fDevice.width(), top.y());
        bottom.set(fDevice.width(), bottom.y());
        ret.push_back(GEdge(top, bottom, dir));
        return;
      }

      if (top.x() < 0) {
        GVector v = bottom - top;
        v = v * ((0 - top.x()) / (bottom.x() - top.x()));
        GPoint mid = top + v;
        top.set(0, top.y());
        ans = GEdge(top, mid, dir);
        if (ans.bottom > ans.top) ret.push_back(ans);
        top = mid;
      }

      if (bottom.x() < 0) {
        GVector v = top - bottom;
        v = v * ((0 - bottom.x()) / (top.x() - bottom.x()));
        GPoint mid = bottom + v;
        bottom.set(0, bottom.y());
        ans = GEdge(mid, bottom, dir);
        if (ans.bottom > ans.top) ret.push_back(ans);
        bottom = mid;
      }

      if (top.x() >= fDevice.width()) {
        GVector v = bottom - top;
        v = v * ((top.x() - (0.0  + fDevice.width())) / (top.x() - bottom.x()));
        GPoint mid = top + v;
        top.set(fDevice.width(), top.y());
        ans = GEdge(top, mid, dir);
        if (ans.bottom > ans.top) ret.push_back(ans);
        top = mid;
      }

      if (bottom.x() >= fDevice.width()) {
        GVector v = top - bottom;
        v = v * ((bottom.x() - (0.0 + fDevice.width())) / (bottom.x() - top.x()));
        GPoint mid = bottom + v;
        bottom.set(fDevice.width(), bottom.y());
        ans = GEdge(mid, bottom, dir);
        if (ans.bottom > ans.top) ret.push_back(ans);
        bottom = mid;
      }

      ans = GEdge(top, bottom, dir);
      if (ans.bottom > ans.top) ret.push_back(ans);
    }

    std::vector<GEdge> clip(const GPoint* pts, int count) {
      std::vector<GEdge> ans;
      for (int i = 0; i < count - 1; ++ i) {
        clip_edge(pts[i], pts[i + 1], ans);
      }
      clip_edge(pts[0], pts[count - 1], ans);
      return ans;
    }

    void blit(int y, int left, int right, const GPaint &color) {
      for (int x = std::max(left, 0); x < std::min(right, fDevice.width()); ++ x) {
        paint(x, y, color);
      }
    }

    void drawConvexPolygon(const GPoint* ps, int count, const GPaint &color) override {
      if (count < 3) return;
      GPoint* pts = new GPoint[count];
      curr_m.mapPoints(pts, ps, count);

      if (color.getShader() != nullptr) {
        color.getShader() -> setContext(curr_m);
      }

      std::vector<GEdge> clipped = clip(pts, count);
      std::sort(clipped.begin(), clipped.end());

#define foreach(i, x) for (__typeof(x.begin()) i = x.begin(); i != x.end(); ++ i)

      if ((int) clipped.size() < 2) return;

      int curr_y = clipped[0].top;

      int real_bottom = 0;
      foreach(i, clipped) real_bottom = std::max(real_bottom, i -> bottom);

      int left = 0, right = 1, next = 2, t;
      while (left < (int) clipped.size() && right < (int) clipped.size() && curr_y < real_bottom) {
        if (clipped[left].curr_x > clipped[right].curr_x) {
          t = left;
          left = right;
          right = t;
        }

        int l = GRoundToInt(clipped[left].curr_x),
            r = GRoundToInt(clipped[right].curr_x);

        if (r - l > 0) switch_blit(getPixel(l, curr_y), l, curr_y, r - l, color);
        //raw_blit(curr_y, GRoundToInt(clipped[left].curr_x), GRoundToInt(clipped[right].curr_x), color);
        clipped[left].inc();
        clipped[right].inc();

        ++ curr_y;
        if (curr_y >= clipped[left].bottom) left = next ++;
        if (curr_y >= clipped[right].bottom) right = next ++;
      }

      delete[] pts;
    }

    void drawTriangle(const GPoint pts[3], const GColor colors[3], const GPoint texs[3], const GPaint&);
    void drawQuad(const GPoint verts[4], const GColor colors[4], const GPoint texs[4],
                              int level, const GPaint&) override;
    void drawMesh(const GPoint verts[], const GColor colors[], const GPoint texs[],
                              int count, const int indices[], const GPaint& paint) override;

  private:
    const GBitmap fDevice;

    std::stack<GMatrix> matrix_stack;
    GMatrix curr_m;

    inline void paint(int x, int y, const GPaint& color) {
      assert(0 <= x && x < fDevice.width());
      assert(0 <= y && y < fDevice.height());
      *getPixel(x, y) = blend(color, *getPixel(x, y));
    }

    inline GPixel* getPixel(int x, int y) {
      return fDevice.getAddr(x, y);
    }

    static inline GRect intersect(const GRect &a, const GRect &b) {
      GRect ans;
      ans.setLTRB(std::max(a.left(), b.left()),
          std::max(a.top(), b.top()),
          std::min(a.right(), b.right()),
          std::min(a.bottom(), b.bottom()));
      return ans;
    }
    std::unique_ptr<GShader> final_createRadialGradient(GPoint center, float radius,
                                                                    const GColor colors[], int count,
                                                                    GShader::TileMode mode) override;

};

std::unique_ptr<GCanvas> GCreateCanvas(const GBitmap& device);

void GDrawSomething_rects(GCanvas* canvas);

