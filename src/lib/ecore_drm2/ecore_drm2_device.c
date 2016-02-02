#include "ecore_drm2_private.h"

EAPI const char *
ecore_drm2_device_find(const char *seat)
{
   Eina_List *devs, *l;
   const char *dev, *ret = NULL;
   Eina_Bool found = EINA_FALSE;
   Eina_Bool platform = EINA_FALSE;

   devs = eeze_udev_find_by_subsystem_sysname("drm", "card[0-9]*");
   if (!devs) return NULL;

   EINA_LIST_FOREACH(devs, l, dev)
     {
        const char *dpath;
        const char *dseat;
        const char *dparent;

        dpath = eeze_udev_syspath_get_devpath(dev);
        if (!dpath) continue;

        dseat = eeze_udev_syspath_get_property(dev, "ID_SEAT");
        if (!dseat) dseat = eina_stringshare_add("seat0");

        if ((seat) && (strcmp(seat, dseat)))
          goto cont;
        else if (strcmp(dseat, "seat0"))
          goto cont;

        dparent = eeze_udev_syspath_get_parent_filtered(dev, "pci", NULL);
        if (!dparent)
          {
             dparent =
               eeze_udev_syspath_get_parent_filtered(dev, "platform", NULL);
             platform = EINA_TRUE;
          }

        if (dparent)
          {
             if (!platform)
               {
                  const char *id;

                  id = eeze_udev_syspath_get_sysattr(dparent, "boot_vga");
                  if (id)
                    {
                       if (!strcmp(id, "1")) found = EINA_TRUE;
                       eina_stringshare_del(id);
                    }
               }
             else
               found = EINA_TRUE;

             eina_stringshare_del(dparent);
          }

cont:
        eina_stringshare_del(dpath);
        if (found) break;
     }

   if (!found) goto out;

   ret = eeze_udev_syspath_get_devpath(dev);

out:
   EINA_LIST_FREE(devs, dev)
     eina_stringshare_del(dev);

   return ret;
}

EAPI int
ecore_drm2_device_clock_id_get(int fd)
{
   uint64_t caps;
   int ret;

   EINA_SAFETY_ON_TRUE_RETURN_VAL((fd < 0), -1);

   ret = drmGetCap(fd, DRM_CAP_TIMESTAMP_MONOTONIC, &caps);
   if ((ret == 0) && (caps == 1))
     return CLOCK_MONOTONIC;
   else
     return CLOCK_REALTIME;
}
