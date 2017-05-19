#define EFL_ANIMATION_PROTECTED

#include "evas_common_private.h"
#include "evas_private.h"

#define MY_CLASS EFL_ANIMATION_TRANSLATE_CLASS
#define MY_CLASS_NAME efl_class_name_get(MY_CLASS)

#define EFL_ANIMATION_TRANSLATE_CHECK_OR_RETURN(anim, ...) \
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

#define EFL_ANIMATION_TRANSLATE_DATA_GET(o, pd) \
   Evas_Object_Animation_Translate_Data *pd = efl_data_scope_get(o, EFL_ANIMATION_TRANSLATE_CLASS)

typedef struct _Evas_Object_Animation_Translate_Property
{
   Evas_Coord move_x, move_y, move_z;
   Evas_Coord x, y, z;
} Evas_Object_Animation_Translate_Property;

struct _Evas_Object_Animation_Translate_Data
{
   Evas_Object_Animation_Translate_Property from;
   Evas_Object_Animation_Translate_Property to;
};

EOLIAN static void
_efl_animation_translate_translate_set(Eo *eo_obj, Evas_Object_Animation_Translate_Data *pd, Evas_Coord from_x, Evas_Coord from_y, Evas_Coord from_z, Evas_Coord to_x, Evas_Coord to_y, Evas_Coord to_z)
{
   EFL_ANIMATION_TRANSLATE_CHECK_OR_RETURN(eo_obj);

   pd->from.move_x = from_x;
   pd->from.move_y = from_y;
   pd->from.move_z = from_z;

   pd->to.move_x = to_x;
   pd->to.move_y = to_y;
   pd->to.move_z = to_z;
}

EOLIAN static void
_efl_animation_translate_translate_get(Eo *eo_obj, Evas_Object_Animation_Translate_Data *pd, Evas_Coord *from_x, Evas_Coord *from_y, Evas_Coord *from_z, Evas_Coord *to_x, Evas_Coord *to_y, Evas_Coord *to_z)
{
   EFL_ANIMATION_TRANSLATE_CHECK_OR_RETURN(eo_obj);

   if (from_x)
     *from_x = pd->from.move_x;
   if (from_y)
     *from_y = pd->from.move_y;
   if (from_z)
     *from_z = pd->from.move_z;

   if (to_x)
     *to_x = pd->to.move_x;
   if (to_y)
     *to_y = pd->to.move_y;
   if (to_z)
     *to_z = pd->to.move_z;
}

EOLIAN static void
_efl_animation_translate_translate_x_set(Eo *eo_obj, Evas_Object_Animation_Translate_Data *pd, Evas_Coord from_x, Evas_Coord to_x)
{
   EFL_ANIMATION_TRANSLATE_CHECK_OR_RETURN(eo_obj);

   pd->from.move_x = from_x;

   pd->to.move_x = to_x;
}

EOLIAN static void
_efl_animation_translate_translate_x_get(Eo *eo_obj, Evas_Object_Animation_Translate_Data *pd, Evas_Coord *from_x, Evas_Coord *to_x)
{
   EFL_ANIMATION_TRANSLATE_CHECK_OR_RETURN(eo_obj);

   if (from_x)
     *from_x = pd->from.move_x;

   if (to_x)
     *to_x = pd->to.move_x;
}

EOLIAN static void
_efl_animation_translate_translate_y_set(Eo *eo_obj, Evas_Object_Animation_Translate_Data *pd, Evas_Coord from_y, Evas_Coord to_y)
{
   EFL_ANIMATION_TRANSLATE_CHECK_OR_RETURN(eo_obj);

   pd->from.move_y = from_y;

   pd->to.move_y = to_y;
}

EOLIAN static void
_efl_animation_translate_translate_y_get(Eo *eo_obj, Evas_Object_Animation_Translate_Data *pd, Evas_Coord *from_y, Evas_Coord *to_y)
{
   EFL_ANIMATION_TRANSLATE_CHECK_OR_RETURN(eo_obj);

   if (from_y)
     *from_y = pd->from.move_y;

   if (to_y)
     *to_y = pd->to.move_y;
}

EOLIAN static void
_efl_animation_translate_translate_z_set(Eo *eo_obj, Evas_Object_Animation_Translate_Data *pd, Evas_Coord from_z, Evas_Coord to_z)
{
   EFL_ANIMATION_TRANSLATE_CHECK_OR_RETURN(eo_obj);

   pd->from.move_z = from_z;

   pd->to.move_z = to_z;
}

EOLIAN static void
_efl_animation_translate_translate_z_get(Eo *eo_obj, Evas_Object_Animation_Translate_Data *pd, Evas_Coord *from_z, Evas_Coord *to_z)
{
   EFL_ANIMATION_TRANSLATE_CHECK_OR_RETURN(eo_obj);

   if (from_z)
     *from_z = pd->from.move_z;

   if (to_z)
     *to_z = pd->to.move_z;
}

EOLIAN static void
_efl_animation_translate_coordinate_set(Eo *eo_obj, Evas_Object_Animation_Translate_Data *pd, Evas_Coord from_x, Evas_Coord from_y, Evas_Coord from_z, Evas_Coord to_x, Evas_Coord to_y, Evas_Coord to_z)
{
   EFL_ANIMATION_TRANSLATE_CHECK_OR_RETURN(eo_obj);

   pd->from.x = from_x;
   pd->from.y = from_y;
   pd->from.z = from_z;

   pd->to.x = to_x;
   pd->to.y = to_y;
   pd->to.z = to_z;
}

