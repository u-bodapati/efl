#include "ecore_drm2_private.h"

#ifndef DRM_MAJOR
# define DRM_MAJOR 226
#endif

#ifdef HAVE_SYSTEMD
# include <unistd.h>
# include <fcntl.h>
# include <systemd/sd-login.h>

static void
_cb_session_removed(void *data, const Eldbus_Message *msg)
{
   Ecore_Drm2_Launcher *l;
   const char *errname, *errmsg;
   const char *sid;

   l = data;

   if (eldbus_message_error_get(msg, &errname, &errmsg))
     {
        ERR("Eldbus Message Error: %s %s", errname, errmsg);
        return;
     }

   if (eldbus_message_arguments_get(msg, "s", &sid))
     {
        if (!strcmp(sid, l->sid))
          {
             WRN("Logind session removed");
             ecore_drm2_launcher_restore(l);
          }
     }
}

static void
_cb_device_paused(void *data, const Eldbus_Message *msg)
{
   Ecore_Drm2_Launcher *l;
   const char *errname, *errmsg;
   const char *type;
   uint32_t maj, min;

   l = data;

   if (eldbus_message_error_get(msg, &errname, &errmsg))
     {
        ERR("Eldbus Message Error: %s %s", errname, errmsg);
        return;
     }

   if (eldbus_message_arguments_get(msg, "uus", &maj, &min, &type))
     {
        if (!strcmp(type, "pause"))
          {
             /* TODO: pause device complete */
          }

        if ((l->sync) && (maj == DRM_MAJOR))
          {
             /* TODO: set active (false) */
          }
     }
}

static void
_cb_device_resumed(void *data, const Eldbus_Message *msg)
{
   Ecore_Drm2_Launcher *l;
   const char *errname, *errmsg;
   uint32_t maj, min;
   int fd;

   l = data;

   if (eldbus_message_error_get(msg, &errname, &errmsg))
     {
        ERR("Eldbus Message Error: %s %s", errname, errmsg);
        return;
     }

   if (eldbus_message_arguments_get(msg, "uuh", &maj, &min, &fd))
     {
        if ((l->sync) && (maj == DRM_MAJOR))
          {
             /* TODO: set active (true) */
          }
     }
}

static void
_cb_property_changed(void *data EINA_UNUSED, Eldbus_Proxy *proxy EINA_UNUSED, void *event)
{
   /* Ecore_Drm2_Launcher *l; */
   Eldbus_Proxy_Event_Property_Changed *ev;

   /* l = data; */
   ev = event;

   DBG("DBus Property Changed: %s", ev->name);
}

static Eina_Bool
_logind_control_take(Ecore_Drm2_Launcher *launcher)
{
   Eldbus_Proxy *proxy;
   Eldbus_Message *msg, *reply;
   const char *errname, *errmsg;

   proxy =
     eldbus_proxy_get(launcher->dbus.obj, "org.freedesktop.login1.Session");
   if (!proxy)
     {
        ERR("Could not get proxy for session");
        return EINA_FALSE;
     }

   msg = eldbus_proxy_method_call_new(proxy, "TakeControl");
   if (!msg)
     {
        ERR("Could not create method call for proxy");
        goto msg_err;
     }

   eldbus_message_arguments_append(msg, "b", EINA_FALSE);

   reply = eldbus_proxy_send_and_block(proxy, msg, -1);
   if (eldbus_message_error_get(reply, &errname, &errmsg))
     {
        ERR("Eldbus Message Error: %s %s", errname, errmsg);
        goto msg_err;
     }

   eldbus_message_unref(reply);
   eldbus_proxy_unref(proxy);

   return EINA_TRUE;

msg_err:
   eldbus_proxy_unref(proxy);
   return EINA_FALSE;
}

static void
_logind_control_release(Ecore_Drm2_Launcher *launcher)
{
   Eldbus_Proxy *proxy;
   Eldbus_Message *msg;

   proxy =
     eldbus_proxy_get(launcher->dbus.obj, "org.freedesktop.login1.Session");
   if (!proxy)
     {
        ERR("Could not get proxy for session");
        return;
     }

   msg = eldbus_proxy_method_call_new(proxy, "ReleaseControl");
   if (!msg)
     {
        ERR("Could not create method call for proxy");
        goto end;
     }

   eldbus_proxy_send(proxy, msg, NULL, NULL, -1);

end:
   eldbus_proxy_unref(proxy);
}

