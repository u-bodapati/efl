#include "ecore_drm2_private.h"

static void
_device_calibration_set(Ecore_Drm2_Input_Device *dev)
{
   float cal[6];
   const char *vals;
   const char *sysname;
   const char *device;
   Eina_List *devices;
   int w = 0, h = 0;
   enum libinput_config_status status;

   if (!dev->output) return;

   w = dev->output->current_mode->width;
   h = dev->output->current_mode->height;
   if ((w == 0) || (h == 0)) return;

   if ((!libinput_device_config_calibration_has_matrix(dev->device)) ||
       (libinput_device_config_calibration_get_default_matrix(dev->device, cal) != 0))
     return;

   sysname = libinput_device_get_sysname(dev->device);

   devices = eeze_udev_find_by_subsystem_sysname("input", sysname);
   EINA_LIST_FREE(devices, device)
     {
        vals = eeze_udev_syspath_get_property(device, "WL_CALIBRATION");
        if ((!vals) ||
            (sscanf(vals, "%f %f %f %f %f %f",
                    &cal[0], &cal[1], &cal[2], &cal[3], &cal[4], &cal[5]) != 6))
          goto cont;

        cal[2] /= w;
        cal[5] /= h;

        status =
          libinput_device_config_calibration_set_matrix(dev->device, cal);
        if (status != LIBINPUT_CONFIG_STATUS_SUCCESS)
          WRN("Failed to apply device calibration");

cont:
        eina_stringshare_del(device);
     }
}

static void
_device_configure(Ecore_Drm2_Input_Device *dev)
{
   if (libinput_device_config_tap_get_finger_count(dev->device) > 0)
     {
        Eina_Bool enable = EINA_FALSE;

        enable = libinput_device_config_tap_get_default_enabled(dev->device);
        libinput_device_config_tap_set_enabled(dev->device, enable);
     }

   _device_calibration_set(dev);
}

static void
_pointer_motion_send(Ecore_Drm2_Input_Device *dev)
{
   Ecore_Drm2_Pointer *ptr;
   Ecore_Event_Mouse_Move *ev;

   ptr = _ecore_drm2_input_pointer_get(dev->seat);
   if (!ptr) return;

   ev = calloc(1, sizeof(Ecore_Event_Mouse_Move));
   if (!ev) return;

   /* TODO */
   /* ev->window = ; */
   /* ev->event_window = ; */
   /* ev->root_window = ; */
   ev->timestamp = ptr->timestamp;
   ev->same_screen = 1;

   ev->x = ptr->x;
   ev->y = ptr->y;
   ev->root.x = ptr->x;
   ev->root.y = ptr->y;

   /* TODO: multi-device */

   ecore_event_add(ECORE_EVENT_MOUSE_MOVE, ev, NULL, NULL);
}

static Eina_Bool
_pointer_motion(struct libinput_device *idevice, struct libinput_event_pointer *event)
{
   Ecore_Drm2_Input_Device *dev;
   Ecore_Drm2_Pointer *ptr;

   dev = libinput_device_get_user_data(idevice);
   if (!dev) return EINA_FALSE;

   ptr = _ecore_drm2_input_pointer_get(dev->seat);
   if (!ptr) return EINA_FALSE;

   ptr->x += libinput_event_pointer_get_dx(event);
   ptr->y += libinput_event_pointer_get_dy(event);
   ptr->timestamp = libinput_event_pointer_get_time(event);

   _pointer_motion_send(dev);

   return EINA_TRUE;
}

static Eina_Bool
_pointer_motion_abs(struct libinput_device *idevice, struct libinput_event_pointer *event)
{
   Ecore_Drm2_Input_Device *dev;
   Ecore_Drm2_Pointer *ptr;
   int w, h;

   dev = libinput_device_get_user_data(idevice);
   if ((!dev) || (!dev->output)) return EINA_FALSE;

   ptr = _ecore_drm2_input_pointer_get(dev->seat);
   if (!ptr) return EINA_FALSE;

   w = dev->output->current_mode->width;
   h = dev->output->current_mode->height;

   ptr->x = libinput_event_pointer_get_absolute_x_transformed(event, w);
   ptr->y = libinput_event_pointer_get_absolute_y_transformed(event, h);
   ptr->timestamp = libinput_event_pointer_get_time(event);

   _pointer_motion_send(dev);

   return EINA_TRUE;
}

