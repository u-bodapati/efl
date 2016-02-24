#include "ecore_drm2_private.h"

static int
_plane_supported(Ecore_Drm2_Launcher *launcher, Ecore_Drm2_Output *output, uint32_t supported)
{
   int c;

   for (c = 0; c < launcher->num_crtcs; c++)
     {
        if (launcher->crtcs[c] != output->crtc_id) continue;
        if (supported & (1 << c)) return -1;
     }

   return 0;
}

EAPI Eina_Bool
ecore_drm2_planes_create(Ecore_Drm2_Launcher *launcher, int fd)
{
   Ecore_Drm2_Output *output;
   Ecore_Drm2_Plane *plane;
   drmModePlaneRes *res;
   Eina_List *l;
   uint32_t i;
   int cw = 0, ch = 0;

   EINA_SAFETY_ON_NULL_RETURN_VAL(launcher, EINA_FALSE);
   EINA_SAFETY_ON_TRUE_RETURN_VAL((fd < 0), EINA_FALSE);

   drmSetClientCap(fd, DRM_CLIENT_CAP_UNIVERSAL_PLANES, 1);

   /* NB: Testing only */
   drmSetClientCap(fd, DRM_CLIENT_CAP_ATOMIC, 1);

   ecore_drm2_device_cursor_size_get(fd, &cw, &ch);

   res = drmModeGetPlaneResources(fd);
   if (!res)
     {
        ERR("Failed to get plane resources: %s\n", strerror(errno));
        return EINA_FALSE;
     }

   for (i = 0; i < res->count_planes; i++)
     {
        drmModePlane *dplane;
        drmModeObjectPropertiesPtr props;

        dplane = drmModeGetPlane(fd, res->planes[i]);
        if (!dplane) continue;

        plane = calloc(1, sizeof(Ecore_Drm2_Plane) +
                       ((sizeof(uint32_t)) * dplane->count_formats));
        if (!plane)
          {
             ERR("Could not allocate space for plane");
             drmModeFreePlane(dplane);
             continue;
          }

        plane->crtcs = dplane->possible_crtcs;
        plane->id = dplane->plane_id;
        plane->num_formats = dplane->count_formats;
        memcpy(plane->formats, dplane->formats,
               plane->num_formats * sizeof(plane->formats[0]));

        unsigned int j;
        DBG("Plane %d", plane->id);
        DBG("\tFormats");
        for (j = 0; j < plane->num_formats; j++)
          {
             DBG("\t\t%4.4s", (char *)&plane->formats[j]);
          }

        props =
          drmModeObjectGetProperties(fd, plane->id, DRM_MODE_OBJECT_PLANE);
        if (props)
          {
             uint32_t j;
             int type = -1, k = 0;

             for (j = 0; type == -1 && j < props->count_props; j++)
               {
                  drmModePropertyPtr prop;

                  prop = drmModeGetProperty(fd, props->props[j]);
                  if (!prop) continue;

                  if (!strcmp(prop->name, "type"))
                    plane->type = props->prop_values[j];
                  else if (!strcmp(prop->name, "rotation"))
                    {
                       plane->rotation_prop_id = props->props[j];
                       plane->rotation = props->prop_values[j];

                       for (k = 0; k < prop->count_enums; k++)
                         {
                            int r = -1;

                            if (!strcmp(prop->enums[k].name, "rotate-0"))
                              r = ECORE_DRM2_PLANE_ROTATION_NORMAL;
                            else if (!strcmp(prop->enums[k].name, "rotate-90"))
                              r = ECORE_DRM2_PLANE_ROTATION_90;
                            else if (!strcmp(prop->enums[k].name, "rotate-180"))
                              r = ECORE_DRM2_PLANE_ROTATION_180;
                            else if (!strcmp(prop->enums[k].name, "rotate-270"))
                              r = ECORE_DRM2_PLANE_ROTATION_270;
                            else if (!strcmp(prop->enums[k].name, "reflect-x"))
                              r = ECORE_DRM2_PLANE_ROTATION_REFLECT_X;
                            else if (!strcmp(prop->enums[k].name, "reflect-y"))
                              r = ECORE_DRM2_PLANE_ROTATION_REFLECT_Y;

                            if (r != -1)
                              {
                                 plane->supported_rotations |= r;
                                 plane->rotation_map[ffs(r)] =
                                   1 << prop->enums[k].value;
                              }
                         }
                    }

                  if (plane->type == DRM_PLANE_TYPE_CURSOR)
                    {
                       plane->cw = cw;
                       plane->ch = ch;
                    }

                  drmModeFreeProperty(prop);
               }
          }

        drmModeFreePlane(dplane);

        EINA_LIST_FOREACH(launcher->outputs, l, output)
          {
             if (!_plane_supported(launcher, output, plane->crtcs))
               continue;

             DBG("Adding Plane %d to Output %d", plane->id, output->crtc_id);

             output->planes = eina_list_append(output->planes, plane);
          }
     }

   drmModeFreePlaneResources(res);

   return EINA_TRUE;
}

EAPI void
ecore_drm2_planes_destroy(Ecore_Drm2_Launcher *launcher, int fd)
{
   Ecore_Drm2_Plane *plane;
   Ecore_Drm2_Output *output;
   Eina_List *l;

   EINA_SAFETY_ON_NULL_RETURN(launcher);

   EINA_LIST_FOREACH(launcher->outputs, l, output)
     {
        EINA_LIST_FREE(output->planes, plane)
          {
             drmModeSetPlane(fd, plane->id, output->crtc_id, 0, 0,
                             0, 0, 0, 0, 0, 0, 0, 0);
             free(plane);
          }
     }
}

