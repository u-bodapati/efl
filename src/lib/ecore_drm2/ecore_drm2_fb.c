#include "ecore_drm2_private.h"
#include <sys/mman.h>

static Eina_Bool
_fb2_create(Ecore_Drm2_Fb *fb)
{
   struct drm_mode_fb_cmd2 cmd;
   uint32_t hdls[4], pitches[4], offsets[4];

   hdls[0] = fb->hdl;
   pitches[0] = fb->stride;
   offsets[0] = 0;

   memset(&cmd, 0, sizeof(struct drm_mode_fb_cmd2));
   cmd.fb_id = 0;
   cmd.width = fb->w;
   cmd.height = fb->h;
   cmd.pixel_format = fb->format;
   cmd.flags = 0;
   memcpy(cmd.handles, hdls, 4 * sizeof(hdls[0]));
   memcpy(cmd.pitches, pitches, 4 * sizeof(pitches[0]));
   memcpy(cmd.offsets, offsets, 4 * sizeof(offsets[0]));

   if (drmIoctl(fb->fd, DRM_IOCTL_MODE_ADDFB2, &cmd))
     return EINA_FALSE;

   fb->id = cmd.fb_id;

   return EINA_TRUE;
}

EAPI Ecore_Drm2_Fb *
ecore_drm2_fb_create(int fd, int width, int height, int depth, int bpp, unsigned int format)
{
   Ecore_Drm2_Fb *fb;
   struct drm_mode_create_dumb carg;
   struct drm_mode_destroy_dumb darg;
   struct drm_mode_map_dumb marg;
   int ret;

   EINA_SAFETY_ON_TRUE_RETURN_VAL((fd < 0), NULL);

   fb = calloc(1, sizeof(Ecore_Drm2_Fb));
   if (!fb) return NULL;

   fb->fd = fd;
   fb->w = width;
   fb->h = height;
   fb->bpp = bpp;
   fb->depth = depth;
   fb->format = format;

   memset(&carg, 0, sizeof(struct drm_mode_create_dumb));
   carg.bpp = bpp;
   carg.width = width;
   carg.height = height;

   ret = drmIoctl(fd, DRM_IOCTL_MODE_CREATE_DUMB, &carg);
   if (ret) goto err;

   fb->hdl = carg.handle;
   fb->size = carg.size;
   fb->stride = carg.pitch;

   if (!_fb2_create(fb))
     {
        ret =
          drmModeAddFB(fd, width, height, depth, bpp, fb->stride, fb->hdl, &fb->id);
        if (ret)
          {
             ERR("Could not add framebuffer: %m");
             goto add_err;
          }
     }

   memset(&marg, 0, sizeof(struct drm_mode_map_dumb));
   marg.handle = fb->hdl;
   ret = drmIoctl(fd, DRM_IOCTL_MODE_MAP_DUMB, &marg);
   if (ret)
     {
        ERR("Could not map framebuffer: %m");
        goto map_err;
     }

   fb->mmap = mmap(NULL, fb->size, PROT_WRITE, MAP_SHARED, fd, marg.offset);
   if (fb->mmap == MAP_FAILED)
     {
        ERR("Could not mmap framebuffer memory: %m");
        goto map_err;
     }

   return fb;

map_err:
   drmModeRmFB(fd, fb->id);
add_err:
   memset(&darg, 0, sizeof(struct drm_mode_destroy_dumb));
   darg.handle = fb->hdl;
   drmIoctl(fd, DRM_IOCTL_MODE_DESTROY_DUMB, &darg);
err:
   free(fb);
   return NULL;
}

EAPI void
ecore_drm2_fb_destroy(Ecore_Drm2_Fb *fb)
{
   struct drm_mode_destroy_dumb darg;

   EINA_SAFETY_ON_NULL_RETURN(fb);
   EINA_SAFETY_ON_NULL_RETURN(fb->mmap);

   if (fb->id) drmModeRmFB(fb->fd, fb->id);
   munmap(fb->mmap, fb->size);

   memset(&darg, 0, sizeof(struct drm_mode_destroy_dumb));
   darg.handle = fb->hdl;
   drmIoctl(fb->fd, DRM_IOCTL_MODE_DESTROY_DUMB, &darg);
   free(fb);
}

EAPI void *
ecore_drm2_fb_data_get(Ecore_Drm2_Fb *fb)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(fb, NULL);

   return fb->mmap;
}

EAPI unsigned int
ecore_drm2_fb_size_get(Ecore_Drm2_Fb *fb)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(fb, 0);

   return fb->size;
}

EAPI unsigned int
ecore_drm2_fb_stride_get(Ecore_Drm2_Fb *fb)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(fb, 0);

   return fb->stride;
}
