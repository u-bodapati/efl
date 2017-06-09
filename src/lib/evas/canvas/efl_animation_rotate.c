#define EFL_ANIMATION_PROTECTED

#include "evas_common_private.h"
#include "evas_private.h"

#define MY_CLASS EFL_ANIMATION_ROTATE_CLASS
#define MY_CLASS_NAME efl_class_name_get(MY_CLASS)

#define EFL_ANIMATION_ROTATE_CHECK_OR_RETURN(anim, ...) \
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

typedef struct _Evas_Object_Animation_Rotate_Property
{
   double angle;
} Evas_Object_Animation_Rotate_Property;

typedef struct _Evas_Object_Animation_Rotate_Absolute_Pivot
{
   Evas_Coord x, y, z;
} Evas_Object_Animation_Rotate_Absolute_Pivot;

typedef struct _Evas_Object_Animation_Rotate_Relative_Pivot
{
   double x, y, z;
} Evas_Object_Animation_Rotate_Relative_Pivot;


struct _Evas_Object_Animation_Rotate_Data
{
   Evas_Object_Animation_Rotate_Property       from;
   Evas_Object_Animation_Rotate_Property       to;
   Evas_Object_Animation_Rotate_Absolute_Pivot abs_pivot;
   Evas_Object_Animation_Rotate_Relative_Pivot rel_pivot;
   Eina_Bool use_rel_pivot;
};

EOLIAN static void
_efl_animation_rotate_angle_set(Eo *eo_obj, Evas_Object_Animation_Rotate_Data *pd, double from_angle, double to_angle)
{
   EFL_ANIMATION_ROTATE_CHECK_OR_RETURN(eo_obj);

   pd->from.angle = from_angle;

   pd->to.angle = to_angle;
}

EOLIAN static void
_efl_animation_rotate_angle_get(Eo *eo_obj, Evas_Object_Animation_Rotate_Data *pd, double *from_angle, double *to_angle)
{
   EFL_ANIMATION_ROTATE_CHECK_OR_RETURN(eo_obj);

   if (from_angle)
     *from_angle = pd->from.angle;

   if (to_angle)
     *to_angle = pd->to.angle;
}

EOLIAN static void
_efl_animation_rotate_relative_pivot_set(Eo *eo_obj, Evas_Object_Animation_Rotate_Data *pd, double pivot_x, double pivot_y, double pivot_z)
{
   EFL_ANIMATION_ROTATE_CHECK_OR_RETURN(eo_obj);

   pd->rel_pivot.x = pivot_x;
   pd->rel_pivot.y = pivot_y;
   pd->rel_pivot.z = pivot_z;

   pd->use_rel_pivot = EINA_TRUE;
}

EOLIAN static void
_efl_animation_rotate_relative_pivot_get(Eo *eo_obj, Evas_Object_Animation_Rotate_Data *pd, double *pivot_x, double *pivot_y, double *pivot_z)
{
   EFL_ANIMATION_ROTATE_CHECK_OR_RETURN(eo_obj);

   if (pivot_x)
     *pivot_x = pd->rel_pivot.x;

   if (pivot_y)
     *pivot_y = pd->rel_pivot.y;

   if (pivot_z)
     *pivot_z = pd->rel_pivot.z;
}

EOLIAN static void
_efl_animation_rotate_absolute_pivot_set(Eo *eo_obj, Evas_Object_Animation_Rotate_Data *pd, Evas_Coord pivot_x, Evas_Coord pivot_y, Evas_Coord pivot_z)
{
   EFL_ANIMATION_ROTATE_CHECK_OR_RETURN(eo_obj);

   pd->abs_pivot.x = pivot_x;
   pd->abs_pivot.y = pivot_y;
   pd->abs_pivot.z = pivot_z;

   pd->use_rel_pivot = EINA_FALSE;
}

EOLIAN static void
_efl_animation_rotate_absolute_pivot_get(Eo *eo_obj, Evas_Object_Animation_Rotate_Data *pd, Evas_Coord *pivot_x, Evas_Coord *pivot_y, Evas_Coord *pivot_z)
{
   EFL_ANIMATION_ROTATE_CHECK_OR_RETURN(eo_obj);

   if (pivot_x)
     *pivot_x = pd->abs_pivot.x;

   if (pivot_y)
     *pivot_y = pd->abs_pivot.y;

   if (pivot_z)
     *pivot_z = pd->abs_pivot.z;
}

static void
_animate_cb(void *data, const Efl_Event *event)
{
   Efl_Animation_Animate_Event_Info *event_info = event->info;
}

EOLIAN static Efl_Object *
_efl_animation_rotate_efl_object_constructor(Eo *eo_obj, Evas_Object_Animation_Rotate_Data *pd)
{
   eo_obj = efl_constructor(efl_super(eo_obj, MY_CLASS));

   pd->from.angle = 0.0;
   pd->to.angle = 0.0;

   pd->rel_pivot.x = 0.5;
   pd->rel_pivot.y = 0.5;
   pd->rel_pivot.z = 0.5;

   pd->abs_pivot.x = 0;
   pd->abs_pivot.y = 0;
   pd->abs_pivot.z = 0;

   pd->use_rel_pivot = EINA_TRUE;

   efl_event_callback_add(eo_obj, EFL_ANIMATION_EVENT_ANIMATE, _animate_cb, NULL);

   return eo_obj;
}

EOLIAN static void
_efl_animation_rotate_efl_object_destructor(Eo *eo_obj, Evas_Object_Animation_Rotate_Data *pd EINA_UNUSED)
{
   efl_destructor(efl_super(eo_obj, MY_CLASS));
}

#include "efl_animation_rotate.eo.c"
