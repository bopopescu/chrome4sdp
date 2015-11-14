// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "skia/ext/pixel_ref_utils.h"

#include <algorithm>

#include "third_party/skia/include/core/SkBitmapDevice.h"
#include "third_party/skia/include/core/SkCanvas.h"
#include "third_party/skia/include/core/SkData.h"
#include "third_party/skia/include/core/SkDraw.h"
#include "third_party/skia/include/core/SkPath.h"
#include "third_party/skia/include/core/SkPixelRef.h"
#include "third_party/skia/include/core/SkRRect.h"
#include "third_party/skia/include/core/SkRect.h"
#include "third_party/skia/include/core/SkShader.h"
#include "third_party/skia/include/core/SkTextBlob.h"
#include "third_party/skia/include/utils/SkNoSaveLayerCanvas.h"
#include "third_party/skia/src/core/SkRasterClip.h"

namespace skia {

namespace {

class DiscardablePixelRefSet {
 public:
  DiscardablePixelRefSet(
      std::vector<PixelRefUtils::PositionPixelRef>* pixel_refs)
      : pixel_refs_(pixel_refs) {}

  void Add(SkPixelRef* pixel_ref,
           const SkRect& rect,
           const SkMatrix& matrix,
           SkFilterQuality filter_quality) {
    // We should only be saving discardable pixel refs.
    SkASSERT(pixel_ref);
    SkASSERT(pixel_ref->isLazyGenerated());

    PixelRefUtils::PositionPixelRef position_pixel_ref;
    position_pixel_ref.pixel_ref = pixel_ref;
    position_pixel_ref.pixel_ref_rect = rect;
    position_pixel_ref.matrix = matrix;
    position_pixel_ref.filter_quality = filter_quality;
    pixel_refs_->push_back(position_pixel_ref);
  }

 private:
  std::vector<PixelRefUtils::PositionPixelRef>* pixel_refs_;
};

class GatherPixelRefDevice : public SkBitmapDevice {
 public:
  GatherPixelRefDevice(const SkBitmap& bm,
                       DiscardablePixelRefSet* pixel_ref_set)
      : SkBitmapDevice(bm), pixel_ref_set_(pixel_ref_set) {}

  void drawPaint(const SkDraw& draw, const SkPaint& paint) override {
    SkBitmap bitmap;
    if (GetBitmapFromPaint(paint, &bitmap)) {
      SkRect clip_rect = SkRect::Make(draw.fRC->getBounds());
      AddBitmap(bitmap, clip_rect, *draw.fMatrix, paint.getFilterQuality());
    }
  }

