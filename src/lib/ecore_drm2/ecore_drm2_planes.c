#include "ecore_drm2_private.h"

EAPI Eina_Bool
ecore_drm2_planes_create(Ecore_Drm2_Launcher *launcher, int fd)
{
   Ecore_Drm2_Plane *plane;
   drmModePlaneRes *res;
   drmModePlane *dplane;
   uint32_t i;

   EINA_SAFETY_ON_NULL_RETURN_VAL(launcher, EINA_FALSE);
   EINA_SAFETY_ON_TRUE_RETURN_VAL((fd < 0), EINA_FALSE);

   res = drmModeGetPlaneResources(fd);
   if (!res)
     {
        ERR("Failed to get plane resources: %s\n", strerror(errno));
        return EINA_FALSE;
     }

   for (i = 0; i < res->count_planes; i++)
     {
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

        /* TODO: release FBs */

        free(plane);
     }
}
