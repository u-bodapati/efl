#define EFL_ANIMATION_PROTECTED

#include "evas_common_private.h"
#include "evas_private.h"

#define MY_CLASS EFL_ANIMATION_CLASS
#define MY_CLASS_NAME efl_class_name_get(MY_CLASS)

#define EFL_ANIMATION_CHECK_OR_RETURN(anim, ...) \
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

struct _Evas_Object_Animation_Data
{
   /*
    * FIXME: The following properties should be added.
    * Current Interpolation Function
    * Current State
    * State (State Change) Callbacks
    * Eina_Bool auto_delete;
    * int repeat_time;
    * int current_loop;               //Default value is 1
    * Eina_Bool is_reverse;
    */
   Eo       *target;

   double    duration;

   Eina_Bool deleted : 1;
};

EOLIAN static void
_efl_animation_target_set(Eo *eo_obj, Evas_Object_Animation_Data *pd, Evas_Object *target)
{
   EFL_ANIMATION_CHECK_OR_RETURN(eo_obj);

   pd->target = target;
}

EOLIAN static Evas_Object *
_efl_animation_target_get(Eo *eo_obj, Evas_Object_Animation_Data *pd)
{
   EFL_ANIMATION_CHECK_OR_RETURN(eo_obj, NULL);

   return pd->target;
}


EOLIAN static void
_efl_animation_duration_set(Eo *eo_obj, Evas_Object_Animation_Data *pd, double duration)
{
   EFL_ANIMATION_CHECK_OR_RETURN(eo_obj);

   if (duration <= 0.0) return;

   pd->duration = duration;
}

EOLIAN static double
_efl_animation_duration_get(Eo *eo_obj, Evas_Object_Animation_Data *pd)
{
   EFL_ANIMATION_CHECK_OR_RETURN(eo_obj, 0.0);

   return pd->duration;
}

EOLIAN static Eina_Bool
_efl_animation_is_deleted(Eo *eo_obj, Evas_Object_Animation_Data *pd)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(eo_obj, EINA_TRUE);

   return pd->deleted;
}

EOLIAN static Eina_Bool
_efl_animation_start(Eo *eo_obj, Evas_Object_Animation_Data *pd EINA_UNUSED)
{
   EFL_ANIMATION_CHECK_OR_RETURN(eo_obj, EINA_FALSE);

   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_efl_animation_cancel(Eo *eo_obj, Evas_Object_Animation_Data *pd EINA_UNUSED)
{
   EFL_ANIMATION_CHECK_OR_RETURN(eo_obj, EINA_FALSE);

   return EINA_TRUE;
}

EOLIAN static Efl_Object *
_efl_animation_efl_object_constructor(Eo *eo_obj, Evas_Object_Animation_Data *pd)
{
   eo_obj = efl_constructor(efl_super(eo_obj, MY_CLASS));

   pd->duration = 0.0;
   pd->target = NULL;

   pd->deleted = EINA_FALSE;

   return eo_obj;
}

EOLIAN static void
_efl_animation_efl_object_destructor(Eo *eo_obj, Evas_Object_Animation_Data *pd EINA_UNUSED)
{
   efl_destructor(efl_super(eo_obj, MY_CLASS));
}

#include "efl_animation.eo.c"
