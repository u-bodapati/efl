#include "ecore_drm2_private.h"
#include <ctype.h>

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

   for (; j < conn->count_encoders; j++)
     {
        enc = drmModeGetEncoder(fd, conn->encoders[j]);
        if (!enc)
          {
             ERR("Failed to get encoder");
             return -1;
          }

        pcrtcs = enc->possible_crtcs;
        drmModeFreeEncoder(enc);

        for (i = 0; i < res->count_crtcs; i++)
          {
             if ((pcrtcs & (1 << i)) &&
                 !(alloc & (1 << res->crtcs[i])))
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

static int
_connector_current_mode_get(drmModeConnector *conn, int fd, drmModeModeInfo *mode)
{
   drmModeEncoder *enc;
   drmModeCrtc *crtc;

   memset(mode, 0, sizeof(*mode));
   enc = drmModeGetEncoder(fd, conn->encoder_id);
   if (enc)
     {
        crtc = drmModeGetCrtc(fd, enc->crtc_id);
        drmModeFreeEncoder(enc);
        if (!crtc) return -1;
        if (crtc->mode_valid)
          {
             if (mode) *mode = crtc->mode;
          }
        drmModeFreeCrtc(crtc);
     }

   return 0;
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

static Ecore_Drm2_Output_Mode *
_output_start_mode_get(Ecore_Drm2_Output *output, const drmModeModeInfo *cmode)
{
   Eina_List *l;
   Ecore_Drm2_Output_Mode *current, *omode, *pmode, *best;

   EINA_LIST_REVERSE_FOREACH(output->modes, l, omode)
     {
        if (!memcmp(&cmode, &omode->info, sizeof(cmode)))
          current = omode;

        if (omode->flags & DRM_MODE_TYPE_PREFERRED)
          pmode = omode;

        best = omode;
     }

   if ((!current) && (cmode->clock != 0))
     {
        current = _output_mode_add(output, cmode);
        if (!current) return NULL;
     }

   if (current) return current;
   else if (pmode) return pmode;
   else if (best) return best;
   return NULL;
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
        /* if (output->edid.serial[0] != '\0') */
        /*   eina_stringshare_replace(&output->serial, output->edid.serial); */
     }

   drmModeFreePropertyBlob(blob);
}

static Eina_Bool
_output_create(Ecore_Drm2_Launcher *launcher, const drmModeRes *res, drmModeConnector *conn, int fd, int x, int y, int *w)
{
   Ecore_Drm2_Output *output;
   Ecore_Drm2_Output_Mode *omode, *cmode;
   drmModeModeInfo current_mode;
   int i = 0;

   if (w) *w = 0;

   i = _output_crtc_find(res, conn, fd, launcher->crtc_allocator);
   if (i < 0)
     {
        ERR("Could not find usable crtc/encoder pair");
        return EINA_FALSE;
     }

   output = calloc(1, sizeof(Ecore_Drm2_Output));
   if (!output) return EINA_FALSE;

   output->name = eina_stringshare_add(_output_name_get(conn));
   output->make = eina_stringshare_add("unknown");
   output->model = eina_stringshare_add("unknown");
   output->serial = eina_stringshare_add("unknown");
   output->subpixel = _output_subpixel_get(conn->subpixel);

   /* TODO: fix me */
   output->format = launcher->format;

   output->crtc_id = res->crtcs[i];
   output->pipe = i;
   launcher->crtc_allocator |= (1 << output->crtc_id);
   output->conn_id = conn->connector_id;
   launcher->conn_allocator |= (1 << output->conn_id);

   output->ocrtc = drmModeGetCrtc(fd, output->crtc_id);
   output->dpms_prop = _output_property_get(conn, "DPMS", fd);

   if (_connector_current_mode_get(conn, fd, &current_mode) < 0)
     goto err;

   for (; i < conn->count_modes; i++)
     {
        omode = _output_mode_add(output, &conn->modes[i]);
        if (!omode) goto err;
     }

   /* TODO: ?? disable output if config is off ?? */

   cmode = _output_start_mode_get(output, &current_mode);
   if (!cmode)
     {
        ERR("Could not get initial mode for output");
        goto curr_err;
     }

   output->current_mode = cmode;
   output->current_mode->flags |= DRM_MODE_TYPE_DEFAULT;

   output->x = x;
   output->y = y;
   output->phys_width = conn->mmWidth;
   output->phys_height = conn->mmHeight;

   if (conn->connection == DRM_MODE_CONNECTED)
     output->connected = EINA_TRUE;

   /* TODO: backlight */

   _output_edid_find(output, conn, fd);

   /* TODO: gamma */

   /* TODO: plane init */

   DBG("Created Output: %s", output->name);
   DBG("\tGeom: %d %d %d %d", output->x, output->y,
       output->current_mode->width, output->current_mode->height);

   launcher->outputs = eina_list_append(launcher->outputs, output);

   if (w) *w = output->current_mode->width;

   return EINA_TRUE;

curr_err:
   EINA_LIST_FREE(output->modes, omode)
     free(omode);
err:
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

   for (; i < res->count_connectors; i++)
     {
        conn = drmModeGetConnector(fd, res->connectors[i]);
        if (!conn) continue;

        /* if (conn->connection == DRM_MODE_CONNECTED) */
          {
             if (!_output_create(launcher, res, conn, fd, x, y, &w))
               {
                  drmModeFreeConnector(conn);
                  continue;
               }
             x += w;
          }

        drmModeFreeConnector(conn);
     }

   drmModeFreeResources(res);

   return EINA_TRUE;

err:
   drmModeFreeResources(res);
   return EINA_FALSE;
}
