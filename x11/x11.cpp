#include "x11.h"

extern "C" {
#define HAVE_LIBJPEG 1
#include <jpeglib.h>
#include "../glm/glm.h"
}

#include "../RendererPlugin.h"
#include "../opengl_helpers.h"


// ---------------------------------------------------------------------------
// Class:    WindowInfo
// Design:   Belongs to OpenGL component
// Purpose:
// Updated:  Sep 10, 2012
// ---------------------------------------------------------------------------
typedef struct _WindowInfo
{
  XWindowAttributes attrib;
  Window window;
  Pixmap pixmap;
  GLXPixmap glxpixmap;
  GLuint texture;
  _WindowInfo() : window(0),pixmap(0),glxpixmap(0),texture(0) {}
} WindowInfo;

Window input;
Window stage_win;
std::map<Window, WindowInfo> redirectedWindows;

// TODO: get rid of global variables
Display *dpy;
Screen *scrn;
Window root;
Window overlay;

GLMmodel *pmodel = 0;

int xi_opcode;
int xfixes_event_base;

///// FROM COMPIZ
static int errors = 0;

// ===========================================================================
// Component: Interface with X11
// TODO: split into a separate file
// ===========================================================================

// ---------------------------------------------------------------------------
// Function: errorHandler
// Design:   Belongs to X11 component
// Purpose:  Sets the error handler for X11 events
// Updated:  Sep 10, 2012
// ---------------------------------------------------------------------------
int errorHandler(Display *dpy, XErrorEvent *e)
{
#define DEBUG 1
#ifdef DEBUG
  char str[128];
#endif

  errors++;

#ifdef DEBUG
  XGetErrorDatabaseText (dpy, "XlibMessage", "XError", "", str, 128);
  fprintf (stderr, "%s", str);

  XGetErrorText (dpy, e->error_code, str, 128);
  fprintf (stderr, ": %s\n  ", str);

  XGetErrorDatabaseText (dpy, "XlibMessage", "MajorCode", "%d", str, 128);
  fprintf (stderr, str, e->request_code);

  sprintf (str, "%d", e->request_code);
  XGetErrorDatabaseText (dpy, "XRequest", str, "", str, 128);
  if (strcmp (str, ""))
    fprintf (stderr, " (%s)", str);
  fprintf (stderr, "\n  ");

  XGetErrorDatabaseText (dpy, "XlibMessage", "MinorCode", "%d", str, 128);
  fprintf (stderr, str, e->minor_code);
  fprintf (stderr, "\n  ");

  XGetErrorDatabaseText (dpy, "XlibMessage", "ResourceID", "%d", str, 128);
  fprintf (stderr, str, e->resourceid);
  fprintf (stderr, "\n");

  /* abort (); */
#endif
#undef DEBUG

  return 0;
}

/* TODO: Is this function still needed?
int checkForError (Display *dpy)
{
  int e;

  XSync (dpy, false);

  e = errors;
  errors = 0;

  return e;
}
*/

// ---------------------------------------------------------------------------
// Function: allow_input_passthrough
// Design:   Belongs to X11 component
// Purpose:
// Updated:  Sep 10, 2012
// ---------------------------------------------------------------------------
void allow_input_passthrough(Window w)
{
  XserverRegion region = XFixesCreateRegion(dpy, 0, 0);

  XFixesSetWindowShapeRegion(dpy, w, ShapeBounding, 0, 0, 0);
  XFixesSetWindowShapeRegion(dpy, w, ShapeInput, 0, 0, region);

  XFixesDestroyRegion(dpy, region);
}


