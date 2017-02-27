#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define ELM_LAYOUT_PROTECTED

#include <Elementary.h>
#include "elm_priv.h"
#include "efl_ui_widget_pagecontrol.h"

#define MY_CLASS EFL_UI_PAGECONTROL_CLASS

#define MY_CLASS_NAME "Efl_Ui_Pagecontrol"


static void
_map_uv_set(Evas_Object *obj, Evas_Map *map)
{
   Evas_Coord x, y, w, h;

   // FIXME: only handles filled obj
   if (efl_isa(obj, EFL_CANVAS_IMAGE_INTERNAL_CLASS) &&
       !evas_object_image_source_get(obj))
     {
        int iw, ih;
        evas_object_image_size_get(obj, &iw, &ih);
        evas_object_geometry_get(obj, &x, &y, &w, &h);
        evas_map_util_points_populate_from_geometry(map, x, y, w, h, 0);
        evas_map_point_image_uv_set(map, 0, 0, 0);
        evas_map_point_image_uv_set(map, 1, iw, 0);
        evas_map_point_image_uv_set(map, 2, iw, ih);
        evas_map_point_image_uv_set(map, 3, 0, ih);
     }
   else
     {
        evas_object_geometry_get(obj, &x, &y, &w, &h);
        evas_map_util_points_populate_from_geometry(map, x, y, w, h, 0);
     }
}

static void
_scroll_effect(Evas_Object *obj, Evas_Object *o1, Evas_Object *o2, double t)
{
   EFL_UI_PAGECONTROL_DATA_GET(obj, sd);

   int w = sd->w * t;

   efl_gfx_position_set(o1, sd->x - w, sd->y);
   efl_gfx_position_set(o2, sd->x + sd->w - w, sd->y);

   if (t == 0) efl_gfx_visible_set(sd->backclip, EINA_FALSE);
   else efl_gfx_visible_set(sd->backclip, EINA_TRUE);
}

static void
_cube_effect(Evas_Object *obj, Evas_Object *o1, Evas_Object *o2, double t)
{
   EFL_UI_PAGECONTROL_DATA_GET(obj, sd);

   Evas_Map *mf, *mb;
   double p, deg, pp;
   Evas_Coord cx, cy, px, py, foc;
   int lx, ly, lz, lr, lg, lb, lar, lag, lab;

   mf = evas_map_new(4);
   mb = evas_map_new(4);

   _map_uv_set(o1, mf);
   _map_uv_set(o2, mb);

   cx = sd->x + (sd->w / 2);
   cy = sd->y + (sd->h / 2);

   px = sd->x + (sd->w / 2);
   py = sd->y + (sd->h / 2);
   foc = 2048;

   lx = cx;
   ly = cy;
   lz = -10000;
   lr = 255;
   lg = 255;
   lb = 255;
   lar = 0;
   lag = 0;
   lab = 0;

   p = 1.0 - t;
   pp = p;
   //if (!lin) pp = (p * p);
   p = 1.0 - pp;
   deg = -90.0 * p;

   evas_map_util_3d_rotate(mf, 0.0, deg, 0.0, cx, cy, sd->w / 2);
   evas_map_util_3d_rotate(mb, 0.0, deg + 90, 0.0, cx, cy, sd->w / 2);

   //clip show/hide update
   evas_map_util_3d_lighting(mf, lx, ly, lz, lr, lg, lb, lar, lag, lab);
   evas_map_util_3d_perspective(mf, px, py, 0, foc);
   evas_object_map_set(o1, mf);
   evas_object_map_enable_set(o1, EINA_TRUE);
   if (evas_map_util_clockwise_get(mf)) evas_object_show(sd->foreclip);
   else evas_object_hide(sd->foreclip);

   evas_map_util_3d_lighting(mb, lx, ly, lz, lr, lg, lb, lar, lag, lab);
   evas_map_util_3d_perspective(mb, px, py, 0, foc);
   evas_object_map_set(o2, mb);
   evas_object_map_enable_set(o2, EINA_TRUE);
   if (evas_map_util_clockwise_get(mb)) evas_object_show(sd->backclip);
   else evas_object_hide(sd->backclip);

   evas_map_free(mf);
   evas_map_free(mb);
}

