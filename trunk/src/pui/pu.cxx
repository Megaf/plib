
#include "puLocal.h"

#ifdef PU_NOT_USING_GLUT
#include <iostream.h>
#endif

#define PU_STRING_X_FUDGE 6
#define PU_STRING_Y_FUDGE 6

int puRefresh = TRUE ;

static int puWindowWidth  = 400 ;
static int puWindowHeight = 400 ;

#ifdef PU_NOT_USING_GLUT

int puGetWindow       () { return 0              ; }
int puGetWindowHeight () { return puWindowHeight ; }
int puGetWindowWidth  () { return puWindowWidth  ; }

void puSetWindowSize ( int width, int height )
{
  puWindowWidth  = width  ;
  puWindowHeight = height ;
}

static int fontBase = 0;
static int fontSize[257];
#else

static bool openGLSize = false;


void puSetResizeMode ( bool mode ) {
  openGLSize = mode ;
}


int puGetWindow ()
{
  return glutGetWindow () ;
}


int puGetWindowHeight ()
{
  if ( ! openGLSize )
	return glutGet ( (GLenum) GLUT_WINDOW_HEIGHT ) ;
  else
	return puWindowHeight ;
}


int puGetWindowWidth ()
{
  if ( ! openGLSize )
	return glutGet ( (GLenum) GLUT_WINDOW_WIDTH ) ;
  else
	return puWindowWidth ;
}


void puSetWindowSize ( int width, int height )
{
  if ( ! openGLSize )
  fprintf ( stderr, "PUI: puSetWindowSize shouldn't be used with GLUT.\n" ) ;
  else
  {
    puWindowWidth  = width  ;
    puWindowHeight = height ;
  }
}

#endif

puColour _puDefaultColourTable[] =
{
  { 0.5f, 0.5f, 0.5f, 1.0f }, /* PUCOL_FOREGROUND */
  { 0.3f, 0.3f, 0.3f, 1.0f }, /* PUCOL_BACKGROUND */
  { 0.7f, 0.7f, 0.7f, 1.0f }, /* PUCOL_HIGHLIGHT  */
  { 0.0f, 0.0f, 0.0f, 1.0f }, /* PUCOL_LABEL      */
  { 1.0f, 1.0f, 1.0f, 1.0f }, /* PUCOL_TEXT       */

  { 0.0f, 0.0f, 0.0f, 0.0f }  /* ILLEGAL */
} ;
 

puValue::~puValue () {}  

static int _puCursor_enable = FALSE ;
static int _puCursor_x      = 0 ;
static int _puCursor_y      = 0 ;
static float _puCursor_bgcolour [4] = { 1.0f, 1.0f, 1.0f, 1.0f } ; 
static float _puCursor_fgcolour [4] = { 0.0f, 0.0f, 0.0f, 1.0f } ;  

void   puHideCursor ( void ) { _puCursor_enable = FALSE ; }
void   puShowCursor ( void ) { _puCursor_enable = TRUE  ; }
int    puCursorIsHidden ( void ) { return ! _puCursor_enable ; }

void puCursor ( int x, int y )
{
  _puCursor_x = x ;
  _puCursor_y = y ;
}


static void puDrawCursor ( int x, int y )
{
  glColor4fv ( _puCursor_bgcolour ) ;  

  glBegin    ( GL_TRIANGLES ) ;
  glVertex2i ( x, y ) ;
  glVertex2i ( x + 13, y -  4 ) ;
  glVertex2i ( x +  4, y - 13 ) ;

  glVertex2i ( x +  8, y -  3 ) ;
  glVertex2i ( x + 17, y - 12 ) ;
  glVertex2i ( x + 12, y - 17 ) ;

  glVertex2i ( x + 12, y - 17 ) ;
  glVertex2i ( x +  3, y -  8 ) ;
  glVertex2i ( x +  8, y -  3 ) ;
  glEnd      () ;

  glColor4fv ( _puCursor_fgcolour ) ;  

  glBegin    ( GL_TRIANGLES ) ;
  glVertex2i ( x+1, y-1 ) ;
  glVertex2i ( x + 11, y -  4 ) ;
  glVertex2i ( x +  4, y - 11 ) ;

  glVertex2i ( x +  8, y -  5 ) ;
  glVertex2i ( x + 15, y - 12 ) ;
  glVertex2i ( x + 12, y - 15 ) ;

  glVertex2i ( x + 12, y - 15 ) ;
  glVertex2i ( x +  5, y -  8 ) ;
  glVertex2i ( x +  8, y -  5 ) ;
  glEnd      () ;
}


// Pointer to linked list of objects to delete
// as a result of keyboarding or mouse clicking

static puObject *objects_to_delete = NULL;


void puDeleteObject ( puObject *ob )
{
  puGroup *parent = ob->getParent () ;
  
  // Remove from parent interface
  if ( parent != ob && parent != NULL )
    parent -> remove ( ob ) ;

  // Pop live interface
  if ( ob -> getType () & PUCLASS_INTERFACE )
    puPopLiveInterface ( (puInterface*)ob ) ;

  ob -> prev = NULL ;                       // Add to linked list to be deleted
  ob -> next = objects_to_delete ;
  objects_to_delete = ob ;
  ob -> setParent ( NULL ) ;
}


static void puCleanUpJunk ()
{
  // Step through the linked list of objects to delete, removing them.
  while ( objects_to_delete != NULL )
  {
    puObject *next_ob = objects_to_delete ->next ;
    delete objects_to_delete ;
    objects_to_delete = next_ob ;
  }
}


