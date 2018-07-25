#include <assert.h>
#include <ctype.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/keycodes.h>
#include <time.h>
#include <stdbool.h>
#include <math.h>

#include "bbutil.h"

#include <GLES/gl.h>
#include <GLES/glext.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#include "png.h"

EGLDisplay egl_disp;
EGLSurface egl_surf;

static EGLConfig egl_conf;
static EGLContext egl_ctx;

static screen_context_t screen_ctx;
static screen_window_t screen_win;
static screen_display_t screen_disp;
static int nbuffers = 2;

static int initialized = 0;

static void
bbutil_egl_perror(const char *msg) {
    static const char *errmsg[] = {
        "function succeeded",
        "EGL is not initialized, or could not be initialized, for the specified display",
        "cannot access a requested resource",
        "failed to allocate resources for the requested operation",
        "an unrecognized attribute or attribute value was passed in an attribute list",
        "an EGLConfig argument does not name a valid EGLConfig",
        "an EGLContext argument does not name a valid EGLContext",
        "the current surface of the calling thread is no longer valid",
        "an EGLDisplay argument does not name a valid EGLDisplay",
        "arguments are inconsistent",
        "an EGLNativePixmapType argument does not refer to a valid native pixmap",
        "an EGLNativeWindowType argument does not refer to a valid native window",
        "one or more argument values are invalid",
        "an EGLSurface argument does not name a valid surface configured for rendering",
        "a power management event has occurred",
    };

    fprintf(stderr, "%s: %s\n", msg, errmsg[eglGetError() - EGL_SUCCESS]);
}
EGLConfig bbutil_choose_config(EGLDisplay egl_disp, enum RENDERING_API api) {
    EGLConfig egl_conf = (EGLConfig)0;
    EGLConfig *egl_configs;
    EGLint egl_num_configs;
    EGLint val;
    EGLBoolean rc;
    EGLint i;

    rc = eglGetConfigs(egl_disp, NULL, 0, &egl_num_configs);
    if (rc != EGL_TRUE) {
        bbutil_egl_perror("eglGetConfigs");
        return egl_conf;
    }
    if (egl_num_configs == 0) {
        fprintf(stderr, "eglGetConfigs: could not find a configuration\n");
        return egl_conf;
    }

    egl_configs = malloc(egl_num_configs * sizeof(*egl_configs));
    if (egl_configs == NULL) {
        fprintf(stderr, "could not allocate memory for %d EGL configs\n", egl_num_configs);
        return egl_conf;
    }

    rc = eglGetConfigs(egl_disp, egl_configs,
        egl_num_configs, &egl_num_configs);
    if (rc != EGL_TRUE) {
        bbutil_egl_perror("eglGetConfigs");
        free(egl_configs);
        return egl_conf;
    }

    for (i = 0; i < egl_num_configs; i++) {
        eglGetConfigAttrib(egl_disp, egl_configs[i], EGL_SURFACE_TYPE, &val);
        if (!(val & EGL_WINDOW_BIT)) {
            continue;
        }

        eglGetConfigAttrib(egl_disp, egl_configs[i], EGL_RENDERABLE_TYPE, &val);
        if (!(val & api)) {
        	continue;
        }

        eglGetConfigAttrib(egl_disp, egl_configs[i], EGL_DEPTH_SIZE, &val);
        if ((api & (GL_ES_1|GL_ES_2)) && (val == 0)) {
            continue;
        }

        eglGetConfigAttrib(egl_disp, egl_configs[i], EGL_RED_SIZE, &val);
        if (val != 8) {
            continue;
        }
        eglGetConfigAttrib(egl_disp, egl_configs[i], EGL_GREEN_SIZE, &val);
        if (val != 8) {
            continue;
        }

        eglGetConfigAttrib(egl_disp, egl_configs[i], EGL_BLUE_SIZE, &val);
        if (val != 8) {
            continue;
        }

        eglGetConfigAttrib(egl_disp, egl_configs[i], EGL_BUFFER_SIZE, &val);
        if (val != 32) {
            continue;
        }

        egl_conf = egl_configs[i];
        break;
    }

    free(egl_configs);

    if (egl_conf == (EGLConfig)0) {
        fprintf(stderr, "bbutil_choose_config: could not find a matching configuration\n");
    }

    return egl_conf;
}

