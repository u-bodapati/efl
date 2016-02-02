#include "ecore_drm2_private.h"

static int
_cb_open_restricted(const char *path, int flags, void *data)
{
   return -1;
}

static void
_cb_close_restricted(int fd, void *data)
{

}

const struct libinput_interface _input_interface =
{
   _cb_open_restricted,
   _cb_close_restricted,
};

static int
_udev_process_event(struct libinput_event *event)
{
   /* struct libinput *libinput; */
   struct libinput_device *device;
   int ret = 1;

   /* libinput = libinput_event_get_context(event); */
   device = libinput_event_get_device(event);

   switch (libinput_event_get_type(event))
     {
      case LIBINPUT_EVENT_DEVICE_ADDED:
        DBG("Udev Event: Device Added: %s", libinput_device_get_name(device));
        break;
      case LIBINPUT_EVENT_DEVICE_REMOVED:
        DBG("Udev Event: Device Removed: %s", libinput_device_get_name(device));
        break;
      default:
        ret = 0;
     }

   return ret;
}

static void
_process_event(struct libinput_event *event)
{
   DBG("Process Input Event");
   if (_udev_process_event(event)) return;
   /* if (_evdev_process_event(event)) return; */
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

EAPI Ecore_Drm2_Input *
ecore_drm2_input_init(Ecore_Drm2_Launcher *launcher, const char *seat)
{
   Ecore_Drm2_Input *input;

   EINA_SAFETY_ON_NULL_RETURN_VAL(seat, EINA_FALSE);

   input = calloc(1, sizeof(Ecore_Drm2_Input));
   if (!input) return NULL;

   input->launcher = launcher;

   input->libinput =
     libinput_udev_create_context(&_input_interface, input, eeze_udev_get());
   if (!input->libinput)
     {
        ERR("libinput could not create udev context");
        goto udev_err;
     }

   if (libinput_udev_assign_seat(input->libinput, seat) != 0)
     {
        ERR("libinput could not assign udev seat");
        goto seat_err;
     }

   _process_events(input);

   ecore_drm2_input_enable(input);

   return input;

seat_err:
   libinput_unref(input->libinput);
udev_err:
   free(input);
   return NULL;
}

EAPI Eina_Bool
ecore_drm2_input_enable(Ecore_Drm2_Input *input)
{
   int fd;

   EINA_SAFETY_ON_NULL_RETURN_VAL(input, EINA_FALSE);

   fd = libinput_get_fd(input->libinput);

   input->hdlr =
     ecore_main_fd_handler_add(fd, ECORE_FD_READ, _cb_input_dispatch, input,
                               NULL, NULL);
   if (!input->hdlr)
     {
        ERR("Could not create input fd handler");
        return EINA_FALSE;
     }

   if (input->suspended)
     {
        if (libinput_resume(input->libinput) != 0)
          goto err;

        input->suspended = EINA_FALSE;
        _process_events(input);
     }

   return EINA_TRUE;

err:
   if (input->hdlr) ecore_main_fd_handler_del(input->hdlr);
   input->hdlr = NULL;
   return EINA_FALSE;
}
