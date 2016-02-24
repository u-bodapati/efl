#include "ecore_drm2_private.h"
#include <ctype.h>

#define INSIDE(x, y, xx, yy, ww, hh) \
   (((x) < ((xx) + (ww))) && ((y) < ((yy) + (hh))) && \
       ((x) >= (xx)) && ((y) >= (yy)))

#define EDID_DESCRIPTOR_ALPHANUMERIC_DATA_STRING 0xfe
#define EDID_DESCRIPTOR_DISPLAY_PRODUCT_NAME 0xfc
#define EDID_DESCRIPTOR_DISPLAY_PRODUCT_SERIAL_NUMBER 0xff
#define EDID_OFFSET_DATA_BLOCKS 0x36
#define EDID_OFFSET_LAST_BLOCK 0x6c
#define EDID_OFFSET_PNPID 0x08
#define EDID_OFFSET_SERIAL 0x0c

static const char *conn_types[] =
{
   "None", "VGA", "DVI-I", "DVI-D", "DVI-A",
   "Composite", "S-Video", "LVDS", "Component", "DIN",
   "DisplayPort", "HDMI-A", "HDMI-B", "TV", "eDP", "Virtual", "DSI",
};

static int
_output_crtc_find(const drmModeRes *res, const drmModeConnector *conn, int fd, int alloc)
{
   drmModeEncoder *enc;
   uint32_t pcrtcs;
   int i, j = 0;

   for (j = 0; j < conn->count_encoders; j++)
     {
        enc = drmModeGetEncoder(fd, conn->encoders[j]);
        if (!enc)
          {
             WRN("Failed to get encoder");
             continue;
          }

        pcrtcs = enc->possible_crtcs;
        drmModeFreeEncoder(enc);

        for (i = 0; i < res->count_crtcs; i++)
          {
             if ((pcrtcs & (1 << i)) &&
                 (!(alloc & (1 << res->crtcs[i]))))
               return i;
          }
     }

   return -1;
}

static int
_output_subpixel_get(int subpixel)
{
   switch (subpixel)
     {
      case DRM_MODE_SUBPIXEL_UNKNOWN:
        return 0; // WL_OUTPUT_SUBPIXEL_UNKNOWN;
      case DRM_MODE_SUBPIXEL_NONE:
        return 1; //WL_OUTPUT_SUBPIXEL_NONE;
      case DRM_MODE_SUBPIXEL_HORIZONTAL_RGB:
        return 2; //WL_OUTPUT_SUBPIXEL_HORIZONTAL_RGB;
      case DRM_MODE_SUBPIXEL_HORIZONTAL_BGR:
        return 3; // WL_OUTPUT_SUBPIXEL_HORIZONTAL_BGR;
      case DRM_MODE_SUBPIXEL_VERTICAL_RGB:
        return 4; // WL_OUTPUT_SUBPIXEL_VERTICAL_RGB;
      case DRM_MODE_SUBPIXEL_VERTICAL_BGR:
        return 5; //WL_OUTPUT_SUBPIXEL_VERTICAL_BGR;
      default:
        return 0; // WL_OUTPUT_SUBPIXEL_UNKNOWN;
     }
}

static char *
_output_name_get(const drmModeConnector *conn)
{
   char name[DRM_CONNECTOR_NAME_LEN];
   const char *type = NULL;

   if (conn->connector_type < EINA_C_ARRAY_LENGTH(conn_types))
     type = conn_types[conn->connector_type];
   else
     type = "UNKNOWN";

   snprintf(name, sizeof(name), "%s-%d", type, conn->connector_type_id);
   return strdup(name);
}

static drmModePropertyPtr
_output_property_get(drmModeConnectorPtr conn, const char *name, int fd)
{
   drmModePropertyPtr prop;
   int i = 0;

   for (; i < conn->count_props; i++)
     {
        prop = drmModeGetProperty(fd, conn->props[i]);
        if (!prop) continue;

        if (!strcmp(prop->name, name)) return prop;

        drmModeFreeProperty(prop);
     }

   return NULL;
}

