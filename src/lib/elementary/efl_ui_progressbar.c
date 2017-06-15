#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define ELM_INTERFACE_ATSPI_ACCESSIBLE_PROTECTED
#define ELM_LAYOUT_PROTECTED

#include <Elementary.h>

#include "elm_priv.h"
#include "efl_ui_progressbar_private.h"
#include "elm_widget_layout.h"

#include "efl_ui_progressbar_internal_part.eo.h"
#include "elm_part_helper.h"

#define MY_CLASS EFL_UI_PROGRESSBAR_CLASS

#define MY_CLASS_NAME "Efl.Ui.Progressbar"
#define MY_CLASS_NAME_LEGACY "elm_progressbar"

static const char SIG_CHANGED[] = "changed";

#define MIN_RATIO_LVL 0.0
#define MAX_RATIO_LVL 1.0

/* smart callbacks coming from elm progressbar objects (besides the
 * ones coming from elm layout): */
static const Evas_Smart_Cb_Description _smart_callbacks[] = {
   {SIG_CHANGED, ""},
   {SIG_WIDGET_LANG_CHANGED, ""}, /**< handled by elm_widget */
   {SIG_WIDGET_ACCESS_CHANGED, ""}, /**< handled by elm_widget */
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

static Efl_Ui_Progress_Status *
_progress_status_new(const char *part_name, double val)
{
   Efl_Ui_Progress_Status *ps;
   ps = calloc(1, sizeof(Efl_Ui_Progress_Status));
   ps->part_name = eina_stringshare_add(part_name);
   ps->val = val;
   return ps;
}

static inline void
_progress_status_free(Efl_Ui_Progress_Status *ps)
{
   eina_stringshare_del(ps->part_name);
   free(ps);
}

static inline Eina_Bool
_is_horizontal(Efl_Orient orientation)
{
   if (orientation == EFL_ORIENT_LEFT ||
       orientation == EFL_ORIENT_RIGHT)
     return EINA_TRUE;

   return EINA_FALSE;
}

static inline Eina_Bool
_is_inverted(Efl_Orient orientation)
{
   if (orientation == EFL_ORIENT_LEFT ||
       orientation == EFL_ORIENT_UP)
     return EINA_TRUE;

   return EINA_FALSE;
}

static void
_units_set(Evas_Object *obj)
{
   ELM_PROGRESSBAR_DATA_GET(obj, sd);

   if (sd->unit_format_func)
     {
        char *buf;

        buf = sd->unit_format_func(sd->val);
        elm_layout_text_set(obj, "elm.text.status", buf);
        if (sd->unit_format_free) sd->unit_format_free(buf);
     }
   else if (sd->units)
     {
        char buf[1024];

        snprintf(buf, sizeof(buf), sd->units, 100 * sd->val);
        elm_layout_text_set(obj, "elm.text.status", buf);
     }
   else elm_layout_text_set(obj, "elm.text.status", NULL);
}

static void
_val_set(Evas_Object *obj)
{
   Eina_Bool rtl;
   double pos;

   ELM_PROGRESSBAR_DATA_GET(obj, sd);
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   rtl = efl_ui_mirrored_get(obj);
   Efl_Ui_Progress_Status *ps;
   Eina_List *l;

   EINA_LIST_FOREACH(sd->progress_status, l, ps)
      {
         pos = ps->val;
         if ((!rtl && _is_inverted(sd->orientation)) ||
              (rtl && ((sd->orientation == EFL_ORIENT_UP) ||
                (sd->orientation == EFL_ORIENT_RIGHT))))
            pos = MAX_RATIO_LVL - pos;

         edje_object_part_drag_value_set
           (wd->resize_obj, ps->part_name, pos, pos);
      }
}

EOLIAN static void
_efl_ui_progressbar_elm_layout_sizing_eval(Eo *obj, Efl_Ui_Progressbar_Data *_pd EINA_UNUSED)
{
   Evas_Coord minw = -1, minh = -1;
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   edje_object_size_min_restricted_calc
     (wd->resize_obj, &minw, &minh, minw, minh);
   evas_object_size_hint_min_set(obj, minw, minh);
   evas_object_size_hint_max_set(obj, -1, -1);
}

/* FIXME: replicated from elm_layout just because progressbar's icon
 * spot is elm.swallow.content, not elm.swallow.icon. Fix that
 * whenever we can changed the theme API */
static void
_icon_signal_emit(Evas_Object *obj)
{
   char buf[64];

   snprintf(buf, sizeof(buf), "elm,state,icon,%s",
            elm_layout_content_get(obj, "icon") ? "visible" : "hidden");

   elm_layout_signal_emit(obj, buf, "elm");
}

/* FIXME: replicated from elm_layout just because progressbar's icon
 * spot is elm.swallow.content, not elm.swallow.icon. Fix that
 * whenever we can changed the theme API */
EOLIAN static Eina_Bool
_efl_ui_progressbar_elm_widget_sub_object_del(Eo *obj, Efl_Ui_Progressbar_Data *_pd EINA_UNUSED, Evas_Object *sobj)
{
   Eina_Bool int_ret = EINA_FALSE;
   int_ret = elm_obj_widget_sub_object_del(efl_super(obj, MY_CLASS), sobj);
   if (!int_ret) return EINA_FALSE;

   _icon_signal_emit(obj);

   return EINA_TRUE;
}

/* FIXME: replicated from elm_layout just because progressbar's icon
 * spot is elm.swallow.content, not elm.swallow.icon. Fix that
 * whenever we can changed the theme API */
static Eina_Bool
_efl_ui_progressbar_content_set(Eo *obj, Efl_Ui_Progressbar_Data *_pd EINA_UNUSED, const char *part, Evas_Object *content)
{
   Eina_Bool int_ret = EINA_FALSE;
   int_ret = efl_content_set(efl_part(efl_super(obj, MY_CLASS), part), content);
   if (!int_ret) return EINA_FALSE;

   _icon_signal_emit(obj);

   return EINA_TRUE;
}

EOLIAN static Elm_Theme_Apply
_efl_ui_progressbar_elm_widget_theme_apply(Eo *obj, Efl_Ui_Progressbar_Data *sd)
{
   Elm_Theme_Apply int_ret = ELM_THEME_APPLY_FAILED;
   ELM_LAYOUT_DATA_GET(obj, ld);
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, ELM_THEME_APPLY_FAILED);

   if (_is_horizontal(sd->orientation))
     eina_stringshare_replace(&ld->group, "horizontal");
   else eina_stringshare_replace(&ld->group, "vertical");

   int_ret = elm_obj_widget_theme_apply(efl_super(obj, MY_CLASS));
   if (!int_ret) return ELM_THEME_APPLY_FAILED;

   if (sd->pulse)
     elm_layout_signal_emit(obj, "elm,state,pulse", "elm");
   else
     elm_layout_signal_emit(obj, "elm,state,fraction", "elm");

   if (sd->pulse_state)
     elm_layout_signal_emit(obj, "elm,state,pulse,start", "elm");

   if (((sd->units) || (sd->unit_format_func)) && (!sd->pulse))
     elm_layout_signal_emit(obj, "elm,state,units,visible", "elm");

   if (_is_horizontal(sd->orientation))
     evas_object_size_hint_min_set
       (sd->spacer, (double)sd->size * efl_ui_scale_get(obj) *
       elm_config_scale_get(), 1);
   else
     evas_object_size_hint_min_set
       (sd->spacer, 1, (double)sd->size * efl_ui_scale_get(obj) *
       elm_config_scale_get());

   if (_is_inverted(sd->orientation))
     elm_layout_signal_emit(obj, "elm,state,inverted,on", "elm");
   else
     elm_layout_signal_emit(obj, "elm,state,inverted,off", "elm");

   _units_set(obj);
   _val_set(obj);

   /* FIXME: replicated from elm_layout just because progressbar's
    * icon spot is elm.swallow.content, not elm.swallow.icon. Fix that
    * whenever we can changed the theme API */
   _icon_signal_emit(obj);

   edje_object_message_signal_process(wd->resize_obj);

   elm_layout_sizing_eval(obj);

   return int_ret;
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
   char *ret;
   Eina_Strbuf *buf;
   buf = eina_strbuf_new();

   const char *txt = elm_layout_text_get(obj, "elm.text.status");
   if (txt) eina_strbuf_append(buf, txt);

   if (elm_widget_disabled_get(obj))
     eina_strbuf_append(buf, " state: disabled");

   if (eina_strbuf_length_get(buf))
     {
        ret = eina_strbuf_string_steal(buf);
        eina_strbuf_free(buf);
        return ret;
     }

   eina_strbuf_free(buf);
   return NULL;
}

