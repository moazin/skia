#include <iostream>
#include <include/core/SkCanvas.h>
#include <include/core/SkSurface.h>
#include <include/docs/SkPDFDocument.h>
#include <include/core/SkPaint.h>
#include <include/core/SkPath.h>
#include <include/core/SkPictureRecorder.h>
#include <include/core/SkDrawable.h>

#include <include/core/SkTypeface.h>
#include <include/core/SkFont.h>

int main(void)
{
  sk_sp<SkTypeface> typeface = SkTypeface::MakeFromFile((const char*)"fonts/Trajan.otf", 0);
  SkFont font(typeface, 500);
  SkPaint paint;
  //paint.setStyle(SkPaint::kFill_Style);
  paint.setAntiAlias(true);

#define PNG
#ifdef PNG
  auto skRasterSurface = SkSurface::MakeRaster(SkImageInfo::Make(2000, 2000, kBGRA_8888_SkColorType, kPremul_SkAlphaType));
  auto skRasterCanvas = skRasterSurface->getCanvas();
#else
  SkFILEWStream pdfStream("out.pdf");
  auto skRasterSurface = SkPDF::MakeDocument(&pdfStream);
  SkCanvas *skRasterCanvas = skRasterSurface->beginPage(SkIntToScalar(2000), SkIntToScalar(2000));
#endif

  skRasterCanvas->clear(SK_ColorWHITE);
  skRasterCanvas->drawString(std::string("AHMED").c_str(), 40, 1000, font, paint);

#ifdef PNG
  auto skImage = skRasterSurface->makeImageSnapshot();
  if (!skImage)
      return 0;
  sk_sp<SkData> pngData(skImage->encodeToData(SkEncodedImageFormat::kPNG, 100));
  if (!pngData)
      return 0;

  SkFILEWStream out("output.png");
  (void)out.write(pngData->data(), pngData->size());
#else
  skRasterSurface->close();
#endif
  return 0;
}
