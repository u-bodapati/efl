#define EFL_ANIMATION_INSTANCE_PROTECTED

#include "evas_common_private.h"
#include "evas_private.h"

#define MY_CLASS EFL_ANIMATION_INSTANCE_GROUP_CLASS
#define MY_CLASS_NAME efl_class_name_get(MY_CLASS)

#define EFL_ANIMATION_INSTANCE_GROUP_CHECK_OR_RETURN(anim, ...) \
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

#define EFL_ANIMATION_INSTANCE_GROUP_DATA_GET(o, pd) \
   Evas_Object_Animation_Instance_Group_Data *pd = efl_data_scope_get(o, EFL_ANIMATION_INSTANCE_GROUP_CLASS)

struct _Evas_Object_Animation_Instance_Group_Data
{
   Eina_List *instances;
};

EOLIAN static void
_efl_animation_instance_group_instance_add(Eo *eo_obj, Evas_Object_Animation_Instance_Group_Data *pd, Efl_Animation_Instance *instance)
{
   EFL_ANIMATION_INSTANCE_GROUP_CHECK_OR_RETURN(eo_obj);

   if (!instance) return;

   pd->instances = eina_list_append(pd->instances, instance);
}

EOLIAN static void
_efl_animation_instance_group_instance_del(Eo *eo_obj, Evas_Object_Animation_Instance_Group_Data *pd, Efl_Animation_Instance *instance)
{
   EFL_ANIMATION_INSTANCE_GROUP_CHECK_OR_RETURN(eo_obj);

   if (!instance) return;

   pd->instances = eina_list_remove(pd->instances, instance);
}

EOLIAN static Eina_List *
_efl_animation_instance_group_instances_get(Eo *eo_obj, Evas_Object_Animation_Instance_Group_Data *pd)
{
   EFL_ANIMATION_INSTANCE_GROUP_CHECK_OR_RETURN(eo_obj, NULL);

   return pd->instances;
}

EOLIAN static Efl_Object *
_efl_animation_instance_group_efl_object_constructor(Eo *eo_obj, Evas_Object_Animation_Instance_Group_Data *pd)
{
   eo_obj = efl_constructor(efl_super(eo_obj, MY_CLASS));

   pd->instances = NULL;

   return eo_obj;
}

EOLIAN static void
_efl_animation_instance_group_efl_object_destructor(Eo *eo_obj, Evas_Object_Animation_Instance_Group_Data *pd)
{
   Efl_Animation_Instance *inst;

   EINA_LIST_FREE(pd->instances, inst)
     efl_del(inst);

   efl_destructor(efl_super(eo_obj, MY_CLASS));
}

#include "efl_animation_instance_group.eo.c"
