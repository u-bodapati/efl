#include "ecore_drm2_private.h"
#include <ctype.h>

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
   Ecore_Drm2_Touch *touch;
   Ecore_Event_Mouse_Move *ev;

   ptr = _ecore_drm2_input_pointer_get(dev->seat);
   if (!ptr) return;

   ev = calloc(1, sizeof(Ecore_Event_Mouse_Move));
   if (!ev) return;

   /* TODO: focused window */
   /* ev->window = ; */
   /* ev->event_window = ; */
   /* ev->root_window = ; */
   ev->timestamp = ptr->timestamp;
   ev->same_screen = 1;

   ev->x = ptr->x;
   ev->y = ptr->y;
   ev->root.x = ptr->x;
   ev->root.y = ptr->y;

   touch = _ecore_drm2_input_touch_get(dev->seat);
   if (touch) ev->multi.device = touch->slot;
   ev->multi.radius = 1;
   ev->multi.radius_x = 1;
   ev->multi.radius_y = 1;
   ev->multi.pressure = 1.0;
   ev->multi.angle = 0.0;
   ev->multi.x = ev->x;
   ev->multi.y = ev->y;
   ev->multi.root.x = ev->x;
   ev->multi.root.y = ev->y;

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
   Ecore_Drm2_Touch *touch;
   Ecore_Event_Mouse_Button *ev;

   ptr = _ecore_drm2_input_pointer_get(dev->seat);
   if (!ptr) return;

   ev = calloc(1, sizeof(Ecore_Event_Mouse_Button));
   if (!ev) return;

   /* TODO: focused window */
   /* ev->window = ; */
   /* ev->event_window = ; */
   /* ev->root_window = ; */
   ev->timestamp = ptr->timestamp;
   ev->same_screen = 1;

   ev->x = ptr->x;
   ev->y = ptr->y;
   ev->root.x = ptr->x;
   ev->root.y = ptr->y;

   touch = _ecore_drm2_input_touch_get(dev->seat);
   if (touch) ev->multi.device = touch->slot;
   ev->multi.radius = 1;
   ev->multi.radius_x = 1;
   ev->multi.radius_y = 1;
   ev->multi.pressure = 1.0;
   ev->multi.angle = 0.0;
   ev->multi.x = ev->x;
   ev->multi.y = ev->y;
   ev->multi.root.x = ev->x;
   ev->multi.root.y = ev->y;

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

   /* TODO: focused window */
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

static void
_keyboard_key_send(Ecore_Drm2_Seat *seat, enum libinput_key_state state, const char *keyname, const char *key, const char *compose, unsigned int code, unsigned int timestamp)
{
   Ecore_Event_Key *ev;

   ev = calloc(1, sizeof(Ecore_Event_Key) + strlen(key) + strlen(keyname) + 3);
   if (!ev) return;

   ev->keyname = (char *)(ev + 1);
   ev->key = ev->keyname + strlen(keyname) + 1;
   ev->compose = NULL;
   if (compose) ev->compose = ev->key + strlen(key) + 1;

   strcpy((char *)ev->keyname, keyname);
   strcpy((char *)ev->key, key);
   if (compose) strcpy((char *)ev->compose, compose);

   ev->string = ev->compose;
   ev->keycode = code;
   ev->modifiers = seat->modifiers;
   ev->timestamp = timestamp;
   ev->same_screen = 1;

   /* TODO: focused window */
   /* ev->window = ; */
   /* ev->event_window = ; */
   /* ev->root_window = ; */

   if (state == LIBINPUT_KEY_STATE_PRESSED)
     ecore_event_add(ECORE_EVENT_KEY_DOWN, ev, NULL, NULL);
   else
     ecore_event_add(ECORE_EVENT_KEY_UP, ev, NULL, NULL);
}