EOLIAN static void
_efl_ui_progressbar_efl_canvas_group_group_add(Eo *obj, Efl_Ui_Progressbar_Data *priv)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   efl_canvas_group_add(efl_super(obj, MY_CLASS));
   elm_widget_sub_object_parent_add(obj);

   priv->orientation = EFL_ORIENT_RIGHT;
   priv->units = eina_stringshare_add("%.0f %%");
   priv->val = MIN_RATIO_LVL;

   if (!elm_layout_theme_set
       (obj, "progressbar", "horizontal", elm_widget_style_get(obj)))
     CRI("Failed to set layout!");

   priv->spacer = evas_object_rectangle_add(evas_object_evas_get(obj));
   evas_object_color_set(priv->spacer, 0, 0, 0, 0);
   evas_object_pass_events_set(priv->spacer, EINA_TRUE);

   elm_layout_content_set(obj, "elm.swallow.bar", priv->spacer);

   _units_set(obj);
   _val_set(obj);

   elm_layout_sizing_eval(obj);

   if (_elm_config->access_mode == ELM_ACCESS_MODE_ON)
     elm_widget_can_focus_set(obj, EINA_TRUE);

   _elm_access_object_register(obj, wd->resize_obj);
   _elm_access_text_set
     (_elm_access_info_get(obj), ELM_ACCESS_TYPE, E_("progressbar"));
   _elm_access_callback_set
     (_elm_access_info_get(obj), ELM_ACCESS_INFO, _access_info_cb, NULL);
   _elm_access_callback_set
     (_elm_access_info_get(obj), ELM_ACCESS_STATE, _access_state_cb, NULL);
}

