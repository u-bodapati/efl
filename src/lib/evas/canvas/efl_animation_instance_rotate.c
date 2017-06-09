#define EFL_ANIMATION_INSTANCE_PROTECTED

#include "evas_common_private.h"
#include "evas_private.h"

#define MY_CLASS EFL_ANIMATION_INSTANCE_ROTATE_CLASS
#define MY_CLASS_NAME efl_class_name_get(MY_CLASS)

#define EFL_ANIMATION_INSTANCE_ROTATE_CHECK_OR_RETURN(anim, ...) \
   do { \
      if (!anim) { \
         CRI("Efl_Animation_Instance " # anim " is NULL!"); \
         return __VA_ARGS__; \
      } \
      if (efl_animation_instance_is_deleted(anim)) { \
         ERR("Efl_Animation_Instance " # anim " has already been deleted!"); \
         return __VA_ARGS__; \
      } \
   } while (0)

#define EFL_ANIMATION_INSTANCE_ROTATE_DATA_GET(o, pd) \
   Evas_Object_Animation_Instance_Rotate_Data *pd = efl_data_scope_get(o, EFL_ANIMATION_INSTANCE_ROTATE_CLASS)

typedef struct _Evas_Object_Animation_Instance_Rotate_Property
{
   double degree;
} Evas_Object_Animation_Instance_Rotate_Property;

typedef struct _Evas_Object_Animation_Instance_Rotate_Absolute_Pivot
{
   Evas_Coord cx, cy;
} Evas_Object_Animation_Instance_Rotate_Absolute_Pivot;

typedef struct _Evas_Object_Animation_Instance_Rotate_Relative_Pivot
{
   Efl_Canvas_Object *obj;
   double             cx, cy;
} Evas_Object_Animation_Instance_Rotate_Relative_Pivot;

struct _Evas_Object_Animation_Instance_Rotate_Data
{
   Evas_Object_Animation_Instance_Rotate_Property       from;
   Evas_Object_Animation_Instance_Rotate_Property       to;

   Evas_Object_Animation_Instance_Rotate_Absolute_Pivot abs_pivot;
   Evas_Object_Animation_Instance_Rotate_Relative_Pivot rel_pivot;

   Eina_Bool use_rel_pivot;
};

EOLIAN static void
_efl_animation_instance_rotate_rotate_set(Eo *eo_obj, Evas_Object_Animation_Instance_Rotate_Data *pd, double from_degree, double to_degree, Efl_Canvas_Object *pivot, double cx, double cy)
{
   EFL_ANIMATION_INSTANCE_ROTATE_CHECK_OR_RETURN(eo_obj);

   pd->from.degree = from_degree;
   pd->to.degree = to_degree;

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
        Eo *target = efl_animation_instance_target_get(eo_obj);
        if (target)
          evas_object_geometry_get(target, &x, &y, &w, &h);
     }

   pd->abs_pivot.cx = x + (w * cx);
   pd->abs_pivot.cy = y + (h * cy);

   pd->use_rel_pivot = EINA_TRUE;
}

EOLIAN static void
_efl_animation_instance_rotate_rotate_get(Eo *eo_obj, Evas_Object_Animation_Instance_Rotate_Data *pd, double *from_degree, double *to_degree, Efl_Canvas_Object **pivot, double *cx, double *cy)
{
   EFL_ANIMATION_INSTANCE_ROTATE_CHECK_OR_RETURN(eo_obj);

   //Update relative pivot based on absolute pivot
   if (!pd->use_rel_pivot)
     {
        Evas_Coord x = 0;
        Evas_Coord y = 0;
        Evas_Coord w = 0;
        Evas_Coord h = 0;

        Eo *target = efl_animation_instance_target_get(eo_obj);
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

   if (from_degree)
     *from_degree = pd->from.degree;

   if (to_degree)
     *to_degree = pd->to.degree;

   if (pivot)
     *pivot = pd->rel_pivot.obj;

   if (cx)
     *cx = pd->rel_pivot.cx;

   if (cy)
     *cy = pd->rel_pivot.cy;
}

EOLIAN static void
_efl_animation_instance_rotate_rotate_absolute_set(Eo *eo_obj, Evas_Object_Animation_Instance_Rotate_Data *pd, double from_degree, double to_degree, Evas_Coord cx, Evas_Coord cy)
{
   EFL_ANIMATION_INSTANCE_ROTATE_CHECK_OR_RETURN(eo_obj);

   pd->from.degree = from_degree;
   pd->to.degree = to_degree;

   pd->abs_pivot.cx = cx;
   pd->abs_pivot.cy = cy;

   //Update relative pivot based on absolute pivot
   Evas_Coord x = 0;
   Evas_Coord y = 0;
   Evas_Coord w = 0;
   Evas_Coord h = 0;

   Eo *target = efl_animation_instance_target_get(eo_obj);
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
_efl_animation_instance_rotate_rotate_absolute_get(Eo *eo_obj, Evas_Object_Animation_Instance_Rotate_Data *pd, double *from_degree, double *to_degree, Evas_Coord *cx, Evas_Coord *cy)
{
   EFL_ANIMATION_INSTANCE_ROTATE_CHECK_OR_RETURN(eo_obj);

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
             Eo *target = efl_animation_instance_target_get(eo_obj);
             if (target)
               evas_object_geometry_get(target, &x, &y, &w, &h);
          }

        pd->abs_pivot.cx = x + (w * pd->rel_pivot.cx);
        pd->abs_pivot.cy = y + (h * pd->rel_pivot.cy);
     }

   if (from_degree)
     *from_degree = pd->from.degree;

   if (to_degree)
     *to_degree = pd->to.degree;

   if (cx)
     *cx = pd->abs_pivot.cx;

   if (cy)
     *cy = pd->abs_pivot.cy;
}

static void
_pre_animate_cb(void *data, const Efl_Event *event)
{
   EFL_ANIMATION_INSTANCE_ROTATE_DATA_GET(event->object, pd);
   Efl_Animation_Animate_Event_Info *event_info = event->info;

   double progress = event_info->progress;

   Eo *target = efl_animation_instance_target_get(event->object);
   if (!target) return;

   double degree =
      (pd->from.degree * (1.0 - progress)) + (pd->to.degree * progress);

   if (efl_gfx_map_has(target))
     efl_gfx_map_reset(target);

   if (pd->use_rel_pivot)
     {
        efl_gfx_map_rotate(target,
                           degree,
                           pd->rel_pivot.obj,
                           pd->rel_pivot.cx, pd->rel_pivot.cy);
     }
   else
     {
        efl_gfx_map_rotate_absolute(target,
                                    degree,
                                    pd->abs_pivot.cx, pd->abs_pivot.cy);
     }
}

EOLIAN static Efl_Object *
_efl_animation_instance_rotate_efl_object_constructor(Eo *eo_obj, Evas_Object_Animation_Instance_Rotate_Data *pd)
{
   eo_obj = efl_constructor(efl_super(eo_obj, MY_CLASS));

   pd->from.degree = 0.0;
   pd->to.degree = 0.0;

   pd->rel_pivot.obj = NULL;
   pd->rel_pivot.cx = 0.5;
   pd->rel_pivot.cy = 0.5;

   pd->abs_pivot.cx = 0;
   pd->abs_pivot.cy = 0;

   pd->use_rel_pivot = EINA_TRUE;

   //pre animate event is supported within class only (protected event)
   efl_event_callback_add(eo_obj, EFL_ANIMATION_INSTANCE_EVENT_PRE_ANIMATE, _pre_animate_cb, NULL);

   return eo_obj;
}

EOLIAN static void
_efl_animation_instance_rotate_efl_object_destructor(Eo *eo_obj, Evas_Object_Animation_Instance_Rotate_Data *pd EINA_UNUSED)
{
   efl_destructor(efl_super(eo_obj, MY_CLASS));
}

#include "efl_animation_instance_rotate.eo.c"
