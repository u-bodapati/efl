#ifndef TRIANGULATING_STROKER_H_
#define TRIANGULATING_STROKER_H_

#include <Efl.h>

typedef struct _Triangulating_Stroker Triangulating_Stroker;
struct _Triangulating_Stroker
{
   Eina_Inarray *vertices;
   Eina_Inarray *arc_pts;  // intermediate array for storing arc points
   float cx, cy;           // current points
   float nvx, nvy;         // normal vector
   float width;
   float miter_limit;

   int roundness;            // Number of line segments in a round join
   float sin_theta;          // sin(roundness / 360);
   float cos_theta;          // cos(roundness / 360);
   float curvyness_mul;
   float curvyness_add;

   Efl_Gfx_Join join_style;
   Efl_Gfx_Cap  cap_style;
};

/**
 * Creates a new triangulating stroker.
 *
 */
Triangulating_Stroker *triangulating_stroker_new(void);

/**
 * Frees the given Stroker and any associated resource.
 *
 * stroker The given Stroker.
 */
void triangulating_stroker_free(Triangulating_Stroker *stroker);

/**
 * Process the command list to generate triangle strips.
 * The alogrithm handles multiple contour by adding invisible triangles.
 *
 * cmds :        commnad list
 * pts  :        point list.
 * cmd_count :   number of commands.
 * pt_count  :   number of points.
 *
 * output : It generates the outline in the form of triangle strips store in vertices array.
 *          The array can be used to copy the data to a VBO and draw the data using TRIANGLE_STRIP.  
 */
void triangulating_stroker_process(Triangulating_Stroker *stroker, const Efl_Gfx_Path_Command *cmds, const double *pts, int cmd_count, int pt_count);

#endif // TRIANGULATING_STROKER_H_