// ---------------------------------------------------------------------------
// Function: prep_root
// Design:   Belongs to X11 component
// Purpose:
// Updated:  Sep 10, 2012
// ---------------------------------------------------------------------------
void prep_root(void)
{
  dpy = XOpenDisplay(0);
  display = dpy;

  bool hasNamePixmap = false;
  int event_base, error_base;
  if (XCompositeQueryExtension(dpy, &event_base, &error_base)) {
    int major = 0, minor = 2; // The highest version we support
    XCompositeQueryVersion( dpy, &major, &minor );
    hasNamePixmap = ( major > 0 || minor >= 2 );
    std::cout << "1. has composite extension! hasNamePixmap: "
              << hasNamePixmap << std::endl << std::endl;
  }

  // XInput Extension available?
  int event, error;
  if (!XQueryExtension(dpy, "XInputExtension", &xi_opcode, &event, &error)) {
    std::cerr << "X Input extension not available." << std::endl;
    exit(EXIT_FAILURE);
  }

  // Which version of XI2? We support 2.0
  int major = 2, minor = 0;
  if (XIQueryVersion(dpy, &major, &minor) == BadRequest) {
    std::cerr << "XI2 not available. Server supports "
              << major << "." << minor << std::endl;
    exit(EXIT_FAILURE);
  }

  // XFixes Extension available?
  int /*fixesversion(5), */fixeserror;
  if (!XFixesQueryExtension(dpy, &xfixes_event_base, &fixeserror)) {
    std::cerr << "X Fixes extension not available." << std::endl;
    exit(EXIT_FAILURE);
  }

  // XFixes Version available? We support 5.0
  int fixes_major(5), fixes_minor;
  if (!XFixesQueryVersion(dpy, &fixes_major, &fixes_minor)) {
    std::cerr << "X Fixes version 5 not available. Server supports "
              << fixes_major << "." << fixes_minor << std::endl;
    exit(EXIT_FAILURE);
  }

  root = DefaultRootWindow(dpy);
  scrn = DefaultScreenOfDisplay(dpy);
  screen = XDefaultScreen(dpy);
  for (int i = 0; i < ScreenCount(dpy); ++i) {
    XSelectInput(dpy, RootWindow(dpy, i), SubstructureNotifyMask |
                                          PointerMotionMask |
                                          KeyPressMask);
    XCompositeRedirectSubwindows(dpy, RootWindow(dpy, i),
                                 CompositeRedirectAutomatic);
    XSync(dpy, false);
    XIEventMask evmask;
    unsigned char mask[2] = { 0, 0 };

    XISetMask(mask, XI_HierarchyChanged | XI_Motion | XI_RawMotion);
    evmask.deviceid = XIAllDevices;
    evmask.mask_len = sizeof(mask);
    evmask.mask = mask;

    XISelectEvents(dpy, RootWindow(dpy, i), &evmask, 1);
    XFixesSelectCursorInput(dpy, RootWindow(dpy, i),
                            XFixesDisplayCursorNotifyMask);
  }

  XIEventMask evmask;
  unsigned char mask[2] = { 0, 0 };

  XISetMask(mask, XI_HierarchyChanged | XI_Motion | XI_RawMotion);
  evmask.deviceid = XIAllDevices;
  evmask.mask_len = sizeof(mask);
  evmask.mask = mask;

  XISelectEvents(dpy, DefaultRootWindow(dpy), &evmask, 1);
  XFixesSelectCursorInput(dpy, DefaultRootWindow(dpy),
                          XFixesDisplayCursorNotifyMask);
}

// ---------------------------------------------------------------------------
// Function: prep_overlay
// Design:   Belongs to X11 component
// Purpose:
// Updated:  Sep 10, 2012
// ---------------------------------------------------------------------------
void prep_overlay(void)
{
  overlay = XCompositeGetOverlayWindow(dpy, root);
  allow_input_passthrough(overlay);
}


// ---------------------------------------------------------------------------
// Function: prep_stage
// Design:   Belongs to X11 component
// Purpose:
// Updated:  Sep 10, 2012
// ---------------------------------------------------------------------------
void prep_stage(Window window_)
{
  XReparentWindow(dpy, window_, overlay, 0, 0);
  XSelectInput(dpy, window_, ExposureMask |
                            PointerMotionMask |
                            KeyPressMask |
                            SubstructureNotifyMask);
  allow_input_passthrough(window_);
}

