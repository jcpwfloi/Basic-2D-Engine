#include "Canvas.h"

std::unique_ptr<GCanvas> GCreateCanvas(const GBitmap& device) {
  if (!device.pixels()) {
    return nullptr;
  }
  return std::unique_ptr<GCanvas>(new Canvas(device));
}

void GDrawSomething_rects(GCanvas* canvas) {
}
