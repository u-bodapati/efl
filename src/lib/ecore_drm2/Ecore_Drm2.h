#ifndef _ECORE_DRM2_H
# define _ECORE_DRM2_H

# include <Ecore.h>
# include <Elput.h>

# ifdef EAPI
#  undef EAPI
# endif

# ifdef _MSC_VER
#  ifdef BUILDING_DLL
#   define EAPI __declspec(dllexport)
#  else // ifdef BUILDING_DLL
#   define EAPI __declspec(dllimport)
#  endif // ifdef BUILDING_DLL
# else // ifdef _MSC_VER
#  ifdef __GNUC__
#   if __GNUC__ >= 4
#    define EAPI __attribute__ ((visibility("default")))
#   else // if __GNUC__ >= 4
#    define EAPI
#   endif // if __GNUC__ >= 4
#  else // ifdef __GNUC__
#   define EAPI
#  endif // ifdef __GNUC__
# endif // ifdef _MSC_VER

# ifdef EFL_BETA_API_SUPPORT

/* opaque structure to represent a drm device */
typedef struct _Ecore_Drm2_Device Ecore_Drm2_Device;

/* opaque structure to represent a framebuffer object */
typedef struct _Ecore_Drm2_Fb Ecore_Drm2_Fb;

/* opaque structure to represent an output device */
typedef struct _Ecore_Drm2_Output Ecore_Drm2_Output;

/* opaque structure to represent an output mode */
typedef struct _Ecore_Drm2_Output_Mode Ecore_Drm2_Output_Mode;

/* structure to represent event for output changes */
typedef struct _Ecore_Drm2_Event_Output_Changed
{
   unsigned int id;
   int x, y, w, h;
   int phys_width, phys_height;
   unsigned int refresh, scale;
   int subpixel, transform;
   const char *make, *model, *name;
   Eina_Bool connected : 1;
   Eina_Bool enabled : 1;
} Ecore_Drm2_Event_Output_Changed;

EAPI extern int ECORE_DRM2_EVENT_OUTPUT_CHANGED;

/**
 * @file
 * @brief Ecore functions for dealing with drm, virtual terminals
 *
 * @defgroup Ecore_Drm2_Group Ecore_Drm2 - Drm Integration
 * @ingroup Ecore
 *
 * Ecore_Drm2 provides a wrapper and functions for using libdrm
 *
 * @li @ref Ecore_Drm2_Init_Group
 * @li @ref Ecore_Drm2_Device_Group
 * @li @ref Ecore_Drm2_Output_Group
 */

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
 * @since 1.18
 */
EAPI int ecore_drm2_init(void);

/**
 * Shutdown the Ecore_Drm2 library
 *
 * @return  The number of times the library has been initialized without
 *          being shutdown. 0 is returned if an error occurs.
 *
 * @ingroup Ecore_Drm2_Init_Group
 * @since 1.18
 */
EAPI int ecore_drm2_shutdown(void);

/**
 * @defgroup Ecore_Drm2_Device_Group Drm device functions
 *
 * Functions that deal with finding, opening, closing, or obtaining various
 * information about a drm device
 */

/**
 * Try to find a drm device on a given seat
 *
 * @param seat
 * @param tty
 * @param sync
 *
 * @return A newly allocated Ecore_Drm2_Device on success, NULL otherwise
 *
 * @ingroup Ecore_Drm2_Device_Group
 * @since 1.18
 */
EAPI Ecore_Drm2_Device *ecore_drm2_device_find(const char *seat, unsigned int tty, Eina_Bool sync);

/**
 * Try to open a given Ecore_Drm2_Device
 *
 * @param device
 *
 * @return A valid file descriptor if open succeeds, -1 otherwise.
 *
 * @ingroup Ecore_Drm2_Device_Group
 * @since 1.18
 */
EAPI int ecore_drm2_device_open(Ecore_Drm2_Device *device);

/**
 * Close an open Ecore_Drm2_Device
 *
 * @param device
 *
 * @ingroup Ecore_Drm2_Device_Group
 * @since 1.18
 */
EAPI void ecore_drm2_device_close(Ecore_Drm2_Device *device);

/**
 * Free a given Ecore_Drm2_Device
 *
 * @param device
 *
 * @ingroup Ecore_Drm2_Device_Group
 * @since 1.18
 */