  void drawPoints(const SkDraw& draw,
                  SkCanvas::PointMode mode,
                  size_t count,
                  const SkPoint points[],
                  const SkPaint& paint) override {
    SkBitmap bitmap;
    if (!GetBitmapFromPaint(paint, &bitmap))
      return;

    if (count == 0)
      return;

    SkPoint min_point = points[0];
    SkPoint max_point = points[0];
    for (size_t i = 1; i < count; ++i) {
      const SkPoint& point = points[i];
      min_point.set(std::min(min_point.x(), point.x()),
                    std::min(min_point.y(), point.y()));
      max_point.set(std::max(max_point.x(), point.x()),
                    std::max(max_point.y(), point.y()));
    }

    SkRect bounds = SkRect::MakeLTRB(
        min_point.x(), min_point.y(), max_point.x(), max_point.y());

    GatherPixelRefDevice::drawRect(draw, bounds, paint);
  }
  void drawRect(const SkDraw& draw,
                const SkRect& rect,
                const SkPaint& paint) override {
    SkBitmap bitmap;
    if (GetBitmapFromPaint(paint, &bitmap)) {
      SkRect mapped_rect;
      draw.fMatrix->mapRect(&mapped_rect, rect);
      if (mapped_rect.intersects(SkRect::Make(draw.fRC->getBounds()))) {
        AddBitmap(bitmap, mapped_rect, *draw.fMatrix, paint.getFilterQuality());
      }
    }
  }
  void drawOval(const SkDraw& draw,
                const SkRect& rect,
                const SkPaint& paint) override {
    GatherPixelRefDevice::drawRect(draw, rect, paint);
  }
  void drawRRect(const SkDraw& draw,
                 const SkRRect& rect,
                 const SkPaint& paint) override {
    GatherPixelRefDevice::drawRect(draw, rect.rect(), paint);
  }
  void drawPath(const SkDraw& draw,
                const SkPath& path,
                const SkPaint& paint,
                const SkMatrix* pre_path_matrix,
                bool path_is_mutable) override {
    SkBitmap bitmap;
    if (!GetBitmapFromPaint(paint, &bitmap))
      return;

    SkRect path_bounds = path.getBounds();
    SkRect final_rect;
    if (pre_path_matrix != NULL)
      pre_path_matrix->mapRect(&final_rect, path_bounds);
    else
      final_rect = path_bounds;

    GatherPixelRefDevice::drawRect(draw, final_rect, paint);
  }
  void drawBitmap(const SkDraw& draw,
                  const SkBitmap& bitmap,
                  const SkMatrix& matrix,
                  const SkPaint& paint) override {
    SkMatrix total_matrix;
    total_matrix.setConcat(*draw.fMatrix, matrix);

    SkRect bitmap_rect = SkRect::MakeWH(bitmap.width(), bitmap.height());
    SkRect mapped_rect;
    total_matrix.mapRect(&mapped_rect, bitmap_rect);
    AddBitmap(bitmap, mapped_rect, total_matrix, paint.getFilterQuality());

    SkBitmap paint_bitmap;
    if (GetBitmapFromPaint(paint, &paint_bitmap)) {
      AddBitmap(paint_bitmap, mapped_rect, total_matrix,
                paint.getFilterQuality());
    }
  }
  void drawBitmapRect(const SkDraw& draw,
                      const SkBitmap& bitmap,
                      const SkRect* src_or_null,
                      const SkRect& dst,
                      const SkPaint& paint,
                      SkCanvas::SrcRectConstraint) override {
    SkRect bitmap_rect = SkRect::MakeWH(bitmap.width(), bitmap.height());
    SkMatrix matrix;
    matrix.setRectToRect(bitmap_rect, dst, SkMatrix::kFill_ScaleToFit);
    GatherPixelRefDevice::drawBitmap(draw, bitmap, matrix, paint);
  }
  void drawSprite(const SkDraw& draw,
                  const SkBitmap& bitmap,
                  int x,
                  int y,
                  const SkPaint& paint) override {
    // Sprites aren't affected by current matrix, so we can't reuse drawRect.
    SkMatrix matrix;
    matrix.setTranslate(x, y);

    SkRect bitmap_rect = SkRect::MakeWH(bitmap.width(), bitmap.height());
    SkRect mapped_rect;
    matrix.mapRect(&mapped_rect, bitmap_rect);

    SkMatrix identity;
    identity.setIdentity();
    // Sprites aren't affected by current matrix, so use the identity matrix.
    AddBitmap(bitmap, mapped_rect, identity, paint.getFilterQuality());
    SkBitmap paint_bitmap;
    if (GetBitmapFromPaint(paint, &paint_bitmap))
      AddBitmap(paint_bitmap, mapped_rect, identity, paint.getFilterQuality());
  }
  void drawImage(const SkDraw& draw,
                 const SkImage* image,
                 SkScalar x,
                 SkScalar y,
                 const SkPaint& paint) override {
    const SkMatrix image_matrix = SkMatrix::MakeTrans(x, y);
    DrawImageInternal(draw, image, image_matrix, paint);
  }
  void drawImageRect(const SkDraw& draw,
                     const SkImage* image,
                     const SkRect* src_or_null,
                     const SkRect& dst,
                     const SkPaint& paint,
                     SkCanvas::SrcRectConstraint) override {
    const SkRect src = src_or_null
        ? *src_or_null
        : SkRect::MakeIWH(image->width(), image->height());
    const SkMatrix image_matrix =
        SkMatrix::MakeRectToRect(src, dst, SkMatrix::kFill_ScaleToFit);
    DrawImageInternal(draw, image, image_matrix, paint);
  }
  void drawText(const SkDraw& draw,
                const void* text,
                size_t len,
                SkScalar x,
                SkScalar y,
                const SkPaint& paint) override {
    SkBitmap bitmap;
    if (!GetBitmapFromPaint(paint, &bitmap))
      return;

    // Math is borrowed from SkBBoxRecord
    SkRect bounds;
    paint.measureText(text, len, &bounds);
    SkPaint::FontMetrics metrics;
    paint.getFontMetrics(&metrics);

    if (paint.isVerticalText()) {
      SkScalar h = bounds.fBottom - bounds.fTop;
      if (paint.getTextAlign() == SkPaint::kCenter_Align) {
        bounds.fTop -= h / 2;
        bounds.fBottom -= h / 2;
      }
      bounds.fBottom += metrics.fBottom;
      bounds.fTop += metrics.fTop;
    } else {
      SkScalar w = bounds.fRight - bounds.fLeft;
      if (paint.getTextAlign() == SkPaint::kCenter_Align) {
        bounds.fLeft -= w / 2;
        bounds.fRight -= w / 2;
      } else if (paint.getTextAlign() == SkPaint::kRight_Align) {
        bounds.fLeft -= w;
        bounds.fRight -= w;
      }
      bounds.fTop = metrics.fTop;
      bounds.fBottom = metrics.fBottom;
    }

    SkScalar pad = (metrics.fBottom - metrics.fTop) / 2;
    bounds.fLeft -= pad;
    bounds.fRight += pad;
    bounds.fLeft += x;
    bounds.fRight += x;
    bounds.fTop += y;
    bounds.fBottom += y;

    GatherPixelRefDevice::drawRect(draw, bounds, paint);
  }
  void drawPosText(const SkDraw& draw,
                   const void* text,
                   size_t len,
                   const SkScalar pos[],
                   int scalars_per_pos,
                   const SkPoint& offset,
                   const SkPaint& paint) override {
    SkBitmap bitmap;
    if (!GetBitmapFromPaint(paint, &bitmap))
      return;

    if (len == 0)
      return;

    // Similar to SkDraw asserts.
    SkASSERT(scalars_per_pos == 1 || scalars_per_pos == 2);

    SkPoint min_point = SkPoint::Make(offset.x() + pos[0],
                                      offset.y() + (2 == scalars_per_pos ? pos[1] : 0));
    SkPoint max_point = min_point;

    for (size_t i = 0; i < len; ++i) {
      SkScalar x = offset.x() + pos[i * scalars_per_pos];
      SkScalar y = offset.y() + (2 == scalars_per_pos ? pos[i * scalars_per_pos + 1] : 0);

      min_point.set(std::min(x, min_point.x()), std::min(y, min_point.y()));
      max_point.set(std::max(x, max_point.x()), std::max(y, max_point.y()));
    }

    SkRect bounds = SkRect::MakeLTRB(
        min_point.x(), min_point.y(), max_point.x(), max_point.y());

    // Math is borrowed from SkBBoxRecord
    SkPaint::FontMetrics metrics;
    paint.getFontMetrics(&metrics);

    bounds.fTop += metrics.fTop;
    bounds.fBottom += metrics.fBottom;

    SkScalar pad = (metrics.fTop - metrics.fBottom) / 2;
    bounds.fLeft += pad;
    bounds.fRight -= pad;

    GatherPixelRefDevice::drawRect(draw, bounds, paint);
  }
  void drawTextOnPath(const SkDraw& draw,
                      const void* text,
                      size_t len,
                      const SkPath& path,
                      const SkMatrix* matrix,
                      const SkPaint& paint) override {
    SkBitmap bitmap;
    if (!GetBitmapFromPaint(paint, &bitmap))
      return;

    // Math is borrowed from SkBBoxRecord
    SkRect bounds = path.getBounds();
    SkPaint::FontMetrics metrics;
    paint.getFontMetrics(&metrics);

    SkScalar pad = metrics.fTop;
    bounds.fLeft += pad;
    bounds.fRight -= pad;
    bounds.fTop += pad;
    bounds.fBottom -= pad;

    GatherPixelRefDevice::drawRect(draw, bounds, paint);
  }
  void drawTextBlob(const SkDraw& draw,
                    const SkTextBlob* blob,
                    SkScalar x, SkScalar y,
                    const SkPaint& paint,
                    SkDrawFilter*) override {
    SkBitmap bitmap;
    if (!GetBitmapFromPaint(paint, &bitmap))
      return;

    const SkRect bounds = blob->bounds().makeOffset(x, y);
    GatherPixelRefDevice::drawRect(draw, bounds, paint);
  }

