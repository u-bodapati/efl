#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define ELM_INTERFACE_ATSPI_WIDGET_ACTION_PROTECTED
#define ELM_INTERFACE_ATSPI_ACCESSIBLE_PROTECTED
#define ELM_LAYOUT_PROTECTED

#include <Elementary.h>
#include "elm_priv.h"
#include "elm_widget_button.h"
#include "elm_widget_layout.h"
#include "elm_button_internal_part.eo.h"
#include "elm_part_helper.h"

#define MY_CLASS ELM_BUTTON_CLASS

#define MY_CLASS_NAME "Elm_Button"
#define MY_CLASS_NAME_LEGACY "elm_button"

#define ELM_BUTTON(func, ...) _elm_button_ ## func(Eo *obj EINA_UNUSED, Elm_Button_Data *pd EINA_UNUSED, ## __VA_ARGS__)
#define ELM_BUTTON_OVERRIDE(func, base, ...) _elm_button_ ## base ## _ ## func(Eo *obj EINA_UNUSED, Elm_Button_Data *pd EINA_UNUSED, ## __VA_ARGS__)
#define ELM_BUTTON_PRIV(func, ...) _ ## func(Eo *obj EINA_UNUSED, Elm_Button_Data *pd EINA_UNUSED, ## __VA_ARGS__)
#define ELM_BUTTON_DATA(obj) ((Elm_Button_Data*)efl_data_scope_get(obj, ELM_BUTTON_CLASS))

static Eina_Bool ELM_BUTTON(content_set, const char *part, Efl_Canvas_Object *content);
static Eina_Bool ELM_BUTTON(admits_autorepeat_get);
static void      ELM_BUTTON(autorepeat_set, Eina_Bool on);
static Eina_Bool ELM_BUTTON(autorepeat_get);
static void      ELM_BUTTON(autorepeat_initial_timeout_set, double t);
static double    ELM_BUTTON(autorepeat_initial_timeout_get);
static void      ELM_BUTTON(autorepeat_gap_timeout_set, double t);
static double    ELM_BUTTON(autorepeat_gap_timeout_get);

static void      ELM_BUTTON_OVERRIDE(sizing_eval, elm_layout);
static const Elm_Layout_Part_Alias_Description* ELM_BUTTON_OVERRIDE(content_aliases_get, elm_layout);
static const Elm_Layout_Part_Alias_Description* ELM_BUTTON_OVERRIDE(text_aliases_get, elm_layout);
static Eina_Bool ELM_BUTTON_OVERRIDE(activate, elm_widget, Elm_Activate act);
static Elm_Theme_Apply ELM_BUTTON_OVERRIDE(theme_apply, elm_widget);
static Eina_Bool ELM_BUTTON_OVERRIDE(sub_object_del, elm_widget, Efl_Canvas_Object *sobj);
static Eina_Bool ELM_BUTTON_OVERRIDE(widget_event, elm_widget, Efl_Canvas_Object *src, Evas_Callback_Type type, void *event_info);
static void      ELM_BUTTON_OVERRIDE(group_add, efl_canvas_group);
static Eo*       ELM_BUTTON_OVERRIDE(constructor, efl_object);
static Eina_Bool ELM_BUTTON_OVERRIDE(focus_next_manager_is, elm_widget);
static Eina_Bool ELM_BUTTON_OVERRIDE(focus_direction_manager_is, elm_widget);
static const Elm_Atspi_Action* ELM_BUTTON_OVERRIDE(elm_actions_get, elm_interface_atspi_widget_action);

static void      ELM_BUTTON_PRIV(activate);
static void      ELM_BUTTON_PRIV(icon_signal_emit);
static Eina_Bool ELM_BUTTON_PRIV(admits_autorepeat_get);

static const char SIG_CLICKED[] = "clicked";
static const char SIG_REPEATED[] = "repeated";
static const char SIG_PRESSED[] = "pressed";
static const char SIG_UNPRESSED[] = "unpressed";

/* smart callbacks coming from elm button objects (besides the ones
 * coming from elm layout): */
static const Evas_Smart_Cb_Description _smart_callbacks[] = {
   {SIG_CLICKED, ""},
   {SIG_REPEATED, ""},
   {SIG_PRESSED, ""},
   {SIG_UNPRESSED, ""},
   {SIG_LAYOUT_FOCUSED, ""}, /**< handled by elm_layout */
   {SIG_LAYOUT_UNFOCUSED, ""}, /**< handled by elm_layout */
   {NULL, NULL}
};

static const Elm_Layout_Part_Alias_Description _content_aliases[] =
{
   {"icon", "elm.swallow.content"},
   {NULL, NULL}
};

static const Elm_Layout_Part_Alias_Description _text_aliases[] =
{
   {"default", "elm.text"},
   {NULL, NULL}
};