static void
_page_update(Evas_Object *obj, int page, double t)
{
   EFL_UI_PAGECONTROL_DATA_GET(obj, sd);

   //1. pick two pages and put them into clippers
   int p1 = page;
   int p2 = (page + 1 + sd->cnt) % sd->cnt;
   ERR("page %d %d", p1, p2);

   Evas_Object *o1, *o2, *tmp;
   int i;
   for (i = 0; i < sd->cnt; i++)
     {
        if (i == p1)
          {
             o1 = efl_pack_content_get(obj, p1);
             evas_object_clip_set(o1, sd->foreclip);
          }
        else if (i == p2)
          {
             o2 = efl_pack_content_get(obj, p2);
             evas_object_clip_set(o2, sd->backclip);
          }
        else
          {
             tmp = efl_pack_content_get(obj, i);
             evas_object_clip_set(tmp, sd->invis);
          }
     }

   sd->func[sd->effect](obj, o1, o2, t);
}

static void
_update_job(void *data)
{
   Evas_Object *obj = data;
   EFL_UI_PAGECONTROL_DATA_GET(obj, sd);

   double t;

   sd->job = NULL;

   //1. calculate rotation factor t based on mouse position
   if (sd->dir == 0 || sd->dir == 1)
     t = ((double)sd->mouse_down_x - (double)sd->mouse_x) / (double)sd->w;
   else if (sd->dir == 2 || sd->dir == 3)
     t = ((double)sd->mouse_down_y - (double)sd->mouse_y) / (double)sd->h;

   if (t > 1.0) t = 1.0;
   if (t < -1.0) t = -1.0;

   double rel = sd->t + t;
   rel += (double) sd->cnt;
   int page = (int) rel;
   double ratio = rel - (double) page;
   page = page % sd->cnt;

   ERR("page %d ratio %lf", page, ratio);

   _page_update(obj, page, ratio);
}

static void
_mouse_down_cb(void *data,
               Evas *e EINA_UNUSED,
               Evas_Object *obj EINA_UNUSED,
               void *event_info)
{
   Evas_Event_Mouse_Down *ev = event_info;
   Evas_Object *pc = data;
   EFL_UI_PAGECONTROL_DATA_GET(pc, sd);

   if (ev->button != 1) return;
   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) return;

   ELM_SAFE_FREE(sd->animator, ecore_animator_del);

   sd->move_started = EINA_FALSE;
   sd->mouse_down = EINA_TRUE;

   sd->mouse_x = ev->canvas.x - sd->x;
   sd->mouse_y = ev->canvas.y - sd->y;
   sd->mouse_down_x = sd->mouse_x;
   sd->mouse_down_y = sd->mouse_y;
}

static void
_mouse_up_cb(void *data,
             Evas *e EINA_UNUSED,
             Evas_Object *obj EINA_UNUSED,
             void *event_info)
{
   Evas_Event_Mouse_Up *ev = event_info;
   Evas_Object *pc = data;
   EFL_UI_PAGECONTROL_DATA_GET(pc, sd);

   double t;

   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) return;
   if (!sd->mouse_down) return;

   sd->mouse_down = EINA_FALSE;
   if (!sd->move_started) return;

   ELM_SAFE_FREE(sd->job, ecore_job_del);

   sd->mouse_x = ev->canvas.x - sd->x;
   sd->mouse_y = ev->canvas.y - sd->y;

   //1. calculate rotation factor t based on mouse position
   if (sd->dir == 0 || sd->dir == 1)
     t = ((double)sd->mouse_down_x - (double)sd->mouse_x) / (double)sd->w;
   else if (sd->dir == 2 || sd->dir == 3)
     t = ((double)sd->mouse_down_y - (double)sd->mouse_y) / (double)sd->h;

   if (t > 1.0) t = 1.0;
   if (t < -1.0) t = -1.0;

   sd->t += t;

   if (sd->t < 0) sd->t += (double) sd->cnt;

   //condition check

   //animation
}

