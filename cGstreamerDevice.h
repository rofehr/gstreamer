#include <vdr/config.h>
#include <vdr/device.h>
#include <vdr/osd.h>
#include <vdr/plugin.h>
#include <vdr/args.h>

#include <gst/gst.h>
#include <gio/gio.h>
#include <gst/app/gstappsrc.h>
#include <gst/video/videooverlay.h>
#include <gst/video/video-overlay-composition.h>

#include <X11/Xlib.h>
#include <X11/extensions/Xcomposite.h>
#include <X11/extensions/shape.h>
#include <X11/extensions/Xrender.h>
#include <cairo/cairo.h>
#include <cairo/cairo-xlib.h>
#include <X11/xpm.h>
#include <X11/Xatom.h>

#include <GL/gl.h>
#include <GL/glx.h>

#include "osdgst.h"

static XVisualInfo vinfo;
static Visual *visual;
static Display *dpy = NULL;
static GstElement *appsrc = NULL;
static GstElement *gdkpixbufoverlay = NULL;
static GstElement *filesink = NULL;
static GstElement *bin = NULL;
static GstElement *sink = NULL;


static GstBus *bus = NULL;
static GError *error = NULL;
static gchar *uri = NULL;
static  gchar *local_uri =NULL;
static int blackColor;
static int whiteColor;

static Window win = 0;
static Window glX_win = 0;
static GC gc;

// Global Defines
static bool live_stream_is_runnig = FALSE;
static int ilive_stream_count = 0;


static int VisDataMain[] = {
		GLX_RENDER_TYPE, GLX_RGBA_BIT,
		GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
		GLX_DOUBLEBUFFER, True,
		GLX_RED_SIZE, 8,
		GLX_GREEN_SIZE, 8,
		GLX_BLUE_SIZE, 8,
		GLX_ALPHA_SIZE, 8,
		GLX_DEPTH_SIZE, 16,
		None
};


#define CHUNK_SIZE 4096

#define LOCAL_DEBUG 1

#ifdef LOCAL_DEBUG
#define TEMP_PATH "/var/cache/dummy.ts"
#else
#define TEMP_PATH "/var/cache/dummy.ts"
#endif


/* playbin flags */
typedef enum {
	GST_PLAY_FLAG_VIDEO = (1 << 0), /* We want to play vidoe output */
	GST_PLAY_FLAG_AUDIO = (1 << 1), /* We want to play audio ourput */
	GST_PLAY_FLAG_TEXT = (1 << 2) /* We want subtitle output */
} GstPlayFlags;

cOsd *cGstreamerOsdProvider::Osd;

class cGstreamerDevice : cDevice {
public:

	cGstreamerDevice();

	~cGstreamerDevice();

	void Init();

	bool HasDecoder(void) const;
	
	bool CanReplay(void) const;

	bool SetPlayMode(ePlayMode PlayMode);

	int PlayVideo(const uchar *Data, int Length);

	int PlayAudio(const uchar *Data, int Length, uchar Id);

	int PlayTsVideo(const uchar *Data, int Length);

	int PlayTsAudio(const uchar *Data, int Length);

	int PlayTsSubtitle(const uchar *Data, int Length);

	int PlayPes(const uchar *Data, int Length, bool VideoOnly = false);

	int push_to_buffer(const uchar *Data, int Length);

	int PlayTs(const uchar *Data, int Length, bool VideoOnly = false);

	bool Poll(cPoller &Poller, int TimeoutMs = 0);

	bool Flush(int TimeoutMs = 0);

	bool Start(void);

	void StartReplay();

protected:

	void MakePrimaryDevice(bool On);

	void StartReplayBuffer();

	void ShowOverlay();


};