static Eina_Bool _key_action_activate(Evas_Object *obj, const char *params);

static const Elm_Action key_actions[] = {
   {"activate", _key_action_activate},
   {NULL, NULL}
};

static void
ELM_BUTTON_PRIV(activate)
{
   if (!pd) return;

   ELM_SAFE_FREE(pd->timer, ecore_timer_del);
   pd->repeating = EINA_FALSE;

   if ((_elm_config->access_mode == ELM_ACCESS_MODE_OFF) ||
       (_elm_access_2nd_click_timeout(obj)))
     {
        if (_elm_config->access_mode != ELM_ACCESS_MODE_OFF)
          _elm_access_say(E_("Clicked"));
        if (!elm_widget_disabled_get(obj) &&
            !evas_object_freeze_events_get(obj))
          efl_event_callback_legacy_call
            (obj, EFL_UI_EVENT_CLICKED, NULL);
     }
}

EOLIAN static void
ELM_BUTTON_OVERRIDE(sizing_eval, elm_layout)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);
   Evas_Coord minw = -1, minh = -1;

   elm_coords_finger_size_adjust(1, &minw, 1, &minh);
   edje_object_size_min_restricted_calc
     (wd->resize_obj, &minw, &minh, minw, minh);
   evas_object_size_hint_min_set(obj, minw, minh);
}

EOLIAN static Eina_Bool
ELM_BUTTON_OVERRIDE(activate, elm_widget, Elm_Activate act)
{
   if (elm_widget_disabled_get(obj)) return EINA_FALSE;
   if (act != ELM_ACTIVATE_DEFAULT) return EINA_FALSE;
   if (evas_object_freeze_events_get(obj)) return EINA_FALSE;

   efl_event_callback_legacy_call
     (obj, EFL_UI_EVENT_CLICKED, NULL);
   elm_layout_signal_emit(obj, "elm,anim,activate", "elm");

   return EINA_TRUE;
}

/* FIXME: replicated from elm_layout just because button's icon spot
 * is elm.swallow.content, not elm.swallow.icon. Fix that whenever we
 * can changed the theme API */
static void
ELM_BUTTON_PRIV(icon_signal_emit)
{
   char buf[64];

   snprintf(buf, sizeof(buf), "elm,state,icon,%s",
            elm_layout_content_get(obj, "icon") ? "visible" : "hidden");

   elm_layout_signal_emit(obj, buf, "elm");
   edje_object_message_signal_process(elm_layout_edje_get(obj));
   elm_obj_layout_sizing_eval(obj);
}

/* FIXME: replicated from elm_layout just because button's icon spot
 * is elm.swallow.content, not elm.swallow.icon. Fix that whenever we
 * can changed the theme API */
EOLIAN static Elm_Theme_Apply
ELM_BUTTON_OVERRIDE(theme_apply, elm_widget)
{
   Elm_Theme_Apply int_ret = ELM_THEME_APPLY_FAILED;

   int_ret = elm_obj_widget_theme_apply(efl_super(obj, MY_CLASS));
   if (!int_ret) return ELM_THEME_APPLY_FAILED;
   _icon_signal_emit(obj, pd);

   return int_ret;
}

/* FIXME: replicated from elm_layout just because button's icon spot
 * is elm.swallow.content, not elm.swallow.icon. Fix that whenever we
 * can changed the theme API */
EOLIAN static Eina_Bool
ELM_BUTTON_OVERRIDE(sub_object_del, elm_widget, Efl_Canvas_Object *sobj)
{
   Eina_Bool int_ret = EINA_FALSE;

   int_ret = elm_obj_widget_sub_object_del(efl_super(obj, MY_CLASS), sobj);
   if (!int_ret) return EINA_FALSE;

   _icon_signal_emit(obj, pd);

   return EINA_TRUE;
}

/* FIXME: replicated from elm_layout just because button's icon spot
 * is elm.swallow.content, not elm.swallow.icon. Fix that whenever we
 * can changed the theme API */
static Eina_Bool
ELM_BUTTON(content_set, const char *part, Efl_Canvas_Object *content)
{
   Eina_Bool int_ret = EINA_FALSE;

   int_ret = efl_content_set(efl_part(efl_super(obj, MY_CLASS), part), content);
   if (!int_ret) return EINA_FALSE;

   _icon_signal_emit(obj, pd);

   return EINA_TRUE;
}

static Eina_Bool
_key_action_activate(Evas_Object *obj, const char *params EINA_UNUSED)
{
   elm_layout_signal_emit(obj, "elm,anim,activate", "elm");
   _activate(obj, ELM_BUTTON_DATA(obj));
   return EINA_TRUE;
}