static void
_mouse_move_cb(void *data,
               Evas *e EINA_UNUSED,
               Evas_Object *obj EINA_UNUSED,
               void *event_info)
{
   Evas_Event_Mouse_Move *ev = event_info;
   Evas_Object *pc = data;
   EFL_UI_PAGECONTROL_DATA_GET(pc, sd);

   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) return;
   if (!sd->mouse_down) return;

   sd->mouse_x = ev->cur.canvas.x - sd->x;
   sd->mouse_y = ev->cur.canvas.y - sd->y;

   if (!sd->move_started)
     {
        //direction decision
        Evas_Coord dx, dy;
        dx = sd->mouse_x - sd->mouse_down_x;
        dy = sd->mouse_y - sd->mouse_down_y;

        if (((dx * dx) + (dy * dy)) <=
            (_elm_config->finger_size * _elm_config->finger_size / 4))
          return;

        if ((dx < 0) && (abs(dx) > abs(dy))) sd->dir = 0; //left
        else if ((dx >= 0) && (abs(dx) > abs(dy))) sd->dir = 1; //right
        else if ((dy < 0) && (abs(dy) >= abs(dx))) sd->dir = 2; //up
        else if ((dy >= 0) && (abs(dy) >= abs(dx))) sd->dir = 3; //down
        else return;

        sd->move_started = EINA_TRUE;
     }

   ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;

   ecore_job_del(sd->job);
   sd->job = ecore_job_add(_update_job, pc);
}

static void
_event_handler_create(Eo *obj, Efl_Ui_Pagecontrol_Data *sd)
{
   sd->event = evas_object_rectangle_add(evas_object_evas_get(obj));

   evas_object_color_set(sd->event, 0, 0, 0, 0);
   efl_gfx_position_set(sd->event, sd->x, sd->y);
   efl_gfx_size_set(sd->event, sd->w, sd->h);
   efl_gfx_visible_set(sd->event, EINA_TRUE);

   evas_object_event_callback_add(sd->event, EVAS_CALLBACK_MOUSE_DOWN, _mouse_down_cb, obj);
   evas_object_event_callback_add(sd->event, EVAS_CALLBACK_MOUSE_UP, _mouse_up_cb, obj);
   evas_object_event_callback_add(sd->event, EVAS_CALLBACK_MOUSE_MOVE, _mouse_move_cb, obj);
}

EOLIAN static void
_efl_ui_pagecontrol_efl_canvas_group_group_add(Eo *obj,
                                               Efl_Ui_Pagecontrol_Data *sd)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   efl_canvas_group_add(efl_super(obj, MY_CLASS));
   elm_widget_sub_object_parent_add(obj);

   sd->foreclip = evas_object_rectangle_add(evas_object_evas_get(obj));
   evas_object_static_clip_set(sd->foreclip, EINA_TRUE);
   efl_gfx_visible_set(sd->foreclip, EINA_TRUE);

   sd->backclip = evas_object_rectangle_add(evas_object_evas_get(obj));
   evas_object_static_clip_set(sd->backclip, EINA_TRUE);

   sd->invis = evas_object_rectangle_add(evas_object_evas_get(obj));
   evas_object_static_clip_set(sd->invis, EINA_TRUE);

   _event_handler_create(obj, sd);

   sd->cnt = 0;
   sd->cur_page = -1;
   sd->t = 0;

   sd->effect = 0;

   //function initialization
   sd->func[0] = _scroll_effect;
   sd->func[1] = _cube_effect;
}

