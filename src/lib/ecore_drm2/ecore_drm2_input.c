#include "ecore_drm2_private.h"

static int
_cb_open_restricted(const char *path, int flags, void *data)
{
   Ecore_Drm2_Launcher *launcher;

   launcher = data;
   return ecore_drm2_launcher_open(launcher, path, flags);
}

static void
_cb_close_restricted(int fd, void *data)
{
   Ecore_Drm2_Launcher *launcher;

   launcher = data;
   ecore_drm2_launcher_close(launcher, fd);
}

const struct libinput_interface _input_interface =
{
   _cb_open_restricted,
   _cb_close_restricted,
};

static void
_pointer_destroy(Ecore_Drm2_Pointer *ptr)
{
   /* TODO: free any other pointer resource */
   free(ptr);
}

static void
_touch_destroy(Ecore_Drm2_Touch *touch)
{
   /* TODO: free any other touch resources */
   free(touch);
}

static void
_keyboard_info_destroy(Ecore_Drm2_Keyboard_Info *info)
{
   if (--info->refs > 0) return;

   xkb_keymap_unref(info->keymap.map);
   if (info->keymap.area) munmap(info->keymap.area, info->keymap.size);
   if (info->keymap.fd) close(info->keymap.fd);
   free(info);
}

static void
_keyboard_destroy(Ecore_Drm2_Keyboard *kbd)
{
   free((char *)kbd->names.rules);
   free((char *)kbd->names.model);
   free((char *)kbd->names.layout);
   free((char *)kbd->names.variant);
   free((char *)kbd->names.options);

   if (kbd->state) xkb_state_unref(kbd->state);
   if (kbd->info) _keyboard_info_destroy(kbd->info);
   xkb_context_unref(kbd->context);

   xkb_keymap_unref(kbd->pending_map);

   free(kbd);
}

static Ecore_Drm2_Keyboard *
_keyboard_create(Ecore_Drm2_Seat *seat)
{
   Ecore_Drm2_Keyboard *kbd;

   kbd = calloc(1, sizeof(Ecore_Drm2_Keyboard));
   if (!kbd) return NULL;

   kbd->seat = seat;

   return kbd;
}

static int
_keyboard_fd_get(off_t size)
{
   int fd = 0, blen = 0, len = 0;
   const char *path;
   char tmp[PATH_MAX];
   long flags;

   blen = sizeof(tmp) - 1;

   if (!(path = getenv("XDG_RUNTIME_DIR")))
     return -1;

   len = strlen(path);
   if (len < blen)
     {
        strcpy(tmp, path);
        strcat(tmp, "/ecore-drm2-keymap-XXXXXX");
     }
   else
     return -1;

   if ((fd = mkstemp(tmp)) < 0) return -1;

   flags = fcntl(fd, F_GETFD);
   if (flags < 0)
     {
        close(fd);
        return -1;
     }

   if (fcntl(fd, F_SETFD, (flags | FD_CLOEXEC)) == -1)
     {
        close(fd);
        return -1;
     }

   if (ftruncate(fd, size) < 0)
     {
        close(fd);
        return -1;
     }

   unlink(tmp);
   return fd;
}