EOLIAN static Eina_Bool
ELM_BUTTON_OVERRIDE(widget_event, elm_widget, Efl_Canvas_Object *src, Evas_Callback_Type type, void *event_info)
{
   (void) src;
   Evas_Event_Key_Down *ev = event_info;

   if (type != EVAS_CALLBACK_KEY_DOWN) return EINA_FALSE;
   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) return EINA_FALSE;

   if (!_elm_config_key_binding_call(obj, MY_CLASS_NAME, ev, key_actions))
     return EINA_FALSE;

   ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
   return EINA_TRUE;
}

static void
_on_clicked_signal(void *data,
                   Evas_Object *obj EINA_UNUSED,
                   const char *emission EINA_UNUSED,
                   const char *source EINA_UNUSED)
{
   _activate(data, ELM_BUTTON_DATA(data));
}

static Eina_Bool
_autorepeat_send(void *data)
{
   ELM_BUTTON_DATA_GET_OR_RETURN_VAL(data, pd, ECORE_CALLBACK_CANCEL);

   efl_event_callback_legacy_call
     (data, EFL_UI_EVENT_REPEATED, NULL);
   if (!pd->repeating)
     {
        pd->timer = NULL;
        return ECORE_CALLBACK_CANCEL;
     }

   return ECORE_CALLBACK_RENEW;
}

static Eina_Bool
_autorepeat_initial_send(void *data)
{
   ELM_BUTTON_DATA_GET_OR_RETURN_VAL(data, pd, ECORE_CALLBACK_CANCEL);

   ELM_SAFE_FREE(pd->timer, ecore_timer_del);
   pd->repeating = EINA_TRUE;
   _autorepeat_send(data);
   pd->timer = ecore_timer_add(pd->ar_gap_timeout, _autorepeat_send, data);

   return ECORE_CALLBACK_CANCEL;
}

static void
_on_pressed_signal(void *data,
                   Evas_Object *obj EINA_UNUSED,
                   const char *emission EINA_UNUSED,
                   const char *source EINA_UNUSED)
{
   ELM_BUTTON_DATA_GET_OR_RETURN(data, pd);

   if ((pd->autorepeat) && (!pd->repeating))
     {
        if (pd->ar_initial_timeout <= 0.0)
          _autorepeat_initial_send(data);  /* call immediately */
        else
          pd->timer = ecore_timer_add
              (pd->ar_initial_timeout, _autorepeat_initial_send, data);
     }

   efl_event_callback_legacy_call
     (data, EFL_UI_EVENT_PRESSED, NULL);
}

static void
_on_unpressed_signal(void *data,
                     Evas_Object *obj EINA_UNUSED,
                     const char *emission EINA_UNUSED,
                     const char *source EINA_UNUSED)
{
   ELM_BUTTON_DATA_GET_OR_RETURN(data, pd);

   ELM_SAFE_FREE(pd->timer, ecore_timer_del);
   pd->repeating = EINA_FALSE;
   efl_event_callback_legacy_call
     (data, EFL_UI_EVENT_UNPRESSED, NULL);
}

static char *
_access_info_cb(void *data EINA_UNUSED, Evas_Object *obj)
{
   const char *txt = elm_widget_access_info_get(obj);

   if (!txt) txt = elm_layout_text_get(obj, NULL);
   if (txt) return strdup(txt);

   return NULL;
}

static char *
_access_state_cb(void *data EINA_UNUSED, Evas_Object *obj)
{
   if (elm_widget_disabled_get(obj))
     return strdup(E_("State: Disabled"));

   return NULL;
}

EOLIAN static void
ELM_BUTTON_OVERRIDE(group_add, efl_canvas_group)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   efl_canvas_group_add(efl_super(obj, MY_CLASS));
   elm_widget_sub_object_parent_add(obj);

   edje_object_signal_callback_add
     (wd->resize_obj, "elm,action,click", "*",
     _on_clicked_signal, obj);
   edje_object_signal_callback_add
     (wd->resize_obj, "elm,action,press", "*",
     _on_pressed_signal, obj);
   edje_object_signal_callback_add
     (wd->resize_obj, "elm,action,unpress", "*",
     _on_unpressed_signal, obj);

   _elm_access_object_register(obj, wd->resize_obj);
   _elm_access_text_set
     (_elm_access_info_get(obj), ELM_ACCESS_TYPE, E_("Button"));
   _elm_access_callback_set
     (_elm_access_info_get(obj), ELM_ACCESS_INFO, _access_info_cb, NULL);
   _elm_access_callback_set
     (_elm_access_info_get(obj), ELM_ACCESS_STATE, _access_state_cb, obj);

   elm_widget_can_focus_set(obj, EINA_TRUE);
   if (!elm_layout_theme_set(obj, "button", "base", elm_widget_style_get(obj)))
     CRI("Failed to set layout!");
}