// ---------------------------------------------------------------------------
// Function: prep_input
// Design:   Belongs to X11 component
// Purpose:
// Updated:  Sep 10, 2012
// ---------------------------------------------------------------------------
void prep_input(void)
{
  XWindowAttributes attr;
  XGetWindowAttributes(dpy, root, &attr);
  input = XCreateWindow(dpy, root,
                        0, 0,  /* x, y */
                        attr.width, attr.height,
                        0, 0, /* border width, depth */
                        InputOnly, DefaultVisual (dpy, 0), 0, 0);

  XSelectInput(dpy, input, StructureNotifyMask |
                            FocusChangeMask |
                            PointerMotionMask |
                            KeyPressMask |
                            KeyReleaseMask |
                            ButtonPressMask |
                            ButtonReleaseMask |
                            PropertyChangeMask);
  XMapWindow(dpy, input);
}


// ---------------------------------------------------------------------------
// Function: prep_input2
// Design:   Belongs to X11 component
// Purpose:
// Updated:  Sep 10, 2012
// TODO:     Why is it even needed? Can we directly call XSetInputFocus()?
// ---------------------------------------------------------------------------
void prep_input2 (void)
{
  XSetInputFocus (dpy, window, RevertToPointerRoot, CurrentTime);
}


// ---------------------------------------------------------------------------
// Function: WaitForNotify
// Design:   Belongs to X11 component
// Purpose:
// Updated:  Sep 10, 2012
// ---------------------------------------------------------------------------
Bool WaitForNotify(Display *d, XEvent *e, char *arg)
{
  return (e->type == MapNotify) && (e->xmap.window == (Window)arg);
}


// ===========================================================================
// Component: OpenGL engine
// TODO: split into a separate file
// ===========================================================================

// ---------------------------------------------------------------------------
// Function: unbindRedirectedWindow
// Design:   Interface between X11 and OpenGL component
// Purpose:
// Updated:  Sep 10, 2012
// ---------------------------------------------------------------------------
void unbindRedirectedWindow(Display *display_, Window window)
{
  if (redirectedWindows.find(window) != redirectedWindows.end()) {
    const WindowInfo &windowInfo = redirectedWindows[window];

    if (windowInfo.texture > 0)
      glDeleteTextures(1, &windowInfo.texture);

    if (windowInfo.glxpixmap > 0) {
      glXReleaseTexImageEXT (display_, windowInfo.glxpixmap, GLX_FRONT_LEFT_EXT);
      glXDestroyGLXPixmap(display_, windowInfo.glxpixmap);
    }

    if (windowInfo.pixmap > 0)
      XFreePixmap(display_, windowInfo.pixmap);

    redirectedWindows.erase(window);
  }
}


// ---------------------------------------------------------------------------
// Function: bindRedirectedWindowToTexture
// Design:   Interface between X11 and OpenGL component
// Purpose:
// Updated:  Sep 10, 2012
// ---------------------------------------------------------------------------
void bindRedirectedWindowToTexture(Display *display_, Window window_, int screen_)
{
  static const int pixmapAttribs[] = { GLX_TEXTURE_TARGET_EXT,
                                       GLX_TEXTURE_2D_EXT,
                                       GLX_TEXTURE_FORMAT_EXT,
                                       GLX_TEXTURE_FORMAT_RGBA_EXT,
                                       None };

  WindowInfo windowInfo;
  XWindowAttributes *attrib;
  if (redirectedWindows.find(window_) != redirectedWindows.end()) {
    windowInfo = redirectedWindows[window_];
  } else {
    XGetWindowAttributes( display_, window_, &windowInfo.attrib );
    if (windowInfo.attrib.map_state != IsViewable) {
      redirectedWindows[window_] = windowInfo;
      return;
    }

    Pixmap pixmap = XCompositeNameWindowPixmap(display_, window_);
    if (!pixmap) {
      redirectedWindows[window_] = windowInfo;
      return;
    }
    GLXPixmap glxpixmap = glXCreatePixmap(display_, fbconfig, pixmap,
                                          pixmapAttribs);

    GLuint tempTexture;
    glGenTextures(1, &tempTexture);
    windowInfo.pixmap    = pixmap;
    windowInfo.glxpixmap = glxpixmap;
    windowInfo.texture   = tempTexture;
    windowInfo.window    = window_;

    redirectedWindows[window_] = windowInfo;

    glBindTexture(GL_TEXTURE_2D, windowInfo.texture);
    glXBindTexImageEXT(display_, windowInfo.glxpixmap, GLX_FRONT_LEFT_EXT, 0);
  }
  attrib = &windowInfo.attrib;

  if (attrib->map_state != IsViewable)
    return;

  const double w = (double)attrib->width / (double)width;
  const double h = (double)attrib->height / (double)height;
  const double right = w;
  const double t = -h;

  glBindTexture(GL_TEXTURE_2D, windowInfo.texture);
  glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  // draw using pixmap as texture
  const double originX = (double)attrib->x/(double)width - 0.5;
  const double originY = (double)-attrib->y/(double)height + 0.5;
  const double originZ = (renderToTexture) ? 0 : -1.21;

  glColor4f(1, 1, 1, 1);
  glBegin(GL_TRIANGLE_STRIP);
  glTexCoord2d(0, bottom);
  glVertex3f(originX, originY, originZ);

  glTexCoord2d(1, bottom);
  glVertex3f(originX+right, originY, originZ);

  glTexCoord2d(0, top);
  glVertex3f(originX, originY + t, originZ);

  glTexCoord2d(1, top);
  glVertex3f(originX + right, originY + t, originZ);
  glEnd();

  glBindTexture(GL_TEXTURE_2D, 0);
  checkForErrors();
}