EOLIAN static void
_efl_ui_progressbar_efl_canvas_group_group_del(Eo *obj, Efl_Ui_Progressbar_Data *sd)
{
   Efl_Ui_Progress_Status *progress_obj;

   eina_stringshare_del(sd->units);
   sd->units = NULL;
   if (sd->progress_status)
      {
         EINA_LIST_FREE(sd->progress_status, progress_obj)
           {
              _progress_status_free(progress_obj);
           }
      }

   efl_canvas_group_del(efl_super(obj, MY_CLASS));
}

EOLIAN static const Elm_Layout_Part_Alias_Description*
_efl_ui_progressbar_elm_layout_text_aliases_get(Eo *obj EINA_UNUSED, Efl_Ui_Progressbar_Data *_pd EINA_UNUSED)
{
   return _text_aliases;
}

EOLIAN static const Elm_Layout_Part_Alias_Description*
_efl_ui_progressbar_elm_layout_content_aliases_get(Eo *obj EINA_UNUSED, Efl_Ui_Progressbar_Data *_pd EINA_UNUSED)
{
   return _content_aliases;
}

EAPI Evas_Object *
elm_progressbar_add(Evas_Object *parent)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);
   Evas_Object *obj = efl_add(MY_CLASS, parent);
   return obj;
}

EOLIAN static Eo *
_efl_ui_progressbar_efl_object_constructor(Eo *obj, Efl_Ui_Progressbar_Data *_pd EINA_UNUSED)
{
   obj = efl_constructor(efl_super(obj, MY_CLASS));
   efl_canvas_object_type_set(obj, MY_CLASS_NAME_LEGACY);
   evas_object_smart_callbacks_descriptions_set(obj, _smart_callbacks);
   elm_interface_atspi_accessible_role_set(obj, ELM_ATSPI_ROLE_PROGRESS_BAR);

   return obj;
}

EOLIAN static void
_efl_ui_progressbar_pulse_mode_set(Eo *obj EINA_UNUSED, Efl_Ui_Progressbar_Data *sd, Eina_Bool pulse)
{
   pulse = !!pulse;
   if (sd->pulse == pulse) return;

   sd->pulse = pulse;

   elm_obj_widget_theme_apply(obj);
}

