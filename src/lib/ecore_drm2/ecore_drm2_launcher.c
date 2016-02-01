#include "ecore_drm2_private.h"

static Ecore_Drm2_Interface *ifaces[] =
{
#ifdef HAVE_SYSTEMD_LOGIN
   &_logind_iface,
#endif
   NULL, /* &_launch_iface, */
   NULL, /* &_direct_iface, */
   NULL,
};

EAPI Ecore_Drm2_Launcher *
ecore_drm2_launcher_connect(const char *seat, int tty, Eina_Bool sync)
{
   Ecore_Drm2_Interface **it;

   for (it = ifaces; *it != NULL; it++)
     {
        Ecore_Drm2_Interface *iface;
        Ecore_Drm2_Launcher *l;

        iface = *it;
        if (iface->connect(&l, seat, tty, sync))
          return l;
     }

   return NULL;
}