static Ecore_Drm2_Output_Mode *
_output_mode_add(Ecore_Drm2_Output *output, const drmModeModeInfo *info)
{
   Ecore_Drm2_Output_Mode *mode;
   uint64_t refresh;

   mode = calloc(1, sizeof(Ecore_Drm2_Output_Mode));
   if (!mode) return NULL;

   mode->flags = 0;
   mode->width = info->hdisplay;
   mode->height = info->vdisplay;

   refresh = (info->clock * 1000000LL / info->htotal + info->vtotal / 2) /
     info->vtotal;

   if (info->flags & DRM_MODE_FLAG_INTERLACE)
     refresh *= 2;
   if (info->flags & DRM_MODE_FLAG_DBLSCAN)
     refresh /= 2;
   if (info->vscan > 1)
     refresh /= info->vscan;

   mode->refresh = refresh;
   mode->info = *info;

   if (info->type & DRM_MODE_TYPE_PREFERRED)
     mode->flags |= DRM_MODE_TYPE_PREFERRED;

   output->modes = eina_list_append(output->modes, mode);

   return mode;
}

static void 
_output_edid_parse_string(const uint8_t *data, char text[])
{
   int i = 0, rep = 0;

   strncpy(text, (const char *)data, 12);

   for (; text[i] != '\0'; i++)
     {
        if ((text[i] == '\n') || (text[i] == '\r'))
          {
             text[i] = '\0';
             break;
          }
     }

   for (i = 0; text[i] != '\0'; i++)
     {
        if (!isprint(text[i]))
          {
             text[i] = '-';
             rep++;
          }
     }

   if (rep > 4) text[0] = '\0';
}

static int 
_output_edid_parse(Ecore_Drm2_Output *output, const uint8_t *data, size_t len)
{
   int i = 0;
   uint32_t serial;

   if (len < 128) return -1;
   if ((data[0] != 0x00) || (data[1] != 0xff)) return -1;

   output->edid.pnp[0] = 'A' + ((data[EDID_OFFSET_PNPID + 0] & 0x7c) / 4) - 1;
   output->edid.pnp[1] = 
     'A' + ((data[EDID_OFFSET_PNPID + 0] & 0x3) * 8) + 
     ((data[EDID_OFFSET_PNPID + 1] & 0xe0) / 32) - 1;
   output->edid.pnp[2] = 'A' + (data[EDID_OFFSET_PNPID + 1] & 0x1f) - 1;
   output->edid.pnp[3] = '\0';

   serial = (uint32_t) data[EDID_OFFSET_SERIAL + 0];
   serial += (uint32_t) data[EDID_OFFSET_SERIAL + 1] * 0x100;
   serial += (uint32_t) data[EDID_OFFSET_SERIAL + 2] * 0x10000;
   serial += (uint32_t) data[EDID_OFFSET_SERIAL + 3] * 0x1000000;
   if (serial > 0)
     sprintf(output->edid.serial, "%lu", (unsigned long)serial);

   for (i = EDID_OFFSET_DATA_BLOCKS; i <= EDID_OFFSET_LAST_BLOCK; i += 18)
     {
        if (data[i] != 0) continue;
        if (data[i + 2] != 0) continue;

        if (data[i + 3] == EDID_DESCRIPTOR_DISPLAY_PRODUCT_NAME)
          _output_edid_parse_string(&data[i + 5], output->edid.monitor);
        else if (data[i + 3] == EDID_DESCRIPTOR_DISPLAY_PRODUCT_SERIAL_NUMBER)
          _output_edid_parse_string(&data[i + 5], output->edid.serial);
        else if (data[i + 3] == EDID_DESCRIPTOR_ALPHANUMERIC_DATA_STRING)
          _output_edid_parse_string(&data[i + 5], output->edid.eisa);
     }

   return 0;
}

static void 
_output_edid_find(Ecore_Drm2_Output *output, drmModeConnector *conn, int fd)
{
   drmModePropertyBlobPtr blob = NULL;
   drmModePropertyPtr prop;
   int i = 0, ret = 0;

   for (; i < conn->count_props && !blob; i++)
     {
        if (!(prop = drmModeGetProperty(fd, conn->props[i])))
          continue;
        if ((prop->flags & DRM_MODE_PROP_BLOB) && 
            (!strcmp(prop->name, "EDID")))
          {
             blob = drmModeGetPropertyBlob(fd, conn->prop_values[i]);
          }
        drmModeFreeProperty(prop);
        if (blob) break;
     }

   if (!blob) return;

   /* output->edid.blob = eina_memdup(blob->data, blob->length, 1); */

   ret = _output_edid_parse(output, blob->data, blob->length);
   if (!ret)
     {
        if (output->edid.pnp[0] != '\0')
          eina_stringshare_replace(&output->make, output->edid.pnp);
        if (output->edid.monitor[0] != '\0')
          eina_stringshare_replace(&output->model, output->edid.monitor);
        if (output->edid.serial[0] != '\0')
          eina_stringshare_replace(&output->serial, output->edid.serial);
     }

   drmModeFreePropertyBlob(blob);
}