EAPI void ecore_drm2_device_free(Ecore_Drm2_Device *device);

/**
 * Get the type of clock used by a given Ecore_Drm2_Device
 *
 * @param device
 *
 * @return The clockid_t used by this drm device
 *
 * @ingroup Ecore_Drm2_Device_Group
 * @since 1.18
 */
EAPI int ecore_drm2_device_clock_id_get(Ecore_Drm2_Device *device);

/**
 * Get the size of the cursor supported by a given Ecore_Drm2_Device
 *
 * @param device
 * @param width
 * @param height
 *
 * @ingroup Ecore_Drm2_Device_Group
 * @since 1.18
 */
EAPI void ecore_drm2_device_cursor_size_get(Ecore_Drm2_Device *device, int *width, int *height);

/**
 * @defgroup Ecore_Drm2_Output_Group Drm output functions
 *
 * Functions that deal with setup of outputs
 */

/**
 * Iterate drm resources and create outputs
 *
 * @param device
 *
 * @return EINA_TRUE on success, EINA_FALSE otherwise
 *
 * @ingroup Ecore_Drm2_Output_Group
 * @since 1.18
 */
EAPI Eina_Bool ecore_drm2_outputs_create(Ecore_Drm2_Device *device);

/**
 * Destroy any created outputs
 *
 * @param device
 *
 * @ingroup Ecore_Drm2_Output_Group
 * @since 1.18
 */
EAPI void ecore_drm2_outputs_destroy(Ecore_Drm2_Device *device);

/**
 * Get the list of outputs from a drm device
 *
 * @param device
 *
 * @return
 *
 * @ingroup Ecore_Drm2_Output_Group
 * @since 1.18
 */
EAPI const Eina_List *ecore_drm2_outputs_get(Ecore_Drm2_Device *device);

/**
 * Get the dpms level of a given output
 *
 * @param output
 *
 * @return Integer value representing the state of DPMS on a given output
 *         or -1 on error
 *
 * @ingroup Ecore_Drm2_Output_Group
 * @since 1.18
 */
EAPI int ecore_drm2_output_dpms_get(Ecore_Drm2_Output *output);

/**
 * Set the dpms level of a given output
 *
 * @param output
 * @param level
 *
 * @ingroup Ecore_Drm2_Output_Group
 * @since 1.18
 */
EAPI void ecore_drm2_output_dpms_set(Ecore_Drm2_Output *output, int level);

/**
 * Get the edid of a given output
 *
 * @param output
 *
 * @return A string representing the edid
 *
 * @ingroup Ecore_Drm2_Output_Group
 * @since 1.18
 */
EAPI char *ecore_drm2_output_edid_get(Ecore_Drm2_Output *output);

/**
 * Get if a given output has a backlight
 *
 * @param output
 *
 * @return EINA_TRUE if this output has a backlight, EINA_FALSE otherwise
 *
 * @ingroup Ecore_Drm2_Output_Group
 * @since 1.18
 */
EAPI Eina_Bool ecore_drm2_output_backlight_get(Ecore_Drm2_Output *output);

/**
 * Find an output at the given position
 *
 * @param device
 * @param x
 * @param y
 *
 * @return An Ecore_Drm2_Output which exists at the given coordinates, or NULL on failure
 *
 * @ingroup Ecore_Drm2_Output_Group
 * @since 1.18
 */
EAPI Ecore_Drm2_Output *ecore_drm2_output_find(Ecore_Drm2_Device *device, int x, int y);

/**
 * Get the geometry of a given output
 *
 * @param output
 * @param x
 * @param y
 * @param w
 * @param h
 *
 * @ingroup Ecore_Drm2_Output_Group
 * @since 1.18
 */
EAPI void ecore_drm2_output_geometry_get(Ecore_Drm2_Output *output, int *x, int *y, int *w, int *h);

/**
 * Get the id of the crtc that an output is using
 *
 * @param output
 *
 * @return A valid crtc id or 0 on failure
 *
 * @ingroup Ecore_Drm2_Output_Group
 * @since 1.18
 */
EAPI unsigned int ecore_drm2_output_crtc_get(Ecore_Drm2_Output *output);

# endif

#endif
