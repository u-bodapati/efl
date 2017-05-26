#define EFL_ANIMATION_PROTECTED

#include "evas_common_private.h"
#include "evas_private.h"

#define MY_CLASS EFL_ANIMATION_GROUP_SEQUENTIAL_CLASS
#define MY_CLASS_NAME efl_class_name_get(MY_CLASS)

#define EFL_ANIMATION_GROUP_SEQUENTIAL_CHECK_OR_RETURN(anim, ...) \
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

#define EFL_ANIMATION_GROUP_SEQUENTIAL_DATA_GET(o, pd) \
   Evas_Object_Animation_Group_Sequential_Data *pd = efl_data_scope_get(o, EFL_ANIMATION_GROUP_SEQUENTIAL_CLASS)

struct _Evas_Object_Animation_Group_Sequential_Data
{
   int current_index;
};

static Eina_Bool _index_animation_start(Eo *eo_obj, int index);

static void
_post_end_cb(void *data, const Efl_Event *event)
{
   Eo *eo_obj = data;

   EFL_ANIMATION_GROUP_SEQUENTIAL_DATA_GET(eo_obj, pd);

   efl_event_callback_del(event->object, EFL_ANIMATION_EVENT_POST_END,
                          _post_end_cb, eo_obj);

   pd->current_index++;

   Eina_List *animations = efl_animation_group_animations_get(eo_obj);
   if (eina_list_count(animations) == pd->current_index)
     {
        pd->current_index = 0;

        //pre end event is supported within class only (protected event)
        efl_event_callback_call(eo_obj, EFL_ANIMATION_EVENT_PRE_END, NULL);
        efl_event_callback_call(eo_obj, EFL_ANIMATION_EVENT_END, NULL);
        //post end event is supported within class only (protected event)
        efl_event_callback_call(eo_obj, EFL_ANIMATION_EVENT_POST_END, NULL);

        return;
     }

   _index_animation_start(eo_obj, pd->current_index);
}

static Eina_Bool
_index_animation_start(Eo *eo_obj, int index)
{
   Efl_Animation *anim =
      eina_list_nth(efl_animation_group_animations_get(eo_obj), index);
   if (!anim || efl_animation_is_deleted(anim))
     return EINA_FALSE;

   //post end event is supported within class only (protected event)
   efl_event_callback_add(anim, EFL_ANIMATION_EVENT_POST_END, _post_end_cb,
                          eo_obj);

   return efl_animation_start(anim);
}

EOLIAN static Eina_Bool
_efl_animation_group_sequential_efl_animation_start(Eo *eo_obj, Evas_Object_Animation_Group_Sequential_Data *pd)
{
   EFL_ANIMATION_GROUP_SEQUENTIAL_CHECK_OR_RETURN(eo_obj, EINA_FALSE);

   pd->current_index = 0;

   efl_event_callback_call(eo_obj, EFL_ANIMATION_EVENT_START, NULL);

   return _index_animation_start(eo_obj, pd->current_index);
}

EOLIAN static Efl_Animation *
_efl_animation_group_sequential_efl_animation_dup(Eo *eo_obj, Evas_Object_Animation_Group_Sequential_Data *pd EINA_UNUSED)
{
   EFL_ANIMATION_GROUP_SEQUENTIAL_CHECK_OR_RETURN(eo_obj, NULL);

   Efl_Animation *animation = efl_add(MY_CLASS, NULL);

   double duration = efl_animation_duration_get(eo_obj);
   efl_animation_duration_set(animation, duration);

   Eo *target = efl_animation_target_get(eo_obj);
   efl_animation_target_set(animation, target);

   Eina_Bool state_keep = efl_animation_final_state_keep_get(eo_obj);
   efl_animation_final_state_keep_set(animation, state_keep);

   Eina_List *animations = efl_animation_group_animations_get(eo_obj);
   Eina_List *l;
   Efl_Animation *child_anim;
   EINA_LIST_FOREACH(animations, l, child_anim)
     {
        efl_animation_group_animation_add(animation, child_anim);
     }

   return animation;
}

#include "efl_animation_group_sequential.eo.c"