void puInit ( void )
{
  static int firsttime = TRUE ;

  if ( firsttime )
  {
    if ( glGetCurrentContext () == NULL )
    {
      fprintf ( stderr,
      "FATAL: puInit called without a valid OpenGL context.\n");
      exit ( 1 ) ;
    }

    puInterface *base_interface = new puInterface ( 0, 0 ) ;
    puPushGroup         ( base_interface ) ;
    puPushLiveInterface ( base_interface ) ;
    firsttime = FALSE ;
#ifdef PU_NOT_USING_GLUT

    /* Create bitmaps for the device context font's first 256 glyphs */

    fontBase = glGenLists(256);
    assert(fontBase);
    HDC hdc = wglGetCurrentDC();

    /* Make the system font the device context's selected font */

    SelectObject (hdc, GetStockObject (SYSTEM_FONT)); 

    int *tempSize = &fontSize[1];

    if ( ! GetCharWidth32 ( hdc, 1, 255, tempSize ) &&
         ! GetCharWidth   ( hdc, 1, 255, tempSize ) )
    {
      LPVOID lpMsgBuf ;

      FormatMessage ( FORMAT_MESSAGE_ALLOCATE_BUFFER |
	              FORMAT_MESSAGE_FROM_SYSTEM,
	              NULL,
	              GetLastError(),
	              MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
	              (LPTSTR) &lpMsgBuf,
	              0, NULL ) ;

      fprintf ( stderr, "PUI: Error: %s\n", (char *)lpMsgBuf ) ;
      LocalFree ( lpMsgBuf ) ;
    }

    wglUseFontBitmaps ( hdc, 0, 256, fontBase ) ;
#endif
  }
}

static void puSetOpenGLState ( void )
{
  int w = puGetWindowWidth  () ;
  int h = puGetWindowHeight () ;

  if ( ! openGLSize )
    glPushAttrib   ( GL_ENABLE_BIT | GL_TRANSFORM_BIT | GL_LIGHTING_BIT ) ;
  else
  glPushAttrib   ( GL_ENABLE_BIT | GL_VIEWPORT_BIT | GL_TRANSFORM_BIT | GL_LIGHTING_BIT ) ;

  glDisable      ( GL_LIGHTING   ) ;
  glDisable      ( GL_FOG        ) ;
  glDisable      ( GL_TEXTURE_2D ) ;
  glDisable      ( GL_DEPTH_TEST ) ;
  glDisable      ( GL_CULL_FACE  ) ;
 
  if ( ! openGLSize )
  glViewport     ( 0, 0, w, h ) ;
 
  glMatrixMode   ( GL_PROJECTION ) ;
  glPushMatrix   () ;
  glLoadIdentity () ;
  gluOrtho2D     ( 0, w, 0, h ) ;
  glMatrixMode   ( GL_MODELVIEW ) ;
  glPushMatrix   () ;
  glLoadIdentity () ;
}

static void puRestoreOpenGLState ( void )
{
  glMatrixMode   ( GL_PROJECTION ) ;
  glPopMatrix    () ;
  glMatrixMode   ( GL_MODELVIEW ) ;
  glPopMatrix    () ;
  glPopAttrib    () ;
}


void  puDisplay ( void )
{
  puCleanUpJunk () ;

  puSetOpenGLState () ;
  puGetUltimateLiveInterface () -> draw ( 0, 0 ) ;

  int h = puGetWindowHeight () ;

  if ( _puCursor_enable )
    puDrawCursor ( _puCursor_x,
                   h - _puCursor_y ) ;

  puRestoreOpenGLState () ;
}


void  puDisplay ( int window_number )  /* Redraw only the current window */
{
  puCleanUpJunk () ;

  puSetOpenGLState () ;
  puInterface *base_interface = puGetUltimateLiveInterface () ;
  puObject *ob = base_interface -> getFirstChild () ;
  while ( ob )
  {
    if ( ob -> getWindow () == window_number )
      ob -> draw ( 0, 0 ) ;

    ob = ob -> getNextObject () ;
  }

  int h = puGetWindowHeight () ;

  if ( _puCursor_enable )
    puDrawCursor ( _puCursor_x,
                   h - _puCursor_y ) ;

  puRestoreOpenGLState () ;
}


int puKeyboard ( int key, int updown )
{
  int return_value = puGetBaseLiveInterface () -> checkKey ( key, updown ) ;
  
  puCleanUpJunk () ;
  
  return return_value ;
}


static int last_buttons = 0 ;
int puMouse ( int button, int updown, int x, int y )
{
  puCursor ( x, y ) ;
  
  int h = puGetWindowHeight () ;
  
  if ( updown == PU_DOWN )
    last_buttons |=  ( 1 << button ) ;
  else
    last_buttons &= ~( 1 << button ) ;
  
  int return_value =  puGetBaseLiveInterface () -> checkHit ( button,
    updown, x, h - y ) ;
  
  puCleanUpJunk () ;
  
  return return_value ;
}


int puMouse ( int x, int y )
{
  puCursor ( x, y ) ;
  
  if ( last_buttons == 0 )
    return FALSE ;
  
  int button =
    (last_buttons & (1<<PU_LEFT_BUTTON  )) ?  PU_LEFT_BUTTON   :
    (last_buttons & (1<<PU_MIDDLE_BUTTON)) ?  PU_MIDDLE_BUTTON :
    (last_buttons & (1<<PU_RIGHT_BUTTON )) ?  PU_RIGHT_BUTTON  : 0 ;
  
  int h = puGetWindowHeight () ;
  
  int return_value = puGetBaseLiveInterface () -> checkHit ( button,
    PU_DRAG, x, h - y ) ;
  
  puCleanUpJunk () ;
  
  return return_value ;
}