int
bbutil_init_egl(screen_context_t ctx, enum RENDERING_API api, enum ORIENTATION orientation) {
    int usage;
    int format = SCREEN_FORMAT_RGBX8888;
    EGLint interval = 1;
    int rc;
    EGLint attributes[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE };

    if (api == GL_ES_1) {
    	usage = SCREEN_USAGE_OPENGL_ES1 | SCREEN_USAGE_ROTATION;
    } else if (api == GL_ES_2) {
    	usage = SCREEN_USAGE_OPENGL_ES2 | SCREEN_USAGE_ROTATION;
    } else if (api == VG) {
    	usage = SCREEN_USAGE_OPENVG | SCREEN_USAGE_ROTATION;
    } else {
    	fprintf(stderr, "invalid api setting\n");
    	return EXIT_FAILURE;
    }

    //Simple egl initialization
    screen_ctx = ctx;

    egl_disp = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (egl_disp == EGL_NO_DISPLAY) {
        bbutil_egl_perror("eglGetDisplay");
        bbutil_terminate();
        return EXIT_FAILURE;
    }

    rc = eglInitialize(egl_disp, NULL, NULL);
    if (rc != EGL_TRUE) {
        bbutil_egl_perror("eglInitialize");
        bbutil_terminate();
        return EXIT_FAILURE;
    }

    if ((api == GL_ES_1) || (api == GL_ES_2)) {
    	rc = eglBindAPI(EGL_OPENGL_ES_API);
    } else if (api == VG) {
    	rc = eglBindAPI(EGL_OPENVG_API);
    }

    if (rc != EGL_TRUE) {
        bbutil_egl_perror("eglBindApi");
        bbutil_terminate();
        return EXIT_FAILURE;
    }

    egl_conf = bbutil_choose_config(egl_disp, api);
    if (egl_conf == (EGLConfig)0) {
        bbutil_terminate();
        return EXIT_FAILURE;
    }

    if (api == GL_ES_2) {
    	egl_ctx = eglCreateContext(egl_disp, egl_conf, EGL_NO_CONTEXT, attributes);
    } else {
    	egl_ctx = eglCreateContext(egl_disp, egl_conf, EGL_NO_CONTEXT, NULL);
    }

    if (egl_ctx == EGL_NO_CONTEXT) {
        bbutil_egl_perror("eglCreateContext");
        bbutil_terminate();
        return EXIT_FAILURE;
    }

    rc = screen_create_window(&screen_win, screen_ctx);
    if (rc) {
        perror("screen_create_window");
        bbutil_terminate();
        return EXIT_FAILURE;
    }

    rc = screen_set_window_property_iv(screen_win, SCREEN_PROPERTY_FORMAT, &format);
    if (rc) {
        perror("screen_set_window_property_iv(SCREEN_PROPERTY_FORMAT)");
        bbutil_terminate();
        return EXIT_FAILURE;
    }

    rc = screen_set_window_property_iv(screen_win, SCREEN_PROPERTY_USAGE, &usage);
    if (rc) {
        perror("screen_set_window_property_iv(SCREEN_PROPERTY_USAGE)");
        bbutil_terminate();
        return EXIT_FAILURE;
    }

    rc = screen_get_window_property_pv(screen_win, SCREEN_PROPERTY_DISPLAY, (void **)&screen_disp);
    if (rc) {
        perror("screen_get_window_property_pv");
        bbutil_terminate();
        return EXIT_FAILURE;
    }

    if (orientation != AUTO) {
		int screen_resolution[2];
		rc = screen_get_display_property_iv(screen_disp, SCREEN_PROPERTY_SIZE, screen_resolution);
		if (rc) {
			perror("screen_get_display_property_iv");
			bbutil_terminate();
			return EXIT_FAILURE;
		}

		int angle = atoi(getenv("ORIENTATION"));
		int buffer_size[2] = {screen_resolution[0], screen_resolution[1]};
		int flip = false;

		if (((angle == 90) || (angle == 270)) && (orientation == LANDSCAPE)) {
			angle = 0;
			buffer_size[0] = screen_resolution[1];
			buffer_size[1] = screen_resolution[0];
			flip = true;
		} else if (((angle == 0) || (angle == 180)) && (orientation == PORTRAIT)) {
			buffer_size[0] = screen_resolution[1];
			buffer_size[1] = screen_resolution[0];
			flip = true;
		}

		if (flip) {
			rc = screen_set_window_property_iv(screen_win, SCREEN_PROPERTY_ROTATION, &angle);
			if (rc) {
				perror("screen_set_window_property_iv");
				bbutil_terminate();
				return EXIT_FAILURE;
			}

			rc = screen_set_window_property_iv(screen_win, SCREEN_PROPERTY_BUFFER_SIZE, buffer_size);
			if (rc) {
				perror("screen_set_window_property_iv");
				bbutil_terminate();
				return EXIT_FAILURE;
			}
		}
    }

    rc = screen_create_window_buffers(screen_win, nbuffers);
	if (rc) {
		perror("screen_create_window_buffers");
		bbutil_terminate();
		return EXIT_FAILURE;
	}

    egl_surf = eglCreateWindowSurface(egl_disp, egl_conf, screen_win, NULL);
    if (egl_surf == EGL_NO_SURFACE) {
        bbutil_egl_perror("eglCreateWindowSurface");
        bbutil_terminate();
        return EXIT_FAILURE;
    }

    rc = eglMakeCurrent(egl_disp, egl_surf, egl_surf, egl_ctx);
    if (rc != EGL_TRUE) {
        bbutil_egl_perror("eglMakeCurrent");
        bbutil_terminate();
        return EXIT_FAILURE;
    }

    rc = eglSwapInterval(egl_disp, interval);
    if (rc != EGL_TRUE) {
        bbutil_egl_perror("eglSwapInterval");
        bbutil_terminate();
        return EXIT_FAILURE;
    }

    initialized = true;

    return EXIT_SUCCESS;
}

void
bbutil_terminate() {
    //Typical EGL cleanup
	if (egl_disp != EGL_NO_DISPLAY) {
	    eglMakeCurrent(egl_disp, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
	    if (egl_surf != EGL_NO_SURFACE) {
	        eglDestroySurface(egl_disp, egl_surf);
	        egl_surf = EGL_NO_SURFACE;
	    }
	    if (egl_ctx != EGL_NO_CONTEXT) {
	        eglDestroyContext(egl_disp, egl_ctx);
	        egl_ctx = EGL_NO_CONTEXT;
	    }
	    if (screen_win != NULL) {
	        screen_destroy_window(screen_win);
	        screen_win = NULL;
	    }
	    eglTerminate(egl_disp);
	    egl_disp = EGL_NO_DISPLAY;
	}
	eglReleaseThread();

	initialized = false;
}

void
bbutil_swap() {
    int rc = eglSwapBuffers(egl_disp, egl_surf);
    if (rc != EGL_TRUE) {
        bbutil_egl_perror("eglSwapBuffers");
    }
}