static Ecore_Drm2_Keyboard_Info *
_keyboard_info_create(struct xkb_keymap *keymap)
{
   Ecore_Drm2_Keyboard_Info *info;
   char *str;

   info = calloc(1, sizeof(Ecore_Drm2_Keyboard_Info));
   if (!info) return NULL;

   info->keymap.map = xkb_keymap_ref(keymap);
   info->refs = 1;

   info->mods.shift =
     xkb_keymap_mod_get_index(info->keymap.map, XKB_MOD_NAME_SHIFT);
   info->mods.caps =
     xkb_keymap_mod_get_index(info->keymap.map, XKB_MOD_NAME_CAPS);
   info->mods.ctrl =
     xkb_keymap_mod_get_index(info->keymap.map, XKB_MOD_NAME_CTRL);
   info->mods.alt =
     xkb_keymap_mod_get_index(info->keymap.map, XKB_MOD_NAME_ALT);
   info->mods.super =
     xkb_keymap_mod_get_index(info->keymap.map, XKB_MOD_NAME_LOGO);
   info->mods.mod2 = xkb_keymap_mod_get_index(info->keymap.map, "Mod2");
   info->mods.mod3 = xkb_keymap_mod_get_index(info->keymap.map, "Mod3");
   info->mods.mod5 = xkb_keymap_mod_get_index(info->keymap.map, "Mod5");

   info->leds.num =
     xkb_keymap_led_get_index(info->keymap.map, XKB_LED_NAME_NUM);
   info->leds.caps =
     xkb_keymap_led_get_index(info->keymap.map, XKB_LED_NAME_CAPS);
   info->leds.scroll =
     xkb_keymap_led_get_index(info->keymap.map, XKB_LED_NAME_SCROLL);

   str = xkb_keymap_get_as_string(info->keymap.map, XKB_KEYMAP_FORMAT_TEXT_V1);
   if (!str) goto err;

   info->keymap.size = strlen(str) + 1;

   info->keymap.fd = _keyboard_fd_get(info->keymap.size);
   if (info->keymap.fd < 0) goto err_fd;

   info->keymap.area =
     mmap(NULL, info->keymap.size, PROT_READ | PROT_WRITE,
          MAP_SHARED, info->keymap.fd, 0);
   if (info->keymap.area == MAP_FAILED) goto err_map;

   strcpy(info->keymap.area, str);
   free(str);

   return info;

err_map:
   close(info->keymap.fd);
err_fd:
   free(str);
err:
   xkb_keymap_unref(info->keymap.map);
   free(info);
   return NULL;
}

static Eina_Bool
_keyboard_global_build(Ecore_Drm2_Keyboard *kbd)
{
   struct xkb_keymap *keymap;

   kbd->context = xkb_context_new(0);
   if (!kbd->context) return EINA_FALSE;

   if (!kbd->names.rules) kbd->names.rules = strdup("evdev");
   if (!kbd->names.model) kbd->names.model = strdup("pc105");
   if (!kbd->names.layout) kbd->names.layout = strdup("us");

   keymap = xkb_keymap_new_from_names(kbd->context, &kbd->names, 0);
   if (!keymap) return EINA_FALSE;

   kbd->info = _keyboard_info_create(keymap);
   xkb_keymap_unref(keymap);

   if (!kbd->info) return EINA_FALSE;
   return EINA_TRUE;
}

static void
_keyboard_state_reset(Ecore_Drm2_Keyboard *kbd)
{
   struct xkb_state *state;

   state = xkb_state_new(kbd->info->keymap.map);
   if (!state) return;

   xkb_state_unref(kbd->state);
   kbd->state = state;
}

static Ecore_Drm2_Seat *
_udev_seat_create(Ecore_Drm2_Input *input, const char *name)
{
   Ecore_Drm2_Seat *seat;

   seat = calloc(1, sizeof(Ecore_Drm2_Seat));
   if (!seat) return NULL;

   seat->name = eina_stringshare_add(name);

   input->seats = eina_list_append(input->seats, seat);

   return seat;
}

static void
_udev_seat_destroy(Ecore_Drm2_Seat *seat)
{
   Ecore_Drm2_Input_Device *dev;

   EINA_LIST_FREE(seat->devices, dev)
     _ecore_drm2_input_device_destroy(dev);

   if (seat->ptr) _pointer_destroy(seat->ptr);
   if (seat->kbd) _keyboard_destroy(seat->kbd);
   if (seat->touch) _touch_destroy(seat->touch);

   eina_stringshare_del(seat->name);
   free(seat);
}

