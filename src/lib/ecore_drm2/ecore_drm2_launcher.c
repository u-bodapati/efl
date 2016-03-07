#include "ecore_drm2_private.h"
#include <fcntl.h>

static Ecore_Drm2_Launcher_Interface *_ifaces[] =
{
#ifdef HAVE_SYSTEMD
   &_logind_iface,
#endif
   NULL, /* &_launch_iface, */
   NULL, /* &_direct_iface, */
   NULL,
};

void
_ecore_drm2_launcher_activate_send(Ecore_Drm2_Launcher *launcher, Eina_Bool active)
{
   Ecore_Drm2_Event_Activate *ev;

   if (!launcher->active == !active) return;

   launcher->active = active;

   ev = calloc(1, sizeof(Ecore_Drm2_Event_Activate));
   if (!ev) return;

   ev->active = active;

   ecore_event_add(ECORE_DRM2_EVENT_ACTIVATE, ev, NULL, NULL);
}

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

   if (flags < 0) flags = O_RDWR;

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

EAPI void
ecore_drm2_launcher_restore(Ecore_Drm2_Launcher *launcher)
{
   EINA_SAFETY_ON_NULL_RETURN(launcher);
   EINA_SAFETY_ON_NULL_RETURN(launcher->iface);

   if (launcher->iface->restore)
     launcher->iface->restore(launcher);
}

EAPI Eina_Bool
ecore_drm2_launcher_active_get(Ecore_Drm2_Launcher *launcher)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(launcher, EINA_FALSE);
   return launcher->active;
}

EAPI unsigned int
ecore_drm2_launcher_crtc_count_get(Ecore_Drm2_Launcher *launcher)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(launcher, -1);
   return launcher->num_crtcs;
}

EAPI unsigned int *
ecore_drm2_launcher_crtcs_get(Ecore_Drm2_Launcher *launcher)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(launcher, 0);
   return launcher->crtcs;
}

EAPI void
ecore_drm2_launcher_screen_size_range_get(Ecore_Drm2_Launcher *launcher, int *minw, int *minh, int *maxw, int *maxh)
{
   if (minw) *minw = 0;
   if (minh) *minh = 0;
   if (maxw) *maxw = 0;
   if (maxh) *maxh = 0;

   EINA_SAFETY_ON_NULL_RETURN(launcher);

   if (minw) *minw = launcher->min.width;
   if (minh) *minh = launcher->min.height;
   if (maxw) *maxw = launcher->max.width;
   if (maxh) *maxh = launcher->max.height;
}

EAPI void
ecore_drm2_launcher_outputs_geometry_get(Ecore_Drm2_Launcher *launcher, int *x, int *y, int *w, int *h)
{
   Ecore_Drm2_Output *output;
   const Eina_List *l;
   int ow = 0, oh = 0;

   if (x) *x = 0;
   if (y) *y = 0;
   if (w) *w = 0;
   if (h) *h = 0;

   EINA_SAFETY_ON_NULL_RETURN(launcher);

   EINA_LIST_FOREACH(launcher->outputs, l, output)
     {
        if ((!output->connected) || (!output->enabled)) continue;
        if (output->cloned) continue;
        ow += MAX(ow, output->current_mode->width);
        oh = MAX(oh, output->current_mode->height);
     }

   if (w) *w = ow;
   if (h) *h = oh;
}
