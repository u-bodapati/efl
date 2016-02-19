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

struct _Ecore_Drm2_Fb
{
   int fd;
   int w, h;
   int depth, bpp;
   uint32_t id, hdl;
   uint32_t stride, size;
   uint32_t format;

   void *mmap;

   Eina_Bool gbm : 1;
   Eina_Bool from_client : 1;
};

struct _Ecore_Drm2_Plane
{
   uint32_t id;
   uint32_t crtcs;
   uint32_t num_formats;
   int32_t x, y, cw, ch;
   int32_t type;

   int32_t sx, sy; // src
   uint32_t sw, sh;
   uint32_t dx, dy; // dest
   uint32_t dw, dh;

   void *output;

   uint32_t formats[];
};

struct _Ecore_Drm2_Output_Mode
{
   uint32_t flags;
   int32_t width, height;
   uint32_t refresh;
   drmModeModeInfo info;
};

struct _Ecore_Drm2_Output
{
   int id, pipe;

   int x, y;
   int phys_width, phys_height;

   const char *name;
   const char *make, *model, *serial;

   uint32_t subpixel;

   uint32_t crtc_id;
   uint32_t conn_id;

   Ecore_Drm2_Output_Mode *current_mode;

   drmModeCrtcPtr ocrtc;
   drmModePropertyPtr dpms_prop;

   struct
     {
        char eisa[13];
        char monitor[13];
        char pnp[5];
        char serial[13];
     } edid;

   Eina_List *modes;
   Eina_List *planes;

   Eina_Bool connected : 1;
};

typedef struct _Ecore_Drm2_Pointer
{
   int x, y;
   int buttons;
   unsigned int timestamp;

   struct
     {
        int x, y;
     } hot;

   struct
     {
        int x, y;
        unsigned int button;
        unsigned int serial;
        unsigned int timestamp;
     } grab;

   Ecore_Drm2_Seat *seat;
} Ecore_Drm2_Pointer;

struct _Ecore_Drm2_Seat
{
   const char *name;

   struct
     {
        int kbd, ptr, touch;
     } count;

   Ecore_Drm2_Pointer *ptr;

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
   Ecore_Drm2_Output *output;

   struct libinput_device *device;
   Ecore_Drm2_Input_Device_Capability caps;
};

struct _Ecore_Drm2_Launcher
{
   Ecore_Drm2_Launcher_Interface *iface;

   const char *seat;

   char *sid;
   unsigned int vt_num;

   uint32_t *crtcs;
   int num_crtcs;
   uint32_t crtc_allocator;
   uint32_t conn_allocator;

   struct
     {
        uint32_t width, height;
     } min, max;

   struct
     {
        char *path;
        Eldbus_Object *obj;
        Eldbus_Connection *conn;
     } dbus;

   Ecore_Drm2_Input input;

   Eina_List *outputs;

   Eina_Bool sync;
};

Eina_Bool _ecore_drm2_dbus_open(Eldbus_Connection **conn);
void _ecore_drm2_dbus_close(Eldbus_Connection *conn);

Ecore_Drm2_Input_Device *_ecore_drm2_input_device_create(Ecore_Drm2_Seat *seat, struct libinput_device *device);
void _ecore_drm2_input_device_destroy(Ecore_Drm2_Input_Device *device);
int _ecore_drm2_input_device_event_process(struct libinput_event *event);
void _ecore_drm2_input_device_output_set(Ecore_Drm2_Input_Device *device, Ecore_Drm2_Output *output);

Eina_Bool _ecore_drm2_input_pointer_init(Ecore_Drm2_Seat *seat);
void _ecore_drm2_input_pointer_release(Ecore_Drm2_Seat *seat);
Ecore_Drm2_Pointer *_ecore_drm2_input_pointer_get(Ecore_Drm2_Seat *seat);

/* Eina_Bool _ecore_drm2_input_keyboard_init(Ecore_Drm2_Seat *seat, struct xkb_keymap *keymap); */
/* void _ecore_drm2_input_keyboard_release(Ecore_Drm2_Seat *seat); */

/* Eina_Bool _ecore_drm2_input_touch_init(Ecore_Drm2_Seat *seat); */
/* void _ecore_drm2_input_touch_release(Ecore_Drm2_Seat *seat); */

extern Ecore_Drm2_Launcher_Interface _logind_iface;

#endif