// ---------------------------------------------------------------------------
// Function: loadMonitorModel
// Design:   Belongs to OpenGL component
// Purpose:  Loads the model of the monitor into GLM
// Updated:  Sep 10, 2012
// ---------------------------------------------------------------------------
void loadMonitorModel()
{
  if (!pmodel) {  // load the model
    pmodel = glmReadOBJ("lcd_monitor.obj");
    if (!pmodel) {
      std::cout << "\nUsage: objviewV2 <-s> <obj filename>\n";
      exit(EXIT_FAILURE);
    }
    glmUnitize(pmodel);
    glmVertexNormals(pmodel, 90.0, GL_TRUE);
  }
}


// ---------------------------------------------------------------------------
// Function: getCursorTexture
// Design:   Belongs (mostly) to OpenGL component
// Purpose:  Converts mouse pointer from image to texture and uploads the
//           texture into OpenGL
// Updated:  Sep 10, 2012
// TODO:     Get XFixesCursorImage elsewhere and pass into this function
// ---------------------------------------------------------------------------
void getCursorTexture()
{
  if (!cursorTexture)
    glGenTextures(1, &cursorTexture);

  XFixesCursorImage *cursor_image = XFixesGetCursorImage(dpy);

  // Annoyingly, xfixes specifies the data to be 32bit, but places it in
  // an unsigned long * which can be 64 bit. So we need to iterate over a
  // 64bit structure to put it in a 32bit structure.
  // std::cerr << cursor_image->width << "x" << cursor_image->height
  //           << std::endl;
  std::vector<GLuint> pixels(cursor_image->width * cursor_image->height + 1);

  for (int i = 0; i < cursor_image->width * cursor_image->height; ++i) {
    pixels[i] = cursor_image->pixels[i] & 0xffffffff;
    pixels[i] = (pixels[i] >> 24 & 0x000000FF) ?
                      (pixels[i] | 0xFF000000) : pixels[i];
  }

  glBindTexture(GL_TEXTURE_2D, cursorTexture);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, cursor_image->width,
               cursor_image->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, &pixels[0]);
  glBindTexture(GL_TEXTURE_2D, 0);

  XFree(cursor_image);
}


// ===========================================================================
// Component: Input Hardware Control (keyboard, mouse, hydra, tablet, etc)
// TODO: split into a separate file
// ===========================================================================

