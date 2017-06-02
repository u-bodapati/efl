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

#define EFL_ANIMATION_ROTATE_DATA_GET(o, pd) \
   Evas_Object_Animation_Rotate_Data *pd = efl_data_scope_get(o, EFL_ANIMATION_ROTATE_CLASS)

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

EOLIAN static Efl_Animation *
_efl_animation_rotate_efl_animation_dup(Eo *eo_obj, Evas_Object_Animation_Rotate_Data *pd)
{
   EFL_ANIMATION_ROTATE_CHECK_OR_RETURN(eo_obj, NULL);

   Efl_Animation_Rotate *animation = efl_add(MY_CLASS, NULL);

   double duration = efl_animation_duration_get(eo_obj);
   efl_animation_duration_set(animation, duration);

   Eo *target = efl_animation_target_get(eo_obj);
   efl_animation_target_set(animation, target);

   Eina_Bool state_keep = efl_animation_final_state_keep_get(eo_obj);
   efl_animation_final_state_keep_set(animation, state_keep);

   EFL_ANIMATION_ROTATE_DATA_GET(animation, new_pd);

   new_pd->from.angle = pd->from.angle;
   new_pd->to.angle = pd->to.angle;

   new_pd->rel_pivot.x = pd->rel_pivot.x;
   new_pd->rel_pivot.y = pd->rel_pivot.y;
   new_pd->rel_pivot.z = pd->rel_pivot.z;

   new_pd->abs_pivot.x = pd->rel_pivot.x;
   new_pd->abs_pivot.y = pd->rel_pivot.y;
   new_pd->abs_pivot.z = pd->rel_pivot.z;

   new_pd->use_rel_pivot = pd->use_rel_pivot;

   return animation;
}

EOLIAN static Efl_Animation_Instance *
_efl_animation_rotate_efl_animation_instance_create(Eo *eo_obj, Evas_Object_Animation_Rotate_Data *pd)
{
   EFL_ANIMATION_ROTATE_CHECK_OR_RETURN(eo_obj, NULL);

   Efl_Animation_Instance_Rotate *instance
      = efl_add(EFL_ANIMATION_INSTANCE_ROTATE_CLASS, NULL);

   Eo *target = efl_animation_target_get(eo_obj);
   efl_animation_instance_target_set(instance, target);

   Eina_Bool state_keep = efl_animation_final_state_keep_get(eo_obj);
   efl_animation_instance_final_state_keep_set(instance, state_keep);

   double duration = efl_animation_duration_get(eo_obj);
   efl_animation_instance_duration_set(instance, duration);

   efl_animation_instance_rotate_angle_set(instance,
                                           pd->from.angle, pd->to.angle);

   efl_animation_instance_rotate_relative_pivot_set(instance,
                                                    pd->rel_pivot.x,
                                                    pd->rel_pivot.y,
                                                    pd->rel_pivot.z);

   efl_animation_instance_rotate_absolute_pivot_set(instance,
                                                    pd->abs_pivot.x,
                                                    pd->abs_pivot.y,
                                                    pd->abs_pivot.z);

   return instance;
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

   return eo_obj;
}

EOLIAN static void
_efl_animation_rotate_efl_object_destructor(Eo *eo_obj, Evas_Object_Animation_Rotate_Data *pd EINA_UNUSED)
{
   efl_destructor(efl_super(eo_obj, MY_CLASS));
}

#include "efl_animation_rotate.eo.c"
