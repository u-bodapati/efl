#ifndef _ECORE_DRM2_PRIVATE_H
# define _ECORE_DRM2_PRIVATE_H

# ifdef HAVE_CONFIG_H
#  include "config.h"
# endif

# include "Ecore.h"
# include "ecore_private.h"
# include "Ecore_Input.h"
# include "Eeze.h"
# include "Eldbus.h"
# include <Ecore_Drm2.h>

# include <sys/ioctl.h>
# include <linux/vt.h>
# include <linux/kd.h>
# include <linux/major.h>

# include <xf86drm.h>
# include <xf86drmMode.h>
# include <drm_fourcc.h>

# include <linux/input.h>
# include <libinput.h>

extern int _ecore_drm2_log_dom;

# ifdef ECORE_DRM2_DEFAULT_LOG_COLOR
#  undef ECORE_DRM2_DEFAULT_LOG_COLOR
# endif
# define ECORE_DRM2_DEFAULT_LOG_COLOR EINA_COLOR_BLUE

# ifdef ERR
#  undef ERR
# endif
# define ERR(...) EINA_LOG_DOM_ERR(_ecore_drm2_log_dom, __VA_ARGS__)

# ifdef DBG
#  undef DBG
# endif
# define DBG(...) EINA_LOG_DOM_DBG(_ecore_drm2_log_dom, __VA_ARGS__)

# ifdef INF
#  undef INF
# endif
# define INF(...) EINA_LOG_DOM_INFO(_ecore_drm2_log_dom, __VA_ARGS__)

# ifdef WRN
#  undef WRN
# endif
# define WRN(...) EINA_LOG_DOM_WARN(_ecore_drm2_log_dom, __VA_ARGS__)

# ifdef CRIT
#  undef CRIT
# endif
# define CRIT(...) EINA_LOG_DOM_CRIT(_ecore_drm2_log_dom, __VA_ARGS__)

typedef struct _Ecore_Drm2_Launcher_Interface
{
   Eina_Bool (*connect)(Ecore_Drm2_Launcher **launcher, const char *seat, unsigned int tty, Eina_Bool sync);
   void (*disconnect)(Ecore_Drm2_Launcher *launcher);
   int (*open)(Ecore_Drm2_Launcher *launcher, const char *path, int flags);
   void (*close)(Ecore_Drm2_Launcher *launcher, int fd);
   int (*activate)(Ecore_Drm2_Launcher *launcher, int vt);
   void (*restore)(Ecore_Drm2_Launcher *launcher);
} Ecore_Drm2_Launcher_Interface;

typedef enum _Ecore_Drm2_Input_Device_Capability
{
   EVDEV_SEAT_POINTER = (1 << 0),
   EVDEV_SEAT_KEYBOARD = (1 << 1),
   EVDEV_SEAT_TOUCH = (1 << 2)
} Ecore_Drm2_Input_Device_Capability;

struct _Ecore_Drm2_Seat
{
   const char *name;

   struct
     {
        int kbd, ptr, touch;
     } count;

   Eina_List *devices;
};

struct _Ecore_Drm2_Input
{
   struct libinput *libinput;

   Ecore_Fd_Handler *hdlr;

   Eina_List *seats;

   Eina_Bool suspended : 1;
};

struct _Ecore_Drm2_Input_Device
{
   Ecore_Drm2_Seat *seat;

   int fd;
   const char *path;
   const char *output_name;

   struct libinput_device *device;
   Ecore_Drm2_Input_Device_Capability caps;
};

struct _Ecore_Drm2_Launcher
{
   Ecore_Drm2_Launcher_Interface *iface;

   const char *seat;

   char *sid;
   unsigned int vt_num;

   struct
     {
        char *path;
        Eldbus_Object *obj;
        Eldbus_Connection *conn;
     } dbus;

   Ecore_Drm2_Input input;

   Eina_Bool sync;
};

Eina_Bool _ecore_drm2_dbus_open(Eldbus_Connection **conn);
void _ecore_drm2_dbus_close(Eldbus_Connection *conn);

Ecore_Drm2_Input_Device *_ecore_drm2_input_device_create(Ecore_Drm2_Seat *seat, struct libinput_device *device);
void _ecore_drm2_input_device_destroy(Ecore_Drm2_Input_Device *device);
int _ecore_drm2_input_device_event_process(struct libinput_event *event);

extern Ecore_Drm2_Launcher_Interface _logind_iface;

#endif