  void drawVertices(const SkDraw& draw,
                    SkCanvas::VertexMode,
                    int vertex_count,
                    const SkPoint verts[],
                    const SkPoint texs[],
                    const SkColor colors[],
                    SkXfermode* xmode,
                    const uint16_t indices[],
                    int index_count,
                    const SkPaint& paint) override {
    GatherPixelRefDevice::drawPoints(
        draw, SkCanvas::kPolygon_PointMode, vertex_count, verts, paint);
  }
  void drawDevice(const SkDraw&,
                  SkBaseDevice*,
                  int x,
                  int y,
                  const SkPaint&) override {}

 protected:
  bool onReadPixels(const SkImageInfo& info,
                    void* pixels,
                    size_t rowBytes,
                    int x,
                    int y) override {
    return false;
  }

  bool onWritePixels(const SkImageInfo& info,
                     const void* pixels,
                     size_t rowBytes,
                     int x,
                     int y) override {
    return false;
  }

 private:
  DiscardablePixelRefSet* pixel_ref_set_;

  void AddBitmap(const SkBitmap& bm,
                 const SkRect& rect,
                 const SkMatrix& matrix,
                 SkFilterQuality filter_quality) {
    const SkRect canvas_rect = SkRect::Make(imageInfo().bounds());
    if (rect.intersects(canvas_rect) && bm.pixelRef()->isLazyGenerated()) {
      pixel_ref_set_->Add(bm.pixelRef(), rect, matrix, filter_quality);
    }
  }

