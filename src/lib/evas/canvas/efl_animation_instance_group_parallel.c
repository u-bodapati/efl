#define EFL_ANIMATION_INSTANCE_PROTECTED

#include "evas_common_private.h"
#include "evas_private.h"

#define MY_CLASS EFL_ANIMATION_INSTANCE_GROUP_PARALLEL_CLASS
#define MY_CLASS_NAME efl_class_name_get(MY_CLASS)

#define EFL_ANIMATION_INSTANCE_GROUP_PARALLEL_CHECK_OR_RETURN(anim, ...) \
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

#define EFL_ANIMATION_INSTANCE_GROUP_PARALLEL_DATA_GET(o, pd) \
   Evas_Object_Animation_Instance_Group_Parallel_Data *pd = efl_data_scope_get(o, EFL_ANIMATION_INSTANCE_GROUP_PARALLEL_CLASS)

struct _Evas_Object_Animation_Instance_Group_Parallel_Data
{
   int        count;
   int        animate_count;
   int        end_count;

   Eina_Bool  composite : 1;
};

static void
_pre_animate_cb(void *data, const Efl_Event *event)
{
   Eo *eo_obj = data;

   EFL_ANIMATION_INSTANCE_GROUP_PARALLEL_DATA_GET(eo_obj, pd);

   pd->animate_count++;

   if (pd->animate_count == pd->count)
     {
        if (!pd->composite)
          {
             Eina_List *insts
                = efl_animation_instance_group_instances_get(eo_obj);
             Eina_List *l;
             Efl_Animation_Instance *inst;

             //Reset previous map effect of all instances
             EINA_LIST_FOREACH(insts, l, inst)
               {
                  efl_animation_instance_map_reset(inst);
               }
          }

        pd->animate_count = 0;

        //pre animate event is supported within class only (protected event)
        efl_event_callback_call(eo_obj,
                                EFL_ANIMATION_INSTANCE_EVENT_PRE_ANIMATE,
                                event->info);
     }
}

static void
_post_end_cb(void *data, const Efl_Event *event)
{
   Eo *eo_obj = data;

   EFL_ANIMATION_INSTANCE_GROUP_PARALLEL_DATA_GET(eo_obj, pd);

   efl_event_callback_del(event->object,
                          EFL_ANIMATION_INSTANCE_EVENT_PRE_ANIMATE,
                          _pre_animate_cb, eo_obj);

   efl_event_callback_del(event->object, EFL_ANIMATION_INSTANCE_EVENT_POST_END,
                          _post_end_cb, eo_obj);

   pd->end_count++;

   if (pd->count == pd->end_count)
     {
        //pre end event is supported within class only (protected event)
        efl_event_callback_call(eo_obj, EFL_ANIMATION_INSTANCE_EVENT_PRE_END, NULL);
        efl_event_callback_call(eo_obj, EFL_ANIMATION_INSTANCE_EVENT_END, NULL);
        //post end event is supported within class only (protected event)
        efl_event_callback_call(eo_obj, EFL_ANIMATION_INSTANCE_EVENT_POST_END, NULL);
     }
}

static Eina_Bool
_start(Eo *eo_obj, Evas_Object_Animation_Instance_Group_Parallel_Data *pd)
{
   pd->count = 0;
   pd->animate_count = 0;
   pd->end_count = 0;

   efl_event_callback_call(eo_obj, EFL_ANIMATION_INSTANCE_EVENT_START, NULL);

   Eina_Bool ret = EINA_FALSE;

   Eina_List *animations = efl_animation_instance_group_instances_get(eo_obj);
   Eina_List *l;
   Efl_Animation *anim;
   EINA_LIST_FOREACH(animations, l, anim)
     {
        //Data should be registered before animation starts
        pd->count++;

        //pre animate event is supported within class only (protected event)
        efl_event_callback_add(anim,
                               EFL_ANIMATION_INSTANCE_EVENT_PRE_ANIMATE,
                               _pre_animate_cb, eo_obj);

        //post end event is supported within class only (protected event)
        efl_event_callback_add(anim, EFL_ANIMATION_INSTANCE_EVENT_POST_END,
                               _post_end_cb, eo_obj);

        if (efl_animation_instance_composite_start(anim))
          {
             ret = EINA_TRUE;
          }
        else
          {
             pd->count--;

             efl_event_callback_del(anim,
                                    EFL_ANIMATION_INSTANCE_EVENT_PRE_ANIMATE,
                                    _pre_animate_cb, eo_obj);

             efl_event_callback_del(anim,
                                    EFL_ANIMATION_INSTANCE_EVENT_POST_END,
                                    _post_end_cb, eo_obj);
          }
     }

   return ret;
}

EOLIAN static Eina_Bool
_efl_animation_instance_group_parallel_efl_animation_instance_start(Eo *eo_obj, Evas_Object_Animation_Instance_Group_Parallel_Data *pd)
{
   EFL_ANIMATION_INSTANCE_GROUP_PARALLEL_CHECK_OR_RETURN(eo_obj, EINA_FALSE);

   pd->composite = EINA_FALSE;

   return _start(eo_obj, pd);
}

EOLIAN static Eina_Bool
_efl_animation_instance_group_parallel_efl_animation_instance_composite_start(Eo *eo_obj, Evas_Object_Animation_Instance_Group_Parallel_Data *pd)
{
   EFL_ANIMATION_INSTANCE_GROUP_PARALLEL_CHECK_OR_RETURN(eo_obj, EINA_FALSE);

   pd->composite = EINA_TRUE;

   return _start(eo_obj, pd);
}

#include "efl_animation_instance_group_parallel.eo.c"
