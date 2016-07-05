#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdlib.h>

#include "Ecore.h"
#include "ecore_private.h"

/**
 * @struct _Ecore_Devicei
 * Contains information about a device.
 */
struct _Ecore_Device
{
   ECORE_MAGIC; /**< ecore magic data */
   Eina_Stringshare *name; /**< name of device */
   Eina_Stringshare *desc; /**< description of device */
   Eina_Stringshare *identifier; /**< identifier of device */
   Ecore_Device_Class clas; /**<Device class */
   Ecore_Device_Subclass subclas; /**< device subclass */
};

static Eina_List *_ecore_devices = NULL;

void _ecore_device_free(Ecore_Device *dev);

EAPI Ecore_Device *
ecore_device_add(void)
{
   Ecore_Device *dev;

   dev = calloc(1, sizeof(Ecore_Device));
   if (!dev) return NULL;
   ECORE_MAGIC_SET(dev, ECORE_MAGIC_DEV);
   _ecore_devices = eina_list_append(_ecore_devices, dev);
   return dev;
}

EAPI void
ecore_device_del(Ecore_Device *dev)
{
   if (!dev) return;

   if (!ECORE_MAGIC_CHECK(dev, ECORE_MAGIC_DEV))
     {
        ECORE_MAGIC_FAIL(dev, ECORE_MAGIC_DEV, "ecore_device_del");
        return;
     }
   _ecore_device_free(dev);
}

EAPI const Eina_List *
ecore_device_list(void)
{
   return _ecore_devices;
}

EAPI void
ecore_device_name_set(Ecore_Device *dev, const char *name)
{
   if (!ECORE_MAGIC_CHECK(dev, ECORE_MAGIC_DEV))
     {
        ECORE_MAGIC_FAIL(dev, ECORE_MAGIC_DEV, "ecore_device_name_set");
        return;
     }
   eina_stringshare_replace(&dev->name, name);
}

EAPI Eina_Stringshare *
ecore_device_name_get(const Ecore_Device *dev)
{
   if (!ECORE_MAGIC_CHECK(dev, ECORE_MAGIC_DEV))
     {
        ECORE_MAGIC_FAIL(dev, ECORE_MAGIC_DEV, "ecore_device_name_get");
        return NULL;
     }
   return dev->name;
}

EAPI void
ecore_device_description_set(Ecore_Device *dev, const char *desc)
{
   if (!ECORE_MAGIC_CHECK(dev, ECORE_MAGIC_DEV))
     {
        ECORE_MAGIC_FAIL(dev, ECORE_MAGIC_DEV, "ecore_device_description_set");
        return;
     }
   eina_stringshare_replace(&dev->desc, desc);
}

EAPI Eina_Stringshare *
ecore_device_description_get(const Ecore_Device *dev)
{
   if (!ECORE_MAGIC_CHECK(dev, ECORE_MAGIC_DEV))
     {
        ECORE_MAGIC_FAIL(dev, ECORE_MAGIC_DEV, "ecore_device_description_get");
        return NULL;
     }
   return dev->desc;
}

EAPI void
ecore_device_identifier_set(Ecore_Device *dev, const char *identifier)
{
   if (!ECORE_MAGIC_CHECK(dev, ECORE_MAGIC_DEV))
     {
        ECORE_MAGIC_FAIL(dev, ECORE_MAGIC_DEV, "ecore_device_identifier_set");
        return;
     }
   eina_stringshare_replace(&dev->identifier, identifier);
}

EAPI Eina_Stringshare *
ecore_device_identifier_get(const Ecore_Device *dev)
{
   if (!ECORE_MAGIC_CHECK(dev, ECORE_MAGIC_DEV))
     {
        ECORE_MAGIC_FAIL(dev, ECORE_MAGIC_DEV, "ecore_device_identifier_get");
        return NULL;
     }
   return dev->identifier;
}

EAPI void
ecore_device_class_set(Ecore_Device *dev, Ecore_Device_Class clas)
{
   if (!ECORE_MAGIC_CHECK(dev, ECORE_MAGIC_DEV))
     {
        ECORE_MAGIC_FAIL(dev, ECORE_MAGIC_DEV, "ecore_device_class_set");
        return;
     }
   dev->clas = clas;
}

EAPI Ecore_Device_Class
ecore_device_class_get(const Ecore_Device *dev)
{
   if (!ECORE_MAGIC_CHECK(dev, ECORE_MAGIC_DEV))
     {
        ECORE_MAGIC_FAIL(dev, ECORE_MAGIC_DEV, "ecore_device_class_get");
        return ECORE_DEVICE_CLASS_NONE;
     }
   return dev->clas;
}

EAPI void
ecore_device_subclass_set(Ecore_Device *dev, Ecore_Device_Subclass subclas)
{
   if (!ECORE_MAGIC_CHECK(dev, ECORE_MAGIC_DEV))
     {
        ECORE_MAGIC_FAIL(dev, ECORE_MAGIC_DEV, "ecore_device_class_set");
        return;
     }
   dev->subclas = subclas;
}

EAPI Ecore_Device_Subclass
ecore_device_subclass_get(const Ecore_Device *dev)
{
   if (!ECORE_MAGIC_CHECK(dev, ECORE_MAGIC_DEV))
     {
        ECORE_MAGIC_FAIL(dev, ECORE_MAGIC_DEV, "ecore_device_class_get");
        return ECORE_DEVICE_SUBCLASS_NONE;
     }
   return dev->subclas;
}

void
_ecore_device_cleanup(void)
{
    Ecore_Device *dev;
    Eina_List *l1, *l2;

    EINA_LIST_FOREACH_SAFE(_ecore_devices, l1, l2, dev)
    _ecore_device_free(dev);
}

void
_ecore_device_free(Ecore_Device *dev)
{
    _ecore_devices = eina_list_remove(_ecore_devices, dev);
    eina_stringshare_del(dev->name);
    eina_stringshare_del(dev->desc);
    eina_stringshare_del(dev->identifier);
    ECORE_MAGIC_SET(dev, 0);
    free(dev);
}
