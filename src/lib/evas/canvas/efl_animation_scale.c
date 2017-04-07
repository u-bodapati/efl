#define EFL_ANIMATION_PROTECTED

#include "evas_common_private.h"
#include "evas_private.h"

#define MY_CLASS EFL_ANIMATION_SCALE_CLASS
#define MY_CLASS_NAME efl_class_name_get(MY_CLASS)

#define EFL_ANIMATION_SCALE_CHECK_OR_RETURN(anim, ...) \
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

typedef struct _Evas_Object_Animation_Scale_Property
{
   double scale_x, scale_y, scale_z;
} Evas_Object_Animation_Scale_Property;

struct _Evas_Object_Animation_Scale_Data
{
   Evas_Object_Animation_Scale_Property from;
   Evas_Object_Animation_Scale_Property to;
};

EOLIAN static void
_efl_animation_scale_scale_set(Eo *eo_obj, Evas_Object_Animation_Scale_Data *pd, double from_scale, double to_scale)
{
   EFL_ANIMATION_SCALE_CHECK_OR_RETURN(eo_obj);

   pd->from.scale_x = from_scale;
   pd->from.scale_y = from_scale;
   pd->from.scale_z = from_scale;

   pd->to.scale_x = to_scale;
   pd->to.scale_y = to_scale;
   pd->to.scale_z = to_scale;
}

EOLIAN static void
_efl_animation_scale_scale_get(Eo *eo_obj, Evas_Object_Animation_Scale_Data *pd, double *from_scale, double *to_scale)
{
   EFL_ANIMATION_SCALE_CHECK_OR_RETURN(eo_obj);

   if (from_scale)
     *from_scale = pd->from.scale_x;

   if (to_scale)
     *to_scale = pd->to.scale_x;
}

EOLIAN static void
_efl_animation_scale_scale_x_set(Eo *eo_obj, Evas_Object_Animation_Scale_Data *pd, double from_x, double to_x)
{
   EFL_ANIMATION_SCALE_CHECK_OR_RETURN(eo_obj);

   pd->from.scale_x = from_x;

   pd->to.scale_x = to_x;
}

EOLIAN static void
_efl_animation_scale_scale_x_get(Eo *eo_obj, Evas_Object_Animation_Scale_Data *pd, double *from_x, double *to_x)
{
   EFL_ANIMATION_SCALE_CHECK_OR_RETURN(eo_obj);

   if (from_x)
     *from_x = pd->from.scale_x;

   if (to_x)
     *to_x = pd->to.scale_x;
}

EOLIAN static void
_efl_animation_scale_scale_y_set(Eo *eo_obj, Evas_Object_Animation_Scale_Data *pd, double from_y, double to_y)
{
   EFL_ANIMATION_SCALE_CHECK_OR_RETURN(eo_obj);

   pd->from.scale_y = from_y;

   pd->to.scale_y = to_y;
}

EOLIAN static void
_efl_animation_scale_scale_y_get(Eo *eo_obj, Evas_Object_Animation_Scale_Data *pd, double *from_y, double *to_y)
{
   EFL_ANIMATION_SCALE_CHECK_OR_RETURN(eo_obj);

   if (from_y)
     *from_y = pd->from.scale_y;

   if (to_y)
     *to_y = pd->to.scale_y;
}

EOLIAN static void
_efl_animation_scale_scale_z_set(Eo *eo_obj, Evas_Object_Animation_Scale_Data *pd, double from_z, double to_z)
{
   EFL_ANIMATION_SCALE_CHECK_OR_RETURN(eo_obj);

   pd->from.scale_z = from_z;

   pd->to.scale_z = to_z;
}

EOLIAN static void
_efl_animation_scale_scale_z_get(Eo *eo_obj, Evas_Object_Animation_Scale_Data *pd, double *from_z, double *to_z)
{
   EFL_ANIMATION_SCALE_CHECK_OR_RETURN(eo_obj);

   if (from_z)
     *from_z = pd->from.scale_z;

   if (to_z)
     *to_z = pd->to.scale_z;
}

EOLIAN static Efl_Object *
_efl_animation_scale_efl_object_constructor(Eo *eo_obj, Evas_Object_Animation_Scale_Data *pd)
{
   eo_obj = efl_constructor(efl_super(eo_obj, MY_CLASS));

   pd->from.scale_x = 1.0;
   pd->from.scale_y = 1.0;
   pd->from.scale_z = 1.0;

   return eo_obj;
}

EOLIAN static void
_efl_animation_scale_efl_object_destructor(Eo *eo_obj, Evas_Object_Animation_Scale_Data *pd EINA_UNUSED)
{
   efl_destructor(efl_super(eo_obj, MY_CLASS));
}

#include "efl_animation_scale.eo.c"