static void
_output_scale_init(Ecore_Drm2_Output *output, Ecore_Drm2_Output_Transform transform, unsigned int scale)
{
   output->transform = transform;

   switch (transform)
     {
      case ECORE_DRM2_OUTPUT_TRANSFORM_90:
      case ECORE_DRM2_OUTPUT_TRANSFORM_270:
      case ECORE_DRM2_OUTPUT_TRANSFORM_FLIPPED_90:
      case ECORE_DRM2_OUTPUT_TRANSFORM_FLIPPED_270:
        output->w = output->current_mode->height;
        output->h = output->current_mode->width;
        break;
      case ECORE_DRM2_OUTPUT_TRANSFORM_NORMAL:
      case ECORE_DRM2_OUTPUT_TRANSFORM_180:
      case ECORE_DRM2_OUTPUT_TRANSFORM_FLIPPED:
      case ECORE_DRM2_OUTPUT_TRANSFORM_FLIPPED_180:
        output->w = output->current_mode->width;
        output->h = output->current_mode->height;
        break;
      default:
        break;
     }

   output->scale = scale;
   output->w /= scale;
   output->h /= scale;
}

static void
_output_matrix_rotate_xy(Eina_Matrix3 *matrix, double x, double y)
{
   Eina_Matrix4 tmp, m;

   eina_matrix4_identity(&tmp);
   eina_matrix4_values_set(&tmp, x, y, 0, 0, -y, x, 0, 0,
                           0, 0, 1, 0, 0, 0, 0, 1);

   eina_matrix3_matrix4_to(&m, matrix);
   eina_matrix4_multiply(&m, &m, &tmp);
   eina_matrix4_matrix3_to(matrix, &m);
}

static void
_output_matrix_update(Ecore_Drm2_Output *output)
{
   Eina_Matrix3 m3;

   eina_matrix4_identity(&output->matrix);
   eina_matrix4_matrix3_to(&m3, &output->matrix);
   eina_matrix3_translate(&m3, -output->x, -output->y);

   switch (output->transform)
     {
      case ECORE_DRM2_OUTPUT_TRANSFORM_FLIPPED:
      case ECORE_DRM2_OUTPUT_TRANSFORM_FLIPPED_90:
      case ECORE_DRM2_OUTPUT_TRANSFORM_FLIPPED_180:
      case ECORE_DRM2_OUTPUT_TRANSFORM_FLIPPED_270:
        eina_matrix3_translate(&m3, -output->w, 0);
        break;
      default:
        break;
     }

   switch (output->transform)
     {
      case ECORE_DRM2_OUTPUT_TRANSFORM_NORMAL:
      case ECORE_DRM2_OUTPUT_TRANSFORM_FLIPPED:
      default:
        break;
      case ECORE_DRM2_OUTPUT_TRANSFORM_90:
      case ECORE_DRM2_OUTPUT_TRANSFORM_FLIPPED_90:
        eina_matrix3_translate(&m3, 0, -output->h);
        _output_matrix_rotate_xy(&m3, 0, 1);
        break;
      case ECORE_DRM2_OUTPUT_TRANSFORM_180:
      case ECORE_DRM2_OUTPUT_TRANSFORM_FLIPPED_180:
        eina_matrix3_translate(&m3, -output->w, -output->h);
        _output_matrix_rotate_xy(&m3, -1, 0);
        break;
      case ECORE_DRM2_OUTPUT_TRANSFORM_270:
      case ECORE_DRM2_OUTPUT_TRANSFORM_FLIPPED_270:
        eina_matrix3_translate(&m3, -output->w, 0);
        _output_matrix_rotate_xy(&m3, 0, -1);
        break;
     }

   if (output->scale != 1)
     eina_matrix3_scale(&m3, output->scale, output->scale);

   eina_matrix3_matrix4_to(&output->matrix, &m3);
   eina_matrix4_inverse(&output->inverse, &output->matrix);
}

