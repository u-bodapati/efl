#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define ELM_LAYOUT_PROTECTED

#include <Elementary.h>
#include "elm_priv.h"
#include "elm_widget_scrollpager.h"
#include "elm_widget_layout.h"

#define MY_CLASS ELM_SCROLLPAGER_CLASS

#define MY_CLASS_NAME "Elm_Scrollpager"

static void
_scroll_cb(Evas_Object *obj, void *data)
{
   int x, y, w, h, page;
   double rel, ratio;
   Evas_Object *parent;
   Elm_Scrollpager_Data *sd;
   Elm_Scrollpager_Item *prev, *next;

   elm_interface_scrollable_content_pos_get(obj, &x, &y);
   elm_interface_scrollable_paging_get(obj, NULL, NULL, &w, &h);
   rel = (double)x / (double)w;
   page = (int) rel;
   ratio = rel - (double)page;

   parent = elm_widget_parent_get(obj);
   sd = efl_data_scope_get(parent, ELM_SCROLLPAGER_CLASS);
   prev = eina_list_nth(sd->items, page);
   next = eina_list_nth(sd->items, page + 1);

   if (prev)
     efl_vg_interpolate(prev->ind_vg, sd->on, sd->off, ratio);
   if (next)
     efl_vg_interpolate(next->ind_vg, sd->off, sd->on, ratio);
}

EOLIAN static void
_elm_scrollpager_item_append(Eo *obj, Elm_Scrollpager_Data *sd, Eo *item_obj)
{
   //TODO make item class
   Elm_Scrollpager_Item *item = malloc(sizeof(Elm_Scrollpager_Item));

   item->layout = efl_add(ELM_LAYOUT_CLASS, obj);
   if (!elm_layout_theme_set(item->layout, "scrollpager", "page", elm_widget_style_get(obj)))
     CRI("Failed to set layout!");
   efl_gfx_visible_set(item->layout, EINA_TRUE);
   elm_box_pack_end(sd->box, item->layout);

   item->rect = evas_object_rectangle_add(evas_object_evas_get(item->layout));
   elm_object_part_content_set(item->layout, "rect", item->rect);
   Evas_Coord w, h;
   efl_gfx_size_get(obj, &w, &h);
   evas_object_size_hint_min_set(item->rect, w, h);

   item->obj = item_obj;
   efl_content_set(item->layout, item->obj);

   sd->items = eina_list_append(sd->items, item);

   item->page_num = sd->page_cnt;
   sd->page_cnt++;

   Evas_Object *ind = evas_object_vg_add(evas_object_evas_get(obj));
   efl_gfx_visible_set(ind, EINA_TRUE);
   evas_object_size_hint_min_set(ind, 10, 10);
   elm_box_pack_end(sd->indicator, ind);

   item->ind = ind;

   Efl_VG *r = evas_object_vg_root_node_get(ind);

   item->ind_vg = efl_add(EFL_VG_SHAPE_CLASS, r);

   if (item->page_num == sd->cur_page) 
     efl_vg_interpolate(item->ind_vg, sd->on, sd->off, 0.0);
   else
     efl_vg_interpolate(item->ind_vg, sd->on, sd->off, 1.0);
}

EOLIAN static void
_elm_scrollpager_efl_gfx_size_set(Eo *obj, Elm_Scrollpager_Data *sd, Evas_Coord w, Evas_Coord h)
{
   efl_gfx_size_set(efl_super(obj, MY_CLASS), w, h);

   //elm_interface_scrollable_page_size_set(sd->scroller, w, h);
   elm_interface_scrollable_paging_set(sd->scroller, 0, 0, w, h);

   Eina_List *l;
   Elm_Scrollpager_Item *it;
   EINA_LIST_FOREACH(sd->items, l, it)
     {
        evas_object_size_hint_min_set(it->rect, w, h);
     }
}

EOLIAN static void
_elm_scrollpager_efl_canvas_group_group_add(Eo *obj, Elm_Scrollpager_Data *sd)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   efl_canvas_group_add(efl_super(obj, MY_CLASS));
   elm_widget_sub_object_parent_add(obj);

   if (!elm_layout_theme_set(obj, "scrollpager", "base", elm_widget_style_get(obj)))
     CRI("Failed to set layout!");

   sd->scroller = elm_scroller_add(obj);
   elm_widget_sub_object_add(obj, sd->scroller);
   evas_object_show(sd->scroller);
   efl_content_set(obj, sd->scroller);
   elm_interface_scrollable_bounce_allow_set(sd->scroller, EINA_FALSE, EINA_FALSE);
   elm_interface_scrollable_policy_set(sd->scroller, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_OFF);
   elm_interface_scrollable_scroll_cb_set(sd->scroller, _scroll_cb);

   sd->box = elm_box_add(sd->scroller);
   elm_box_horizontal_set(sd->box, EINA_TRUE);
   elm_widget_sub_object_add(obj, sd->box);
   //efl_content_set(sd->scroller, sd->box);
   elm_object_content_set(sd->scroller, sd->box);
   evas_object_show(sd->box);

   sd->indicator = elm_box_add(obj);
   elm_box_horizontal_set(sd->indicator, EINA_TRUE);
   elm_widget_sub_object_add(obj, sd->indicator);
   //efl_content_set(sd->scroller, sd->box);
   elm_object_part_content_set(obj, "indicator", sd->indicator);
   evas_object_show(sd->indicator);
   elm_box_padding_set(sd->indicator, 10, 10);

   sd->page_cnt = 0;
   sd->cur_page = 0;

   sd->on = efl_add(EFL_VG_SHAPE_CLASS, NULL);
   evas_vg_shape_append_circle(sd->on, 5, 5, 5);
   evas_vg_node_color_set(sd->on, 0, 0, 0, 255);

   sd->off = efl_add(EFL_VG_SHAPE_CLASS, NULL);
   evas_vg_shape_append_circle(sd->off, 5, 5, 5);
   evas_vg_node_color_set(sd->off, 255, 255, 255, 255);
}

EOLIAN static Eo *
_elm_scrollpager_efl_object_constructor(Eo *obj, Elm_Scrollpager_Data *sd)
{
   obj = efl_constructor(efl_super(obj, MY_CLASS));

   return obj;
}

#include "elm_scrollpager.eo.c"