// ---------------------------------------------------------------------------
// Function: setup_hotkey
// Design:   Belongs to Input Hardware Control component
// Purpose:
// Updated:  Sep 10, 2012
// ---------------------------------------------------------------------------
void setup_hotkey(Display *display_)
{
  unsigned int    modifiers       = ControlMask | ShiftMask;
  int             keycode         = XKeysymToKeycode(display_, XK_Y);
  Window          grab_window     = root;
  Bool            owner_events    = False;
  int             pointer_mode    = GrabModeAsync;
  int             keyboard_mode   = GrabModeAsync;

  XGrabKey(display_, keycode, modifiers, grab_window, owner_events,
           pointer_mode, keyboard_mode);

  XSelectInput(display_, root, KeyPressMask);
}


// ---------------------------------------------------------------------------
// Function: disallow_input_passthrough
// Design:   Belongs to ?
// Purpose:
// Updated:  Sep 10, 2012
// ---------------------------------------------------------------------------
void disallow_input_passthrough(Window w)
{
  XRectangle r;
  r.x = r.y = 0;
  r.width = width;
  r.height = height;
  XserverRegion region = XFixesCreateRegion(dpy, &r, 1);

  XFixesSetWindowShapeRegion(dpy, w, ShapeBounding, 0, 0, region);
  XFixesSetWindowShapeRegion(dpy, w, ShapeInput, 0, 0, region);

  XFixesDestroyRegion(dpy, region);
}


// ---------------------------------------------------------------------------
// Function: toggleControl
// Design:   Belongs to Input Hardware Control component
// Purpose:  Toggles user action between positional control and desktop work
// Updated:  Sep 10, 2012
// ---------------------------------------------------------------------------
void toggleControl()
{
  controlDesktop = !controlDesktop;
  if (controlDesktop) {
    allow_input_passthrough(overlay);
    allow_input_passthrough(window);

    XUngrabKeyboard(dpy, CurrentTime);
    XUngrabPointer(dpy, CurrentTime);
  } else {
    disallow_input_passthrough(overlay);
    disallow_input_passthrough(window);

    XIEventMask eventmask;
    eventmask.deviceid = XIAllDevices;
    eventmask.mask_len = XIMaskLen(XI_RawMotion);
    std::vector<unsigned char> vecMask(eventmask.mask_len + 1, (unsigned char)0);
    eventmask.mask = &vecMask[0];

    // Now set the mask
    XISetMask(eventmask.mask, XI_Motion);
    XISetMask(eventmask.mask, XI_RawMotion);
    XISetMask(eventmask.mask, XI_KeyPress);
    XISetMask(eventmask.mask, XI_KeyRelease);

    // Select on the window
    XISelectEvents(dpy, root, &eventmask, 1);
    XISelectEvents(dpy, window, &eventmask, 1);
    XISelectEvents(dpy, overlay, &eventmask, 1);

    XIGrabDevice(dpy, XIAllDevices, root, CurrentTime,
                 0 /*cursor*/, GrabModeAsync, 0 /*pairedMode*/,
                 False, &eventmask);
    XIGrabDevice(dpy, XIAllDevices, overlay, CurrentTime,
                 0 /*cursor*/, GrabModeAsync, 0 /*pairedMode*/,
                 False, &eventmask);
    XIGrabDevice(dpy, XIAllDevices, window, CurrentTime,
                 0 /*cursor*/, GrabModeAsync, 0 /*pairedMode*/,
                 False, &eventmask);

    XGrabKeyboard(dpy, DefaultRootWindow(dpy), False, GrabModeAsync,
                  GrabModeAsync, CurrentTime);
    XGrabPointer(dpy, DefaultRootWindow(dpy), False,
                 ButtonPressMask |
                 ButtonReleaseMask |
                 PointerMotionMask |
                 FocusChangeMask |
                 EnterWindowMask |
                 LeaveWindowMask,
                 GrabModeAsync, GrabModeAsync, DefaultRootWindow(dpy), None,
                 CurrentTime);
  }
}


// ---------------------------------------------------------------------------
// Function: processKey
// Design:   Belongs to Input Hardware Control component
// Purpose:
// Updated:  Sep 10, 2012
// ---------------------------------------------------------------------------
void processKey(XKeyEvent ke)
{
  static KeyCode toggleKey = XKeysymToKeycode(dpy, XK_Y);

  if ((ke.state & ControlMask) &&
      (ke.state & ShiftMask) &&
       ke.keycode == toggleKey) {
    toggleControl();
    return;
  }
}