static Ecore_Drm2_Backlight *
_output_backlight_init(unsigned int conn_type)
{
   const char *dev, *t;
   Eina_List *devs, *l;
   Eina_Bool found = EINA_FALSE;
   Ecore_Drm2_Backlight *bl = NULL;
   Ecore_Drm2_Backlight_Type type = 0;

   devs = eeze_udev_find_by_filter("backlight", NULL, NULL);

   EINA_LIST_FOREACH(devs, l, dev)
     {
        t = eeze_udev_syspath_get_sysattr(dev, "type");
        if (!t) continue;

        if (!strcmp(t, "raw"))
          type = ECORE_DRM2_BACKLIGHT_RAW;
        else if (!strcmp(t, "platform"))
          type = ECORE_DRM2_BACKLIGHT_PLATFORM;
        else if (!strcmp(t, "firmware"))
          type = ECORE_DRM2_BACKLIGHT_FIRMWARE;

        if ((conn_type == DRM_MODE_CONNECTOR_LVDS) ||
            (conn_type == DRM_MODE_CONNECTOR_eDP) ||
            (type == ECORE_DRM2_BACKLIGHT_RAW))
          found = EINA_TRUE;

        eina_stringshare_del(t);
        if (found) break;
     }

   if (found)
     {
        bl = calloc(1, sizeof(Ecore_Drm2_Backlight));
        if (bl)
          {
             bl->type = type;
             bl->path = eina_stringshare_add(dev);
          }
     }

   EINA_LIST_FREE(devs, dev)
     eina_stringshare_del(dev);

   return bl;
}

static void
_output_backlight_shutdown(Ecore_Drm2_Backlight *bl)
{
   if (!bl) return;
   if (bl->path) eina_stringshare_del(bl->path);
   free(bl);
}