static Ecore_Drm2_Seat *
_udev_seat_named_get(Ecore_Drm2_Input *input, const char *name)
{
   Eina_List *l;
   Ecore_Drm2_Seat *seat;

   EINA_LIST_FOREACH(input->seats, l, seat)
     {
        if (!strcmp(seat->name, name))
          return seat;
     }

   return _udev_seat_create(input, name);
}

static Ecore_Drm2_Seat *
_udev_seat_get(Ecore_Drm2_Input *input, struct libinput_device *device)
{
   struct libinput_seat *lseat;
   const char *name;

   lseat = libinput_device_get_seat(device);
   name = libinput_seat_get_logical_name(lseat);
   /* name = libinput_seat_get_physical_name(lseat); */

   return _udev_seat_named_get(input, name);
}

static void
_device_added(Ecore_Drm2_Launcher *launcher, struct libinput_device *device)
{
   Eina_List *l;
   Ecore_Drm2_Input *input;
   Ecore_Drm2_Output *output;
   Ecore_Drm2_Seat *seat;
   Ecore_Drm2_Input_Device *dev;
   const char *oname;

   input = &launcher->input;

   seat = _udev_seat_get(input, device);

   dev = _ecore_drm2_input_device_create(seat, device);

   /* TODO: get pointer and clamp ?? */

   oname = libinput_device_get_output_name(device);
   if (oname)
     {
        dev->output_name = eina_stringshare_add(oname);
        EINA_LIST_FOREACH(launcher->outputs, l, output)
          {
             if (!output->name) continue;
             if (!strcmp(output->name, oname))
               {
                  _ecore_drm2_input_device_output_set(dev, output);
                  break;
               }
          }
     }
   else if ((!dev->output) && (eina_list_count(launcher->outputs) > 0))
     {
        output = eina_list_nth(launcher->outputs, 0);
        if (output) _ecore_drm2_input_device_output_set(dev, output);
     }

   seat->devices = eina_list_append(seat->devices, dev);
}

static void
_device_removed(Ecore_Drm2_Input *input, struct libinput_device *device)
{
   Ecore_Drm2_Seat *seat;
   Ecore_Drm2_Input_Device *dev;

   dev = libinput_device_get_user_data(device);
   if (!dev) return;

   seat = _udev_seat_get(input, device);
   if (seat)
     seat->devices = eina_list_remove(seat->devices, dev);

   _ecore_drm2_input_device_destroy(dev);
}

static int
_udev_process_event(struct libinput_event *event)
{
   Ecore_Drm2_Launcher *launch;
   struct libinput *libinput;
   struct libinput_device *device;
   int ret = 1;

   libinput = libinput_event_get_context(event);
   device = libinput_event_get_device(event);
   launch = libinput_get_user_data(libinput);

   switch (libinput_event_get_type(event))
     {
      case LIBINPUT_EVENT_DEVICE_ADDED:
        DBG("Input Device Added: %s", libinput_device_get_name(device));
        _device_added(launch, device);
        break;
      case LIBINPUT_EVENT_DEVICE_REMOVED:
        DBG("Input Device Removed: %s", libinput_device_get_name(device));
        _device_removed(&launch->input, device);
        break;
      default:
        ret = 0;
     }

   return ret;
}

static void
_process_event(struct libinput_event *event)
{
   if (_udev_process_event(event)) return;
   if (_ecore_drm2_input_device_event_process(event)) return;
}

static void
_process_events(Ecore_Drm2_Input *input)
{
   struct libinput_event *event;

   while ((event = libinput_get_event(input->libinput)))
     {
        _process_event(event);
        libinput_event_destroy(event);
     }
}

static Eina_Bool
_cb_input_dispatch(void *data, Ecore_Fd_Handler *hdlr EINA_UNUSED)
{
   Ecore_Drm2_Input *input;

   input = data;

   if (libinput_dispatch(input->libinput) != 0)
     WRN("libinput failed to dispatch events");

   _process_events(input);

   return EINA_TRUE;
}

