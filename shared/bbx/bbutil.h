#ifndef _UTILITY_H_INCLUDED
#define _UTILITY_H_INCLUDED

#include <EGL/egl.h>
#include <screen/screen.h>
#include <sys/platform.h>

extern EGLDisplay egl_disp;
extern EGLSurface egl_surf;


enum RENDERING_API {GL_ES_1 = EGL_OPENGL_ES_BIT, GL_ES_2 = EGL_OPENGL_ES2_BIT, VG = EGL_OPENVG_BIT};
enum ORIENTATION { PORTRAIT, LANDSCAPE, AUTO};

#define BBUTIL_DEFAULT_FONT "/usr/fonts/font_repository/monotype/arial.ttf"

__BEGIN_DECLS

/**
 * Initializes EGL
 *
 * \param libscreen context that will be used for EGL setup
 * \param rendering API that will be used
 * \param desired starting orientation
 * \return EXIT_SUCCESS if initialization succeeded otherwise EXIT_FAILURE
 */
int bbutil_init_egl(screen_context_t ctx, enum RENDERING_API api, enum ORIENTATION orientation);

/**
 * Terminates EGL
 */
void bbutil_terminate();

/**
 * Swaps default bbutil window surface to the screen
 */
void bbutil_swap();

int bbutil_rotate_screen_surface(int angle);

__END_DECLS

#endif
