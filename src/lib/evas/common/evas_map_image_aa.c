/*
 * evas_map_image_aa.c
 *
 *  Created on: Nov 21, 2014
 *      Author: hermet
 */

#define READY() \
  if (eidx == 0) \
    { \
       x1 = edge2.x; \
       x2 = spans[y].span[0].x[0]; \
    } \
  else \
    { \
       x1 = spans[y].span[0].x[1]; \
       x2 = edge2.x; \
    }

#define NEXT(xx) \
   do \
     { \
        if (spans[y].span[0].x[0] != -1) \
          { \
             edge1.x = edge2.x; \
             edge1.y = edge2.y; \
             edge2.x = (xx); \
             edge2.y = y; \
          } \
     } \
   while (0)

//Vertical Inside Direction
#define VERT_INSIDE(dir, rewind, y_advance) \
   do \
     { \
        cov_range = edge2.y - edge1.y; \
        coverage = (256 / (cov_range + 1)); \
        for (ry = 1; ry < ((rewind) + 1); ry++) \
          { \
             ridx = (y - ry) + (y_advance); \
             if (spans[ridx].aa_len[(dir)] > 1) continue; \
             spans[ridx].aa_len[(dir)] = 1; \
             if ((dir) == 1) \
               { \
                  spans[ridx].aa_cov[(dir)] = \
                     (256 - (coverage * (ry + (cov_range - (rewind))))); \
               } \
             else \
               { \
                  spans[ridx].aa_cov[(dir)] = \
                     (coverage * (ry + (cov_range - (rewind)))); \
               } \
          } \
        last_aa = 4; \
     } \
   while (0)

//Vertical Outside Direction
#define VERT_OUTSIDE(dir, rewind, y_advance, cov_range) \
   do \
     { \
        coverage = (256 / ((cov_range) + 1)); \
        for (ry = 1; ry < ((rewind) + 1); ry++) \
          { \
             ridx = (y - ry) + (y_advance); \
             if (spans[ridx].aa_len[(dir)] > 1) continue; \
             spans[ridx].aa_len[(dir)] = 1; \
             if (dir == 1) \
               { \
                  spans[ridx].aa_cov[(dir)] = \
                     (coverage * (ry + (cov_range - (rewind)))); \
               } \
             else \
               { \
                  spans[ridx].aa_cov[(dir)] = \
                     (256 - (coverage * (ry + ((cov_range) - (rewind))))); \
               } \
          } \
        last_aa = 2; \
     } \
   while(0)

//Horizontal Inside Direction
#define HORIZ_INSIDE(dir, yy, xx, xx2) \
   do \
     { \
        tmp = (xx - xx2); \
        if (tmp > spans[(yy)].aa_len[(dir)]) \
          { \
             spans[(yy)].aa_len[(dir)] = tmp; \
             spans[(yy)].aa_cov[(dir)] = \
                (256 / (spans[(yy)].aa_len[(dir)] + 1)); \
          } \
        last_aa = 3; \
     } \
   while (0)

//Horizontal Outside Direction
#define HORIZ_OUTSIDE(dir, yy, xx, xx2) \
   do \
     { \
        tmp = (xx) - (xx2); \
        if (tmp > spans[(yy)].aa_len[(dir)]) \
          { \
             spans[(yy)].aa_len[(dir)] = tmp; \
             spans[(yy)].aa_cov[(dir)] = \
                (256 / (spans[(yy)].aa_len[(dir)] + 1)); \
          } \
        last_aa = 1; \
     } \
   while (0)

static inline DATA32
aa_convert(Line *line, int ww, int w, int aa_left_range, DATA32 val)
{
   //Left Edge Anti Anliasing
   if (aa_left_range < ww)
     {
        return INTERP_256((line->aa_cov[0] * (w - ww + 1)), val,
                          0x00000000);
     }
   //Right Edge Anti Aliasing
   if (line->aa_len[1] >= ww)
     {
        return INTERP_256(256 - (line->aa_cov[1] * (line->aa_len[1] - ww + 1)),
                          val, 0x00000000);
     }
   return val;
}