static Ecore_Drm2_Pointer *
_pointer_create(Ecore_Drm2_Seat *seat)
{
   Ecore_Drm2_Pointer *ptr;

   ptr = calloc(1, sizeof(Ecore_Drm2_Pointer));
   if (!ptr) return NULL;

   ptr->x = 100;
   ptr->y = 100;
   /* ptr->sx = -1000000; */
   /* ptr->sy = -1000000; */

   ptr->seat = seat;

   return ptr;
}

static Ecore_Drm2_Touch *
_touch_create(Ecore_Drm2_Seat *seat)
{
   Ecore_Drm2_Touch *touch;

   touch = calloc(1, sizeof(Ecore_Drm2_Touch));
   if (!touch) return NULL;

   touch->seat = seat;

   return touch;
}

static void
_touch_state_reset(Ecore_Drm2_Touch *touch)
{
   touch->points = 0;
}

Eina_Bool
_ecore_drm2_input_pointer_init(Ecore_Drm2_Seat *seat)
{
   Ecore_Drm2_Pointer *ptr;

   if (seat->ptr)
     {
        seat->count.ptr += 1;
        if (seat->count.ptr == 1)
          {
             /* TODO: update seat caps */
             return EINA_TRUE;
          }
     }

   ptr = _pointer_create(seat);
   if (!ptr) return EINA_FALSE;

   seat->ptr = ptr;
   seat->count.ptr = 1;

   /* TODO: update seat caps */

   return EINA_TRUE;
}

void
_ecore_drm2_input_pointer_release(Ecore_Drm2_Seat *seat)
{
   Ecore_Drm2_Pointer *ptr;

   ptr = seat->ptr;

   seat->count.ptr--;
   if (seat->count.ptr == 0)
     {
        ptr->buttons = 0;

        /* TODO: update seat caps */
     }
}

Ecore_Drm2_Pointer *
_ecore_drm2_input_pointer_get(Ecore_Drm2_Seat *seat)
{
   if (!seat) return NULL;
   if (seat->count.ptr) return seat->ptr;
   return NULL;
}

Eina_Bool
_ecore_drm2_input_keyboard_init(Ecore_Drm2_Seat *seat, struct xkb_keymap *keymap)
{
   Ecore_Drm2_Keyboard *kbd;

   if (seat->kbd)
     {
        seat->count.kbd += 1;
        if (seat->count.kbd == 1)
          {
             /* TODO: update seat caps */
             return EINA_TRUE;
          }
     }

   kbd = _keyboard_create(seat);
   if (!kbd) return EINA_FALSE;

   if (keymap)
     {
        kbd->info = _keyboard_info_create(keymap);
        if (!kbd->info) goto err;
     }
   else
     {
        if (!_keyboard_global_build(kbd)) goto err;
        kbd->info->refs++;
     }

   kbd->state = xkb_state_new(kbd->info->keymap.map);
   if (!kbd->state)
     {
        ERR("Failed to init XKB state");
        goto err;
     }

   seat->kbd = kbd;
   seat->count.kbd = 1;

   /* TODO: update seat caps */

   return EINA_TRUE;

err:
   if (kbd->info) _keyboard_info_destroy(kbd->info);
   free(kbd);
   return EINA_FALSE;
}

void
_ecore_drm2_input_keyboard_release(Ecore_Drm2_Seat *seat)
{
   seat->count.kbd--;
   if (seat->count.kbd == 0)
     {
        _keyboard_state_reset(seat->kbd);
        /* TODO: update seat caps */
     }
}

Ecore_Drm2_Keyboard *
_ecore_drm2_input_keyboard_get(Ecore_Drm2_Seat *seat)
{
   if (!seat) return NULL;
   if (seat->count.kbd) return seat->kbd;
   return NULL;
}

Eina_Bool
_ecore_drm2_input_touch_init(Ecore_Drm2_Seat *seat)
{
   Ecore_Drm2_Touch *touch;

   if (seat->touch)
     {
        seat->count.touch += 1;
        if (seat->count.touch == 1)
          {
             /* TODO: update seat caps */
             return EINA_TRUE;
          }
     }

   touch = _touch_create(seat);
   if (!touch) return EINA_FALSE;

   seat->touch = touch;
   seat->count.touch = 1;

   /* TODO: update seat caps */

   return EINA_TRUE;
}

