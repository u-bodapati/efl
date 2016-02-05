#ifndef _ECORE_DRM2_H
# define _ECORE_DRM2_H

# include <Ecore.h>
# include <Eeze.h>

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

/* opaque structure to represent a launcher */
typedef struct _Ecore_Drm2_Launcher Ecore_Drm2_Launcher;

/* opaque structure to represent an input */
typedef struct _Ecore_Drm2_Input Ecore_Drm2_Input;

/* opaque structure to represent a seat */
typedef struct _Ecore_Drm2_Seat Ecore_Drm2_Seat;

/* opaque structure to represent an input device */
typedef struct _Ecore_Drm2_Input_Device Ecore_Drm2_Input_Device;

/* opaque structure to represent a plane */
typedef struct _Ecore_Drm2_Plane Ecore_Drm2_Plane;

/* opaque structure to represent an output */
typedef struct _Ecore_Drm2_Output Ecore_Drm2_Output;

/* opaque structure to represent an output mode */
typedef struct _Ecore_Drm2_Output_Mode Ecore_Drm2_Output_Mode;

/* opaque structure to represent a framebuffer */
typedef struct _Ecore_Drm2_Fb Ecore_Drm2_Fb;

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
 * @li @ref Ecore_Drm2_Launcher_Group
 * @li @ref Ecore_Drm2_Device_Group
 * @li @ref Ecore_Drm2_Input_Group
 * @li @ref Ecore_Drm2_Plane_Group
 * @li @ref Ecore_Drm2_Output_Group
 * @li @ref Ecore_Drm2_Fb_Group
 *
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
 * Shutdown the Ecore_Drm2 library.
 *
 * @return  The number of times the library has been initialized without
 *          being shutdown. 0 is returned if an error occurs.
 *
 * @ingroup Ecore_Drm2_Init_Group
 * @since 1.18
 */
EAPI int ecore_drm2_shutdown(void);

/**
 * @defgroup Ecore_Drm2_Device_Group Drm library device functions
 *
 * Functions that support various device actions
 */

/**
 * Try to find a drm card on the given seat
 *
 * @param seat
 *
 * @return The device name found
 *
 * @ingroup Ecore_Drm2_Device_Group
 * @since 1.18
 */
EAPI const char *ecore_drm2_device_find(const char *seat);

/**
 * Get the type of clock used by this drm device
 *
 * @param fd
 *
 * @return The clockid_t used by this drm card
 *
 * @ingroup Ecore_Drm2_Device_Group
 * @since 1.18
 */
EAPI int ecore_drm2_device_clock_id_get(int fd);

/**
 * Get the size of the cursor supported by drm device
 *
 * @param fd
 * @param width
 * @param height
 *
 * @ingroup Ecore_Drm2_Device_Group
 * @since 1.18
 */
EAPI void ecore_drm2_device_cursor_size_get(int fd, int *width, int *height);

/**
 * @defgroup Ecore_Drm2_Launcher_Group Drm launcher functions
 *
 * Functions that deal with setup of launcher
 */

/**
 * Create a launcher on the specified seat
 *
 * @param seat
 * @param tty
 * @param sync
 *
 * @return A Ecore_Drm2_Launcher on success, NULL on failure
 *
 * @ingroup Ecore_Drm2_Launcher_Group
 * @since 1.18
 */
EAPI Ecore_Drm2_Launcher *ecore_drm2_launcher_connect(const char *seat, unsigned int tty, Eina_Bool sync);

/**
 * Disconnect a launcher
 *
 * @param launcher
 *
 * @ingroup Ecore_Drm2_Launcher_Group
 * @since 1.18
 */
EAPI void ecore_drm2_launcher_disconnect(Ecore_Drm2_Launcher *launcher);

/**
 * Request launcher to open a file
 *
 * @param launcher
 * @param path
 * @param flags
 *
 * @return
 *
 * @ingroup Ecore_Drm2_Launcher_Group
 * @since 1.18
 */
EAPI int ecore_drm2_launcher_open(Ecore_Drm2_Launcher *launcher, const char *path, int flags);

/**
 * Request launcher to close a file
 *
 * @param launcher
 * @param fd
 *
 * @ingroup Ecore_Drm2_Launcher_Group
 * @since 1.18
 */
EAPI void ecore_drm2_launcher_close(Ecore_Drm2_Launcher *launcher, int fd);

/**
 * Request launcher to activate a given vt
 *
 * @param launcher
 * @param vt
 *
 * @return result
 *
 * @ingroup Ecore_Drm2_Launcher_Group
 * @since 1.18
 */
EAPI int ecore_drm2_launcher_activate(Ecore_Drm2_Launcher *launcher, int vt);

/**
 * Request launcher to restore state
 *
 * @param launcher
 *
 * @ingroup Ecore_Drm2_Launcher_Group
 * @since 1.18
 */
EAPI void ecore_drm2_launcher_restore(Ecore_Drm2_Launcher *launcher);

