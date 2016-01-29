#include "ecore_drm2_private.h"

static int _ecore_drm2_init_count = 0;

int _ecore_drm2_log_dom = -1;

/**
 * @defgroup Ecore_Drm2_Init_Group Drm library Init and Shutdown functions
 *
 * Functions that start and shutdown the Ecore_Drm2 library
 */

/**
 * Initialize the Ecore_Drm2 library
 *
 * @return  The number of times the library has been initialized without
 *          being shut down. 0 is returned if an error occurs.
 *
 * @ingroup Ecore_Drm2_Init_Group
 */
EAPI int
ecore_drm2_init(void)
{
   if (++_ecore_drm2_init_count != 1) return _ecore_drm2_init_count;

   if (!eina_init()) goto eina_err;

   if (!ecore_init())
     {
        EINA_LOG_ERR("Could not initialize Ecore library");
        goto ecore_err;
     }

   if (!ecore_event_init())
     {
        EINA_LOG_ERR("Could not initialize Ecore_Event library");
        goto ecore_event_err;
     }

   if (!eeze_init())
     {
        EINA_LOG_ERR("Could not initialize Eeze library");
        goto eeze_err;
     }

   _ecore_drm2_log_dom =
     eina_log_domain_register("ecore_drm2", ECORE_DRM2_DEFAULT_LOG_COLOR);
   if (!_ecore_drm2_log_dom)
     {
        EINA_LOG_ERR("Could not create logging domain for Ecore_Drm2");
        goto log_err;
     }

   return _ecore_drm2_init_count;

log_err:
   eeze_shutdown();
eeze_err:
   ecore_event_shutdown();
ecore_event_err:
   ecore_shutdown();
ecore_err:
   eina_shutdown();
eina_err:
   return --_ecore_drm2_init_count;
}
