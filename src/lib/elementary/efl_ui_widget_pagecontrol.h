
#ifndef EFL_UI_WIDGET_PAGECONTROL_H
#define EFL_UI_WIDGET_PAGECONTROL_H

#include "Elementary.h"

typedef struct _Efl_Ui_Pagecontrol_Data
{
   Eina_List            *content_list;
   Evas_Object          *event;
   Evas_Object          *foreclip, *backclip, *invis;
   Evas_Object          *mydata;

   Ecore_Animator       *animator;
   Ecore_Job            *job;

   void (*func[3])(Evas_Object *, Evas_Object *, Evas_Object *, double);

   Evas_Coord            x, y, w, h;
   Evas_Coord            mouse_x, mouse_y;
   Evas_Coord            mouse_down_x, mouse_down_y;

   Efl_Ui_Pagecontrol_Effect effect;
   int                   cnt;
   int                   cur_page;
   int                   dir;
   double                t;

   Eina_Bool             move_started : 1;
   Eina_Bool             mouse_down : 1;

} Efl_Ui_Pagecontrol_Data;

#define EFL_UI_PAGECONTROL_DATA_GET(o, sd) \
   Efl_Ui_Pagecontrol_Data *sd = efl_data_scope_get(o, EFL_UI_PAGECONTROL_CLASS)

#endif