EOLIAN static Eina_Bool
_efl_ui_progressbar_pulse_mode_get(Eo *obj EINA_UNUSED, Efl_Ui_Progressbar_Data *sd)
{
   return sd->pulse;
}

EOLIAN static void
_efl_ui_progressbar_efl_orientation_orientation_set(Eo *obj, Efl_Ui_Progressbar_Data *sd, Efl_Orient dir)
{
   sd->orientation = dir;

   elm_obj_widget_theme_apply(obj);
}

EOLIAN static Efl_Orient
_efl_ui_progressbar_efl_orientation_orientation_get(Eo *obj EINA_UNUSED, Efl_Ui_Progressbar_Data *sd)
{
   return sd->orientation;
}

EOLIAN static void
_efl_ui_progressbar_efl_ui_range_span_size_set(Eo *obj, Efl_Ui_Progressbar_Data *sd, Evas_Coord size)
{
   if (sd->size == size) return;

   sd->size = size;

   if (_is_horizontal(sd->orientation))
     evas_object_size_hint_min_set
       (sd->spacer, (double)sd->size * efl_ui_scale_get(obj) *
       elm_config_scale_get(), 1);
   else
     evas_object_size_hint_min_set
       (sd->spacer, 1, (double)sd->size * efl_ui_scale_get(obj) *
       elm_config_scale_get());

   elm_layout_sizing_eval(obj);
}

EOLIAN static Evas_Coord
_efl_ui_progressbar_efl_ui_range_span_size_get(Eo *obj EINA_UNUSED, Efl_Ui_Progressbar_Data *sd)
{
   return sd->size;
}

static void
_progressbar_part_value_set(Eo *obj, Efl_Ui_Progressbar_Data *sd, const char *part_name, double val)
{
   Efl_Ui_Progress_Status *ps;
   Eina_Bool  existing_ps = EINA_FALSE;
   Eina_List *l;

   if (val < MIN_RATIO_LVL) val = MIN_RATIO_LVL;
   if (val > MAX_RATIO_LVL) val = MAX_RATIO_LVL;

   if (!strcmp(part_name, "elm.cur.progressbar"))
     sd->val = val;

   EINA_LIST_FOREACH(sd->progress_status, l, ps)
     {
        if (!strcmp(ps->part_name, part_name))
          {
             existing_ps = EINA_TRUE;
             break;
          }
     }

   if (!existing_ps)
      {
         ps = _progress_status_new(part_name, val);
         sd->progress_status = eina_list_append(sd->progress_status, ps);
      }
   else
      ps->val = val;

   _val_set(obj);
   _units_set(obj);
   efl_event_callback_legacy_call
     (obj, EFL_UI_PROGRESSBAR_EVENT_CHANGED, NULL);
}

static double
_progressbar_part_value_get(Efl_Ui_Progressbar_Data *sd, const char* part)
{
   Efl_Ui_Progress_Status *ps;
   Eina_List *l;

   EINA_LIST_FOREACH(sd->progress_status, l, ps)
     {
        if (!strcmp(ps->part_name, part)) return ps->val;
     }

   return 0.0;
}

EOLIAN static void
_efl_ui_progressbar_efl_ui_range_range_value_set(Eo *obj, Efl_Ui_Progressbar_Data *sd, double val)
{
   if (EINA_DBL_EQ(sd->val, val)) return;

   _progressbar_part_value_set(obj, sd, "elm.cur.progressbar", val);
}

EOLIAN static double
_efl_ui_progressbar_efl_ui_range_range_value_get(Eo *obj EINA_UNUSED, Efl_Ui_Progressbar_Data *sd)
{
   return sd->val;
}

EOLIAN static void
_efl_ui_progressbar_efl_ui_range_range_unit_format_set(Eo *obj, Efl_Ui_Progressbar_Data *sd, const char *units)
{
   const char *sig;
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   eina_stringshare_replace(&sd->units, units);
   sig = (units) ? "elm,state,units,visible" : "elm,state,units,hidden";
   elm_layout_signal_emit(obj, sig, "elm");
   edje_object_message_signal_process(wd->resize_obj);

   _units_set(obj);
   elm_layout_sizing_eval(obj);
}

