/********************
 * typedefs         *
 *******************/


/****************************
 * Extension                *
 ***************************/

typedef struct _Evas_Object_Textblock_Extension Evas_Textblock_Extension_Data;

/**
 * @internal
 * @typedef Evas_Object_Textblock_Paragraph
 * A layouting paragraph.
 */
typedef struct _Evas_Object_Textblock_Paragraph Evas_Object_Textblock_Paragraph;

struct _Evas_Object_Textblock_Extension
{
   Evas_Object                        *parent;

   int                                 last_w, last_h;
   struct {
      int                              l, r, t, b;
   } style_pad;
   double                              valign;
   char                               *markup_text;
   void                               *engine_data;
   const char                         *repch;
   const char                         *bidi_delimiters;
   struct {
      int                              w, h, oneline_h;
      Eina_Bool                        valid : 1;
   } formatted, native;
   Eina_Bool                           redraw : 1;
   Eina_Bool                           changed : 1;
   Eina_Bool                           content_changed : 1;
   Eina_Bool                           format_changed : 1;
   Eina_Bool                           have_ellipsis : 1;
   Eina_Bool                           legacy_newline : 1;
   Eina_Bool                           obstacle_changed : 1;
   Eina_Bool                           done : 1;
};