// ---------------------------------------------------------------------------
// Function: debugRawMotion
// Design:   Belongs to Input Hardware Control component
// Purpose:  Prints immediate motion information for debugging purposes.
//           Not used in the release version.
// Updated:  Sep 10, 2012
// ---------------------------------------------------------------------------
void debugRawMotion(XIRawEvent *event)
{
  double *raw_valuator = event->raw_values;
  double *valuator = event->valuators.values;

  for (int i = 0; i < event->valuators.mask_len * 8; i++) {
    if (XIMaskIsSet(event->valuators.mask, i)) {
      std::cout << "Acceleration on valuator " << i << ": "
                << *valuator - *raw_valuator << std::endl;
      valuator++;
      raw_valuator++;
    }
  }
}


// ---------------------------------------------------------------------------
// Function: processRawMotion
// Design:   Belongs to Input Hardware Control component
// Purpose:  Modifies desktop orientation/position using mouse as a tracker
// Updated:  Sep 10, 2012
// ---------------------------------------------------------------------------
void processRawMotion(XIRawEvent *event, Desktop3DLocation& loc)
{
  double *raw_valuator = event->raw_values;
  double *valuator = event->valuators.values;
  double xRotation, yRotation;

  for (int i = 0; i < event->valuators.mask_len * 8; i++) {
    if (XIMaskIsSet(event->valuators.mask, i)) {
      switch (i) {
      case 0:
        yRotation = loc.getYRotation();
        yRotation += (double)(*valuator - *raw_valuator) /
                     (double)width * 180.0;
        loc.setYRotation(yRotation);
        relativeMouseY = (double)(*valuator - *raw_valuator);
        break;
      case 1:
        xRotation = loc.getXRotation();
        xRotation += (double)(*valuator - *raw_valuator) /
                     (double)width * 180.0;
        loc.setXRotation(xRotation);
        relativeMouseX = (double)(*valuator - *raw_valuator);
        break;
      }
      valuator++;
      raw_valuator++;
    }
  }
}


// ---------------------------------------------------------------------------
// Function: processKey
// Design:   Belongs to Input Hardware Control component
// Purpose:  Modifies desktop orientation/position based on key pressed
//           Also toggles barrel distort and ground layer
// Updated:  Sep 10, 2012
// ---------------------------------------------------------------------------
static bool jump = false;
void processXInput2Key(XIDeviceEvent *event, bool pressed, Desktop3DLocation& loc)
{
  static KeyCode B = XKeysymToKeycode(dpy, XK_B); // toggle barrel distort
  static KeyCode G = XKeysymToKeycode(dpy, XK_G); // toggle ground

  static KeyCode W = XKeysymToKeycode(dpy, XK_W);
  static KeyCode S = XKeysymToKeycode(dpy, XK_S);
  static KeyCode A = XKeysymToKeycode(dpy, XK_A);
  static KeyCode D = XKeysymToKeycode(dpy, XK_D);
  static KeyCode Q = XKeysymToKeycode(dpy, XK_Q);
  static KeyCode E = XKeysymToKeycode(dpy, XK_E);
  static KeyCode R = XKeysymToKeycode(dpy, XK_R);
  static KeyCode SPACE = XKeysymToKeycode(dpy, XK_space);

  if (!controlDesktop) {
    if (pressed) {
      if (event->detail == W) {
        walkForward = 1;
      } else if (event->detail == S) {
        walkForward = -1;
      } else if (event->detail == A) {
        strafeRight = -1;
      } else if (event->detail == D) {
        strafeRight = 1;
      } else if (event->detail == Q) {
        strafeRight = -1;
      } else if (event->detail == E) {
        strafeRight = 1;
      } else if (event->detail == B) {
          barrelDistort = !barrelDistort;
      } else if (event->detail == G) {
          showGround = !showGround;
      } else if (event->detail == R) {
       std::cout << "RESET POSITION!" << std::endl;
       // Reset the desktop location to all zero state
       loc.resetState();
      } else if (event->detail == SPACE) {
        jump = true;
      }
    } else {
      if (walkForward ==  1 && event->detail == W)
        walkForward = 0;
      if (walkForward == -1 && event->detail == S)
        walkForward = 0;
      if (strafeRight ==  1 && (event->detail == D || event->detail == E))
        strafeRight = 0;
      if (strafeRight == -1 && (event->detail == A || event->detail == Q))
        strafeRight = 0;
      if (jump && event->detail == SPACE)
        jump = false;
    }
  }
}