static void
_pointer_button_send(Ecore_Drm2_Input_Device *dev, enum libinput_button_state state)
{
   Ecore_Drm2_Pointer *ptr;
   Ecore_Event_Mouse_Button *ev;

   ptr = _ecore_drm2_input_pointer_get(dev->seat);
   if (!ptr) return;

   ev = calloc(1, sizeof(Ecore_Event_Mouse_Button));
   if (!ev) return;

   /* TODO */
   /* ev->window = ; */
   /* ev->event_window = ; */
   /* ev->root_window = ; */
   ev->timestamp = ptr->timestamp;
   ev->same_screen = 1;

   ev->x = ptr->x;
   ev->y = ptr->y;
   ev->root.x = ptr->x;
   ev->root.y = ptr->y;

   /* TODO: multi-device */

   /* TODO: double/triple click  */

   ev->buttons = ptr->buttons;

   if (state)
     ecore_event_add(ECORE_EVENT_MOUSE_BUTTON_DOWN, ev, NULL, NULL);
   else
     ecore_event_add(ECORE_EVENT_MOUSE_BUTTON_UP, ev, NULL, NULL);
}

static Eina_Bool
_pointer_button(struct libinput_device *idevice, struct libinput_event_pointer *event)
{
   Ecore_Drm2_Input_Device *dev;
   Ecore_Drm2_Pointer *ptr;
   int count;
   enum libinput_button_state state;
   unsigned int btn;

   dev = libinput_device_get_user_data(idevice);
   if (!dev) return EINA_FALSE;

   ptr = _ecore_drm2_input_pointer_get(dev->seat);
   if (!ptr) return EINA_FALSE;

   state = libinput_event_pointer_get_button_state(event);
   count = libinput_event_pointer_get_seat_button_count(event);

   /* Ignore button events that are not seat wide state changes. */
   if (((state == LIBINPUT_BUTTON_STATE_PRESSED) && (count != 1)) ||
       ((state == LIBINPUT_BUTTON_STATE_RELEASED) && (count != 0)))
     return EINA_FALSE;

   btn = libinput_event_pointer_get_button(event);

   btn = ((btn & 0x00F) + 1);
   if (btn == 3) btn = 2;
   else if (btn == 2) btn = 3;

   ptr->buttons = btn;
   ptr->timestamp = libinput_event_pointer_get_time(event);

   _pointer_button_send(dev, state);

   return EINA_TRUE;
}

static void
_pointer_axis_send(Ecore_Drm2_Input_Device *dev, int direction, int value)
{
   Ecore_Drm2_Pointer *ptr;
   Ecore_Event_Mouse_Wheel *ev;

   ptr = _ecore_drm2_input_pointer_get(dev->seat);
   if (!ptr) return;

   ev = calloc(1, sizeof(Ecore_Event_Mouse_Wheel));
   if (!ev) return;

   /* TODO */
   /* ev->window = ; */
   /* ev->event_window = ; */
   /* ev->root_window = ; */
   ev->timestamp = ptr->timestamp;
   ev->same_screen = 1;

   ev->x = ptr->x;
   ev->y = ptr->y;
   ev->root.x = ptr->x;
   ev->root.y = ptr->y;

   ev->z = value;
   ev->direction = direction;

   ecore_event_add(ECORE_EVENT_MOUSE_WHEEL, ev, NULL, NULL);
}

static double
_pointer_axis_value(struct libinput_event_pointer *event, enum libinput_pointer_axis axis)
{
   enum libinput_pointer_axis_source source;
   double val = 0.0;

   source = libinput_event_pointer_get_axis_source(event);
   switch (source)
     {
      case LIBINPUT_POINTER_AXIS_SOURCE_WHEEL:
        val = 10 * libinput_event_pointer_get_axis_value_discrete(event, axis);
        break;
      case LIBINPUT_POINTER_AXIS_SOURCE_FINGER:
      case LIBINPUT_POINTER_AXIS_SOURCE_CONTINUOUS:
        val = libinput_event_pointer_get_axis_value(event, axis);
        break;
     }

   return val;
}

