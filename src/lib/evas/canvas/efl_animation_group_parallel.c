#define EFL_ANIMATION_PROTECTED

#include "evas_common_private.h"
#include "evas_private.h"

#define MY_CLASS EFL_ANIMATION_GROUP_PARALLEL_CLASS
#define MY_CLASS_NAME efl_class_name_get(MY_CLASS)

#define EFL_ANIMATION_GROUP_PARALLEL_CHECK_OR_RETURN(anim, ...) \
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

#define EFL_ANIMATION_GROUP_PARALLEL_DATA_GET(o, pd) \
   Evas_Object_Animation_Group_Parallel_Data *pd = efl_data_scope_get(o, EFL_ANIMATION_GROUP_PARALLEL_CLASS)

struct _Evas_Object_Animation_Group_Parallel_Data
{
};

EOLIAN static Efl_Animation *
_efl_animation_group_parallel_efl_animation_dup(Eo *eo_obj, Evas_Object_Animation_Group_Parallel_Data *pd EINA_UNUSED)
{
   EFL_ANIMATION_GROUP_PARALLEL_CHECK_OR_RETURN(eo_obj, NULL);

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

EOLIAN static Efl_Animation_Instance *
_efl_animation_group_parallel_efl_animation_instance_create(Eo *eo_obj, Evas_Object_Animation_Group_Parallel_Data *pd)
{
   EFL_ANIMATION_GROUP_PARALLEL_CHECK_OR_RETURN(eo_obj, NULL);

   Efl_Animation_Instance_Group_Parallel *instance
      = efl_add(EFL_ANIMATION_INSTANCE_GROUP_PARALLEL_CLASS, NULL);

   Eo *target = efl_animation_target_get(eo_obj);
   efl_animation_instance_target_set(instance, target);

   Eina_Bool state_keep = efl_animation_final_state_keep_get(eo_obj);
   efl_animation_instance_final_state_keep_set(instance, state_keep);

   double duration = efl_animation_duration_get(eo_obj);
   efl_animation_instance_duration_set(instance, duration);

   Efl_Animation_Instance_Group_Parallel *group_inst
      = efl_add(EFL_ANIMATION_INSTANCE_GROUP_PARALLEL_CLASS, NULL);

   Eina_List *animations = efl_animation_group_animations_get(eo_obj);
   Eina_List *l;
   Efl_Animation *child_anim;
   Efl_Animation_Instance *child_inst;

   EINA_LIST_FOREACH(animations, l, child_anim)
     {
        child_inst = efl_animation_instance_create(child_anim);
        efl_animation_instance_group_instance_add(group_inst, child_inst);
     }

   return group_inst;
}

#include "efl_animation_group_parallel.eo.c"