void
_calc_aa_edges_internal(Line *spans, int eidx, int ystart, int yend)
{
   int y, tmp;
   int coverage, cov_range;
   int ry, ridx;
   Evas_Coord_Point edge1 = { -1, -1 };
   Evas_Coord_Point edge2 = { -1, -1 };
   int x1 = 0, x2 = 0;
   int x3 = 0, x4 = 0;
   int last_aa = 0; //1: horiz_ld, 2: vert_ld 3: horiz_rd, 4: vert_rd

   yend -= ystart;

   //Find Start Edge
   for (y = 0; y < yend; y++)
     {
        if (spans[y].span[0].x[0] == -1) continue;
        edge1.x = edge2.x = spans[y].span[0].x[eidx];
        edge1.y = edge2.y = y;
        break;
     }

   //Calculates AA Edges
   for (y++; y <= yend; y++)
     {
        READY();

       //Outside Incremental
       if (x1 > x2)
         {
            //Horizontal Edge
            if ((y - edge2.y) == 1)
              {
                  HORIZ_OUTSIDE(eidx, y, x1, x2);

                 //Leftovers
                 if ((y + 1) == yend)
                   {
                      HORIZ_OUTSIDE(1, (y + 1), x1, x2);
                      return;
                   }
              }
            //Vertical Edge
            else if ((x1 - x2) == 1)
              {
                 VERT_OUTSIDE(eidx, (y - edge2.y), 0, (y - edge2.y));
                 if (abs(spans[(y + 1)].span[0].x[eidx] -
                         spans[y].span[0].x[eidx]) == 1)
                   HORIZ_OUTSIDE(eidx, y, x1, x2);
              }
            NEXT(spans[y].span[0].x[eidx]);
         }
       //Inside Incremental
       else if (x2 > x1)
         {
            if (last_aa == 2)
              {
                 VERT_OUTSIDE(1, (y - edge2.y), 0, (y - edge2.y));

                 if (spans[y].span[0].x[0] != -1)
                   {
                      edge1.x = spans[y - 1].span[0].x[eidx];
                      edge1.y = y - 1;
                      edge2.x = spans[y].span[0].x[eidx];
                      edge2.y = y;
                   }
                 else NEXT(spans[y].span[0].x[eidx]);
              }
            //Find Next Edge
            for (y++; y <= yend; y++)
              {
               if (eidx == 0)
                 {
                    x3 = edge2.x;
                    x4 = edge1.x;
                 }
               else
                 {
                    x3 = edge1.x;
                    x4 = edge2.x;
                 }

               READY();

                 //Inside Direction
                 if (x2 > x1)
                   {
                      //Horizontal Edge
                      if ((edge2.y - edge1.y) == 1)
                        {
                           HORIZ_INSIDE(eidx, edge1.y, x3, x4);

                           //Leftovers
                           if ((y + 1) >= yend)
                             {
                                HORIZ_INSIDE(eidx, edge2.y, x3, x4);
                                HORIZ_INSIDE(eidx, y, x3, x4);
                                return;
                             }
                        }
                      //Vertical Edge
                      else if ((x3 - x4) == 1)
                        {
                           VERT_INSIDE(eidx, (edge2.y - edge1.y),
                                       -(y - edge2.y));
                        }
                      NEXT(spans[y].span[0].x[eidx]);
                   }
                 //Reverse. Outside Direction
                 else if (x2 < x1)
                   {
                      //Horizontal Edge
                      if ((edge2.y - edge1.y) == 1)
                        HORIZ_INSIDE(eidx, edge1.y, x3, x4);
                      //Vertical Edge
                      else
                        VERT_INSIDE(eidx, (edge2.y - edge1.y), -(y - edge2.y));

                      NEXT(spans[y].span[0].x[eidx]);
                      break;
                   }
              }
         }
     }
   //Leftovers
   if (edge2.x > edge1.x)
     VERT_OUTSIDE(eidx, (y - edge2.y), 0, (edge2.y - edge1.y));
   else if (edge1.x > edge2.x)
     VERT_INSIDE(eidx, ((y - 1) - edge2.y), -1);
}

static void
_calc_aa_edges(Line *spans, int ystart, int yend)
{
   //left
   _calc_aa_edges_internal(spans, 0, ystart, yend);
   //right
   _calc_aa_edges_internal(spans, 1, ystart, yend);
}