EOLIAN static const char *
_efl_ui_progressbar_efl_ui_range_range_unit_format_get(Eo *obj EINA_UNUSED, Efl_Ui_Progressbar_Data *sd)
{
   return sd->units;
}

EOLIAN static void
_efl_ui_progressbar_pulse_set(Eo *obj, Efl_Ui_Progressbar_Data *sd, Eina_Bool state)
{
   state = !!state;
   if ((!sd->pulse) || (sd->pulse_state == state)) return;

   sd->pulse_state = state;

   if (sd->pulse_state)
     elm_layout_signal_emit(obj, "elm,state,pulse,start", "elm");
   else
     elm_layout_signal_emit(obj, "elm,state,pulse,stop", "elm");
}

EOLIAN static Eina_Bool
_efl_ui_progressbar_pulse_get(Eo *obj EINA_UNUSED, Efl_Ui_Progressbar_Data *sd)
{
   return (sd->pulse_state && sd->pulse);
}

EAPI void
elm_progressbar_value_set(Evas_Object *obj, double val)
{
   efl_ui_range_value_set(obj, val);
}

EAPI double
elm_progressbar_value_get(const Evas_Object *obj)
{
   return efl_ui_range_value_get(obj);
}

EAPI void
elm_progressbar_span_size_set(Evas_Object *obj, Evas_Coord size)
{
   efl_ui_range_span_size_set(obj, size);
}

EAPI Evas_Coord
elm_progressbar_span_size_get(const Evas_Object *obj)
{
   return efl_ui_range_span_size_get(obj);
}

EAPI void
elm_progressbar_unit_format_set(Evas_Object *obj, const char *units)
{
   efl_ui_range_unit_format_set(obj, units);
}

EAPI const char *
elm_progressbar_unit_format_get(const Evas_Object *obj)
{
   return efl_ui_range_unit_format_get(obj);
}

EAPI void
elm_progressbar_unit_format_function_set(Evas_Object *obj, progressbar_func_type func, progressbar_freefunc_type free_func)
{
   const char *sig;
   ELM_PROGRESSBAR_DATA_GET(obj, sd);
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   sd->unit_format_func = func;
   sd->unit_format_free = free_func;
   sig = (func) ? "elm,state,units,visible" : "elm,state,units,hidden";
   elm_layout_signal_emit(obj, sig, "elm");
   edje_object_message_signal_process(wd->resize_obj);

   _units_set(obj);
   elm_layout_sizing_eval(obj);
}

static Efl_Orient
_orientation_get(Eina_Bool horizontal, Eina_Bool inverted)
{
   if (horizontal)
     {
        if (inverted)
          return EFL_ORIENT_LEFT;
        else
          return EFL_ORIENT_RIGHT;
     }
   else
     {
        if (inverted)
          return EFL_ORIENT_UP;
        else
          return EFL_ORIENT_DOWN;
     }
}

EAPI void
elm_progressbar_horizontal_set(Evas_Object *obj, Eina_Bool horizontal)
{
   Efl_Orient dir;
   ELM_PROGRESSBAR_DATA_GET(obj, sd);

   dir = _orientation_get(horizontal, _is_inverted(sd->orientation));

   efl_orientation_set(obj, dir);
}

EAPI Eina_Bool
elm_progressbar_horizontal_get(const Evas_Object *obj)
{
   Efl_Orient dir;
   dir = efl_orientation_get(obj);

   return _is_horizontal(dir);
}

EAPI void
elm_progressbar_inverted_set(Evas_Object *obj, Eina_Bool inverted)
{
   Efl_Orient dir;
   ELM_PROGRESSBAR_DATA_GET(obj, sd);

   dir = _orientation_get(_is_horizontal(sd->orientation), inverted);

   efl_orientation_set(obj, dir);
}

EAPI Eina_Bool
elm_progressbar_inverted_get(const Evas_Object *obj)
{
   Efl_Orient dir;
   dir = efl_orientation_get(obj);

   return _is_inverted(dir);
}