static void
_plane_coord_transform(int w, int h, Ecore_Drm2_Output_Transform transform, int scale, double x, double y, double *rx, double *ry)
{
   *rx = x;
   *ry = y;
   switch (transform)
     {
      case ECORE_DRM2_OUTPUT_TRANSFORM_FLIPPED:
        *rx = w - x;
        *ry = y;
        break;
      case ECORE_DRM2_OUTPUT_TRANSFORM_90:
        *rx = h - y;
        *ry = y;
        break;
      case ECORE_DRM2_OUTPUT_TRANSFORM_FLIPPED_90:
        *rx = h - y;
        *ry = w - x;
        break;
      case ECORE_DRM2_OUTPUT_TRANSFORM_180:
        *rx = w - x;
        *ry = h - y;
        break;
      case ECORE_DRM2_OUTPUT_TRANSFORM_FLIPPED_180:
        *rx = x;
        *ry = h - y;
        break;
      case ECORE_DRM2_OUTPUT_TRANSFORM_270:
        *rx = y;
        *ry = w - x;
        break;
      case ECORE_DRM2_OUTPUT_TRANSFORM_FLIPPED_270:
        *rx = y;
        *ry = x;
        break;
      case ECORE_DRM2_OUTPUT_TRANSFORM_NORMAL:
      default:
        break;
     }

   *rx *= scale;
   *ry *= scale;
}

/* TODO: these functions should scale the fb and set
 * plane src and dest (sx, sy, dx, dy, etc, etc) */
static void
_plane_overlay_prepare(Ecore_Drm2_Plane *plane EINA_UNUSED, Ecore_Drm2_Output *output, Ecore_Drm2_Fb *fb EINA_UNUSED)
{
   Eina_Rectangle *rect;
   double x1, y1, x2, y2;

   rect = eina_rectangle_new(-output->x, -output->y, output->w, output->h);

   _plane_coord_transform(output->w, output->h,
                          output->transform, output->scale,
                          rect->x, rect->y, &x1, &y1);
   _plane_coord_transform(output->w, output->h,
                          output->transform, output->scale,
                          rect->w, rect->h, &x2, &y2);

   if (x1 <= x2)
     {
        rect->x = x1;
        rect->w = x2;
     }
   else
     {
        rect->x = x2;
        rect->w = x1;
     }

   if (y1 <= y2)
     {
        rect->y = y1;
        rect->h = y2;
     }
   else
     {
        rect->y = y2;
        rect->h = y1;
     }

   DBG("Overlay Dest: %d %d %d %d", rect->x, rect->y, rect->w, rect->h);
   DBG("\tSecondary: %d %d %d %d", rect->x, rect->y,
       rect->w - rect->x, rect->h - rect->y);
}

static void
_plane_cursor_prepare(Ecore_Drm2_Plane *plane, Ecore_Drm2_Output *output EINA_UNUSED, Ecore_Drm2_Fb *fb)
{
   plane->dw = fb->w;
   plane->dh = fb->h;
}

EAPI Ecore_Drm2_Plane *
ecore_drm2_plane_find(Ecore_Drm2_Output *output, Ecore_Drm2_Fb *fb, unsigned int format)
{
   Ecore_Drm2_Plane *plane;
   Eina_List *l;
   unsigned int i = 0;

   EINA_SAFETY_ON_NULL_RETURN_VAL(output, NULL);

   EINA_LIST_FOREACH(output->planes, l, plane)
     {
        for (i = 0; i < plane->num_formats; i++)
          {
             if (plane->formats[i] != format) continue;

             if (plane->type == DRM_PLANE_TYPE_CURSOR)
               {
                  if ((fb->w > plane->cw) || (fb->h > plane->ch))
                    continue;
               }
             else if (plane->type == DRM_PLANE_TYPE_PRIMARY)
               {
                  if ((fb->w != output->current_mode->width) ||
                      (fb->h != output->current_mode->height))
                    continue;
               }

             plane->output = output;

             return plane;
          }
     }

   return NULL;
}

EAPI Eina_Bool
ecore_drm2_plane_fb_set(Ecore_Drm2_Plane *plane, Ecore_Drm2_Fb *fb)
{
   Ecore_Drm2_Output *output;
   int ret;

   EINA_SAFETY_ON_NULL_RETURN_VAL(plane, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(plane->output, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(fb, EINA_FALSE);

   output = plane->output;

   plane->sx = 0;
   plane->sy = 0;
   plane->sw = (fb->w << 16);
   plane->sh = (fb->h << 16);

   plane->dx = output->x;
   plane->dy = output->y;
   plane->dw = output->current_mode->width;
   plane->dh = output->current_mode->height;

   switch (plane->type)
     {
      case DRM_PLANE_TYPE_OVERLAY:
        _plane_overlay_prepare(plane, output, fb);
        break;
      case DRM_PLANE_TYPE_CURSOR:
        _plane_cursor_prepare(plane, output, fb);
        break;
      case DRM_PLANE_TYPE_PRIMARY: // direct scanout
      default:
        break;
     }

   /* DBG("Set Plane %d On Output %d", plane->id, output->crtc_id); */
   /* DBG("\tFB: %d %dx%d", fb->id, fb->w, fb->h); */
   /* DBG("\tDest: %d %d %d %d", plane->dx, plane->dy, plane->dw, plane->dh); */
   /* DBG("\tSrc: %d %d %d %d", plane->sx, plane->sy, plane->sw, plane->sh); */

   ret = drmModeSetPlane(fb->fd, plane->id, output->crtc_id, fb->id, 0,
                         plane->dx, plane->dy, plane->dw, plane->dh,
                         plane->sx, plane->sy, plane->sw, plane->sh);
   if (ret)
     {
        ERR("Could not set fb on plane: %m");
        return EINA_FALSE;
     }

   return EINA_TRUE;
}
