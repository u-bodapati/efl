#define EFL_ANIMATION_PROTECTED

#include "evas_common_private.h"
#include "evas_private.h"

#define MY_CLASS EFL_ANIMATION_GROUP_CLASS
#define MY_CLASS_NAME efl_class_name_get(MY_CLASS)

#define EFL_ANIMATION_GROUP_CHECK_OR_RETURN(anim, ...) \
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

#define EFL_ANIMATION_GROUP_DATA_GET(o, pd) \
   Evas_Object_Animation_Group_Data *pd = efl_data_scope_get(o, EFL_ANIMATION_GROUP_CLASS)

struct _Evas_Object_Animation_Group_Data
{
   Eina_List *animations;
};

EOLIAN static void
_efl_animation_group_animation_add(Eo *eo_obj, Evas_Object_Animation_Group_Data *pd, Efl_Animation*animation)
{
   EFL_ANIMATION_GROUP_CHECK_OR_RETURN(eo_obj);

   if (!animation) return;

   pd->animations = eina_list_append(pd->animations, animation);
}

EOLIAN static void
_efl_animation_group_animation_del(Eo *eo_obj, Evas_Object_Animation_Group_Data *pd, Efl_Animation*animation)
{
   EFL_ANIMATION_GROUP_CHECK_OR_RETURN(eo_obj);

   if (!animation) return;

   pd->animations = eina_list_remove(pd->animations, animation);
}

EOLIAN static Eina_List *
_efl_animation_group_animations_get(Eo *eo_obj, Evas_Object_Animation_Group_Data *pd)
{
   EFL_ANIMATION_GROUP_CHECK_OR_RETURN(eo_obj, NULL);

   return pd->animations;
}

EOLIAN static void
_efl_animation_group_efl_animation_target_set(Eo *eo_obj, Evas_Object_Animation_Group_Data *pd, Eo *target)
 {
   EFL_ANIMATION_GROUP_CHECK_OR_RETURN(eo_obj);

   Eina_List *l;
   Efl_Animation *anim;
   EINA_LIST_FOREACH(pd->animations, l, anim)
     {
        efl_animation_target_set(anim, target);
     }

   efl_animation_target_set(efl_super(eo_obj, MY_CLASS), target);
}

EOLIAN static void
_efl_animation_group_efl_animation_final_state_keep_set(Eo *eo_obj, Evas_Object_Animation_Group_Data *pd, Eina_Bool state_keep)
{
   EFL_ANIMATION_GROUP_CHECK_OR_RETURN(eo_obj);

   Eina_List *l;
   Efl_Animation *anim;
   EINA_LIST_FOREACH(pd->animations, l, anim)
     {
        efl_animation_final_state_keep_set(anim, state_keep);
     }

   efl_animation_final_state_keep_set(efl_super(eo_obj, MY_CLASS), state_keep);
}

EOLIAN static Efl_Object *
_efl_animation_group_efl_object_constructor(Eo *eo_obj, Evas_Object_Animation_Group_Data *pd)
{
   eo_obj = efl_constructor(efl_super(eo_obj, MY_CLASS));

   pd->animations = NULL;

   return eo_obj;
}

EOLIAN static void
_efl_animation_group_efl_object_destructor(Eo *eo_obj, Evas_Object_Animation_Group_Data *pd)
{
   Efl_Animation *anim;

   EINA_LIST_FREE(pd->animations, anim)
     efl_del(anim);

   efl_destructor(efl_super(eo_obj, MY_CLASS));
}

#include "efl_animation_group.eo.c"
