#ifndef SNV_PORT_H
#define SNV_PORT_H

#include <ft2build.h>
#include <freetype/freetype.h>

#include <include/core/SkRefCnt.h>
#include <include/core/SkDrawable.h>

  /* Different hook functions can persist data by creating a state structure
   * and putting its address in `library->svg_renderer_state'. Functions can
   * store and retrieve data from this structure
   */
  typedef struct Snv_Port_StateRec_
  {
    double             x;
    double             y;
    SkDrawable *skDrawable;
  } Snv_Port_StateRec;

  typedef struct Snv_Port_StateRec_*  Snv_Port_State;

  FT_Error
  snv_port_init( FT_Library  library );

  void
  snv_port_free( FT_Library  library  );

  FT_Error
  snv_port_render( FT_GlyphSlot slot );

  FT_Error
  snv_port_preset_slot( FT_GlyphSlot  slot, FT_Bool  cache );

#endif
