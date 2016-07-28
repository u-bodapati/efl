#ifndef ELM_WIDGET_IMAGE_H
#define ELM_WIDGET_IMAGE_H

#include "Elementary.h"

/* DO NOT USE THIS HEADER UNLESS YOU ARE PREPARED FOR BREAKING OF YOUR
 * CODE. THIS IS ELEMENTARY'S INTERNAL WIDGET API (for now) AND IS NOT
 * FINAL. CALL elm_widget_api_check(ELM_INTERNAL_API_VERSION) TO CHECK
 * IT AT RUNTIME.
 */

typedef struct _Async_Open_Data Async_Open_Data;
typedef enum
  {
     EFL_UI_IMAGE_PRELOAD_ENABLED,
     EFL_UI_IMAGE_PRELOADING,
     EFL_UI_IMAGE_PRELOADED,
     EFL_UI_IMAGE_PRELOAD_DISABLED
  } Efl_Ui_Image_Preload_Status;

/**
 * @addtogroup Widget
 * @{
 *
 * @section elm-image-class The Elementary Image Class
 *
 * This class defines a common interface for @b image objects having
 * an image as their basic graphics. This interface is so that one can
 * tune various properties of the image, like:
 * - smooth scaling,
 * - orientation,
 * - aspect ratio during resizes, etc.
 *
 * Image files may be set via memory buffers, image files, EET files
 * with image data or Edje files. On the last case (which is
 * exceptional), most of the properties cited above will @b not be
 * changeable anymore.
 */

/**
 * Base widget smart data extended with image instance data.
 */
typedef struct _Efl_Ui_Image_Data Efl_Ui_Image_Data;
struct _Efl_Ui_Image_Data
{
   Evas_Object          *hit_rect;
   Evas_Object          *img;
   Evas_Object          *prev_img;
   Ecore_Timer          *anim_timer;

   Elm_Url              *remote;
   const char           *key;
   void                 *remote_data;

   double                scale;
   double                frame_duration;
   double                align_x, align_y;

   Evas_Coord            img_x, img_y, img_w, img_h;

   int                   load_size;
   int                   frame_count;
   int                   cur_frame;

   Elm_Image_Orient      image_orient; // to support EAPI
   Efl_Orient            orient;
   Efl_Flip              flip;

   struct {
      Ecore_Thread      *th;
      Async_Open_Data   *todo, *done;
      Eina_Stringshare  *file, *key; // only for file_get()
      Eina_Spinlock      lck;
   } async;

   Efl_Ui_Image_Preload_Status preload_status;
   Efl_Ui_Image_Scale_Type scale_type;

   const char           *stdicon;


   struct {
      int       requested_size;
      Eina_Bool use : 1;
   } freedesktop;

   Eina_Bool             aspect_fixed : 1;
   Eina_Bool             fill_inside : 1;
   Eina_Bool             no_scale : 1;
   Eina_Bool             smooth : 1;
   Eina_Bool             show : 1;
   Eina_Bool             edit : 1;
   Eina_Bool             edje : 1;
   Eina_Bool             anim : 1;
   Eina_Bool             play : 1;
   Eina_Bool             async_enable : 1;
   Eina_Bool             async_opening : 1;
   Eina_Bool             async_failed : 1;
   Eina_Bool             scale_up : 1;
   Eina_Bool             scale_down : 1;
};

/**
 * @}
 */

#define EFL_UI_IMAGE_DATA_GET(o, sd) \
  Efl_Ui_Image_Data * sd = eo_data_scope_get(o, EFL_UI_IMAGE_CLASS)

#define EFL_UI_IMAGE_DATA_GET_OR_RETURN(o, ptr)         \
  EFL_UI_IMAGE_DATA_GET(o, ptr);                        \
  if (EINA_UNLIKELY(!ptr))                           \
    {                                                \
       CRI("No widget data for object %p (%s)",      \
           o, evas_object_type_get(o));              \
       return;                                       \
    }

#define EFL_UI_IMAGE_DATA_GET_OR_RETURN_VAL(o, ptr, val) \
  EFL_UI_IMAGE_DATA_GET(o, ptr);                         \
  if (EINA_UNLIKELY(!ptr))                            \
    {                                                 \
       CRI("No widget data for object %p (%s)",       \
           o, evas_object_type_get(o));               \
       return val;                                    \
    }

#define EFL_UI_IMAGE_CHECK(obj)                              \
  if (EINA_UNLIKELY(!eo_isa((obj), EFL_UI_IMAGE_CLASS))) \
    return

#endif