// ---------------------------------------------------------------------------
// Function: renderDesktopToTexture
// Design:   Interface between OpenGL and Input Hardware Control component
// Purpose:
// Updated:  Sep 10, 2012
// ---------------------------------------------------------------------------
void renderDesktopToTexture()
{
  if (renderToTexture) {
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, desktopFBO);
    if (!checkForErrors()) {
      exit(EXIT_FAILURE);
    }
    glClearColor(0, 1, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  }

  long unsigned int wId;
  static std::set<Window> s;

  static Window parent, root2;
  Window *children;
  static unsigned int countChildren;

  Bool mousePositionGrabbed(false);
  Window window_returned;
  int root_x, root_y;
  int win_x, win_y;
  unsigned int mask_return;

  if (ortho && renderToTexture) {
    glViewport(0,0,width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-0.5, 0.5, -0.5, 0.5, -10, 10);
    glMatrixMode(GL_MODELVIEW);
    glDisable(GL_DEPTH_TEST);
  }
  glPushMatrix();

  XSync(dpy, false);
  XGrabServer(dpy);
  mousePositionGrabbed = XQueryPointer(dpy, XDefaultRootWindow(dpy),
                                       &window_returned, &window_returned,
                                       &root_x, &root_y, &win_x, &win_y,
                                       &mask_return);
  XQueryTree(dpy, XDefaultRootWindow(dpy), &parent, &root2, &children,
             &countChildren);

  int d = 0;
  for (unsigned int i = 0; i < countChildren; ++i) {
    wId = children[i];
    if (wId == window || wId == overlay)
      continue;

    if (s.find(wId) == s.end()) {
      s.insert(wId);
      XCompositeRedirectSubwindows( dpy, wId, CompositeRedirectAutomatic);
      XSelectInput(dpy, wId, StructureNotifyMask |
                             PointerMotionMask |
                             ExposureMask);
    }

    if (!renderToTexture) {
      glTranslatef(0, 0, d*0.0001);
    }
    bindRedirectedWindowToTexture(dpy, wId, screen);
    ++d;
  }

  XFree(children);
  glPopMatrix();
  XUngrabServer(dpy);

  if (mousePositionGrabbed == True) {
    glEnable(GL_BLEND);
    double originX = ((double)root_x - 12) / width - 0.5;
    double originY = 0.5 - ((double)root_y + 24.0) / height;
    double originZ = -1.21 + d * 0.0001;
    double t = 24.0 / height;
    double right = 24.0 / width;

    glBindTexture(GL_TEXTURE_2D, cursorTexture);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(1, 1, 1, 1);
    glBegin(GL_TRIANGLE_STRIP);
      glTexCoord2d(0, 1);
      glVertex3f(originX, originY, originZ);

      glTexCoord2d(1, 1);
      glVertex3f(originX + right, originY, originZ);

      glTexCoord2d(0, 0);
      glVertex3f(originX,originY+t,originZ);

      glTexCoord2d(1, 0);
      glVertex3f(originX + right, originY + t, originZ);
    glEnd();
    glBindTexture(GL_TEXTURE_2D, 0);

    glDisable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  }

  if (renderToTexture) {
    if (ortho) {
      glMatrixMode(GL_PROJECTION);
      glLoadIdentity();
//      gluPerspective(120.0f, 0.75, 0.01f, 1000.0f);
      gluPerspective(110.0f, 0.81818181, 0.01f, 1000.0f);
      glMatrixMode(GL_MODELVIEW);
      glEnable(GL_DEPTH_TEST);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
  }
}
