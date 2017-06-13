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

#define EFL_ANIMATION_SCALE_DATA_GET(o, pd) \
   Evas_Object_Animation_Scale_Data *pd = efl_data_scope_get(o, EFL_ANIMATION_SCALE_CLASS)

typedef struct _Evas_Object_Animation_Scale_Property
{
   double scale_x, scale_y;
} Evas_Object_Animation_Scale_Property;

typedef struct _Evas_Object_Animation_Scale_Absolute_Pivot
{
   Evas_Coord cx, cy;
} Evas_Object_Animation_Scale_Absolute_Pivot;

typedef struct _Evas_Object_Animation_Scale_Relative_Pivot
{
   Efl_Canvas_Object *obj;
   double             cx, cy;
} Evas_Object_Animation_Scale_Relative_Pivot;

struct _Evas_Object_Animation_Scale_Data
{
   Evas_Object_Animation_Scale_Property from;
   Evas_Object_Animation_Scale_Property to;

   Evas_Object_Animation_Scale_Absolute_Pivot abs_pivot;
   Evas_Object_Animation_Scale_Relative_Pivot rel_pivot;

   Eina_Bool use_rel_pivot;
};

EOLIAN static void
_efl_animation_scale_scale_set(Eo *eo_obj, Evas_Object_Animation_Scale_Data *pd, double from_scale_x, double from_scale_y, double to_scale_x, double to_scale_y, Efl_Canvas_Object *pivot, double cx, double cy)
{
   EFL_ANIMATION_SCALE_CHECK_OR_RETURN(eo_obj);

   pd->from.scale_x = from_scale_x;
   pd->from.scale_y = from_scale_y;

   pd->to.scale_x = to_scale_x;
   pd->to.scale_y = to_scale_y;

   pd->rel_pivot.obj = pivot;
   pd->rel_pivot.cx = cx;
   pd->rel_pivot.cy = cy;

   //Update absolute pivot based on relative pivot
   Evas_Coord x = 0;
   Evas_Coord y = 0;
   Evas_Coord w = 0;
   Evas_Coord h = 0;

   if (pivot)
     evas_object_geometry_get(pivot, &x, &y, &w, &h);
   else
     {
        Eo *target = efl_animation_target_get(eo_obj);
        if (target)
          evas_object_geometry_get(target, &x, &y, &w, &h);
     }

   pd->abs_pivot.cx = x + (w * cx);
   pd->abs_pivot.cy = y + (h * cy);

   pd->use_rel_pivot = EINA_TRUE;
}

EOLIAN static void
_efl_animation_scale_scale_get(Eo *eo_obj, Evas_Object_Animation_Scale_Data *pd, double *from_scale_x, double *from_scale_y, double *to_scale_x, double *to_scale_y, Efl_Canvas_Object **pivot, double *cx, double *cy)
{
   EFL_ANIMATION_SCALE_CHECK_OR_RETURN(eo_obj);

   //Update relative pivot based on absolute pivot
   if (!pd->use_rel_pivot)
     {
        Evas_Coord x = 0;
        Evas_Coord y = 0;
        Evas_Coord w = 0;
        Evas_Coord h = 0;

        Eo *target = efl_animation_target_get(eo_obj);
        if (target)
          evas_object_geometry_get(target, &x, &y, &w, &h);

        if (w != 0)
          pd->rel_pivot.cx = (double)(pd->abs_pivot.cx - x) / w;
        else
          pd->rel_pivot.cx = 0.0;

        if (h != 0)
          pd->rel_pivot.cy = (double)(pd->abs_pivot.cy - y) / h;
        else
          pd->rel_pivot.cy = 0.0;
     }

   if (from_scale_x)
     *from_scale_x = pd->from.scale_x;

   if (from_scale_y)
     *from_scale_y = pd->from.scale_y;

   if (to_scale_x)
     *to_scale_x = pd->to.scale_x;

   if (to_scale_y)
     *to_scale_y = pd->to.scale_y;

   if (pivot)
     *pivot = pd->rel_pivot.obj;

   if (cx)
     *cx = pd->rel_pivot.cx;

   if (cy)
     *cy = pd->rel_pivot.cy;
}

EOLIAN static void
_efl_animation_scale_scale_absolute_set(Eo *eo_obj, Evas_Object_Animation_Scale_Data *pd, double from_scale_x, double from_scale_y, double to_scale_x, double to_scale_y, Evas_Coord cx, Evas_Coord cy)
{
   EFL_ANIMATION_SCALE_CHECK_OR_RETURN(eo_obj);

   pd->from.scale_x = from_scale_x;
   pd->from.scale_y = from_scale_y;

   pd->to.scale_x = to_scale_x;
   pd->to.scale_y = to_scale_y;

   pd->abs_pivot.cx = cx;
   pd->abs_pivot.cy = cy;

   //Update relative pivot based on absolute pivot
   Evas_Coord x = 0;
   Evas_Coord y = 0;
   Evas_Coord w = 0;
   Evas_Coord h = 0;

   Eo *target = efl_animation_target_get(eo_obj);
   if (target)
     evas_object_geometry_get(target, &x, &y, &w, &h);

   pd->rel_pivot.obj = NULL;

   if (w != 0)
     pd->rel_pivot.cx = (double)(cx - x) / w;
   else
     pd->rel_pivot.cx = 0.0;

   if (h != 0)
     pd->rel_pivot.cy = (double)(cy - y) / h;
   else
     pd->rel_pivot.cy = 0.0;

   pd->use_rel_pivot = EINA_FALSE;
}