static void
_keyboard_modifiers_update(Ecore_Drm2_Keyboard *kbd, Ecore_Drm2_Seat *seat)
{
   xkb_mod_mask_t mask;

   kbd->modifiers.depressed =
     xkb_state_serialize_mods(kbd->state, XKB_STATE_DEPRESSED);
   kbd->modifiers.latched =
     xkb_state_serialize_mods(kbd->state, XKB_STATE_LATCHED);
   kbd->modifiers.locked =
     xkb_state_serialize_mods(kbd->state, XKB_STATE_LOCKED);
   kbd->modifiers.group =
     xkb_state_serialize_mods(kbd->state, XKB_STATE_EFFECTIVE);

   mask = (kbd->modifiers.depressed | kbd->modifiers.latched);

   seat->modifiers = 0;
   if (mask & kbd->info->mods.ctrl)
     seat->modifiers |= ECORE_EVENT_MODIFIER_CTRL;
   if (mask & kbd->info->mods.alt)
     seat->modifiers |= ECORE_EVENT_MODIFIER_ALT;
   if (mask & kbd->info->mods.shift)
     seat->modifiers |= ECORE_EVENT_MODIFIER_SHIFT;
   if (mask & kbd->info->mods.super)
     seat->modifiers |= ECORE_EVENT_MODIFIER_WIN;

   /* TODO: LEDs ? */
}

static void
_keyboard_keymap_send(Ecore_Drm2_Keyboard_Info *info)
{
   Ecore_Drm2_Event_Keymap_Send *ev;

   ev = calloc(1, sizeof(Ecore_Drm2_Event_Keymap_Send));
   if (!ev) return;

   ev->fd = info->keymap.fd;
   ev->size = info->keymap.size;
   ev->format = XKB_KEYMAP_FORMAT_TEXT_V1;

   ecore_event_add(ECORE_DRM2_EVENT_KEYMAP_SEND, ev, NULL, NULL);
}

static void
_keyboard_modifiers_send(Ecore_Drm2_Keyboard *kbd)
{
   Ecore_Drm2_Event_Modifiers_Send *ev;

   ev = calloc(1, sizeof(Ecore_Drm2_Event_Modifiers_Send));
   if (!ev) return;

   ev->depressed = kbd->modifiers.depressed;
   ev->latched = kbd->modifiers.latched;
   ev->locked = kbd->modifiers.locked;
   ev->group = kbd->modifiers.group;

   ecore_event_add(ECORE_DRM2_EVENT_MODIFIERS_SEND, ev, NULL, NULL);
}

static void
_keyboard_keymap_update(Ecore_Drm2_Seat *seat)
{
   Ecore_Drm2_Keyboard *kbd;
   Ecore_Drm2_Keyboard_Info *info;
   struct xkb_state *state;
   xkb_mod_mask_t latched, locked;

   kbd = _ecore_drm2_input_keyboard_get(seat);
   if (!kbd) return;

   info = _keyboard_info_create(kbd->pending_map);
   xkb_keymap_unref(kbd->pending_map);
   kbd->pending_map = NULL;

   if (!info) return;

   state = xkb_state_new(info->keymap.map);
   if (!state)
     {
        _keyboard_info_destroy(info);
        return;
     }

   latched = xkb_state_serialize_mods(kbd->state, XKB_STATE_MODS_LATCHED);
   locked = xkb_state_serialize_mods(kbd->state, XKB_STATE_MODS_LOCKED);
   xkb_state_update_mask(state, 0, latched, locked, 0, 0, 0);

   _keyboard_info_destroy(kbd->info);
   kbd->info = info;

   xkb_state_unref(kbd->state);
   kbd->state = state;

   _keyboard_keymap_send(kbd->info);

   _keyboard_modifiers_update(kbd, seat);

   if ((!latched) && (!locked)) return;

   _keyboard_modifiers_send(kbd);
}

