#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <Ector.h>
#include "ector_private.h"
#include "ector_gl_private.h"
#include "ector_buffer.h"
#include "ector_gl_buffer_tex.eo.h"

#define MY_CLASS ECTOR_GL_BUFFER_TEX_CLASS

typedef struct
{
   Ector_Generic_Buffer_Data *generic;
   unsigned int tex;
   Eina_Bool nodel; // if true, don't delete this tex id, it belongs to evas
} Ector_Gl_Buffer_Tex_Data;

EOLIAN static Eina_Bool
_ector_gl_buffer_tex_attach(Eo *obj EINA_UNUSED, Ector_Gl_Buffer_Tex_Data *pd,
                            unsigned int tex, int width, int height,
                            int l, int r, int t, int b,
                            int tex_x, int tex_y, int tex_w, int tex_h,
                            int atlas_w, int atlas_h,
                            Efl_Gfx_Colorspace cspace)
{
   if (pd->generic->immutable)
     return EINA_FALSE;

   pd->nodel = EINA_TRUE;
   pd->tex = tex;
   pd->generic->cspace = cspace;
   pd->generic->w = width;
   pd->generic->h = height;

   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_ector_gl_buffer_tex_ector_generic_buffer_pixels_set(Eo *obj, Ector_Gl_Buffer_Tex_Data *pd,
                                                     void *pixels, int width, int height,
                                                     int stride, Efl_Gfx_Colorspace cspace,
                                                     Eina_Bool writable,
                                                     unsigned char l, unsigned char r,
                                                     unsigned char t, unsigned char b)
{
   if (pd->generic->immutable)
     return EINA_FALSE;

   GL.glGenTextures(1, &pd->tex);


   return EINA_TRUE;
}

EOLIAN static void *
_ector_gl_buffer_tex_ector_generic_buffer_map(Eo *obj, Ector_Gl_Buffer_Tex_Data *pd, unsigned int *length, Ector_Buffer_Access_Flag mode, unsigned int x, unsigned int y, unsigned int w, unsigned int h, Efl_Gfx_Colorspace cspace, unsigned int *stride)
{

}

EOLIAN static void
_ector_gl_buffer_tex_ector_generic_buffer_unmap(Eo *obj, Ector_Gl_Buffer_Tex_Data *pd, void *data, unsigned int length)
{

}

EOLIAN static uint8_t *
_ector_gl_buffer_tex_ector_generic_buffer_span_get(Eo *obj, Ector_Gl_Buffer_Tex_Data *pd, int x, int y, unsigned int w, Efl_Gfx_Colorspace cspace, unsigned int *length)
{

}

EOLIAN static void
_ector_gl_buffer_tex_ector_generic_buffer_span_free(Eo *obj, Ector_Gl_Buffer_Tex_Data *pd, uint8_t *data)
{

}

EOLIAN static Ector_Buffer_Flag
_ector_gl_buffer_tex_ector_generic_buffer_flags_get(Eo *obj EINA_UNUSED, Ector_Gl_Buffer_Tex_Data *pd)
{
   return ECTOR_BUFFER_FLAG_CPU_READABLE | ECTOR_BUFFER_FLAG_DRAWABLE;
}

EOLIAN static Eo_Base *
_ector_gl_buffer_tex_eo_base_constructor(Eo *obj, Ector_Gl_Buffer_Tex_Data *pd)
{
   EINA_SAFETY_ON_FALSE_RETURN_VAL(GL.init, NULL);
   eo_do_super(obj, MY_CLASS, obj = eo_constructor());
   pd->generic = eo_data_ref(obj, ECTOR_GENERIC_BUFFER_MIXIN);
   return obj;
}

EOLIAN static Eo_Base *
_ector_gl_buffer_tex_eo_base_finalize(Eo *obj, Ector_Gl_Buffer_Tex_Data *pd)
{
   if (!pd->tex)
     {
        CRI("FBO buffer not attached to any FBO");
        return NULL;
     }
   pd->generic->immutable = EINA_TRUE;
   eo_do_super(obj, MY_CLASS, obj = eo_finalize());
   return obj;
}

EOLIAN static void
_ector_gl_buffer_tex_eo_base_destructor(Eo *obj, Ector_Gl_Buffer_Tex_Data *pd)
{
   eo_data_unref(obj, pd->generic);
   eo_do_super(obj, MY_CLASS, eo_destructor());
}

#include "ector_gl_buffer_tex.eo.c"