EOLIAN static void
_efl_animation_scale_scale_absolute_get(Eo *eo_obj, Evas_Object_Animation_Scale_Data *pd, double *from_scale_x, double *from_scale_y, double *to_scale_x, double *to_scale_y, Evas_Coord *cx, Evas_Coord *cy)
{
   EFL_ANIMATION_SCALE_CHECK_OR_RETURN(eo_obj);

   //Update absolute pivot based on relative pivot
   if (pd->use_rel_pivot)
     {
        Evas_Coord x = 0;
        Evas_Coord y = 0;
        Evas_Coord w = 0;
        Evas_Coord h = 0;

        if (pd->rel_pivot.obj)
          evas_object_geometry_get(pd->rel_pivot.obj, &x, &y, &w, &h);
        else
          {
             Eo *target = efl_animation_target_get(eo_obj);
             if (target)
               evas_object_geometry_get(target, &x, &y, &w, &h);
          }

        pd->abs_pivot.cx = x + (w * pd->rel_pivot.cx);
        pd->abs_pivot.cy = y + (h * pd->rel_pivot.cy);
     }

   if (from_scale_x)
     *from_scale_x = pd->from.scale_x;

   if (from_scale_y)
     *from_scale_y = pd->from.scale_y;

   if (to_scale_x)
     *to_scale_x = pd->to.scale_x;

   if (to_scale_y)
     *to_scale_y = pd->to.scale_y;

   if (cx)
     *cx = pd->abs_pivot.cx;

   if (cy)
     *cy = pd->abs_pivot.cy;
}

EOLIAN static Efl_Animation *
_efl_animation_scale_efl_animation_dup(Eo *eo_obj, Evas_Object_Animation_Scale_Data *pd)
{
   EFL_ANIMATION_SCALE_CHECK_OR_RETURN(eo_obj, NULL);

   Efl_Animation_Scale *animation = efl_add(MY_CLASS, NULL);

   double duration = efl_animation_duration_get(eo_obj);
   efl_animation_duration_set(animation, duration);

   Eo *target = efl_animation_target_get(eo_obj);
   efl_animation_target_set(animation, target);

   Eina_Bool state_keep = efl_animation_final_state_keep_get(eo_obj);
   efl_animation_final_state_keep_set(animation, state_keep);

   EFL_ANIMATION_SCALE_DATA_GET(animation, new_pd);

   new_pd->from.scale_x = pd->from.scale_x;
   new_pd->from.scale_y = pd->from.scale_y;

   new_pd->to.scale_x = pd->to.scale_x;
   new_pd->to.scale_y = pd->to.scale_y;

   new_pd->rel_pivot.obj = pd->rel_pivot.obj;
   new_pd->rel_pivot.cx = pd->rel_pivot.cx;
   new_pd->rel_pivot.cy = pd->rel_pivot.cy;

   new_pd->abs_pivot.cx = pd->rel_pivot.cx;
   new_pd->abs_pivot.cy = pd->rel_pivot.cy;

   new_pd->use_rel_pivot = pd->use_rel_pivot;

   return animation;
}

EOLIAN static Efl_Animation_Instance *
_efl_animation_scale_efl_animation_instance_create(Eo *eo_obj, Evas_Object_Animation_Scale_Data *pd)
{
   EFL_ANIMATION_SCALE_CHECK_OR_RETURN(eo_obj, NULL);

   Efl_Animation_Instance_Scale *instance
      = efl_add(EFL_ANIMATION_INSTANCE_SCALE_CLASS, NULL);

   Eo *target = efl_animation_target_get(eo_obj);
   efl_animation_instance_target_set(instance, target);

   Eina_Bool state_keep = efl_animation_final_state_keep_get(eo_obj);
   efl_animation_instance_final_state_keep_set(instance, state_keep);

   double duration = efl_animation_duration_get(eo_obj);
   efl_animation_instance_duration_set(instance, duration);

   if (pd->use_rel_pivot)
     {
        efl_animation_instance_scale_set(instance,
                                         pd->from.scale_x, pd->from.scale_y,
                                         pd->to.scale_x, pd->to.scale_y,
                                         pd->rel_pivot.obj,
                                         pd->rel_pivot.cx, pd->rel_pivot.cy);
     }
   else
     {
        efl_animation_instance_scale_absolute_set(instance,
                                                  pd->from.scale_x, pd->from.scale_y,
                                                  pd->to.scale_x, pd->to.scale_y,
                                                  pd->abs_pivot.cx, pd->abs_pivot.cy);
     }

   return instance;
}

EOLIAN static Efl_Object *
_efl_animation_scale_efl_object_constructor(Eo *eo_obj, Evas_Object_Animation_Scale_Data *pd)
{
   eo_obj = efl_constructor(efl_super(eo_obj, MY_CLASS));

   pd->from.scale_x = 1.0;
   pd->from.scale_y = 1.0;

   pd->rel_pivot.obj = NULL;
   pd->rel_pivot.cx = 0.5;
   pd->rel_pivot.cy = 0.5;

   pd->abs_pivot.cx = 0;
   pd->abs_pivot.cy = 0;

   pd->use_rel_pivot = EINA_TRUE;

   return eo_obj;
}

EOLIAN static void
_efl_animation_scale_efl_object_destructor(Eo *eo_obj, Evas_Object_Animation_Scale_Data *pd EINA_UNUSED)
{
   efl_destructor(efl_super(eo_obj, MY_CLASS));
}

#include "efl_animation_scale.eo.c"
