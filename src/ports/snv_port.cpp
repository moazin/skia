#include <src/ports/snv_port.h>
#include <ft2build.h>
#include <stdlib.h>
#include <math.h>
#include <string>
#include <cstring>

/*
#include <freetype/freetype.h>
#include <freetype/internal/ftobjs.h>
#include <freetype/ftbbox.h>
#include <freetype/otsvg.h>
*/

#include FT_FREETYPE_H
#include <freetype/internal/ftobjs.h>
#include <freetype/otsvg.h>
#include FT_BBOX_H


#include <svgnative/SVGDocument.h>
#include <svgnative/ports/skia/SkiaSVGRenderer.h>
#include <svgnative/SVGRenderer.h>

#include <include/core/SkImage.h>
#include <include/core/SkPictureRecorder.h>
#include <include/core/SkSurface.h>
#include <include/core/SkCanvas.h>

FT_Error
snv_port_init( FT_Library  library )
{
  /* allocate the memory upon initialization */
  library->svg_renderer_state = malloc( sizeof( Snv_Port_StateRec ) );
  return FT_Err_Ok;
}

void
snv_port_free( FT_Library  library )
{
  /* free the memory of the state structure */
  free( library->svg_renderer_state );
}

FT_Error
snv_port_render( FT_GlyphSlot slot )
{
  FT_Error         error = FT_Err_Ok;
  FT_SVG_Document  document       = (FT_SVG_Document)slot->other;
  FT_Size_Metrics  metrics        = document->metrics;
  FT_UShort        units_per_EM   = document->units_per_EM;
  FT_UShort        end_glyph_id   = document->end_glyph_id;
  FT_UShort        start_glyph_id = document->start_glyph_id;

  Snv_Port_State state = (Snv_Port_State)slot->library->svg_renderer_state;
  auto skImageInfo = SkImageInfo::Make(slot->bitmap.width, slot->bitmap.rows, kBGRA_8888_SkColorType, kPremul_SkAlphaType);
  auto surface = SkSurface::MakeRasterDirect(skImageInfo, slot->bitmap.buffer, slot->bitmap.pitch);
  auto canvas = surface->getCanvas();
  canvas->translate(-1 * state->x, -1 * state->y);
  state->skDrawable->draw(canvas);

  slot->bitmap.pixel_mode = FT_PIXEL_MODE_BGRA;
  slot->bitmap.num_grays  = 256;
  slot->format = FT_GLYPH_FORMAT_BITMAP;

  return FT_Err_Ok;
}

FT_Error
snv_port_preset_slot( FT_GlyphSlot  slot, FT_Bool  cache )
{
  FT_Error         error          = FT_Err_Ok;
  FT_SVG_Document  document       = (FT_SVG_Document)slot->other;
  FT_Size_Metrics  metrics        = document->metrics;
  FT_UShort        units_per_EM   = document->units_per_EM;
  FT_UShort        end_glyph_id   = document->end_glyph_id;
  FT_UShort        start_glyph_id = document->start_glyph_id;
  Snv_Port_StateRec state_dummy;
  Snv_Port_State state;

  if (cache)
    state = (Snv_Port_State)slot->library->svg_renderer_state;
  else
    state = &state_dummy;

  std::string svg_doc = "";
  for(int i = 0; i < document->svg_document_length; i++)
  {
    svg_doc += (char)document->svg_document[i];
  }

  auto renderer = std::make_shared<SVGNative::SkiaSVGRenderer>();
  auto snv_document = std::unique_ptr<SVGNative::SVGDocument>(SVGNative::SVGDocument::CreateSVGDocument(svg_doc.c_str(), renderer));
  if (!snv_document)
    return FT_Err_Invalid_SVG_Document;

  /* TODO: we need a mechanism to know if a ViewBox / Width / Height was provided or not */
  float doc_x = snv_document->X();
  float doc_y = snv_document->Y();
  float doc_width = snv_document->Width();
  float doc_height = snv_document->Height();

  float scale_x = ((float)metrics.x_ppem)/((float)doc_width);
  float scale_y = ((float)metrics.y_ppem)/((float)doc_height);

  /* TODO: Add transform here */

  SkPictureRecorder recorder;
  SkRect bnds{-5000, -5000, 5000, 5000};
  auto canvas = recorder.beginRecording(bnds);
  renderer->SetSkCanvas(canvas);
  auto transform = ((SVGNative::SVGRenderer*)renderer.get())->CreateTransform(scale_x, 0, 0, scale_y, 0, 0);
  SVGNative::Rect bounds = snv_document->Bounds(std::move(transform));
  canvas->scale(scale_x, scale_y);
  snv_document->Render();
  auto drawable = recorder.finishRecordingAsDrawable();

  state->x = bounds.x;
  state->y = bounds.y;

  state->skDrawable = drawable.release();

  if (!cache) {
    state->skDrawable->unref();
  }

  slot->bitmap_left   = state->x;
  slot->bitmap_top    = state->y * -1;
  slot->bitmap.rows   = ceil( bounds.height );
  slot->bitmap.width  = ceil( bounds.width );
  slot->bitmap.pitch  = slot->bitmap.width * 4;

  slot->bitmap.pixel_mode = FT_PIXEL_MODE_BGRA;

  return FT_Err_Ok;
}