void
_ecore_drm2_input_touch_release(Ecore_Drm2_Seat *seat)
{
   seat->count.touch--;
   if (seat->count.touch == 0)
     {
        _touch_state_reset(seat->touch);
        /* TODO: update seat caps */
     }
}

Ecore_Drm2_Touch *
_ecore_drm2_input_touch_get(Ecore_Drm2_Seat *seat)
{
   if (!seat) return NULL;
   if (seat->count.touch) return seat->touch;
   return NULL;
}

EAPI Eina_Bool
ecore_drm2_input_init(Ecore_Drm2_Launcher *launch, const char *seat)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(launch, EINA_FALSE);

   memset(&launch->input, 0, sizeof(Ecore_Drm2_Input));

   launch->input.libinput =
     libinput_udev_create_context(&_input_interface, launch, eeze_udev_get());
   if (!launch->input.libinput)
     {
        ERR("libinput could not create udev context");
        goto udev_err;
     }

   if (libinput_udev_assign_seat(launch->input.libinput, seat) != 0)
     {
        ERR("libinput could not assign udev seat");
        goto seat_err;
     }

   _process_events(&launch->input);

   ecore_drm2_input_enable(launch);

   return EINA_TRUE;

seat_err:
   libinput_unref(launch->input.libinput);
udev_err:
   return EINA_FALSE;
}

EAPI void
ecore_drm2_input_shutdown(Ecore_Drm2_Launcher *launcher)
{
   Ecore_Drm2_Seat *seat;

   EINA_SAFETY_ON_NULL_RETURN(launcher);
   EINA_SAFETY_ON_NULL_RETURN(&launcher->input);

   if (launcher->input.hdlr) ecore_main_fd_handler_del(launcher->input.hdlr);

   EINA_LIST_FREE(launcher->input.seats, seat)
     _udev_seat_destroy(seat);

   libinput_unref(launcher->input.libinput);
}

EAPI Eina_Bool
ecore_drm2_input_enable(Ecore_Drm2_Launcher *launcher)
{
   int fd;

   EINA_SAFETY_ON_NULL_RETURN_VAL(launcher, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(&launcher->input, EINA_FALSE);

   fd = libinput_get_fd(launcher->input.libinput);

   launcher->input.hdlr =
     ecore_main_fd_handler_add(fd, ECORE_FD_READ, _cb_input_dispatch,
                               &launcher->input, NULL, NULL);
   if (!launcher->input.hdlr)
     {
        ERR("Could not create input fd handler");
        return EINA_FALSE;
     }

   if (launcher->input.suspended)
     {
        if (libinput_resume(launcher->input.libinput) != 0)
          goto err;

        launcher->input.suspended = EINA_FALSE;
        _process_events(&launcher->input);
     }

   return EINA_TRUE;

err:
   if (launcher->input.hdlr) ecore_main_fd_handler_del(launcher->input.hdlr);
   launcher->input.hdlr = NULL;
   return EINA_FALSE;
}

EAPI void
ecore_drm2_input_pointer_warp(Ecore_Drm2_Launcher *launcher, int x, int y)
{
   Ecore_Drm2_Seat *seat;
   Ecore_Drm2_Input_Device *dev;
   Eina_List *l, *ll;

   EINA_SAFETY_ON_NULL_RETURN(launcher);

   EINA_LIST_FOREACH(launcher->input.seats, l, seat)
     {
        EINA_LIST_FOREACH(seat->devices, ll, dev)
          {
             if (dev->caps & EVDEV_SEAT_POINTER)
               {
                  seat->ptr->x = x;
                  seat->ptr->y = y;
                  /* TODO: post pointer motion event */
               }
          }
     }
}