static int
_logind_device_take(Ecore_Drm2_Launcher *launcher, uint32_t major, uint32_t minor)
{
   Eldbus_Proxy *proxy;
   Eldbus_Message *msg, *reply;
   Eina_Bool p = EINA_FALSE;
   const char *errname, *errmsg;
   int fd = -1;

   proxy =
     eldbus_proxy_get(launcher->dbus.obj, "org.freedesktop.login1.Session");
   if (!proxy)
     {
        ERR("Could not get dbus proxy");
        return -1;
     }

   msg = eldbus_proxy_method_call_new(proxy, "TakeDevice");
   if (!msg)
     {
        ERR("Could not create method call for proxy");
        goto err;
     }

   eldbus_message_arguments_append(msg, "uu", major, minor);

   reply = eldbus_proxy_send_and_block(proxy, msg, -1);
   if (eldbus_message_error_get(reply, &errname, &errmsg))
     {
        ERR("Eldbus Message Error: %s %s", errname, errmsg);
        goto err;
     }

   if (!eldbus_message_arguments_get(reply, "hb", &fd, &p))
     ERR("Could not get UNIX_FD from dbus message");

   eldbus_message_unref(reply);

err:
   eldbus_proxy_unref(proxy);
   return fd;
}

static void
_logind_device_release(Ecore_Drm2_Launcher *launcher, uint32_t major, uint32_t minor)
{
   Eldbus_Proxy *proxy;
   Eldbus_Message *msg;

   proxy =
     eldbus_proxy_get(launcher->dbus.obj, "org.freedesktop.login1.Session");
   if (!proxy)
     {
        ERR("Could not get proxy for session");
        return;
     }

   msg = eldbus_proxy_method_call_new(proxy, "ReleaseDevice");
   if (!msg)
     {
        ERR("Could not create method call for proxy");
        goto end;
     }

   eldbus_message_arguments_append(msg, "uu", major, minor);

   eldbus_proxy_send(proxy, msg, NULL, NULL, -1);

end:
   eldbus_proxy_unref(proxy);
}

static Eina_Bool
_logind_dbus_setup(Ecore_Drm2_Launcher *launcher)
{
   Eldbus_Proxy *proxy;
   int ret = 0;

   ret = asprintf(&launcher->dbus.path,
                  "/org/freedesktop/login1/session/%s", launcher->sid);
   if (ret < 0) return EINA_FALSE;

   launcher->dbus.obj =
     eldbus_object_get(launcher->dbus.conn, "org.freedesktop.login1",
                       launcher->dbus.path);
   if (!launcher->dbus.obj)
     {
        ERR("Could not get dbus object");
        goto obj_err;
     }

   proxy =
     eldbus_proxy_get(launcher->dbus.obj, "org.freedesktop.login1.Manager");
   if (!proxy)
     {
        ERR("Could not get dbus proxy");
        goto proxy_err;
     }

   eldbus_proxy_signal_handler_add(proxy, "SessionRemoved",
                                   _cb_session_removed, launcher);
   eldbus_proxy_unref(proxy);

   proxy =
     eldbus_proxy_get(launcher->dbus.obj, "org.freedesktop.login1.Session");
   if (!proxy)
     {
        ERR("Could not get dbus proxy");
        goto proxy_err;
     }

   eldbus_proxy_signal_handler_add(proxy, "PauseDevice",
                                   _cb_device_paused, launcher);
   eldbus_proxy_signal_handler_add(proxy, "ResumeDevice",
                                   _cb_device_resumed, launcher);
   eldbus_proxy_unref(proxy);

   proxy =
     eldbus_proxy_get(launcher->dbus.obj, "org.freedesktop.DBus.Properties");
   if (!proxy)
     {
        ERR("Could not get dbus proxy");
        goto proxy_err;
     }

   eldbus_proxy_properties_monitor(proxy, EINA_TRUE);
   eldbus_proxy_event_callback_add(proxy, ELDBUS_PROXY_EVENT_PROPERTY_CHANGED,
                                   _cb_property_changed, launcher);
   eldbus_proxy_unref(proxy);

   return EINA_TRUE;

proxy_err:
   eldbus_object_unref(launcher->dbus.obj);
obj_err:
   free(launcher->dbus.path);
   return EINA_FALSE;
}

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
_logind_activate(Ecore_Drm2_Launcher *launcher)
{
   Eldbus_Proxy *proxy;
   Eldbus_Message *msg;

   proxy =
     eldbus_proxy_get(launcher->dbus.obj, "org.freedesktop.login1.Session");
   if (!proxy)
     {
        ERR("Could not get proxy for session");
        return EINA_FALSE;
     }

   msg = eldbus_proxy_method_call_new(proxy, "Activate");
   if (!msg)
     {
        ERR("Could not create method call for proxy");
        goto msg_err;
     }

   eldbus_proxy_send(proxy, msg, NULL, NULL, -1);

   eldbus_proxy_unref(proxy);

   return EINA_TRUE;

msg_err:
   eldbus_proxy_unref(proxy);
   return EINA_FALSE;
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

   if (!_ecore_drm2_dbus_open(&l->dbus.conn))
     {
        ERR("Could not open dbus");
        goto vt_err;
     }

   if (!_logind_dbus_setup(l))
     {
        ERR("Could not setup dbus");
        goto dbus_err;
     }

   if (!_logind_control_take(l))
     {
        ERR("Could not take control of session");
        goto ctrl_err;
     }

   if (!_logind_activate(l))
     {
        ERR("Could not activate session");
        goto actv_err;
     }

   *(Ecore_Drm2_Launcher **)launcher = l;

   return EINA_TRUE;

actv_err:
   _logind_control_release(l);
ctrl_err:
   eldbus_object_unref(l->dbus.obj);
   free(l->dbus.path);
dbus_err:
   _ecore_drm2_dbus_close(l->dbus.conn);
vt_err:
seat_err:
   free(l->sid);
session_err:
   free(l);
   return EINA_FALSE;
}