static Eina_Bool
_pointer_axis(struct libinput_device *idevice, struct libinput_event_pointer *event)
{
   Ecore_Drm2_Input_Device *dev;
   Ecore_Drm2_Pointer *ptr;
   enum libinput_pointer_axis axis;
   Eina_Bool vert = EINA_FALSE, horiz = EINA_FALSE;
   int dir = 0, val = 0;

   dev = libinput_device_get_user_data(idevice);
   if (!dev) return EINA_FALSE;

   ptr = _ecore_drm2_input_pointer_get(dev->seat);
   if (!ptr) return EINA_FALSE;

#ifdef LIBINPUT_HIGHER_08
   vert =
     libinput_event_pointer_has_axis(event,
                                     LIBINPUT_POINTER_AXIS_SCROLL_VERTICAL);
   horiz =
     libinput_event_pointer_has_axis(event,
                                     LIBINPUT_POINTER_AXIS_SCROLL_HORIZONTAL);
   if ((!vert) && (!horiz)) return EINA_FALSE;

   if (vert)
     {
        axis = LIBINPUT_POINTER_AXIS_SCROLL_VERTICAL;
        val = _pointer_axis_value(event, axis);
     }

   if (horiz)
     {
        axis = LIBINPUT_POINTER_AXIS_SCROLL_HORIZONTAL;
        val = _pointer_axis_value(event, axis);
        dir = 1;
     }

#else
   axis = libinput_event_pointer_get_axis(event);
   if (axis == LIBINPUT_POINTER_AXIS_SCROLL_HORIZONTAL) dir = 1;
   val = libinput_event_pointer_get_axis_value(event);
#endif

   ptr->timestamp = libinput_event_pointer_get_time(event);

   _pointer_axis_send(dev, dir, val);

   return EINA_TRUE;
}

Ecore_Drm2_Input_Device *
_ecore_drm2_input_device_create(Ecore_Drm2_Seat *seat, struct libinput_device *device)
{
   Ecore_Drm2_Input_Device *dev;

   dev = calloc(1, sizeof(Ecore_Drm2_Input_Device));
   if (!dev) return NULL;

   dev->seat = seat;
   dev->device = device;

   if (libinput_device_has_capability(device, LIBINPUT_DEVICE_CAP_KEYBOARD))
     {
        /* _ecore_drm2_input_keyboard_init(seat, NULL); */
        dev->caps |= EVDEV_SEAT_KEYBOARD;
     }

   if (libinput_device_has_capability(device, LIBINPUT_DEVICE_CAP_POINTER))
     {
        _ecore_drm2_input_pointer_init(seat);
        dev->caps |= EVDEV_SEAT_POINTER;
     }

   if (libinput_device_has_capability(device, LIBINPUT_DEVICE_CAP_TOUCH))
     {
        /* _ecore_drm2_input_touch_init(seat); */
        dev->caps |= EVDEV_SEAT_TOUCH;
     }

   libinput_device_set_user_data(device, dev);
   libinput_device_ref(device);

   _device_configure(dev);

   return dev;
}

void
_ecore_drm2_input_device_destroy(Ecore_Drm2_Input_Device *device)
{
   if (!device) return;

   if (device->caps & EVDEV_SEAT_POINTER)
     _ecore_drm2_input_pointer_release(device->seat);
   /* if (device->caps & EVDEV_SEAT_KEYBOARD) */
   /*   _ecore_drm2_input_keyboard_release(device->seat); */
   /* if (device->caps & EVDEV_SEAT_TOUCH) */
   /*   _ecore_drm2_input_touch_release(device->seat); */

   libinput_device_unref(device->device);
   eina_stringshare_del(device->output_name);
   free(device);
}

int
_ecore_drm2_input_device_event_process(struct libinput_event *event)
{
   struct libinput_device *idevice;
   int ret = 1;
   Eina_Bool frame = EINA_FALSE;

   idevice = libinput_event_get_device(event);
   switch (libinput_event_get_type(event))
     {
      case LIBINPUT_EVENT_KEYBOARD_KEY:
        break;
      case LIBINPUT_EVENT_POINTER_MOTION:
        frame =
          _pointer_motion(idevice, libinput_event_get_pointer_event(event));
        break;
      case LIBINPUT_EVENT_POINTER_MOTION_ABSOLUTE:
        frame =
          _pointer_motion_abs(idevice, libinput_event_get_pointer_event(event));
        break;
      case LIBINPUT_EVENT_POINTER_BUTTON:
        frame =
          _pointer_button(idevice, libinput_event_get_pointer_event(event));
        break;
      case LIBINPUT_EVENT_POINTER_AXIS:
        frame =
          _pointer_axis(idevice, libinput_event_get_pointer_event(event));
        break;
      case LIBINPUT_EVENT_TOUCH_DOWN:
        break;
      case LIBINPUT_EVENT_TOUCH_MOTION:
        break;
      case LIBINPUT_EVENT_TOUCH_UP:
        break;
      case LIBINPUT_EVENT_TOUCH_FRAME:
        break;
      default:
        ret = 0;
        break;
     }

   if (frame)
     {
        /* TODO: notify pointer frame */
     }

   return ret;
}

void
_ecore_drm2_input_device_output_set(Ecore_Drm2_Input_Device *device, Ecore_Drm2_Output *output)
{
   device->output = output;
   _device_calibration_set(device);
}