/**
 * @defgroup Ecore_Drm2_Input_Group Drm input functions
 *
 * Functions that deal with setup of inputs
 */

/**
 * Initialize input
 *
 * @param seat
 *
 * @return
 *
 * @ingroup Ecore_Drm2_Input_Group
 * @since 1.18
 */
EAPI Eina_Bool ecore_drm2_input_init(Ecore_Drm2_Launcher *launcher, const char *seat);

/**
 * Shutdown input
 *
 * @param
 *
 * @return
 *
 * @ingroup Ecore_Drm2_Input_Group
 * @since 1.18
 */
EAPI void ecore_drm2_input_shutdown(Ecore_Drm2_Launcher *launcher);

/**
 * Enable a given input
 *
 * @param input
 *
 * @return
 *
 * @ingroup Ecore_Drm2_Input_Group
 * @since 1.18
 */
EAPI Eina_Bool ecore_drm2_input_enable(Ecore_Drm2_Launcher *launcher);

/**
 * @defgroup Ecore_Drm2_Plane_Group Drm plane functions
 *
 * Functions that deal with setup of planes
 */

/**
 * Try to create planes
 *
 * @param launcher
 * @param fd
 *
 * @return EINA_TRUE on success, EINA_FALSE on failure
 *
 * @ingroup Ecore_Drm2_Plane_Group
 * @since 1.18
 */
EAPI Eina_Bool ecore_drm2_planes_create(Ecore_Drm2_Launcher *launcher, int fd);

/**
 * Destroy planes
 *
 * @param launcher
 *
 * @ingroup Ecore_Drm2_Plane_Group
 * @since 1.18
 */
EAPI void ecore_drm2_planes_destroy(Ecore_Drm2_Launcher *launcher, int fd);

/**
 * Find a plane of the given type which is supported on the given output
 *
 * @param launcher
 * @param output
 * @param type
 *
 * @return A valid Ecore_Drm2_Plane or NULL on failure
 *
 * @ingroup Ecore_Drm2_Plane_Group
 * @since 1.18
 */
EAPI Ecore_Drm2_Plane *ecore_drm2_plane_find(Ecore_Drm2_Launcher *launcher, Ecore_Drm2_Output *output, int type);

/**
 * @defgroup Ecore_Drm2_Output_Group Drm output functions
 *
 * Functions that deal with setup of outputs
 */

/**
 * Create outputs
 *
 * @param launcher
 * @param fd
 *
 * @return EINA_TRUE on success, EINA_FALSE on failure
 *
 * @ingroup Ecore_Drm2_Output_Group
 * @since 1.18
 */
EAPI Eina_Bool ecore_drm2_outputs_create(Ecore_Drm2_Launcher *launcher, int fd);

/**
 * Find an output at the given coordinates
 *
 * @param launcher
 * @param x
 * @param y
 *
 * @return A valid Ecore_Drm2_Output or NULL on failure
 *
 * @ingroup Ecore_Drm2_Output_Group
 * @since 1.18
 */
EAPI Ecore_Drm2_Output *ecore_drm2_output_find(Ecore_Drm2_Launcher *launcher, int x, int y);

/**
 * Destroy outputs
 *
 * @param launcher
 *
 * @ingroup Ecore_Drm2_Output_Group
 * @since 1.18
 */
EAPI void ecore_drm2_outputs_destroy(Ecore_Drm2_Launcher *launcher, int fd);

/**
 * @defgroup Ecore_Drm2_Fb_Group Drm framebuffer functions
 *
 * Functions that deal with setup of framebuffers
 */

/**
 * Create a new framebuffer object
 *
 * @param fd
 * @param width
 * @param height
 * @param depth
 * @param bpp
 * @param format
 *
 * @return A newly create framebuffer object, or NULL on failure
 *
 * @ingroup Ecore_Drm2_Fb_Group
 * @since 1.18
 */
EAPI Ecore_Drm2_Fb *ecore_drm2_fb_create(int fd, int width, int height, int depth, int bpp, unsigned int format);

/**
 * Destroy a framebuffer object
 *
 * @param fb
 *
 * @ingroup Ecore_Drm2_Fb_Group
 * @since 1.18
 */
EAPI void ecore_drm2_fb_destroy(Ecore_Drm2_Fb *fb);

/**
 * Get a framebuffer's mmap'd data
 *
 * @param fb
 *
 * @return The mmap'd area of the framebuffer or NULL on failure
 *
 * @ingroup Ecore_Drm2_Fb_Group
 * @since 1.18
 */
EAPI void *ecore_drm2_fb_data_get(Ecore_Drm2_Fb *fb);

/**
 * Get a framebuffer's size
 *
 * @param fb
 *
 * @return size of the framebuffers' mmap'd data or 0 on failure
 *
 * @ingroup Ecore_Drm2_Fb_Group
 * @since 1.18
 */
EAPI unsigned int ecore_drm2_fb_size_get(Ecore_Drm2_Fb *fb);

# endif

# undef EAPI
# define EAPI

#endif
