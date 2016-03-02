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

# include <unistd.h>
# include <sys/mman.h>
# include <fcntl.h>

# include <sys/ioctl.h>
# include <linux/vt.h>
# include <linux/kd.h>
# include <linux/major.h>

# include <xf86drm.h>
# include <xf86drmMode.h>
# include <drm_fourcc.h>

# include <linux/input.h>
# include <libinput.h>
# include <xkbcommon/xkbcommon.h>

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

struct _Ecore_Drm2_Backlight
{
   const char *path;
   int max, value;
   Ecore_Drm2_Backlight_Type type;
};

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

   uint32_t rotation;
   uint32_t rotation_map[6];
   uint32_t rotation_prop_id;
   uint32_t supported_rotations;

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

   int x, y, w, h;
   int phys_width, phys_height;
   unsigned int scale;

   Eina_Stringshare *name;
   Eina_Stringshare *make, *model, *serial;

   uint32_t subpixel;

   uint32_t crtc_id;
   uint32_t conn_id;
   uint32_t conn_type;

   Ecore_Drm2_Output_Mode *current_mode;

   Eina_Matrix4 matrix;
   Eina_Matrix4 inverse;
   Ecore_Drm2_Output_Transform transform;

   drmModeCrtcPtr ocrtc;
   drmModePropertyPtr dpms_prop;

   uint16_t gamma;
   Ecore_Drm2_Backlight *backlight;

   struct
     {
        char eisa[13];
        char monitor[13];
        char pnp[5];
        char serial[13];
        unsigned char *blob;
     } edid;

   Eina_List *modes;
   Eina_List *planes;

   Eina_Bool connected : 1;
   Eina_Bool primary : 1;
   Eina_Bool cloned : 1;
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
        unsigned int timestamp;
     } grab;

   struct
     {
        unsigned int threshold;
        unsigned int last_button, prev_button;
        unsigned int last_time, prev_time;
        Eina_Bool double_click : 1;
        Eina_Bool triple_click : 1;
     } mouse;

   Ecore_Drm2_Seat *seat;
} Ecore_Drm2_Pointer;

typedef struct _Ecore_Drm2_Keyboard_Info
{
   int refs;

   struct
     {
        int fd;
        size_t size;
        char *area;

        struct xkb_keymap *map;
     } keymap;

   struct
     {
        xkb_mod_index_t shift;
        xkb_mod_index_t caps;
        xkb_mod_index_t ctrl;
        xkb_mod_index_t alt;
        xkb_mod_index_t mod2;
        xkb_mod_index_t mod3;
        xkb_mod_index_t mod5;
        xkb_mod_index_t super;
     } mods;

   struct
     {
        xkb_led_index_t num;
        xkb_led_index_t caps;
        xkb_led_index_t scroll;
     } leds;
} Ecore_Drm2_Keyboard_Info;

typedef struct _Ecore_Drm2_Keyboard
{
   struct
     {
        unsigned int depressed;
        unsigned int latched;
        unsigned int locked;
        unsigned int group;
     } modifiers;

   struct
     {
        unsigned int key;
        unsigned int timestamp;
     } grab;

   Ecore_Drm2_Keyboard_Info *info;

   struct xkb_state *state;
   struct xkb_keymap *pending_map;

   struct xkb_context *context;
   struct xkb_rule_names names;

   Ecore_Drm2_Seat *seat;
} Ecore_Drm2_Keyboard;

typedef struct _Ecore_Drm2_Touch
{
   int x, y, slot;
   unsigned int points; // number of touch points

   struct
     {
        int id;
        double x, y;
        unsigned int timestamp;
     } grab;

   Ecore_Drm2_Seat *seat;
} Ecore_Drm2_Touch;

struct _Ecore_Drm2_Seat
{
   const char *name;

   struct
     {
        int kbd, ptr, touch;
     } count;

   unsigned int modifiers;

   Ecore_Drm2_Pointer *ptr;
   Ecore_Drm2_Keyboard *kbd;
   Ecore_Drm2_Touch *touch;

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

   /* int fd; */
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

   int fd;
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

   Eeze_Udev_Watch *watch;

   Eina_List *outputs;

   Eina_Bool sync;
   Eina_Bool active;
};

Eina_Bool _ecore_drm2_dbus_open(Eldbus_Connection **conn);
void _ecore_drm2_dbus_close(Eldbus_Connection *conn);

Ecore_Drm2_Input_Device *_ecore_drm2_input_device_create(Ecore_Drm2_Seat *seat, struct libinput_device *device);
void _ecore_drm2_input_device_destroy(Ecore_Drm2_Input_Device *device);
int _ecore_drm2_input_device_event_process(struct libinput_event *event);
void _ecore_drm2_input_device_output_set(Ecore_Drm2_Input_Device *device, Ecore_Drm2_Output *output);
void _ecore_drm2_input_device_pointer_motion_send(Ecore_Drm2_Input_Device *device);

Eina_Bool _ecore_drm2_input_pointer_init(Ecore_Drm2_Seat *seat);
void _ecore_drm2_input_pointer_release(Ecore_Drm2_Seat *seat);
Ecore_Drm2_Pointer *_ecore_drm2_input_pointer_get(Ecore_Drm2_Seat *seat);

Eina_Bool _ecore_drm2_input_keyboard_init(Ecore_Drm2_Seat *seat, struct xkb_keymap *keymap);
void _ecore_drm2_input_keyboard_release(Ecore_Drm2_Seat *seat);
Ecore_Drm2_Keyboard *_ecore_drm2_input_keyboard_get(Ecore_Drm2_Seat *seat);
Ecore_Drm2_Keyboard_Info *_keyboard_info_create(struct xkb_keymap *keymap);
void _keyboard_info_destroy(Ecore_Drm2_Keyboard_Info *info);

Eina_Bool _ecore_drm2_input_touch_init(Ecore_Drm2_Seat *seat);
void _ecore_drm2_input_touch_release(Ecore_Drm2_Seat *seat);
Ecore_Drm2_Touch *_ecore_drm2_input_touch_get(Ecore_Drm2_Seat *seat);

void _ecore_drm2_output_coordinate_transform(Ecore_Drm2_Output *output, int dx, int dy, int *x, int *y);

void _ecore_drm2_launcher_activate_send(Ecore_Drm2_Launcher *launcher, Eina_Bool active);

extern Ecore_Drm2_Launcher_Interface _logind_iface;

#endif
