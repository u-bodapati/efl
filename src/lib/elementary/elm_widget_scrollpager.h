
#ifndef ELM_WIDGET_SCROLLPAGER_H
#define ELM_WIDGET_SCROLLPAGER_H

#include "Elementary.h"

typedef struct _Elm_Scrollpager_Item
{
   int                  page_num;

   Evas_Object          *obj;
   Evas_Object          *layout;
   Evas_Object          *rect;
   Evas_Object          *ind;

   Efl_VG               *ind_vg;

} Elm_Scrollpager_Item;

typedef struct _Elm_Scrollpager_Data
{
   int                  page_cnt;
   int                  cur_page;

   Evas_Object          *mydata;
   Evas_Object          *scroller;
   Evas_Object          *box;
   Evas_Object          *indicator;

   Evas_VG              *on, *off;

   Eina_List            *items;

} Elm_Scrollpager_Data;

#endif
