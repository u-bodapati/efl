#define EFL_ANIMATION_PROTECTED

#include "evas_common_private.h"
#include "evas_private.h"

#define MY_CLASS EFL_ANIMATION_ALPHA_CLASS
#define MY_CLASS_NAME efl_class_name_get(MY_CLASS)

#define EFL_ANIMATION_ALPHA_CHECK_OR_RETURN(anim, ...) \
   do { \
      if (!anim) { \
         CRI("Efl_Animation " # anim " is NULL!"); \
         return __VA_ARGS__; \
      } \
      if (efl_animation_is_deleted(anim)) { \
         ERR("Efl_Animation " # anim " has already been deleted!"); \
         return __VA_ARGS__; \
      } \
   } while (0)

#define EFL_ANIMATION_ALPHA_DATA_GET(o, pd) \
   Evas_Object_Animation_Alpha_Data *pd = efl_data_scope_get(o, EFL_ANIMATION_ALPHA_CLASS)

typedef struct _Evas_Object_Animation_Alpha_Property
{
   double alpha;
} Evas_Object_Animation_Alpha_Property;

struct _Evas_Object_Animation_Alpha_Data
{
   Evas_Object_Animation_Alpha_Property from;
   Evas_Object_Animation_Alpha_Property to;
};

EOLIAN static void
_efl_animation_alpha_alpha_set(Eo *eo_obj, Evas_Object_Animation_Alpha_Data *pd, double from_alpha, double to_alpha)
{
   EFL_ANIMATION_ALPHA_CHECK_OR_RETURN(eo_obj);

   pd->from.alpha = from_alpha;
   pd->to.alpha = to_alpha;
}

EOLIAN static void
_efl_animation_alpha_alpha_get(Eo *eo_obj EINA_UNUSED, Evas_Object_Animation_Alpha_Data *pd, double *from_alpha, double *to_alpha)
{
   EFL_ANIMATION_ALPHA_CHECK_OR_RETURN(eo_obj);

   if (from_alpha)
     *from_alpha = pd->from.alpha;
   if (to_alpha)
     *to_alpha = pd->to.alpha;
}

static void
_animate_cb(void *data EINA_UNUSED, const Efl_Event *event)
{
   EFL_ANIMATION_ALPHA_DATA_GET(event->object, pd);
   Efl_Animation_Animate_Event_Info *event_info = event->info;

   double progress = event_info->progress;

   double alpha
      = (pd->from.alpha * (1.0 - progress)) + (pd->to.alpha * progress);

   //FIXME: The below code is temporary test code
   int color = (int)(alpha * 255);
   Evas_Object *target = efl_animation_target_get(event->object);
   evas_object_color_set(target, color, color, color, color);
}

EOLIAN static Efl_Object *
_efl_animation_alpha_efl_object_constructor(Eo *eo_obj, Evas_Object_Animation_Alpha_Data *pd)
{
   eo_obj = efl_constructor(efl_super(eo_obj, MY_CLASS));

   pd->from.alpha = 1.0;
   pd->to.alpha = 1.0;

   efl_event_callback_add(eo_obj, EFL_ANIMATION_EVENT_ANIMATE, _animate_cb, NULL);

   return eo_obj;
}

EOLIAN static void
_efl_animation_alpha_efl_object_destructor(Eo *eo_obj, Evas_Object_Animation_Alpha_Data *pd EINA_UNUSED)
{
   efl_destructor(efl_super(eo_obj, MY_CLASS));
}

#include "efl_animation_alpha.eo.c"