EOLIAN static Eina_Bool
_efl_ui_progressbar_elm_widget_focus_next_manager_is(Eo *obj EINA_UNUSED, Efl_Ui_Progressbar_Data *_pd EINA_UNUSED)
{
   return EINA_FALSE;
}

EOLIAN static Eina_Bool
_efl_ui_progressbar_elm_widget_focus_direction_manager_is(Eo *obj EINA_UNUSED, Efl_Ui_Progressbar_Data *_pd EINA_UNUSED)
{
   return EINA_FALSE;
}

EOLIAN static void
_efl_ui_progressbar_class_constructor(Efl_Class *klass)
{
   evas_smart_legacy_type_register(MY_CLASS_NAME_LEGACY, klass);
}

/* Efl.Part begin */
ELM_PART_OVERRIDE(efl_ui_progressbar, EFL_UI_PROGRESSBAR, ELM_LAYOUT, Efl_Ui_Progressbar_Data, Elm_Part_Data)

static EOLIAN Eina_Bool
_efl_ui_progressbar_internal_part_efl_container_content_set(Eo *obj, Elm_Part_Data *_pd EINA_UNUSED, Efl_Gfx *content)
{
   Elm_Part_Data *pd = efl_data_scope_get(obj, ELM_LAYOUT_INTERNAL_PART_CLASS);
   Efl_Ui_Progressbar_Data *sd = efl_data_scope_get(pd->obj, EFL_UI_PROGRESSBAR_CLASS);
   ELM_PART_RETURN_VAL(_efl_ui_progressbar_content_set(pd->obj, sd, pd->part, content));
}

EOLIAN static void
_efl_ui_progressbar_internal_part_efl_ui_range_range_value_set(Eo *obj, Elm_Part_Data *_pd EINA_UNUSED, double val)
{
  Elm_Part_Data *pd = efl_data_scope_get(obj, ELM_LAYOUT_INTERNAL_PART_CLASS);
  Efl_Ui_Progressbar_Data *sd = efl_data_scope_get(pd->obj, EFL_UI_PROGRESSBAR_CLASS);

  _progressbar_part_value_set(pd->obj, sd, pd->part, val);
}

EOLIAN static double
_efl_ui_progressbar_internal_part_efl_ui_range_range_value_get(Eo *obj, Elm_Part_Data *_pd EINA_UNUSED)
{
   Elm_Part_Data *pd = efl_data_scope_get(obj, ELM_LAYOUT_INTERNAL_PART_CLASS);
   Efl_Ui_Progressbar_Data *sd = efl_data_scope_get(pd->obj, EFL_UI_PROGRESSBAR_CLASS);

   return _progressbar_part_value_get(sd, pd->part);
}

#include "efl_ui_progressbar_internal_part.eo.c"

/* Efl.Part end */

/* Internal EO APIs and hidden overrides */

#define EFL_UI_PROGRESSBAR_EXTRA_OPS \
   EFL_CANVAS_GROUP_ADD_DEL_OPS(efl_ui_progressbar)

#include "efl_ui_progressbar.eo.c"

EAPI void
elm_progressbar_pulse_set(Evas_Object *obj, Eina_Bool pulse)
{
   efl_ui_progressbar_pulse_mode_set(obj, pulse);
}

EAPI Eina_Bool
elm_progressbar_pulse_get(const Evas_Object *obj)
{
   return efl_ui_progressbar_pulse_mode_get(obj);
}

EAPI void
elm_progressbar_pulse(Evas_Object *obj, Eina_Bool state)
{
   efl_ui_progressbar_pulse_set(obj, state);
}

EAPI Eina_Bool
elm_progressbar_is_pulsing_get(const Evas_Object *obj)
{
   return efl_ui_progressbar_pulse_get(obj);
}

EAPI void
elm_progressbar_part_value_set(Evas_Object *obj, const char *part, double val)
{
   if (EINA_DBL_EQ(efl_ui_range_value_get(efl_part(obj, part)), val)) return;
   efl_ui_range_value_set(efl_part(obj, part), val);
}

EAPI double
elm_progressbar_part_value_get(const Evas_Object *obj, const char *part)
{
   return efl_ui_range_value_get(efl_part(obj, part));
}
