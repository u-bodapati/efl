/*
 * evas_map_image_aa.c
 *
 *  Created on: Nov 21, 2014
 *      Author: hermet
 */

#define READY_TX() \
  if (eidx == 0) \
    { \
       tx[0] = edge2.x; \
       tx[1] = spans[y].span[0].x[0]; \
    } \
  else \
    { \
       tx[0] = spans[y].span[0].x[1]; \
       tx[1] = edge2.x; \
    }

#define READY_TX2() \
   if (eidx == 0) \
     { \
        tx2[0] = edge2.x; \
        tx2[1] = edge1.x; \
     } \
   else \
    { \
       tx2[0] = edge1.x; \
       tx2[1] = edge2.x; \
    } \

#define PUSH_EDGES(xx) \
   if (spans[y].span[0].x[0] != -1) \
     { \
        edge1.x = edge2.x; \
        edge1.y = edge2.y; \
        edge2.x = (xx); \
        edge2.y = y; \
        reset_tx2 = EINA_TRUE; \
     }

//Vertical Inside Direction
#define VERT_INSIDE(dir, rewind, y_advance) \
{ \
   int cov_range = edge2.y - edge1.y; \
   int coverage = (256 / (cov_range + 1)); \
   int ry; \
   for (ry = 1; ry < ((rewind) + 1); ry++) \
     { \
        int ridx = (y - ry) + (y_advance); \
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
   prev_aa = 4; \
}

//Vertical Outside Direction
#define VERT_OUTSIDE(dir, rewind, y_advance, cov_range) \
{ \
   int coverage = (256 / ((cov_range) + 1)); \
   int ry = 1; \
   for (; ry < ((rewind) + 1); ry++) \
     { \
        int ridx = (y - ry) + (y_advance); \
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
   prev_aa = 2; \
}

//Horizontal Inside Direction
#define HORIZ_INSIDE(dir, yy, xx, xx2) \
{ \
   if (((xx) - (xx2)) > spans[(yy)].aa_len[(dir)]) \
     { \
        spans[(yy)].aa_len[(dir)] = ((xx) - (xx2)); \
        spans[(yy)].aa_cov[(dir)] = (256 / (spans[(yy)].aa_len[(dir)] + 1)); \
     } \
   prev_aa = 3; \
}

//Horizontal Outside Direction
#define HORIZ_OUTSIDE(dir, yy, xx, xx2) \
{ \
   if (((xx) - (xx2)) > spans[(yy)].aa_len[(dir)]) \
     { \
        spans[(yy)].aa_len[(dir)] = ((xx) - (xx2)); \
        spans[(yy)].aa_cov[(dir)] = (256 / (spans[(yy)].aa_len[(dir)] + 1)); \
     } \
   prev_aa = 1; \
}

static inline DATA32
_aa_coverage_apply(Line *line, int ww, int w, DATA32 val)
{
   //Left Edge Anti Anliasing
   if ((w - line->aa_len[0]) < ww)
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
   int y;
   Evas_Coord_Point edge1 = { -1, -1 }; //prev-previous edge pixel
   Evas_Coord_Point edge2 = { -1, -1 }; //previous edge pixel

   /* store larger to tx[0] between prev and current edge's x positions. */
   int tx[2] = {0, 0};

   /* store lager to tx2[0] between edge1 and edge2's x positions. */
   int tx2[2] = {0, 0};

   /* previous edge anti-aliased type.
      1: horizontal_outside
      2: vertical outside
      3: horizontal inside
      4: vertical inside */
   int prev_aa = 0;

   Eina_Bool reset_tx2 = EINA_TRUE;

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
   for (y++; y < yend; y++)
     {
       READY_TX()

       //Case1. Outside Incremental
       if (tx[0] > tx[1])
         {
            //Horizontal Edge
            if ((y - edge2.y) == 1)
              {
                  HORIZ_OUTSIDE(eidx, y, tx[0], tx[1])
              }
            //Vertical Edge
            else if (tx[0] > tx[1])
              {
                 VERT_OUTSIDE(eidx, (y - edge2.y), 0, (y - edge2.y))

                 //Just in case: 1 pixel alias next to vertical edge?
                 if (abs(spans[(y + 1)].span[0].x[eidx] -
                         spans[y].span[0].x[eidx]) >= 1)
                   {
                      HORIZ_OUTSIDE(eidx, y, tx[0], tx[1])
                   }
              }
            PUSH_EDGES(spans[y].span[0].x[eidx])
         }
       //Case2. Inside Incremental
       else if (tx[1] > tx[0])
         {
            //Just in case: direction is reversed at the outside vertical edge?
            if (prev_aa == 2)
              {
                 VERT_OUTSIDE(eidx, (y - edge2.y), 0, (y - edge2.y))

                 if (spans[y].span[0].x[0] != -1)
                   {
                      edge1.x = spans[y - 1].span[0].x[eidx];
                      edge1.y = y - 1;
                      edge2.x = spans[y].span[0].x[eidx];
                      edge2.y = y;
                   }
              }
            else PUSH_EDGES(spans[y].span[0].x[eidx])

            //Find next edge
            for (y++; y < yend; y++)
              {
                 READY_TX()
                 if (reset_tx2)
                   {
                      READY_TX2()
                   }

                 //Case 1. Inside Direction
                 if (tx[1] > tx[0])
                   {
                      //Horizontal Edge
                      if ((edge2.y - edge1.y) == 1)
                        {
                           HORIZ_INSIDE(eidx, edge1.y, tx2[0], tx2[1]);
                        }
                      //Vertical Edge
                      else if ((tx2[0] - tx2[1]) == 1)
                        {
                           VERT_INSIDE(eidx, (edge2.y - edge1.y),
                                       -(y - edge2.y))
                        }
                      //Just in case: Right Side Square Edge...?
                      else if (prev_aa == 4)
                        {
                           VERT_INSIDE(eidx, (edge2.y - edge1.y),
                                       -(y - edge2.y))
                           if ((y - edge2.y) == 1)
                             {
                                HORIZ_INSIDE(eidx, edge2.y - 1,
                                             edge2.x, spans[y].span[0].x[eidx]);
                             }
                        }
                      PUSH_EDGES(spans[y].span[0].x[eidx])
                   }
                 //Case 2. Reversed. Outside Direction
                 else if (tx[1] < tx[0])
                   {
                      //Horizontal Edge
                      if ((edge2.y - edge1.y) == 1)
                        HORIZ_INSIDE(eidx, edge1.y, tx2[0], tx2[1])
                      //Vertical Edge
                      else
                        VERT_INSIDE(eidx, (edge2.y - edge1.y), -(y - edge2.y))

                      PUSH_EDGES(spans[y].span[0].x[eidx])
                      break;
                   }
              }
         }
     }

   //Leftovers
   y = yend;
   switch(prev_aa)
     {
        case 1:
          HORIZ_OUTSIDE(eidx, y, tx[0], tx[1]);
          break;
        case 2:
           if (((eidx == 0) && (edge1.x > edge2.x)) ||
               ((eidx == 1) && (edge1.x < edge2.x)))
          VERT_OUTSIDE(eidx, (y - edge2.y + 1), 1, (edge2.y - edge1.y));
          break;
        case 3:
          HORIZ_INSIDE(eidx, edge1.y, tx2[0], tx2[1])
          HORIZ_INSIDE(eidx, (y - 1), tx2[0], tx2[1]);
          break;
        case 4:
           if (((eidx == 0) && (edge1.x < edge2.x)) ||
               ((eidx == 1) && (edge1.x > edge2.x)))
           {
              VERT_INSIDE(eidx, (edge2.y - edge1.y), -(y - edge2.y))
              VERT_INSIDE(eidx, (y - edge2.y) + 1, 1);
           }
          break;
        default:
          break;
     }
}

static void
_calc_aa_edges(Line *spans, int ystart, int yend)
{
   //left side
   _calc_aa_edges_internal(spans, 0, ystart, yend);
   //right side
   _calc_aa_edges_internal(spans, 1, ystart, yend);
}