EOLIAN static Eo *
_efl_ui_pagecontrol_efl_object_constructor(Eo *obj,
                                           Efl_Ui_Pagecontrol_Data *sd)
{
   obj = efl_constructor(efl_super(obj, MY_CLASS));

   return obj;
}

EOLIAN static void
_efl_ui_pagecontrol_efl_gfx_size_set(Eo *obj,
                                     Efl_Ui_Pagecontrol_Data *sd,
                                     Evas_Coord w,
                                     Evas_Coord h)
{
   Eina_List *l;
   Evas_Object *subobj;

   efl_gfx_size_set(efl_super(obj, MY_CLASS), w, h);

   sd->w = w;
   sd->h = h;

   efl_gfx_size_set(sd->foreclip, w, h);
   efl_gfx_size_set(sd->backclip, w, h);
   efl_gfx_size_set(sd->invis, w, h);
   efl_gfx_size_set(sd->event, w, h);

   EINA_LIST_FOREACH(sd->content_list, l, subobj)
     {
        efl_gfx_size_set(subobj, w, h);
     }
}

EOLIAN static void
_efl_ui_pagecontrol_efl_gfx_position_set(Eo *obj,
                                         Efl_Ui_Pagecontrol_Data *sd,
                                         Evas_Coord x,
                                         Evas_Coord y)
{
   efl_gfx_position_set(efl_super(obj, MY_CLASS), x, y);

   sd->x = x;
   sd->y = y;

   efl_gfx_position_set(sd->foreclip, x, y);
   efl_gfx_position_set(sd->backclip, x, y);
   efl_gfx_position_set(sd->invis, x, y);
   efl_gfx_position_set(sd->event, x, y);
}

EOLIAN static int
_efl_ui_pagecontrol_efl_container_content_count(Eo *obj EINA_UNUSED,
                                                Efl_Ui_Pagecontrol_Data *pd)
{
   return eina_list_count(pd->content_list);
}

EOLIAN static Eina_Bool
_efl_ui_pagecontrol_efl_pack_linear_pack_end(Eo *obj,
                                             Efl_Ui_Pagecontrol_Data *pd,
                                             Efl_Gfx *subobj)
{
   //first item
   if (pd->cnt == 0)
     {
        evas_object_clip_set(subobj, pd->foreclip);
        pd->cur_page = 0;
     }
   else
     evas_object_clip_set(subobj, pd->invis);

   pd->cnt += 1;

   pd->content_list = eina_list_append(pd->content_list, subobj);
   efl_gfx_size_set(subobj, pd->w, pd->h);
   efl_gfx_stack_raise(pd->event);

   return EINA_TRUE;
}

EOLIAN static Efl_Gfx *
_efl_ui_pagecontrol_efl_pack_linear_pack_content_get(Eo *obj EINA_UNUSED,
                                                     Efl_Ui_Pagecontrol_Data *pd,
                                                     int index)
{
   return eina_list_nth(pd->content_list, index);
}

EOLIAN static int
_efl_ui_pagecontrol_efl_pack_linear_pack_index_get(Eo *obj EINA_UNUSED,
                                                   Efl_Ui_Pagecontrol_Data *pd,
                                                   const Efl_Gfx *subobj)
{
   return eina_list_data_idx(pd->content_list, (void *)subobj);
}

EOLIAN static void
_efl_ui_pagecontrol_transition_effect_set(Eo *obj,
                                         Efl_Ui_Pagecontrol_Data *pd,
                                         Efl_Ui_Pagecontrol_Effect effect)
{
}

EOLIAN static Efl_Ui_Pagecontrol_Effect
_efl_ui_pagecontrol_transition_effect_get(Eo *obj,
                                          Efl_Ui_Pagecontrol_Data *pd)
{
   return 0;
}

#include "efl_ui_pagecontrol.eo.c"
