/**
 * Edje example demonstrating how to use multiseat filtering.
 *
 * It presents 4 widgets that can be controlled by:
 *   * seat1 only
 *   * seat2 only
 *   * seat1 + seat2
 *   * any seat
 *
 * You'll need at least one Evas engine built for it (excluding the
 * buffer one) that supports multiseat. It may be wayland or
 * X11 with VNC support. Using other engines will lead you to a
 * situation where all seats are reported as the same one ("default").
 *
 * @verbatim
 * edje_cc multiseat_filter.edc && gcc -o edje-multiseat-filter edje-multiseat-filter.c `pkg-config --libs --cflags evas ecore ecore-evas edje`
 * @endverbatim
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#else
# define EINA_UNUSED
#endif

#ifndef PACKAGE_DATA_DIR
#define PACKAGE_DATA_DIR "."
#endif

#include <Ecore.h>
#include <Ecore_Evas.h>
#include <Edje.h>

#define WIDTH  400
#define HEIGHT 400

static const char *GROUPNAME = "example/main";
static const char *EDJE_FILE = PACKAGE_DATA_DIR"/multiseat_filter.edj";

static void
_on_destroy(Ecore_Evas *ee EINA_UNUSED)
{
   ecore_main_loop_quit();
}

static void
_on_canvas_resize(Ecore_Evas *ee)
{
   Evas_Object *edje_obj;
   int w, h;

   edje_obj = ecore_evas_data_get(ee, "edje_obj");

   ecore_evas_geometry_get(ee, NULL, NULL, &w, &h);
   evas_object_resize(edje_obj, w, h);
}

static void
_device_added(void *data, const Efl_Event *event)
{
   Efl_Input_Device *dev = event->info;
   Evas_Object *edje_obj = data;

   if (efl_input_device_type_get(dev) != EFL_INPUT_DEVICE_TYPE_SEAT)
     return;

   efl_canvas_object_seat_focus_add(edje_obj, dev);
}

int
main(int argc EINA_UNUSED, char *argv[] EINA_UNUSED)
{
   const Eina_List *devices, *l;
   Efl_Input_Device *dev;
   Evas_Object *edje_obj;
   Ecore_Evas *ee;
   Evas *evas;

   if (!ecore_evas_init())
     return EXIT_FAILURE;

   if (!edje_init())
     goto shutdown_ecore_evas;

   ee = ecore_evas_new(NULL, 0, 0, WIDTH, HEIGHT, NULL);
   if (!ee) goto shutdown_edje;

   ecore_evas_callback_destroy_set(ee, _on_destroy);
   ecore_evas_callback_resize_set(ee, _on_canvas_resize);
   ecore_evas_title_set(ee, "Edje Multiseat Filter Example");

   evas = ecore_evas_get(ee);

   edje_obj = edje_object_add(evas);

   if (!edje_object_file_set(edje_obj, EDJE_FILE, GROUPNAME))
     printf("failed to set file %s.\n", EDJE_FILE);

   evas_object_move(edje_obj, 0, 0);
   evas_object_resize(edje_obj, WIDTH, HEIGHT);
   evas_object_show(edje_obj);
   ecore_evas_data_set(ee, "edje_obj", edje_obj);

   devices = evas_device_list(evas, NULL);
   EINA_LIST_FOREACH(devices, l, dev)
     {
        if (efl_input_device_type_get(dev) == EFL_INPUT_DEVICE_TYPE_SEAT)
          efl_canvas_object_seat_focus_add(edje_obj, dev);

     }
   efl_event_callback_add(evas, EFL_CANVAS_EVENT_DEVICE_ADDED,
                          _device_added, edje_obj);

   ecore_evas_show(ee);

   ecore_main_loop_begin();

   ecore_evas_free(ee);
   ecore_evas_shutdown();
   edje_shutdown();

   return EXIT_SUCCESS;

shutdown_edje:
   edje_shutdown();
shutdown_ecore_evas:
   ecore_evas_shutdown();

   return EXIT_FAILURE;
}
