#define EFL_ANIMATION_PROTECTED

#include "evas_common_private.h"
#include "evas_private.h"
#include <Ecore.h>

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

#define EFL_ANIMATION_DATA_GET(o, pd) \
   Evas_Object_Animation_Data *pd = efl_data_scope_get(o, EFL_ANIMATION_CLASS)

typedef struct _Target_State
{
   int r, g, b, a;
} Target_State;

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
   Ecore_Animator *animator;

   struct
     {
        double begin;
        double current;
     } time;

   Eo           *target;
   Target_State *target_state;

   double    progress;
   double    duration;

   Eina_Bool deleted : 1;
   Eina_Bool cancelled : 1;
   Eina_Bool ended : 1;
   Eina_Bool state_keep : 1;
};

static void
_target_del_cb(void *data, const Efl_Event *event EINA_UNUSED)
{
   Eo *eo_obj = data;

   EFL_ANIMATION_CHECK_OR_RETURN(eo_obj);
   EFL_ANIMATION_DATA_GET(eo_obj, pd);

   pd->target = NULL;
   efl_animation_cancel(eo_obj);
}

EOLIAN static void
_efl_animation_target_set(Eo *eo_obj, Evas_Object_Animation_Data *pd, Evas_Object *target)
{
   EFL_ANIMATION_CHECK_OR_RETURN(eo_obj);

   efl_event_callback_add(target, EFL_EVENT_DEL, _target_del_cb, eo_obj);

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

EOLIAN static void
_efl_animation_final_state_keep_set(Eo *eo_obj, Evas_Object_Animation_Data *pd, Eina_Bool state_keep)
{
   EFL_ANIMATION_CHECK_OR_RETURN(eo_obj);

   if (pd->state_keep == state_keep) return;

   pd->state_keep = !!state_keep;
}

EOLIAN static Eina_Bool
_efl_animation_final_state_keep_get(Eo *eo_obj, Evas_Object_Animation_Data *pd)
{
   EFL_ANIMATION_CHECK_OR_RETURN(eo_obj, EINA_FALSE);

   return pd->state_keep;
}

static void
_target_state_save(Eo *target, Target_State *target_state)
{
   if (!target || !target_state) return;

   int r, g, b, a;
   evas_object_color_get(target, &r, &g, &b, &a);

   target_state->r = r;
   target_state->g = g;
   target_state->b = b;
   target_state->a = a;
}

static void
_target_state_restore(Eo *target, Target_State *target_state)
{
   if (!target || !target_state) return;

   int r, g, b, a;
   r = target_state->r;
   g = target_state->g;
   b = target_state->b;
   a = target_state->a;

   evas_object_color_set(target, r, g, b, a);
}

static Eina_Bool
_animator_cb(void *data)
{
   Eo *eo_obj = data;
   EFL_ANIMATION_DATA_GET(eo_obj, pd);

   if (pd->cancelled) goto end;

   double elapsed_time, duration;

   pd->time.current = ecore_loop_time_get();
   elapsed_time = pd->time.current - pd->time.begin;
   duration = pd->duration;
   if (elapsed_time > duration)
     elapsed_time = duration;

   if (duration <= 0.0) goto end;

   pd->progress = elapsed_time / duration;

   Efl_Animation_Animate_Event_Info event_info;
   event_info.progress = pd->progress;

   //pre animate event is supported within class only (protected event)
   efl_event_callback_call(eo_obj, EFL_ANIMATION_EVENT_PRE_ANIMATE,
                           &event_info);
   efl_event_callback_call(eo_obj, EFL_ANIMATION_EVENT_ANIMATE, &event_info);
   //post animate event is supported within class only (protected event)
   efl_event_callback_call(eo_obj, EFL_ANIMATION_EVENT_POST_ANIMATE,
                           &event_info);

  /* Not end. Keep going. */
   if (elapsed_time < duration) return ECORE_CALLBACK_RENEW;

end:
   pd->ended = EINA_TRUE;
   pd->animator = NULL;

   //Restore initial state of target object
   if (!pd->state_keep)
     _target_state_restore(pd->target, pd->target_state);

   //pre end event is supported within class only (protected event)
   efl_event_callback_call(eo_obj, EFL_ANIMATION_EVENT_PRE_END, NULL);
   efl_event_callback_call(eo_obj, EFL_ANIMATION_EVENT_END, NULL);
   //post end event is supported within class only (protected event)
   efl_event_callback_call(eo_obj, EFL_ANIMATION_EVENT_POST_END, NULL);

   //FIXME: Delete animation here
   return ECORE_CALLBACK_CANCEL;
}

EOLIAN static Eina_Bool
_efl_animation_start(Eo *eo_obj, Evas_Object_Animation_Data *pd)
{
   EFL_ANIMATION_CHECK_OR_RETURN(eo_obj, EINA_FALSE);

   if (pd->duration <= 0.0)
     return EINA_FALSE;

   //Save current state of target object
   _target_state_save(pd->target, pd->target_state);

   pd->cancelled = EINA_FALSE;
   pd->ended = EINA_FALSE;

   ecore_animator_del(pd->animator);
   pd->animator = NULL;

   pd->time.begin = ecore_loop_time_get();

   //pre start event is supported within class only (protected event)
   efl_event_callback_call(eo_obj, EFL_ANIMATION_EVENT_PRE_START, NULL);
   efl_event_callback_call(eo_obj, EFL_ANIMATION_EVENT_START, NULL);
   //post start event is supported within class only (protected event)
   efl_event_callback_call(eo_obj, EFL_ANIMATION_EVENT_POST_START, NULL);

   pd->animator = ecore_animator_add(_animator_cb, eo_obj);

   _animator_cb(eo_obj);

   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_efl_animation_cancel(Eo *eo_obj, Evas_Object_Animation_Data *pd)
{
   EFL_ANIMATION_CHECK_OR_RETURN(eo_obj, EINA_FALSE);

   pd->cancelled = EINA_TRUE;

   if (pd->animator)
     {
        pd->ended = EINA_TRUE;

        ecore_animator_del(pd->animator);
        pd->animator = NULL;

        //Restore initial state of target object
        if (!pd->state_keep)
          _target_state_restore(pd->target, pd->target_state);

        //pre end event is supported within class only (protected event)
        efl_event_callback_call(eo_obj, EFL_ANIMATION_EVENT_PRE_END, NULL);
        efl_event_callback_call(eo_obj, EFL_ANIMATION_EVENT_END, NULL);
        //post end event is supported within class only (protected event)
        efl_event_callback_call(eo_obj, EFL_ANIMATION_EVENT_POST_END, NULL);
     }

   return EINA_TRUE;
}

EOLIAN static Efl_Animation_State
_efl_animation_state_get(Eo *eo_obj, Evas_Object_Animation_Data *pd)
{
   EFL_ANIMATION_CHECK_OR_RETURN(eo_obj, EFL_ANIMATION_STATE_DELETED);

   if (pd->animator)
     return EFL_ANIMATION_STATE_RUNNING;
   else
     return EFL_ANIMATION_STATE_STOPPED;
}

EOLIAN static Efl_Object *
_efl_animation_efl_object_constructor(Eo *eo_obj, Evas_Object_Animation_Data *pd)
{
   eo_obj = efl_constructor(efl_super(eo_obj, MY_CLASS));

   pd->time.begin = 0.0;
   pd->time.current = 0.0;

   pd->target = NULL;
   pd->target_state = (Target_State *) calloc(1, sizeof(Target_State));

   pd->progress = 0.0;
   pd->duration = 0.0;

   pd->deleted = EINA_FALSE;
   pd->cancelled = EINA_FALSE;
   pd->ended = EINA_FALSE;
   pd->state_keep = EINA_FALSE;

   return eo_obj;
}

EOLIAN static void
_efl_animation_efl_object_destructor(Eo *eo_obj, Evas_Object_Animation_Data *pd)
{
   pd->deleted = EINA_TRUE;

   if (pd->animator)
     {
        ecore_animator_del(pd->animator);
        pd->animator = NULL;
        pd->ended = EINA_TRUE;

        //Restore initial state of target object
        if (!pd->state_keep)
          _target_state_restore(pd->target, pd->target_state);

        //pre end event is supported within class only (protected event)
        efl_event_callback_call(eo_obj, EFL_ANIMATION_EVENT_PRE_END, NULL);
        efl_event_callback_call(eo_obj, EFL_ANIMATION_EVENT_END, NULL);
        //post end event is supported within class only (protected event)
        efl_event_callback_call(eo_obj, EFL_ANIMATION_EVENT_POST_END, NULL);
     }

   if (pd->target)
     efl_event_callback_del(pd->target, EFL_EVENT_DEL, _target_del_cb, eo_obj);

   free(pd->target_state);
   pd->target_state = NULL;

   efl_destructor(efl_super(eo_obj, MY_CLASS));
}

#include "efl_animation.eo.c"