static int
_keyboard_keysym_translate(xkb_keysym_t keysym, unsigned int modifiers, char *buffer, int bytes)
{
   unsigned long hbytes = 0;
   unsigned char c;

   if (!keysym) return 0;
   hbytes = (keysym >> 8);

   if (!(bytes &&
         ((hbytes == 0) ||
          ((hbytes == 0xFF) &&
           (((keysym >= XKB_KEY_BackSpace) && (keysym <= XKB_KEY_Clear)) ||
            (keysym == XKB_KEY_Return) || (keysym == XKB_KEY_Escape) ||
            (keysym == XKB_KEY_KP_Space) || (keysym == XKB_KEY_KP_Tab) ||
            (keysym == XKB_KEY_KP_Enter) ||
            ((keysym >= XKB_KEY_KP_Multiply) && (keysym <= XKB_KEY_KP_9)) ||
            (keysym == XKB_KEY_KP_Equal) || (keysym == XKB_KEY_Delete))))))
     return 0;

   if (keysym == XKB_KEY_KP_Space)
     c = (XKB_KEY_space & 0x7F);
   else if (hbytes == 0xFF)
     c = (keysym & 0x7F);
   else
     c = (keysym & 0xFF);

   if (modifiers & ECORE_EVENT_MODIFIER_CTRL)
     {
        if (((c >= '@') && (c < '\177')) || c == ' ')
          c &= 0x1F;
        else if (c == '2')
          c = '\000';
        else if ((c >= '3') && (c <= '7'))
          c -= ('3' - '\033');
        else if (c == '8')
          c = '\177';
        else if (c == '/')
          c = '_' & 0x1F;
     }
   buffer[0] = c;
   return 1;
}

static void
_keyboard_key(struct libinput_device *idevice, struct libinput_event_keyboard *event)
{
   Ecore_Drm2_Input_Device *dev;
   Ecore_Drm2_Keyboard *kbd;
   enum libinput_key_state state;
   xkb_keysym_t sym = XKB_KEY_NoSymbol;
   const xkb_keysym_t *syms;
   unsigned int code = 0;
   unsigned int nsyms;
   unsigned int timestamp;
   char key[256], keyname[256], buffer[256];
   char *tmp = NULL, *compose = NULL;
   int count;

   dev = libinput_device_get_user_data(idevice);
   if (!dev) return;

   kbd = _ecore_drm2_input_keyboard_get(dev->seat);
   if (!kbd) return;

   state = libinput_event_keyboard_get_key_state(event);
   count = libinput_event_keyboard_get_seat_key_count(event);

   /* Ignore key events that are not seat wide state changes. */
   if (((state == LIBINPUT_KEY_STATE_PRESSED) && (count != 1)) ||
       ((state == LIBINPUT_KEY_STATE_RELEASED) && (count != 0)))
     return;

   code = libinput_event_keyboard_get_key(event) + 8;
   timestamp = libinput_event_keyboard_get_time(event);

   if (state == LIBINPUT_KEY_STATE_PRESSED)
     xkb_state_update_key(kbd->state, code, XKB_KEY_DOWN);
   else
     xkb_state_update_key(kbd->state, code, XKB_KEY_UP);

   nsyms = xkb_key_get_syms(kbd->state, code, &syms);
   if (nsyms == 1) sym = syms[0];

   memset(key, 0, sizeof(key));
   xkb_keysym_get_name(sym, key, sizeof(key));

   memset(keyname, 0, sizeof(keyname));
   memcpy(keyname, key, sizeof(keyname));

   if (keyname[0] == '\0')
     snprintf(keyname, sizeof(keyname), "Keycode-%u", code);

   if (xkb_state_mod_index_is_active(kbd->state, kbd->info->mods.shift,
                                     XKB_STATE_MODS_EFFECTIVE))
     {
        if (keyname[0] != '\0')
          keyname[0] = tolower(keyname[0]);
     }

   _keyboard_modifiers_update(kbd, dev->seat);

   memset(buffer, 0, sizeof(buffer));
   if (_keyboard_keysym_translate(sym, dev->seat->modifiers, 
                                  buffer, sizeof(buffer)))
     {
        compose = eina_str_convert("ISO8859-1", "UTF-8", buffer);
        if (!compose)
          {
             ERR("Ecore_DRM cannot convert input key string '%s' to UTF-8. "
                 "Is Eina built with iconv support?", buffer);
          }
        else
          tmp = compose;
     }

   if (!compose) compose = buffer;

   _keyboard_key_send(dev->seat, state, keyname, key, compose, code, timestamp);

   if (tmp) free(tmp);

   if (kbd->pending_map) _keyboard_keymap_update(dev->seat);

   if (state == LIBINPUT_KEY_STATE_PRESSED)
     {
        kbd->grab.key = code;
        kbd->grab.timestamp = timestamp;
     }
}

