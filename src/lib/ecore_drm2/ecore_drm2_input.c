#include "ecore_drm2_private.h"

static int
_cb_open_restricted(const char *path, int flags, void *data)
{
   Ecore_Drm2_Input *input;

   input = data;

   return ecore_drm2_launcher_open(input->launcher, path, flags);
}

static void
_cb_close_restricted(int fd, void *data)
{
   Ecore_Drm2_Input *input;

   input = data;
   ecore_drm2_launcher_close(input->launcher, fd);
}

const struct libinput_interface _input_interface =
{
   _cb_open_restricted,
   _cb_close_restricted,
};

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
_device_added(Ecore_Drm2_Input *input, struct libinput_device *device)
{
   Ecore_Drm2_Seat *seat;
   Ecore_Drm2_Input_Device *dev;
   const char *oname;

   seat = _udev_seat_get(input, device);
   DBG("Got Seat: %s", seat->name);

   dev = _ecore_drm2_input_device_create(seat, device);

   /* TODO: get pointer */

   oname = libinput_device_get_output_name(device);
   if (oname)
     {
        DBG("\tOutput Name: %s", oname);
        dev->output_name = eina_stringshare_add(oname);
        /* TODO: loop outputs and set on dev */
     }
   /* else if (!dev->output) */
   /*   { */
   /* TODO */
   /*   } */

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
   Ecore_Drm2_Input *input;
   struct libinput *libinput;
   struct libinput_device *device;
   int ret = 1;

   libinput = libinput_event_get_context(event);
   device = libinput_event_get_device(event);
   input = libinput_get_user_data(libinput);

   switch (libinput_event_get_type(event))
     {
      case LIBINPUT_EVENT_DEVICE_ADDED:
        DBG("Udev Event: Device Added: %s", libinput_device_get_name(device));
        _device_added(input, device);
        break;
      case LIBINPUT_EVENT_DEVICE_REMOVED:
        DBG("Udev Event: Device Removed: %s", libinput_device_get_name(device));
        _device_removed(input, device);
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

EAPI void
ecore_drm2_input_shutdown(Ecore_Drm2_Input *input)
{
   Ecore_Drm2_Seat *seat;

   EINA_SAFETY_ON_NULL_RETURN(input);

   if (input->hdlr) ecore_main_fd_handler_del(input->hdlr);

   EINA_LIST_FREE(input->seats, seat)
     _udev_seat_destroy(seat);

   libinput_unref(input->libinput);
   free(input);
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
