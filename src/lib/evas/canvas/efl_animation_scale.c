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
   double scale_x, scale_y, scale_z;
} Evas_Object_Animation_Scale_Property;

struct _Evas_Object_Animation_Scale_Data
{
   Evas_Object_Animation_Scale_Property from;
   Evas_Object_Animation_Scale_Property to;

   Evas_Map *map;
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
   new_pd->from.scale_z = pd->from.scale_z;

   new_pd->to.scale_x = pd->to.scale_x;
   new_pd->to.scale_y = pd->to.scale_y;
   new_pd->to.scale_z = pd->to.scale_z;

   return animation;
}

static Evas_Map *
_map_dup(Eo *target)
{
   if (!target) return NULL;

   Evas_Map *map = NULL;

   if (evas_object_map_enable_get(target) && evas_object_map_get(target))
     {
        map = evas_map_dup(evas_object_map_get(target));
        if (!map) return NULL;
     }
   else
     {
        map = evas_map_new(4);
        if (!map) return NULL;
        evas_map_util_points_populate_from_object_full(map, target, 0);
     }

   return map;
}

static void
_map_init(Evas_Map *map)
{
   evas_map_util_object_move_sync_set(map, EINA_TRUE);

   //FIXME: Need to consider smooth of map
}

static void
_pre_start_cb(void *data EINA_UNUSED, const Efl_Event *event)
{
   EFL_ANIMATION_SCALE_DATA_GET(event->object, pd);

   Eo *target = efl_animation_target_get(event->object);
   if (!target) return;

   pd->map = _map_dup(target);
   if (!pd->map) return;

   _map_init(pd->map);
}

static void
_pre_animate_cb(void *data EINA_UNUSED, const Efl_Event *event)
{
   EFL_ANIMATION_SCALE_DATA_GET(event->object, pd);
   Efl_Animation_Animate_Event_Info *event_info = event->info;

   double progress = event_info->progress;

   Eo *target = efl_animation_target_get(event->object);
   if (!target) return;

   double scale_x =
      (pd->from.scale_x * (1.0 - progress)) + (pd->to.scale_x * progress);

   double scale_y =
      (pd->from.scale_y * (1.0 - progress)) + (pd->to.scale_y * progress);

   if (!pd->map)
     {
        pd->map = _map_dup(target);
        if (!pd->map) return;

        _map_init(pd->map);
     }

   Evas_Map *map = evas_map_dup(pd->map);

   Evas_Coord x, y, w, h;
   evas_object_geometry_get(target, &x, &y, &w, &h);

   evas_map_util_zoom(map, scale_x, scale_y, x + (w / 2), y + (h / 2));

   evas_object_map_set(target, map);
   evas_object_map_enable_set(target, EINA_TRUE);

   evas_map_free(map);
}

static void
_pre_end_cb(void *data EINA_UNUSED, const Efl_Event *event)
{
   EFL_ANIMATION_SCALE_DATA_GET(event->object, pd);

   evas_map_free(pd->map);
   pd->map = NULL;
}

EOLIAN static Efl_Object *
_efl_animation_scale_efl_object_constructor(Eo *eo_obj, Evas_Object_Animation_Scale_Data *pd)
{
   eo_obj = efl_constructor(efl_super(eo_obj, MY_CLASS));

   pd->from.scale_x = 1.0;
   pd->from.scale_y = 1.0;
   pd->from.scale_z = 1.0;

   //pre start event is supported within class only (protected event)
   efl_event_callback_add(eo_obj, EFL_ANIMATION_EVENT_PRE_START, _pre_start_cb, NULL);

   //pre animate event is supported within class only (protected event)
   efl_event_callback_add(eo_obj, EFL_ANIMATION_EVENT_PRE_ANIMATE, _pre_animate_cb, NULL);

   //pre end event is supported within class only (protected event)
   efl_event_callback_add(eo_obj, EFL_ANIMATION_EVENT_PRE_END, _pre_end_cb, NULL);

   return eo_obj;
}

EOLIAN static void
_efl_animation_scale_efl_object_destructor(Eo *eo_obj, Evas_Object_Animation_Scale_Data *pd EINA_UNUSED)
{
   efl_destructor(efl_super(eo_obj, MY_CLASS));
}

#include "efl_animation_scale.eo.c"