EOLIAN static void
_efl_animation_translate_coordinate_get(Eo *eo_obj, Evas_Object_Animation_Translate_Data *pd, Evas_Coord *from_x, Evas_Coord *from_y, Evas_Coord *from_z, Evas_Coord *to_x, Evas_Coord *to_y, Evas_Coord *to_z)
{
   EFL_ANIMATION_TRANSLATE_CHECK_OR_RETURN(eo_obj);

   if (from_x)
     *from_x = pd->from.x;
   if (from_y)
     *from_y = pd->from.y;
   if (from_z)
     *from_z = pd->from.z;

   if (to_x)
     *to_x = pd->to.x;
   if (to_y)
     *to_y = pd->to.y;
   if (to_z)
     *to_z = pd->to.z;
}

EOLIAN static void
_efl_animation_translate_coordinate_x_set(Eo *eo_obj, Evas_Object_Animation_Translate_Data *pd, Evas_Coord from_x, Evas_Coord to_x)
{
   EFL_ANIMATION_TRANSLATE_CHECK_OR_RETURN(eo_obj);

   pd->from.x = from_x;

   pd->to.x = to_x;
}

EOLIAN static void
_efl_animation_translate_coordinate_x_get(Eo *eo_obj, Evas_Object_Animation_Translate_Data *pd, Evas_Coord *from_x, Evas_Coord *to_x)
{
   EFL_ANIMATION_TRANSLATE_CHECK_OR_RETURN(eo_obj);

   if (from_x)
     *from_x = pd->from.x;

   if (to_x)
     *to_x = pd->to.x;
}

EOLIAN static void
_efl_animation_translate_coordinate_y_set(Eo *eo_obj, Evas_Object_Animation_Translate_Data *pd, Evas_Coord from_y, Evas_Coord to_y)
{
   EFL_ANIMATION_TRANSLATE_CHECK_OR_RETURN(eo_obj);

   pd->from.y = from_y;

   pd->to.y = to_y;
}

EOLIAN static void
_efl_animation_translate_coordinate_y_get(Eo *eo_obj, Evas_Object_Animation_Translate_Data *pd, Evas_Coord *from_y, Evas_Coord *to_y)
{
   EFL_ANIMATION_TRANSLATE_CHECK_OR_RETURN(eo_obj);

   if (from_y)
     *from_y = pd->from.y;

   if (to_y)
     *to_y = pd->to.y;
}

EOLIAN static void
_efl_animation_translate_coordinate_z_set(Eo *eo_obj, Evas_Object_Animation_Translate_Data *pd, Evas_Coord from_z, Evas_Coord to_z)
{
   EFL_ANIMATION_TRANSLATE_CHECK_OR_RETURN(eo_obj);

   pd->from.z = from_z;

   pd->to.z = to_z;
}

EOLIAN static void
_efl_animation_translate_coordinate_z_get(Eo *eo_obj, Evas_Object_Animation_Translate_Data *pd, Evas_Coord *from_z, Evas_Coord *to_z)
{
   EFL_ANIMATION_TRANSLATE_CHECK_OR_RETURN(eo_obj);

   if (from_z)
     *from_z = pd->from.z;

   if (to_z)
     *to_z = pd->to.z;
}

EOLIAN static Efl_Animation *
_efl_animation_translate_efl_animation_dup(Eo *eo_obj, Evas_Object_Animation_Translate_Data *pd)
{
   EFL_ANIMATION_TRANSLATE_CHECK_OR_RETURN(eo_obj, NULL);

   Efl_Animation_Translate *animation = efl_add(MY_CLASS, NULL);

   double duration = efl_animation_duration_get(eo_obj);
   efl_animation_duration_set(animation, duration);

   Eo *target = efl_animation_target_get(eo_obj);
   efl_animation_target_set(animation, target);

   Eina_Bool state_keep = efl_animation_final_state_keep_get(eo_obj);
   efl_animation_final_state_keep_set(animation, state_keep);

   EFL_ANIMATION_TRANSLATE_DATA_GET(animation, new_pd);

   new_pd->from.move_x = pd->from.move_x;
   new_pd->from.move_y = pd->from.move_y;
   new_pd->from.move_z = pd->from.move_z;

   new_pd->from.x = pd->from.x;
   new_pd->from.y = pd->from.y;
   new_pd->from.z = pd->from.z;

   new_pd->to.move_x = pd->to.move_x;
   new_pd->to.move_y = pd->to.move_y;
   new_pd->to.move_z = pd->to.move_z;

   new_pd->to.x = pd->to.x;
   new_pd->to.y = pd->to.y;
   new_pd->to.z = pd->to.z;

   return animation;
}

static void
_pre_animate_cb(void *data, const Efl_Event *event)
{
   Efl_Animation_Animate_Event_Info *event_info = event->info;
}

EOLIAN static Efl_Object *
_efl_animation_translate_efl_object_constructor(Eo *eo_obj, Evas_Object_Animation_Translate_Data *pd)
{
   eo_obj = efl_constructor(efl_super(eo_obj, MY_CLASS));

   pd->from.move_x = 0;
   pd->from.move_y = 0;
   pd->from.move_z = 0;

   pd->from.x = 0;
   pd->from.y = 0;
   pd->from.z = 0;

   pd->to.move_x = 0;
   pd->to.move_y = 0;
   pd->to.move_z = 0;

   pd->to.x = 0;
   pd->to.y = 0;
   pd->to.z = 0;

   //pre animate event is supported within class only (protected event)
   efl_event_callback_add(eo_obj, EFL_ANIMATION_EVENT_PRE_ANIMATE, _pre_animate_cb, NULL);

   return eo_obj;
}

EOLIAN static void
_efl_animation_translate_efl_object_destructor(Eo *eo_obj, Evas_Object_Animation_Translate_Data *pd EINA_UNUSED)
{
   efl_destructor(efl_super(eo_obj, MY_CLASS));
}

#include "efl_animation_translate.eo.c"