EOLIAN static const Elm_Layout_Part_Alias_Description*
ELM_BUTTON_OVERRIDE(content_aliases_get, elm_layout)
{
   return _content_aliases;
}

EOLIAN static const Elm_Layout_Part_Alias_Description*
ELM_BUTTON_OVERRIDE(text_aliases_get, elm_layout)
{
   return _text_aliases;
}

EAPI Evas_Object *
elm_button_add(Evas_Object *parent)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);
   Evas_Object *obj = efl_add(MY_CLASS, parent);
   return obj;
}

EOLIAN static Eo *
ELM_BUTTON_OVERRIDE(constructor, efl_object)
{
   obj = efl_constructor(efl_super(obj, MY_CLASS));
   efl_canvas_object_type_set(obj, MY_CLASS_NAME_LEGACY);
   evas_object_smart_callbacks_descriptions_set(obj, _smart_callbacks);
   elm_interface_atspi_accessible_role_set(obj, ELM_ATSPI_ROLE_PUSH_BUTTON);

   return obj;
}

EOLIAN static void
ELM_BUTTON(autorepeat_set, Eina_Bool on)
{
   ELM_SAFE_FREE(pd->timer, ecore_timer_del);
   pd->autorepeat = on;
   pd->repeating = EINA_FALSE;
}

static Eina_Bool
ELM_BUTTON_PRIV(admits_autorepeat_get)
{
   Eina_Bool ret = EINA_FALSE;
   ret = elm_obj_button_admits_autorepeat_get((Eo *) obj);
   return ret;
}

EOLIAN static Eina_Bool
ELM_BUTTON(admits_autorepeat_get)
{
   return EINA_TRUE;
}

EOLIAN static Eina_Bool
ELM_BUTTON(autorepeat_get)
{
   return (_admits_autorepeat_get(obj, pd) & pd->autorepeat);
}

EOLIAN static void
ELM_BUTTON(autorepeat_initial_timeout_set, double t)
{
   if (!_admits_autorepeat_get(obj, pd))
     {
        ERR("this widget does not support auto repetition of clicks.");
        return;
     }

   if (EINA_DBL_EQ(pd->ar_initial_timeout, t)) return;
   ELM_SAFE_FREE(pd->timer, ecore_timer_del);
   pd->ar_initial_timeout = t;
}

EOLIAN static double
ELM_BUTTON(autorepeat_initial_timeout_get)
{
   if (!_admits_autorepeat_get(obj, pd))
      return 0.0;
   else
      return pd->ar_initial_timeout;
}

EOLIAN static void
ELM_BUTTON(autorepeat_gap_timeout_set, double t)
{
   if (!_admits_autorepeat_get(obj, pd))
     {
        ERR("this widget does not support auto repetition of clicks.");
        return;
     }

   if (EINA_DBL_EQ(pd->ar_gap_timeout, t)) return;

   pd->ar_gap_timeout = t;
   if ((pd->repeating) && (pd->timer)) ecore_timer_interval_set(pd->timer, t);
}

EOLIAN static double
ELM_BUTTON(autorepeat_gap_timeout_get)
{
   return pd->ar_gap_timeout;
}

EOLIAN static Eina_Bool
ELM_BUTTON_OVERRIDE(focus_next_manager_is, elm_widget)
{
   return EINA_FALSE;
}

EOLIAN static Eina_Bool
ELM_BUTTON_OVERRIDE(focus_direction_manager_is, elm_widget)
{
   return EINA_FALSE;
}

EOLIAN const Elm_Atspi_Action *
ELM_BUTTON_OVERRIDE(elm_actions_get, elm_interface_atspi_widget_action)
{
   static Elm_Atspi_Action atspi_actions[] = {
          { "activate", "activate", NULL, _key_action_activate },
          { NULL, NULL, NULL, NULL}
   };
   return &atspi_actions[0];
}

static void
_elm_button_class_constructor(Efl_Class *klass)
{
   evas_smart_legacy_type_register(MY_CLASS_NAME_LEGACY, klass);
}

#undef ELM_BUTTON
#undef ELM_BUTTON_OVERRIDE
#undef ELM_BUTTON_PRIV
#undef ELM_BUTTON_DATA

EFL_TEXT_DEFAULT_IMPLEMENT(elm_button, Elm_Button_Data)

/* Efl.Part begin */

ELM_PART_OVERRIDE(elm_button, ELM_BUTTON, ELM_LAYOUT, Elm_Button_Data, Elm_Part_Data)
ELM_PART_OVERRIDE_CONTENT_SET(elm_button, ELM_BUTTON, ELM_LAYOUT, Elm_Button_Data, Elm_Part_Data)
#include "elm_button_internal_part.eo.c"

/* Efl.Part end */

#include "elm_button.eo.c"