static Eina_Bool
_output_create(Ecore_Drm2_Launcher *launcher, const drmModeRes *res, drmModeConnector *conn, int fd, int x, int y, int *w)
{
   Eina_List *l;
   Ecore_Drm2_Output *output;
   Ecore_Drm2_Output_Mode *omode;
   Ecore_Drm2_Output_Mode *current = NULL, *preferred = NULL, *best = NULL;
   drmModeCrtc *crtc;
   drmModeEncoder *enc;
   drmModeModeInfo crtc_mode;
   int i = 0;

   if (w) *w = 0;

   i = _output_crtc_find(res, conn, fd, launcher->crtc_allocator);
   if (i < 0) return EINA_FALSE;

   output = calloc(1, sizeof(Ecore_Drm2_Output));
   if (!output) return EINA_FALSE;

   output->x = x;
   output->y = y;
   output->phys_width = conn->mmWidth;
   output->phys_height = conn->mmHeight;
   output->subpixel = _output_subpixel_get(conn->subpixel);

   output->name = eina_stringshare_add(_output_name_get(conn));
   output->make = eina_stringshare_add("unknown");
   output->model = eina_stringshare_add("unknown");
   output->serial = eina_stringshare_add("unknown");

   output->connected = (conn->connection == DRM_MODE_CONNECTED);

   output->pipe = i;
   /* output->crtc_index = i; */
   output->crtc_id = res->crtcs[i];
   output->conn_id = conn->connector_id;
   launcher->crtc_allocator |= (1 << output->crtc_id);
   launcher->conn_allocator |= (1 << output->conn_id);

   output->ocrtc = drmModeGetCrtc(fd, output->crtc_id);
   output->dpms_prop = _output_property_get(conn, "DPMS", fd);

   memset(&crtc_mode, 0, sizeof(crtc_mode));
   enc = drmModeGetEncoder(fd, conn->encoder_id);
   if (enc)
     {
        crtc = drmModeGetCrtc(fd, enc->crtc_id);
        drmModeFreeEncoder(enc);
        if (!crtc) goto err;
        if (crtc->mode_valid) crtc_mode = crtc->mode;
        drmModeFreeCrtc(crtc);
     }

   for (i = 0; i < conn->count_modes; i++)
     {
        omode = _output_mode_add(output, &conn->modes[i]);
        if (!omode)
          {
             ERR("Could not add mode to output %s", output->name);
             goto err;
          }
     }

   EINA_LIST_REVERSE_FOREACH(output->modes, l, omode)
     {
        if (!memcmp(&crtc_mode, &omode->info, sizeof(crtc_mode)))
          current = omode;
        if (omode->flags & DRM_MODE_TYPE_PREFERRED)
          preferred = omode;
        best = omode;
     }

   if ((!current) && (crtc_mode.clock != 0))
     {
        current = _output_mode_add(output, &crtc_mode);
        if (!current) goto err;
     }

   if (current) output->current_mode = current;
   else if (preferred) output->current_mode = preferred;
   else if (best) output->current_mode = best;

   if (!output->current_mode) goto err;

   output->current_mode->flags |= DRM_MODE_TYPE_DEFAULT;

   /* TODO: ?? disable output if config is off ?? */

   output->backlight = _output_backlight_init(conn->connector_type);

   _output_edid_find(output, conn, fd);

   output->gamma = output->ocrtc->gamma_size;

   _output_scale_init(output, ECORE_DRM2_OUTPUT_TRANSFORM_NORMAL, 1);
   _output_matrix_update(output);

   DBG("Created New Output At %d,%d", output->x, output->y);
   DBG("\tCrtc Pos: %d %d", output->ocrtc->x, output->ocrtc->y);
   DBG("\tCrtc: %d", output->crtc_id);
   DBG("\tConn: %d", output->conn_id);
   DBG("\tMake: %s", output->make);
   DBG("\tModel: %s", output->model);
   DBG("\tName: %s", output->name);
   DBG("\tSerial: %s", output->serial);
   /* DBG("\tCloned: %d", output->cloned); */
   /* DBG("\tPrimary: %d", output->primary); */
   if (output->backlight)
     {
        DBG("\tBacklight");
        switch (output->backlight->type)
          {
           case ECORE_DRM2_BACKLIGHT_RAW:
             DBG("\t\tType: Raw");
             break;
           case ECORE_DRM2_BACKLIGHT_PLATFORM:
             DBG("\t\tType: Platform");
             break;
           case ECORE_DRM2_BACKLIGHT_FIRMWARE:
             DBG("\t\tType: Firmware");
             break;
          }
        DBG("\t\tPath: %s", output->backlight->path);
     }

   EINA_LIST_FOREACH(output->modes, l, omode)
     {
        DBG("\tAdded Mode: %dx%d@%.1f%s%s%s",
            omode->width, omode->height, (omode->refresh / 1000.0),
            (omode->flags & DRM_MODE_TYPE_PREFERRED) ? ", preferred" : "",
            (omode->flags & DRM_MODE_TYPE_DEFAULT) ? ", current" : "",
            (conn->count_modes == 0) ? ", built-in" : "");
     }

   launcher->outputs = eina_list_append(launcher->outputs, output);

   if (w) *w = output->current_mode->width;

   return EINA_TRUE;

err:
   EINA_LIST_FREE(output->modes, omode)
     free(omode);
   eina_stringshare_del(output->name);
   eina_stringshare_del(output->make);
   eina_stringshare_del(output->model);
   eina_stringshare_del(output->serial);
   drmModeFreeCrtc(output->ocrtc);
   launcher->crtc_allocator &= ~(1 << output->crtc_id);
   launcher->conn_allocator &= ~(1 << output->conn_id);
   free(output);

   return EINA_FALSE;
}

static void
_output_destroy(Ecore_Drm2_Launcher *launcher, Ecore_Drm2_Output *output, int fd)
{
   drmModeCrtcPtr ocrtc;

   ocrtc = output->ocrtc;

   _output_backlight_shutdown(output->backlight);

   drmModeFreeProperty(output->dpms_prop);

   drmModeSetCursor(fd, output->crtc_id, 0, 0, 0);

   drmModeSetCrtc(fd, ocrtc->crtc_id, ocrtc->buffer_id,
                  ocrtc->x, ocrtc->y, &output->conn_id, 1, &ocrtc->mode);
   drmModeFreeCrtc(ocrtc);

   launcher->crtc_allocator &= ~(1 << output->crtc_id);
   launcher->conn_allocator &= ~(1 << output->conn_id);

   eina_stringshare_del(output->name);
   eina_stringshare_del(output->make);
   eina_stringshare_del(output->model);
   eina_stringshare_del(output->serial);

   free(output);
}

