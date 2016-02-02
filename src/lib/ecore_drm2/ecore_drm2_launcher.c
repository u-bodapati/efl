#include "ecore_drm2_private.h"

static Ecore_Drm2_Launcher_Interface *_ifaces[] =
{
#ifdef HAVE_SYSTEMD
   &_logind_iface,
#endif
   NULL, /* &_launch_iface, */
   NULL, /* &_direct_iface, */
   NULL,
};

EAPI Ecore_Drm2_Launcher *
ecore_drm2_launcher_connect(const char *seat, unsigned int tty, Eina_Bool sync)
{
   Ecore_Drm2_Launcher_Interface **it;

   for (it = _ifaces; *it != NULL; it++)
     {
        Ecore_Drm2_Launcher_Interface *iface;
        Ecore_Drm2_Launcher *l;

        iface = *it;
        if (iface->connect(&l, seat, tty, sync))
          return l;
     }

   return NULL;
}

EAPI void
ecore_drm2_launcher_disconnect(Ecore_Drm2_Launcher *launcher)
{
   EINA_SAFETY_ON_NULL_RETURN(launcher);
   EINA_SAFETY_ON_NULL_RETURN(launcher->iface);

   if (launcher->iface->disconnect)
     launcher->iface->disconnect(launcher);
}

EAPI int
ecore_drm2_launcher_open(Ecore_Drm2_Launcher *launcher, const char *path, int flags)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(launcher, -1);
   EINA_SAFETY_ON_NULL_RETURN_VAL(launcher->iface, -1);

   if (launcher->iface->open)
     return launcher->iface->open(launcher, path, flags);

   return -1;
}

EAPI void
ecore_drm2_launcher_close(Ecore_Drm2_Launcher *launcher, int fd)
{
   EINA_SAFETY_ON_NULL_RETURN(launcher);
   EINA_SAFETY_ON_NULL_RETURN(launcher->iface);

   if (launcher->iface->close)
     launcher->iface->close(launcher, fd);
}

EAPI int
ecore_drm2_launcher_activate(Ecore_Drm2_Launcher *launcher, int vt)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(launcher, -1);
   EINA_SAFETY_ON_NULL_RETURN_VAL(launcher->iface, -1);

   if (launcher->iface->activate)
     return launcher->iface->activate(launcher, vt);

   return -1;
}
