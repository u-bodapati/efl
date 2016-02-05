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
   Ecore_Drm2_Plane *plane;
   drmModePlaneRes *res;
   uint32_t i;

   EINA_SAFETY_ON_NULL_RETURN_VAL(launcher, EINA_FALSE);
   EINA_SAFETY_ON_TRUE_RETURN_VAL((fd < 0), EINA_FALSE);

   drmSetClientCap(fd, DRM_CLIENT_CAP_UNIVERSAL_PLANES, 1);

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

        props =
          drmModeObjectGetProperties(fd, plane->id, DRM_MODE_OBJECT_PLANE);
        if (props)
          {
             uint32_t j;
             int type = -1;

             for (j = 0; type == -1 && j < props->count_props; j++)
               {
                  drmModePropertyPtr prop;

                  prop = drmModeGetProperty(fd, props->props[j]);
                  if (!prop) continue;

                  if (!strcmp(prop->name, "type"))
                    plane->type = props->prop_values[j];

                  /* TODO: supported rotations */

                  drmModeFreeProperty(prop);
               }
          }

        drmModeFreePlane(dplane);

        launcher->planes = eina_list_append(launcher->planes, plane);
     }

   drmModeFreePlaneResources(res);

   return EINA_TRUE;
}

EAPI void
ecore_drm2_planes_destroy(Ecore_Drm2_Launcher *launcher, int fd)
{
   Ecore_Drm2_Plane *plane;

   EINA_SAFETY_ON_NULL_RETURN(launcher);

   EINA_LIST_FREE(launcher->planes, plane)
     {
        Ecore_Drm2_Output *output;

        output = (Ecore_Drm2_Output *)plane->output;
        if (output)
          drmModeSetPlane(fd, plane->id, output->crtc_id, 0, 0,
                          0, 0, 0, 0, 0, 0, 0, 0);

        free(plane);
     }
}

EAPI Ecore_Drm2_Plane *
ecore_drm2_plane_find(Ecore_Drm2_Launcher *launcher, Ecore_Drm2_Output *output, int type)
{
   Ecore_Drm2_Plane *plane;
   Eina_List *l;

   EINA_SAFETY_ON_NULL_RETURN_VAL(launcher, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(output, NULL);

   EINA_LIST_FOREACH(launcher->planes, l, plane)
     {
        if (plane->type != type) continue;

        if (!_plane_supported(launcher, output, plane->crtcs))
          continue;

        plane->output = output;
        return plane;
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

   ret = drmModeSetPlane(fb->fd, plane->id, output->crtc_id, fb->id, 0,
                         output->x, output->y, fb->w, fb->h, // dest
                         0, 0, (fb->w << 16), (fb->h << 16)); // src
   if (ret)
     {
        ERR("Could not set fb on plane: %m");
        return EINA_FALSE;
     }

   return EINA_TRUE;
}