void
_ecore_drm2_output_coordinate_transform(Ecore_Drm2_Output *output, int dx, int dy, int *x, int *y)
{
   int i, j;
   float p[4], s[4];
   double m[16];

   s[0] = dx;
   s[1] = dy;
   s[2] = 0.0;
   s[3] = 1.0;

   eina_matrix4_values_get(&output->inverse,
                           &m[0], &m[1], &m[2], &m[3],
                           &m[4], &m[5], &m[6], &m[7],
                           &m[8], &m[9], &m[10], &m[11],
                           &m[12], &m[13], &m[14], &m[15]);

   for (i = 0; i < 4; i++)
     {
        p[i] = 0;
        for (j = 0; j < 4; j++)
          p[i] += s[j] * m[i + j * 4];
     }

   if (x) *x = p[0] / p[3];
   if (y) *y = p[1] / p[3];
}

EAPI Eina_Bool
ecore_drm2_outputs_create(Ecore_Drm2_Launcher *launcher, int fd)
{
   drmModeConnector *conn;
   drmModeRes *res;
   int i = 0, x = 0, y = 0, w = 0;

   EINA_SAFETY_ON_NULL_RETURN_VAL(launcher, EINA_FALSE);
   EINA_SAFETY_ON_TRUE_RETURN_VAL((fd < 0), EINA_FALSE);

   res = drmModeGetResources(fd);
   if (!res)
     {
        ERR("Could not get drm resources");
        return EINA_FALSE;
     }

   launcher->crtcs = calloc(res->count_crtcs, sizeof(uint32_t));
   if (!launcher->crtcs)
     {
        ERR("Failed to allocate space for crtcs");
        goto err;
     }

   launcher->min.width = res->min_width;
   launcher->min.height = res->min_height;
   launcher->max.width = res->max_width;
   launcher->max.height = res->max_height;

   launcher->num_crtcs = res->count_crtcs;
   memcpy(launcher->crtcs, res->crtcs, sizeof(uint32_t) * res->count_crtcs);

   for (i = 0; i < res->count_connectors; i++)
     {
        conn = drmModeGetConnector(fd, res->connectors[i]);
        if (!conn) continue;

        if (conn->connection == DRM_MODE_CONNECTED)
          {
             if (!_output_create(launcher, res, conn, fd, x, y, &w))
               goto next;

             x += w;
          }
next:
        drmModeFreeConnector(conn);
     }

   if (eina_list_count(launcher->outputs) < 1) goto err;

   drmModeFreeResources(res);
   return EINA_TRUE;

err:
   drmModeFreeResources(res);
   return EINA_FALSE;
}

EAPI void
ecore_drm2_outputs_destroy(Ecore_Drm2_Launcher *launcher, int fd)
{
   Ecore_Drm2_Output *output;

   EINA_SAFETY_ON_NULL_RETURN(launcher);

   EINA_LIST_FREE(launcher->outputs, output)
     _output_destroy(launcher, output, fd);
}

EAPI Ecore_Drm2_Output *
ecore_drm2_output_find(Ecore_Drm2_Launcher *launcher, int x, int y)
{
   Ecore_Drm2_Output *output;
   Eina_List *l;

   EINA_SAFETY_ON_NULL_RETURN_VAL(launcher, NULL);

   EINA_LIST_FOREACH(launcher->outputs, l, output)
     {
        int ox, oy, ow, oh;

        ox = output->x;
        oy = output->y;
        ow = output->current_mode->width;
        oh = output->current_mode->height;

        if (INSIDE(x, y, ox, oy, ow, oh))
          return output;
     }

   return NULL;
}

EAPI unsigned int
ecore_drm2_output_vblank_get(Ecore_Drm2_Output *output)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(output, 0);

   if (output->pipe > 1)
     {
        return (output->pipe << DRM_VBLANK_HIGH_CRTC_SHIFT) &
          DRM_VBLANK_HIGH_CRTC_MASK;
     }
   else if (output->pipe > 0)
     return DRM_VBLANK_SECONDARY;

   return 0;
}

EAPI int
ecore_drm2_output_crtc_id_get(Ecore_Drm2_Output *output)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(output, 0);
   return output->crtc_id;
}