static void
_touch_event_send(Ecore_Drm2_Input_Device *dev, struct libinput_event_touch *event, int type)
{
   Ecore_Drm2_Touch *touch;
   Ecore_Event_Mouse_Button *ev;
   unsigned int btn = 0;

   touch = _ecore_drm2_input_touch_get(dev->seat);
   if (!touch) return;

   ev = calloc(1, sizeof(Ecore_Event_Mouse_Button));
   if (!ev) return;

   /* TODO: focused window */
   /* ev->window = ; */
   /* ev->event_window = ; */
   /* ev->root_window = ; */
   ev->timestamp = libinput_event_touch_get_time(event);
   ev->same_screen = 1;

   ev->x = touch->x;
   ev->y = touch->y;
   ev->root.x = touch->x;
   ev->root.y = touch->y;

   ev->modifiers = dev->seat->modifiers;

   ev->multi.device = touch->slot;
   ev->multi.radius = 1;
   ev->multi.radius_x = 1;
   ev->multi.radius_y = 1;
   ev->multi.pressure = 1.0;
   ev->multi.angle = 0.0;
   ev->multi.x = ev->x;
   ev->multi.y = ev->y;
   ev->multi.root.x = ev->x;
   ev->multi.root.y = ev->y;

   btn = ((btn & 0x00F) + 1);
   if (btn == 3) btn = 2;
   else if (btn == 2) btn = 3;
   ev->buttons = btn;

   ecore_event_add(type, ev, NULL, NULL);
}

static void
_touch_down(struct libinput_device *idevice, struct libinput_event_touch *event)
{
   Ecore_Drm2_Input_Device *dev;
   Ecore_Drm2_Touch *touch;
   unsigned int timestamp;
   int w, h, slot;

   dev = libinput_device_get_user_data(idevice);
   if (!dev) return;

   touch = _ecore_drm2_input_touch_get(dev->seat);
   if (!touch) return;

   w = dev->output->current_mode->width;
   h = dev->output->current_mode->height;

   slot = libinput_event_touch_get_seat_slot(event);
   timestamp = libinput_event_touch_get_time(event);

   touch->x = libinput_event_touch_get_x_transformed(event, w);
   touch->y = libinput_event_touch_get_y_transformed(event, h);

   /* TODO: transform coordinates ? */

   if (slot == touch->grab.id)
     {
        touch->grab.x = touch->x;
        touch->grab.y = touch->y;
     }

   touch->slot = slot;
   touch->points++;

   _touch_event_send(dev, event, ECORE_EVENT_MOUSE_BUTTON_DOWN);

   if (touch->points == 1)
     {
        touch->grab.id = slot;
        touch->grab.x = touch->x;
        touch->grab.y = touch->y;
        touch->grab.timestamp = timestamp;
     }
}