  void AddImage(const SkImage* image,
                const SkRect& rect,
                const SkMatrix& matrix,
                SkFilterQuality filter_quality) {
    const SkRect canvas_rect = SkRect::Make(imageInfo().bounds());
    if (rect.intersects(canvas_rect) && image->isLazyGenerated()) {
      SkBitmap bm;
      if (image->asLegacyBitmap(&bm, SkImage::kRO_LegacyBitmapMode) && bm.pixelRef()) {
        pixel_ref_set_->Add(bm.pixelRef(), rect, matrix, filter_quality);
      }
    }
  }

  bool GetBitmapFromPaint(const SkPaint& paint, SkBitmap* bm) {
    SkShader* shader = paint.getShader();
    if (shader) {
      // Check whether the shader is a gradient in order to prevent generation
      // of bitmaps from gradient shaders, which implement asABitmap.
      if (SkShader::kNone_GradientType == shader->asAGradient(NULL))
        return shader->asABitmap(bm, NULL, NULL);
    }
    return false;
  }

  void DrawImageInternal(const SkDraw& draw,
                         const SkImage* image,
                         const SkMatrix& matrix,
                         const SkPaint& paint) {
      const SkMatrix total_matrix = SkMatrix::Concat(*draw.fMatrix, matrix);
      const SkRect image_rect = SkRect::MakeIWH(image->width(), image->height());
      SkRect mapped_rect;
      total_matrix.mapRect(&mapped_rect, image_rect);
      AddImage(image, mapped_rect, total_matrix, paint.getFilterQuality());

      SkBitmap paint_bitmap;
      if (GetBitmapFromPaint(paint, &paint_bitmap)) {
        AddBitmap(paint_bitmap, mapped_rect, total_matrix,
                  paint.getFilterQuality());
      }
  }
};

}  // namespace

void PixelRefUtils::GatherDiscardablePixelRefs(
    SkPicture* picture,
    std::vector<PositionPixelRef>* pixel_refs) {
  pixel_refs->clear();
  DiscardablePixelRefSet pixel_ref_set(pixel_refs);

  SkRect picture_bounds = picture->cullRect();
  SkIRect picture_ibounds = picture_bounds.roundOut();
  SkBitmap empty_bitmap;
  // Use right/bottom as the size so that we don't need a translate and, as a
  // result, the information is returned relative to the picture's origin.
  empty_bitmap.setInfo(SkImageInfo::MakeUnknown(picture_ibounds.right(),
                                                picture_ibounds.bottom()));

  GatherPixelRefDevice device(empty_bitmap, &pixel_ref_set);
  SkNoSaveLayerCanvas canvas(&device);

  canvas.drawPicture(picture);
}

}  // namespace skia