#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdlib.h>

#include "Ecore.h"
#include "ecore_private.h"

/* WARNING: This API is not tested yet! */

#ifdef DEBUG
#define SAFETY_CHECK(obj, klass, ...) do { \
   if (EINA_UNLIKELY(!eo_isa(dev, klass)) { \
       ERR("Invalid object type: %s wanted: %d", eo_class_name_get(obj), eo_class_name_get(klass)); \
       return __VA_ARGS__; \
   }} while (0)
#else
#define SAFETY_CHECK(obj, klass, ...) \
   do { if (!obj) return __VA_ARGS__; } while (0)
#endif

static Eina_List *_ecore_devices = NULL;

static void
_del_cb(void *data, const Eo_Event *ev)
{
   Eina_List **plist = data;

   *plist = eina_list_remove(*plist, ev->object);
}

EAPI Ecore_Device *
ecore_device_add(Ecore_Device_Class klass, Ecore_Device_Subclass subklass,
                 const char *name, const char *description, const char *identifier)
{
   Ecore_Device *dev = eo_add(EFL_INPUT_DEVICE_CLASS, ecore_main_loop_get(),
                              efl_input_device_type_set(eo_self, klass),
                              efl_input_device_subtype_set(eo_self, subklass),
                              efl_input_device_name_set(eo_self, name),
                              efl_input_device_description_set(eo_self, description),
                              efl_input_device_identifier_set(eo_self, identifier));
   _ecore_devices = eina_list_append(_ecore_devices, dev);
   eo_event_callback_add(dev, EO_EVENT_DEL, _del_cb, &_ecore_devices);
   return dev;
}

EAPI void
ecore_device_del(Ecore_Device *dev)
{
   eo_del(dev);
}

EAPI Eina_Iterator *
ecore_device_iterate(void)
{
   return _ecore_devices ? eina_list_iterator_new(_ecore_devices) : NULL;
}

EAPI Eina_Stringshare *
ecore_device_name_get(const Ecore_Device *dev)
{
   return efl_input_device_name_get(dev);
}

EAPI Eina_Stringshare *
ecore_device_description_get(const Ecore_Device *dev)
{
   return efl_input_device_description_get(dev);
}

EAPI Eina_Stringshare *
ecore_device_identifier_get(const Ecore_Device *dev)
{
   return efl_input_device_identifier_get(dev);
}

EAPI Ecore_Device_Class
ecore_device_class_get(const Ecore_Device *dev)
{
   return efl_input_device_type_get(dev);
}

EAPI Ecore_Device_Subclass
ecore_device_subclass_get(const Ecore_Device *dev)
{
   return efl_input_device_subtype_get(dev);
}

void
_ecore_device_cleanup(void)
{
    Ecore_Device *dev;
    Eina_List *l1, *l2;

    EINA_LIST_FOREACH_SAFE(_ecore_devices, l1, l2, dev)
      eo_del(dev);
}