static void
_touch_motion_send(Ecore_Drm2_Input_Device *dev, struct libinput_event_touch *event)
{
   Ecore_Drm2_Touch *touch;
   Ecore_Event_Mouse_Move *ev;

   touch = _ecore_drm2_input_touch_get(dev->seat);
   if (!touch) return;

   ev = calloc(1, sizeof(Ecore_Event_Mouse_Move));
   if (!ev) return;

   /* TODO: focused window */
   /* ev->window = ; */
   /* ev->event_window = ; */
   /* ev->root_window = ; */
   ev->timestamp = libinput_event_touch_get_time(event);
   ev->same_screen = 1;

   ev->x = touch->x;
   ev->y = touch->y;
   ev->root.x = touch->x;
   ev->root.y = touch->y;

   ev->modifiers = dev->seat->modifiers;

   ev->multi.device = touch->slot;
   ev->multi.radius = 1;
   ev->multi.radius_x = 1;
   ev->multi.radius_y = 1;
   ev->multi.pressure = 1.0;
   ev->multi.angle = 0.0;
   ev->multi.x = ev->x;
   ev->multi.y = ev->y;
   ev->multi.root.x = ev->x;
   ev->multi.root.y = ev->y;

   ecore_event_add(ECORE_EVENT_MOUSE_MOVE, ev, NULL, NULL);
}

static void
_touch_motion(struct libinput_device *idevice, struct libinput_event_touch *event)
{
   Ecore_Drm2_Input_Device *dev;
   Ecore_Drm2_Touch *touch;
   int w, h;

   dev = libinput_device_get_user_data(idevice);
   if (!dev) return;

   touch = _ecore_drm2_input_touch_get(dev->seat);
   if (!touch) return;

   w = dev->output->current_mode->width;
   h = dev->output->current_mode->height;

   touch->x = libinput_event_touch_get_x_transformed(event, w);
   touch->y = libinput_event_touch_get_y_transformed(event, h);

   /* TODO: transform coordinates ? */

   touch->slot = libinput_event_touch_get_seat_slot(event);

   _touch_motion_send(dev, event);
}

static void
_touch_up(struct libinput_device *idevice, struct libinput_event_touch *event)
{
   Ecore_Drm2_Input_Device *dev;
   Ecore_Drm2_Touch *touch;

   dev = libinput_device_get_user_data(idevice);
   if (!dev) return;

   touch = _ecore_drm2_input_touch_get(dev->seat);
   if (!touch) return;

   touch->points--;
   touch->slot = libinput_event_touch_get_seat_slot(event);

   _touch_event_send(dev, event, ECORE_EVENT_MOUSE_BUTTON_UP);
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
        _ecore_drm2_input_keyboard_init(seat, NULL);
        dev->caps |= EVDEV_SEAT_KEYBOARD;
     }

   if (libinput_device_has_capability(device, LIBINPUT_DEVICE_CAP_POINTER))
     {
        _ecore_drm2_input_pointer_init(seat);
        dev->caps |= EVDEV_SEAT_POINTER;
     }

   if (libinput_device_has_capability(device, LIBINPUT_DEVICE_CAP_TOUCH))
     {
        _ecore_drm2_input_touch_init(seat);
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
   if (device->caps & EVDEV_SEAT_KEYBOARD)
     _ecore_drm2_input_keyboard_release(device->seat);
   if (device->caps & EVDEV_SEAT_TOUCH)
     _ecore_drm2_input_touch_release(device->seat);

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
        _keyboard_key(idevice, libinput_event_get_keyboard_event(event));
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
        _touch_down(idevice, libinput_event_get_touch_event(event));
        break;
      case LIBINPUT_EVENT_TOUCH_MOTION:
        _touch_motion(idevice, libinput_event_get_touch_event(event));
        break;
      case LIBINPUT_EVENT_TOUCH_UP:
        _touch_up(idevice, libinput_event_get_touch_event(event));
        break;
      case LIBINPUT_EVENT_TOUCH_FRAME:
        /* _touch_frame(idevice, libinput_event_get_touch_event(event)); */
        /* break; */
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

void
_ecore_drm2_input_device_pointer_motion_send(Ecore_Drm2_Input_Device *device)
{
   _pointer_motion_send(device);
}
