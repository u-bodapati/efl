#ifdef HAVE_CONFIG_H
# include "config.h"
#else
# define EFL_BETA_API_SUPPORT
# include <Eo.h>
#endif

#include <Ector.h>
#include "ector_gl_private.h"
#include "ector_gl_buffer_fbo.eo.h"

#define MY_CLASS ECTOR_GL_BUFFER_FBO_CLASS

typedef struct
{
   Ector_Generic_Buffer_Data *generic;
   unsigned int fbo, tex;
   Eina_Bool bgra;
} Ector_Gl_Buffer_Fbo_Data;

EOLIAN static void *
_ector_gl_buffer_fbo_ector_generic_buffer_map(Eo *obj, Ector_Gl_Buffer_Fbo_Data *pd, unsigned int *length, Ector_Buffer_Access_Flag mode, unsigned int x, unsigned int y, unsigned int w, unsigned int h, Efl_Gfx_Colorspace cspace, unsigned int *stride)
{

}

EOLIAN static void
_ector_gl_buffer_fbo_ector_generic_buffer_unmap(Eo *obj, Ector_Gl_Buffer_Fbo_Data *pd, void *data, unsigned int length)
{

}

EOLIAN static Eina_Bool
_ector_gl_buffer_fbo_ector_generic_buffer_pixels_set(Eo *obj, Ector_Gl_Buffer_Fbo_Data *pd, void *pixels, int width, int height, int stride, Efl_Gfx_Colorspace cspace, Eina_Bool writable, unsigned char l, unsigned char r, unsigned char t, unsigned char b)
{

}

EOLIAN static uint8_t *
_ector_gl_buffer_fbo_ector_generic_buffer_span_get(Eo *obj, Ector_Gl_Buffer_Fbo_Data *pd, int x, int y, unsigned int w, Efl_Gfx_Colorspace cspace, unsigned int *length)
{

}

EOLIAN static void
_ector_gl_buffer_fbo_ector_generic_buffer_span_free(Eo *obj, Ector_Gl_Buffer_Fbo_Data *pd, uint8_t *data)
{

}

EOLIAN static Ector_Buffer_Flag
_ector_gl_buffer_fbo_ector_generic_buffer_flags_get(Eo *obj, Ector_Gl_Buffer_Fbo_Data *pd)
{

}

EOLIAN static Eina_Bool
_ector_gl_buffer_fbo_generate(Eo *obj, Ector_Gl_Buffer_Fbo_Data *pd, int width, int height, Eina_Bool bgra)
{
   int fnum, tnum, fmt;
   Eina_Bool ret;

   if (pd->generic->immutable)
     return EINA_FALSE;

   pd->bgra = bgra;
   if (pd->bgra)
     fmt = GL_BGRA_EXT;
   else
     fmt = GL_RGBA;

   GL.glGetIntegerv(GL_FRAMEBUFFER_BINDING, &fnum);
   GL.glGetIntegerv(GL_TEXTURE_BINDING_2D, &tnum);

   GL.glGenTextures(1, &pd->tex);
   GL.glBindTexture(GL_TEXTURE_2D, pd->tex);
   GL.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
   GL.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
   GL.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
   GL.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
   GL.glTexImage2D(GL_TEXTURE_2D, 0, fmt, width, height, 0, fmt, GL_UNSIGNED_BYTE, NULL);

   GL.glGenFramebuffers(1, &pd->fbo);
   GL.glBindFramebuffer(GL_FRAMEBUFFER, pd->fbo);
   GL.glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pd->tex, 0);

   GL.glBindFramebuffer(GL_FRAMEBUFFER, fnum);
   GL.glBindTexture(GL_TEXTURE_2D, tnum);

   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_ector_gl_buffer_fbo_attach(Eo *obj, Ector_Gl_Buffer_Fbo_Data *pd, unsigned int fbo)
{
   Eina_Bool ret;
   if (eo_do_ret(obj, ret, eo_finalized_get()))
     {
        ERR("Can not change attached FBO");
        return EINA_FALSE;
     }

   // note: no references in GL, good luck.
   pd->fbo = fbo;

   return EINA_TRUE;
}

EOLIAN static Eo_Base *
_ector_gl_buffer_fbo_eo_base_constructor(Eo *obj, Ector_Gl_Buffer_Fbo_Data *pd)
{
   if (!GL.init) return NULL;
   eo_do_super(obj, MY_CLASS, obj = eo_constructor());
   pd->generic = eo_data_ref(obj, ECTOR_GENERIC_BUFFER_MIXIN);
   return obj;
}

EOLIAN static Eo_Base *
_ector_gl_buffer_fbo_eo_base_finalize(Eo *obj, Ector_Gl_Buffer_Fbo_Data *pd)
{
   if (!pd->fbo)
     {
        CRI("FBO buffer not attached to any FBO");
        return NULL;
     }
   pd->generic->immutable = EINA_TRUE;
   eo_do_super(obj, MY_CLASS, obj = eo_finalize());
   return obj;
}

EOLIAN static void
_ector_gl_buffer_fbo_eo_base_destructor(Eo *obj, Ector_Gl_Buffer_Fbo_Data *pd)
{
   eo_data_unref(obj, pd->generic);
   eo_do_super(obj, MY_CLASS, eo_destructor());
}

#include "ector_gl_buffer_fbo.eo.c"
