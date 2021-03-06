#ifdef HAVE_CONFIG_H
  #include "elementary_config.h"
#endif

#define ELM_INTERFACE_ATSPI_IMAGE_PROTECTED

#include <Elementary.h>
#include "elm_widget.h"
#include "elm_priv.h"

static void
_free_desc_cb(void *data, const Efl_Event *event EINA_UNUSED)
{
   eina_stringshare_del(data);
}

EOLIAN static const char*
_elm_interface_atspi_image_description_get(Eo *obj, void *sd EINA_UNUSED)
{
   const char *descr = NULL;

   descr = efl_key_data_get(obj, "atspi_image_description");
   return descr;
}

EOLIAN static void
_elm_interface_atspi_image_description_set(Eo *obj, void *sd EINA_UNUSED, const char *description)
{
   const char *key = "atspi_image_description";
   const char *descr = eina_stringshare_add(description);
   char *old_descr = efl_key_data_get(obj, key);
   if (old_descr)
     {
        eina_stringshare_del(old_descr);
        efl_event_callback_del(obj, EFL_EVENT_DEL, _free_desc_cb, old_descr);
     }

   if (descr)
     {
        efl_key_data_set(obj, key, descr);
        efl_event_callback_add(obj, EFL_EVENT_DEL, _free_desc_cb, descr);
     }
}

EOLIAN static const char*
_elm_interface_atspi_image_locale_get(Eo *obj EINA_UNUSED, void *sd EINA_UNUSED)
{
   // by default assume that descriptions are given in language of current
   // locale.
   return getenv("LANG");
}

#include "elm_interface_atspi_image.eo.c"
