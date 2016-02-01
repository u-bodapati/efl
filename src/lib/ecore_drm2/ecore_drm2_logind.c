#include "ecore_drm2_private.h"

#ifdef HAVE_SYSTEMD
# include <unistd.h>
# include <systemd/sd-login.h>

static Eina_Bool
_logind_session_vt_get(const char *sid, unsigned int *vt)
{
# ifdef HAVE_SYSTEMD_LOGIN_209
   return sd_session_get_vt(sid, vt);
# else
   int ret;
   char *tty;

   ret = sd_session_get_tty(sid, &tty);
   if (ret < 0) return ret;

   ret = sscanf(tty, "tty%u", vt);
   free(tty);

   if (ret != 1) return EINA_FALSE;
   return EINA_TRUE;
# endif
}

static Eina_Bool
_logind_connect(Ecore_Drm2_Launcher **launcher, const char *seat, unsigned int tty, Eina_Bool sync)
{
   Ecore_Drm2_Launcher *l;
   char *s;
   int ret;

   l = calloc(1, sizeof(Ecore_Drm2_Launcher));
   if (!l) return EINA_FALSE;

   l->iface = &_logind_iface;
   l->sync = sync;
   l->seat = eina_stringshare_add(seat);

   ret = sd_pid_get_session(getpid(), &l->sid);
   if (ret < 0)
     {
        ERR("Could not get systemd session");
        goto session_err;
     }

   ret = sd_session_get_seat(l->sid, &s);
   if (ret < 0)
     {
        ERR("Failed to get session seat");
        free(s);
        goto seat_err;
     }
   else if (strcmp(seat, s))
     {
        ERR("Seat '%s' differs from session seat '%s'", seat, s);
        free(s);
        goto seat_err;
     }

   if (!_logind_session_vt_get(l->sid, &l->vt_num))
     {
        ERR("Could not get session vt");
        goto vt_err;
     }
   else if ((tty > 0) && (l->vt_num != tty))
     {
        ERR("Requested VT %u differs from session VT %u", tty, l->vt_num);
        goto vt_err;
     }

   *(Ecore_Drm2_Launcher **)launcher = l;

   return EINA_TRUE;

vt_err:
seat_err:
   free(l->sid);
session_err:
   free(l);
   return EINA_FALSE;
}

Ecore_Drm2_Interface _logind_iface =
{
   _logind_connect
};

#endif