static void
_logind_disconnect(Ecore_Drm2_Launcher *launcher)
{
   /* release control */
   _logind_control_release(launcher);

   /* dbus destroy */
   eldbus_object_unref(launcher->dbus.obj);
   free(launcher->dbus.path);

   /* dbus close */
   _ecore_drm2_dbus_close(launcher->dbus.conn);

   eina_stringshare_del(launcher->seat);
   free(launcher->sid);
   free(launcher);
}

static int
_logind_open(Ecore_Drm2_Launcher *launcher, const char *path, int flags)
{
   struct stat st;
   int ret, fd;
   int fl;

   ret = stat(path, &st);
   if (ret < 0) return -1;

   if (!S_ISCHR(st.st_mode)) return -1;

   fd = _logind_device_take(launcher, major(st.st_rdev), minor(st.st_rdev));
   if (fd < 0) return fd;

   fl = fcntl(fd, F_GETFL);
   if (fl < 0) goto err;

   if (flags & O_NONBLOCK)
     fl |= O_NONBLOCK;

   ret = fcntl(fd, F_SETFL, fl);
   if (ret < 0) goto err;

   return fd;

err:
   close(fd);
   _logind_device_release(launcher, major(st.st_rdev), minor(st.st_rdev));
   return -1;
}

static void
_logind_close(Ecore_Drm2_Launcher *launcher, int fd)
{
   struct stat st;
   int ret;

   ret = fstat(fd, &st);
   if (ret < 0) return;

   if (!S_ISCHR(st.st_mode)) return;

   _logind_device_release(launcher, major(st.st_rdev), minor(st.st_rdev));
}

static int
_logind_activate_vt(Ecore_Drm2_Launcher *launcher, int vt)
{
   Eldbus_Proxy *proxy;
   Eldbus_Message *msg;
   int ret = 0;

   proxy =
     eldbus_proxy_get(launcher->dbus.obj, "org.freedesktop.login1.Seat");
   if (!proxy)
     {
        ERR("Could not get proxy for session");
        return -1;
     }

   msg = eldbus_proxy_method_call_new(proxy, "SwitchTo");
   if (!msg)
     {
        ERR("Could not create method call for proxy");
        ret = -1;
        goto end;
     }

   eldbus_message_arguments_append(msg, "u", vt);

   eldbus_proxy_send(proxy, msg, NULL, NULL, -1);

end:
   eldbus_proxy_unref(proxy);
   return ret;
}

static void
_logind_restore(Ecore_Drm2_Launcher *launcher EINA_UNUSED)
{

}

Ecore_Drm2_Launcher_Interface _logind_iface =
{
   _logind_connect,
   _logind_disconnect,
   _logind_open,
   _logind_close,
   _logind_activate_vt,
   _logind_restore,
};

#endif
