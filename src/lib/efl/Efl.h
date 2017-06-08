#ifndef _EFL_H
#define _EFL_H

#if defined ( __cplusplus )
extern "C" {
#endif

#include <Eo.h>

#ifdef EAPI
# undef EAPI
#endif

#ifdef _WIN32
# ifdef EFL_EFL_BUILD
#  ifdef DLL_EXPORT
#   define EAPI __declspec(dllexport)
#  else
#   define EAPI
#  endif /* ! DLL_EXPORT */
# else
#  define EAPI __declspec(dllimport)
# endif /* ! EFL_EFL_BUILD */
#else
# ifdef __GNUC__
#  if __GNUC__ >= 4
#   define EAPI __attribute__ ((visibility("default")))
#  else
#   define EAPI
#  endif
# else
#  define EAPI
# endif
#endif /* ! _WIN32 */

#define EFL_VERSION_1_18 1
#define EFL_VERSION_1_19 1

/* Add here all the required ifdef for any @protected method */
#ifdef EFL_EFL_BUILD
# define EFL_PACK_LAYOUT_PROTECTED
# define EFL_GFX_SIZE_HINT_PROTECTED
#endif

/**
 * @ingroup Efl
 * @since 1.18
 *
 * This type is a alias to struct tm.
 * It is intended to be a standard way to reference it in .eo files.
 */
typedef struct tm Efl_Time;

#ifdef EFL_BETA_API_SUPPORT

#include "interfaces/efl_types.eot.h"

#include <Efl_Model_Common.h>

#include "interfaces/efl_vpath_file.eo.h"
#include "interfaces/efl_vpath.eo.h"
#include "interfaces/efl_vpath_core.eo.h"
#include "interfaces/efl_vpath_manager.eo.h"
#include "interfaces/efl_vpath_file_core.eo.h"

/* Data types */
#include "interfaces/efl_gfx_types.eot.h"
#include "interfaces/efl_ui_types.eot.h"
typedef Efl_Gfx_Path_Command_Type Efl_Gfx_Path_Command;

/* Interfaces */
#include "interfaces/efl_config.eo.h"
#include "interfaces/efl_control.eo.h"
#include "interfaces/efl_file.eo.h"
#include "interfaces/efl_image.eo.h"
#include "interfaces/efl_image_animated.eo.h"
#include "interfaces/efl_image_load.eo.h"
#include "interfaces/efl_part.eo.h"
#include "interfaces/efl_player.eo.h"
#include "interfaces/efl_text.eo.h"
#include "interfaces/efl_text_types.eot.h"
#include "interfaces/efl_text_properties.eo.h"
#include "interfaces/efl_orientation.eo.h"
#include "interfaces/efl_flipable.eo.h"
#include "interfaces/efl_ui_base.eo.h"
#include "interfaces/efl_ui_drag.eo.h"
#include "interfaces/efl_ui_spin.eo.h"
#include "interfaces/efl_ui_progress.eo.h"
#include "interfaces/efl_ui_item.eo.h"
#include "interfaces/efl_ui_menu.eo.h"
#include "interfaces/efl_ui_autorepeat.eo.h"

#include "interfaces/efl_screen.eo.h"

#define EFL_ORIENT_0   EFL_ORIENT_UP
#define EFL_ORIENT_90  EFL_ORIENT_RIGHT
#define EFL_ORIENT_180 EFL_ORIENT_DOWN
#define EFL_ORIENT_270 EFL_ORIENT_LEFT

/* Core interface */
#include "interfaces/efl_animator.eo.h"

/* Graphics */
#include "interfaces/efl_gfx.eo.h"
#include "interfaces/efl_gfx_buffer.eo.h"
#include "interfaces/efl_gfx_stack.eo.h"
#include "interfaces/efl_gfx_fill.eo.h"
#include "interfaces/efl_gfx_view.eo.h"
#include "interfaces/efl_gfx_path.eo.h"
#include "interfaces/efl_gfx_shape.eo.h"
#include "interfaces/efl_gfx_gradient.eo.h"
#include "interfaces/efl_gfx_gradient_linear.eo.h"
#include "interfaces/efl_gfx_gradient_radial.eo.h"
#include "interfaces/efl_gfx_filter.eo.h"
#include "interfaces/efl_gfx_size_hint.eo.h"

/* Input events */
#include "interfaces/efl_input_types.eot.h"
#include "interfaces/efl_input_device.eo.h"

/* Canvas & UI */
#include "interfaces/efl_canvas.eo.h"
#include "interfaces/efl_ui_view.eo.h"
#include "interfaces/efl_ui_model_connect.eo.h"
#include "interfaces/efl_ui_factory.eo.h"
#include "interfaces/efl_ui_model_factory_connect.eo.h"

/* Observable interface */
#include "interfaces/efl_observer.eo.h"
#include "interfaces/efl_observable.eo.h"

/* Packing & containers */
#include "interfaces/efl_container.eo.h"
#include "interfaces/efl_pack.eo.h"
#include "interfaces/efl_pack_layout.eo.h"
#include "interfaces/efl_pack_linear.eo.h"
#include "interfaces/efl_pack_grid.eo.h"

/* Input and Output */
#include "interfaces/efl_io_closer.eo.h"
#include "interfaces/efl_io_reader.eo.h"
#include "interfaces/efl_io_writer.eo.h"
#include "interfaces/efl_io_sizer.eo.h"
#include "interfaces/efl_io_positioner.eo.h"

#include "interfaces/efl_io_buffer.eo.h"
#include "interfaces/efl_io_queue.eo.h"

/* Text interfaces */
#include "interfaces/efl_text_font.eo.h"
#include "interfaces/efl_text_style.eo.h"
#include "interfaces/efl_text_format.eo.h"

#else

#ifndef EFL_NOLEGACY_API_SUPPORT
#include "interfaces/efl_gfx_types.eot.h"
#include "interfaces/efl_ui_types.eot.h"
#include "interfaces/efl_input_types.eot.h"
#include "interfaces/efl_gfx_fill.eo.legacy.h"
#include "interfaces/efl_gfx.eo.legacy.h"
#include "interfaces/efl_image.eo.legacy.h"
#include "interfaces/efl_image_animated.eo.legacy.h"
#include "interfaces/efl_input_device.eo.legacy.h"
#include "interfaces/efl_text_types.eot.h"
#endif

typedef Efl_Gfx_Path_Command_Type Efl_Gfx_Path_Command;

#endif

/* work-around bug in gcc --as-needed link optimization */
EAPI void __efl_internal_init(void);

#if defined ( __cplusplus )
}
#endif

#undef EAPI
#define EAPI

#endif
