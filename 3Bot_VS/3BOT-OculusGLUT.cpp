/******************************************************************************/
/*                                                                            */
/* MODULE  : 3BOT-OculusGLUT.cpp (based on GRAPHICS_Example.cpp)              */
/*                                                                            */
/* PURPOSE : 3BOT and Oculus Rift HMD demo using  GLUT                        */
/*                                                                            */
/* DATE    : 05/Dec/2014                                                      */
/*                                                                            */
/* CHANGES                                                                    */
/*                                                                            */
/* V1.0  JNI 05/Dec/2014 - Initial development                                */
/*                                                                            */
/* V1.1  JNI 09/Dec/2014 - Full screen window positioned on HMD desktop.      */
/*                                                                            */
/* V1.2  JNI 16/Dec/2014 - Cleaned up for inclusion with 3BOT Host PCs.       */
/*                                                                            */
/******************************************************************************/

#define MODULE_NAME "3BOT-OculusGLUT"

/******************************************************************************/

#include <assert.h>
#include <glew.h>

#undef main

#include <OVR_CAPI.h>
#include <OVR_CAPI_GL.h>

extern "C"
{
    void ovrhmd_EnableHSWDisplaySDKRender( ovrHmd hmd, ovrBool enabled );
}

#include <motor.h>

/******************************************************************************/

static int win_width, win_height;

static unsigned int fbo=0, fb_tex, fb_depth;
static int fb_width, fb_height;
static int fb_tex_width, fb_tex_height;

static ovrHmd hmd=0;
static ovrSizei eyeres[2];
static ovrEyeRenderDesc eye_rdesc[2];
static ovrGLTexture fb_ovr_tex[2];

TIMER_Frequency IdleFrequency("IdleFrequency");
TIMER_Interval  DrawLatency("DrawLatency");
TIMER_Interval  SwapBufferLatency("SwapBufferLatency");
TIMER_Interval  ClearStereoLatency("ClearStereoLatency");
TIMER_Interval  ClearMonoLatency("ClearMonoLatency");
TIMER_Interval DisplayLatency("DisplayLatency");
TIMER_Interval DrawSceneLatency("DrawSceneLatency");
TIMER_Interval HMD_BeginFrameLatency("HMD_BeginFrameLatency");
TIMER_Interval HMD_EndFrameLatency("HMD_EndFrameLatency");
TIMER_Interval EyeLoopLatency("EyeLoopLatency");

TIMER_Frequency DisplayFrequency("DisplayFrequency");
BOOL DisplayFrequencyFlag=TRUE;

/******************************************************************************/

// Dimensions of HMD graphics workspace (m).
float  GRAPHICS_MinX=-0.4;
float  GRAPHICS_MaxX=0.4;
float  GRAPHICS_MinY=1.05;
float  GRAPHICS_MaxY=1.55;
float  GRAPHICS_MinZ=-1.35;
float  GRAPHICS_MaxZ=-0.85;

// HMD is rotated around X-axis (deg).
float  GRAPHICS_RotateX=30.0; 

float  GRAPHICS_WidthX;
float  GRAPHICS_WidthY;
float  GRAPHICS_WidthZ;

float  GRAPHICS_CentreX;
float  GRAPHICS_CentreY;
float  GRAPHICS_CentreZ;

matrix GRAPHICS_Centre(3,1);

/******************************************************************************/

#define GRID_FLOOR  11
#define GRID_BACK   12
#define GRID_LEFT   13
#define GRID_RIGHT  14

int     CALIBRATE_GridCallList[] = { GRID_FLOOR,GRID_BACK,GRID_LEFT,GRID_RIGHT,-1 };

/******************************************************************************/

#define GRID_XY      1
#define GRID_XZ      2
#define GRID_YZ      3
  
#define GRID_SIZE   10

/******************************************************************************/

void CALIBRATE_GridBuild( float GRAPHICS_MinX, float GRAPHICS_MaxX, float GRAPHICS_MinY, float GRAPHICS_MaxY, float zpos, int grid, int ID )
{
GLint i,g[GRID_SIZE+1];
GLfloat ypos[GRID_SIZE+1],xpos[GRID_SIZE+1];
  
    // Create list of vertices...
    for( i=0; (i <= GRID_SIZE); i++ )
    {
        if( (float)(i/2) == (0.5*(float)i) )
        {
            g[i] = 0; 
        }
        else
        {
            g[i] = GRID_SIZE-1; 
        }

        xpos[i] = GRAPHICS_MinX + (GRAPHICS_MaxX-GRAPHICS_MinX)*i/(GRID_SIZE-1);
        ypos[i] = GRAPHICS_MinY + (GRAPHICS_MaxY-GRAPHICS_MinY)*i/(GRID_SIZE-1);
    }

    // Start a new GL compile list...
    glNewList(ID,GL_COMPILE);
    glColor3f(1.0,1.0,1.0);
    set_color(WHITE);
    glBegin(GL_LINE_STRIP);      

    switch( grid )
    {
        case GRID_XY :
           for( i=0; (i < GRID_SIZE); i++ )
           {
               glVertex3f(xpos[g[i]],ypos[i],zpos);
               glVertex3f(xpos[g[i+1]],ypos[i],zpos);
           }

           for( i=0; (i < GRID_SIZE); i++ )
           {
               glVertex3f(xpos[i],ypos[g[i]],zpos);
               glVertex3f(xpos[i],ypos[g[i+1]],zpos);
           }
           break;
      
        case GRID_XZ :
           for( i=0; (i < GRID_SIZE); i++ )
           {
               glVertex3f(xpos[g[i]],zpos,ypos[i]);
               glVertex3f(xpos[g[i+1]],zpos,ypos[i]);
           }

           for( i=0; (i < GRID_SIZE); i++ )
           {
               glVertex3f(xpos[i],zpos,ypos[g[i]]);
               glVertex3f(xpos[i],zpos,ypos[g[i+1]]);
           }
           break;
      
        case GRID_YZ :
           for( i=0; (i < GRID_SIZE); i++ )
           {
               glVertex3f(zpos,xpos[g[i]],ypos[i]);
               glVertex3f(zpos,xpos[g[i+1]],ypos[i]);
           }

           for( i=0; (i < GRID_SIZE); i++ )
           {
               glVertex3f(zpos,xpos[i],ypos[g[i]]);
               glVertex3f(zpos,xpos[i],ypos[g[i+1]]);
           }
           break;
    }

    glEnd();
    glEndList();
}

/******************************************************************************/

void CALIBRATE_GridBuild( void )
{
    CALIBRATE_GridBuild(GRAPHICS_MinX,GRAPHICS_MaxX,GRAPHICS_MinY,GRAPHICS_MaxY,GRAPHICS_MinZ,GRID_XY,GRID_BACK);
    CALIBRATE_GridBuild(GRAPHICS_MinX,GRAPHICS_MaxX,GRAPHICS_MinZ,GRAPHICS_MaxZ,GRAPHICS_MinY,GRID_XZ,GRID_FLOOR);
    CALIBRATE_GridBuild(GRAPHICS_MinY,GRAPHICS_MaxY,GRAPHICS_MinZ,GRAPHICS_MaxZ,GRAPHICS_MinX,GRID_YZ,GRID_LEFT);
    CALIBRATE_GridBuild(GRAPHICS_MinY,GRAPHICS_MaxY,GRAPHICS_MinZ,GRAPHICS_MaxZ,GRAPHICS_MaxX,GRID_YZ,GRID_RIGHT);
}

/******************************************************************************/

void CALIBRATE_GridDraw( void )
{
int i;

    for( i=0; (CALIBRATE_GridCallList[i] != -1); i++ )
    {
        glPushMatrix();
        glCallList(CALIBRATE_GridCallList[i]);
        glPopMatrix(); 
    }
}

/******************************************************************************/

STRING RobotName="";
int    RobotID=ROBOT_INVALID;
matrix RobotPosition(3,1);             // cm
matrix RobotPositionRaw(3,1);          // cm
matrix RobotPositionOffset(3,1);       // cm
matrix RobotVelocity(3,1);             // cm/sec
matrix RobotForces(3,1);               // N
double RobotForceMax=40.0;             // N
matrix RobotPositionGraphics(3,1);     // HMD workspace units (HWU)
double RobotSpringConstant=-4000.0;    // N/m

TIMER_Frequency RobotControlLoopFrequency("RobotLoop");
TIMER_Interval  RobotControlLoopLatency("RobotLoop");

matrix SpherePosition(3,1);
double SphereRadius = 0.07;
int    SphereColor = RED;
BOOL   SphereInsideFlag=FALSE;

double CursorRadius = 0.01;
int    CursorColor = RED;

/******************************************************************************/

void ProgramExit( void );

/******************************************************************************/

unsigned int next_pow2( unsigned int x )
{
    x -= 1;
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;

    return(x + 1);
}

/******************************************************************************/

// Convert a quaternion to a rotation matrix
void quat_to_matrix( const float *quat, float *mat )
{
    mat[ 0] = 1.0 - 2.0 * quat[1] * quat[1] - 2.0 * quat[2] * quat[2];
    mat[ 4] = 2.0 * quat[0] * quat[1] + 2.0 * quat[3] * quat[2];
    mat[ 8] = 2.0 * quat[2] * quat[0] - 2.0 * quat[3] * quat[1];
    mat[12] = 0.0f;

    mat[ 1] = 2.0 * quat[0] * quat[1] - 2.0 * quat[3] * quat[2];
    mat[ 5] = 1.0 - 2.0 * quat[0]*quat[0] - 2.0 * quat[2]*quat[2];
    mat[ 9] = 2.0 * quat[1] * quat[2] + 2.0 * quat[3] * quat[0];
    mat[13] = 0.0f;

    mat[ 2] = 2.0 * quat[2] * quat[0] + 2.0 * quat[3] * quat[1];
    mat[ 6] = 2.0 * quat[1] * quat[2] - 2.0 * quat[3] * quat[0];
    mat[10] = 1.0 - 2.0 * quat[0]*quat[0] - 2.0 * quat[1]*quat[1];
    mat[14] = 0.0f;

    mat[ 3] = mat[7] = mat[11] = 0.0f;
    mat[15] = 1.0f;
}

/******************************************************************************/

void TimingResults( void )
{
    printf("----------------------------------------------------------------\n");

    DisplayFrequency.Results();
    IdleFrequency.Results();
    DisplayLatency.Results();
    SwapBufferLatency.Results();
    HMD_BeginFrameLatency.Results();
    HMD_EndFrameLatency.Results();
    EyeLoopLatency.Results();
    DrawSceneLatency.Results();

    printf("----------------------------------------------------------------\n");
}

/******************************************************************************/

BOOL OCULUS_Init( void )
{
    ovr_Initialize();

    hmd = ovrHmd_Create(0);

    if( !hmd )
    {
        printf("Failed to open Oculus HMD, falling back to virtual debug HMD\n");

        hmd = ovrHmd_CreateDebug(ovrHmd_DK2);
        if( !hmd )
        {
            printf("Failed to create virtual debug HMD\n");
            return(FALSE);
        }
    }

    printf("HMD Initialized: %s - %s\n",hmd->Manufacturer,hmd->ProductName);
    printf("Resolution: wid=%d hgt=%d\n",hmd->Resolution.w,hmd->Resolution.h);

    return(TRUE);
}

/******************************************************************************/

void OCULUS_Stop( void )
{
    if( hmd )
    {
        ovrHmd_Destroy(hmd);
    }

    ovr_Shutdown();
}

/******************************************************************************/

void GraphicsIdle( void )
{
    IdleFrequency.Loop();

    // Check keyboard events.
    KB_GLUT_Events();

    glutPostRedisplay();
}

/******************************************************************************/

void GraphicsSphereDraw( void );
void GraphicsWireSphereDraw( void );
void GraphicsSphere( matrix &posn, double radius, int color );
void GraphicsWireSphere( matrix &posn, double radius, int color );

/******************************************************************************/

void OCULUS_DrawScene( void )
{
int i;
float LightPosition[][4] = 
{
    { -8,2,10,1 },
    { 0,15,0,1 }
};

float LightColor[][4] = 
{
    { 0.8,0.8,0.8,1 },
    { 0.4,0.3,0.3,1 }
};

    for( i=0; (i < 2); i++ )
    {
        glLightfv(GL_LIGHT0 + i,GL_POSITION,LightPosition[i]);
        glLightfv(GL_LIGHT0 + i,GL_DIFFUSE,LightColor[i]);
    }

    glMatrixMode(GL_MODELVIEW);

    GRAPHICS_ColorSet(WHITE);
    glPushMatrix();
    CALIBRATE_GridDraw();
    glPopMatrix();

    // Fixed object.
    //GraphicsSphere(SpherePosition,SphereRadius,SphereColor);
    GraphicsWireSphere(SpherePosition,SphereRadius,SphereColor);

    // Robot cursor.
    GraphicsSphere(RobotPositionGraphics,CursorRadius,CursorColor);
}

/******************************************************************************/

void OCULUS_Display( void )
{
	int i;
	ovrMatrix4f proj;
	ovrPosef pose[2];
	float rot_mat[16];

        if( DisplayFrequencyFlag )
        {
            DisplayFrequency.Reset();
            DisplayFrequencyFlag = FALSE;
        }

        DisplayFrequency.Loop();
        DisplayLatency.Before();

	/* the drawing starts with a call to ovrHmd_BeginFrame */
        HMD_BeginFrameLatency.Before();
	ovrHmd_BeginFrame(hmd, 0);
        HMD_BeginFrameLatency.After();

	/* start drawing onto our texture render target */
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        EyeLoopLatency.Before();

	/* for each eye ... */
	for(i=0; i<2; i++) {
		ovrEyeType eye = hmd->EyeRenderOrder[i];

		/* -- viewport transformation --
		 * setup the viewport to draw in the left half of the framebuffer when we're
		 * rendering the left eye's view (0, 0, width/2, height), and in the right half
		 * of the framebuffer for the right eye's view (width/2, 0, width/2, height)
		 */
		glViewport(eye == ovrEye_Left ? 0 : fb_width / 2, 0, fb_width / 2, fb_height);

		/* -- projection transformation --
		 * we'll just have to use the projection matrix supplied by the oculus SDK for this eye
		 * note that libovr matrices are the transpose of what OpenGL expects, so we have to
		 * use glLoadTransposeMatrixf instead of glLoadMatrixf to load it.
		 */

                // zmin was 0.5 now 0.05
		proj = ovrMatrix4f_Projection(hmd->DefaultEyeFov[eye], 0.05, 500.0, 1);
		glMatrixMode(GL_PROJECTION);
		glLoadTransposeMatrixf(proj.M[0]);

		/* -- view/camera transformation --
		 * we need to construct a view matrix by combining all the information provided by the oculus
		 * SDK, about the position and orientation of the user's head in the world.
		 */
		pose[eye] = ovrHmd_GetEyePose(hmd, eye);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glTranslatef(eye_rdesc[eye].ViewAdjust.x, eye_rdesc[eye].ViewAdjust.y, eye_rdesc[eye].ViewAdjust.z);
		/* retrieve the orientation quaternion and convert it to a rotation matrix */
		quat_to_matrix(&pose[eye].Orientation.x, rot_mat);
	//	glMultMatrixf(rot_mat);
		/* translate the view matrix with the positional tracking */
		glTranslatef(-pose[eye].Position.x, -pose[eye].Position.y, -pose[eye].Position.z);
		/* move the camera to the eye level of the user */
		glTranslatef(0, -ovrHmd_GetFloat(hmd, OVR_KEY_EYE_HEIGHT, 1.65), 0);

		/* finally draw the scene for this eye */
                glRotatef(GRAPHICS_RotateX,1.0,0.0,0.0);
                DrawSceneLatency.Before();
		OCULUS_DrawScene();
                DrawSceneLatency.After();
	}

        EyeLoopLatency.After();

	/* after drawing both eyes into the texture render target, revert to drawing directly to the
	 * display, and we call ovrHmd_EndFrame, to let the Oculus SDK draw both images properly
	 * compensated for lens distortion and chromatic abberation onto the HMD screen.
	 */
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, win_width, win_height);

        HMD_EndFrameLatency.Before();
	ovrHmd_EndFrame(hmd, pose, &fb_ovr_tex[0].Texture);
        HMD_EndFrameLatency.After();

	assert(glGetError() == GL_NO_ERROR);

        DisplayLatency.After();
}

/******************************************************************************/

void GraphicsDisplay( void )
{
    // Mark time before we start drawing the graphics scene.
    DrawLatency.Before();

    // The good stuff goes here...
    OCULUS_Display();

    // Mark time now that scene has been drawn.
    DrawLatency.After();

    // Display the graphics buffer we've just drawn.
    SwapBufferLatency.Before();
    //glutSwapBuffers();
    SwapBufferLatency.After();
}

/******************************************************************************/

BOOL OCULUS_DisplayToggleFlag=FALSE;
int  OCULUS_DisplayPrimaryX=1;
int  OCULUS_DisplayPrimaryY=1;

void OCULUS_DisplaySecondary( void )
{
    printf("WindowPosition: x=%d, y=%d\n",hmd->WindowsPos.x,hmd->WindowsPos.y);
    printf("WindowSize: w=%d, h=%d\n",hmd->Resolution.w,hmd->Resolution.h);
    glutPositionWindow(hmd->WindowsPos.x,hmd->WindowsPos.y);
    glutReshapeWindow(hmd->Resolution.w,hmd->Resolution.h);
}

/******************************************************************************/

void OCULUS_DisplayPrimary( void )
{
    printf("WindowPosition: x=%d, y=%d\n",OCULUS_DisplayPrimaryX,OCULUS_DisplayPrimaryY);
    printf("WindowSize: w=%d, h=%d\n",hmd->Resolution.w,hmd->Resolution.h);
    glutPositionWindow(OCULUS_DisplayPrimaryX,OCULUS_DisplayPrimaryY);
    glutReshapeWindow(hmd->Resolution.w,hmd->Resolution.h);
}

/******************************************************************************/

void OCULUS_DisplayToggle( void )
{
    OCULUS_DisplayToggleFlag = !OCULUS_DisplayToggleFlag;

    if( OCULUS_DisplayToggleFlag )
    {
        OCULUS_DisplaySecondary();
    }
    else
    {
        OCULUS_DisplayPrimary();
    }
}

/******************************************************************************/

void toggle_hmd_fullscreen(void)
{
static int fullscr=FALSE,prev_x,prev_y;

    fullscr = !fullscr;

    if( fullscr )
    {
        // Go fullscreen on the rift. Save current window position, and move it
        // to the rift's part of the desktop before going fullscreen

        printf("hmd->WindowsPos.x=%d, hmd->WindowsPos.y=%d\n",hmd->WindowsPos.x,hmd->WindowsPos.y);
        printf("hmd->Resolution.w=%d, hmd->Resolution.h=%d\n",hmd->Resolution.w,hmd->Resolution.h);
        glutPositionWindow(hmd->WindowsPos.x,hmd->WindowsPos.y);
        glutReshapeWindow(hmd->Resolution.w,hmd->Resolution.h);

        //glutFullScreen();

        //SDL_GetWindowPosition(win,&prev_x,&prev_y);
        //SDL_SetWindowPosition(win,hmd->WindowsPos.x,hmd->WindowsPos.y);
        //SDL_SetWindowSize(win,1920,1080);
        //SDL_SetWindowBordered(win,SDL_FALSE);
    }
    else
    {
        // Return to windowed mode and move the window back to its original position 
        OCULUS_DisplayPrimary();
        //SDL_SetWindowFullscreen(win,0);
        //SDL_SetWindowPosition(win,prev_x,prev_y);
    }
}

/******************************************************************************/

void GraphicsKeyboard( unsigned char key, int x, int y )
{
    // Process keyboard input.
    switch( key )
    {
        case ESC : 
            ProgramExit();

        case 'f':		// Press 'f' to move the window to the HMD.
        case 'F':
            toggle_hmd_fullscreen();
            break;
    }
}

/******************************************************************************/

void update_rtarg(int width, int height)
{
// Update_rtarg creates (and/or resizes) the render target used to draw the two stereo views


    if( !fbo )
    {
        // If fbo does not exist, then nothing does... create every OpenGL object

        glGenFramebuffers(1,&fbo);

        glGenTextures(1,&fb_tex);

        glGenRenderbuffers(1,&fb_depth);


        glBindTexture(GL_TEXTURE_2D,fb_tex);

        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);

        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);

    }


    glBindFramebuffer(GL_FRAMEBUFFER, fbo);


    // Calculate the next power of two in both dimensions and use that as a texture size */
    fb_tex_width = next_pow2(width);
    fb_tex_height = next_pow2(height);


    // Create and attach the texture that will be used as a color buffer */
    glBindTexture(GL_TEXTURE_2D,fb_tex);
    glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,fb_tex_width,fb_tex_height,0,GL_RGBA,GL_UNSIGNED_BYTE,0);
    glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,fb_tex,0);


    // Create and attach the renderbuffer that will serve as our z-buffer */
    glBindRenderbuffer(GL_RENDERBUFFER,fb_depth);
    glRenderbufferStorage(GL_RENDERBUFFER,GL_DEPTH_COMPONENT,fb_tex_width,fb_tex_height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,GL_RENDERBUFFER,fb_depth);


    if( glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE )
    {
        printf("Incomplete framebuffer!\n");
    }


    glBindFramebuffer(GL_FRAMEBUFFER,0);
    printf("Created render target: %d x %d (texture size: %d x %d)\n",width,height,fb_tex_width,fb_tex_height);
}

/******************************************************************************/

BOOL OCULUS_Start( void )
{
int i, x, y;
unsigned int flags, dcaps;
union ovrGLConfig glcfg;

    win_width = hmd->Resolution.w;
    win_height = hmd->Resolution.h;

    // Configure (enable or disable) position and rotation tracking.
    //ovrHmd_ConfigureTracking(hmd,0xffffffff,0);
    ovrHmd_ConfigureTracking(hmd,0,0);

    // Retrieve the optimal render target resolution for each eye
    eyeres[0] = ovrHmd_GetFovTextureSize(hmd,ovrEye_Left,hmd->DefaultEyeFov[0],1.0);
    eyeres[1] = ovrHmd_GetFovTextureSize(hmd,ovrEye_Right,hmd->DefaultEyeFov[1],1.0);

    for( i=0; (i < 2); i++ )
    {
        printf("Eye[%d] Resolution: wid=%d hgt=%d\n",i,eyeres[i].w,eyeres[i].h);
    }

    // Create a single render target texture to encompass both eyes
    fb_width = eyeres[0].w + eyeres[1].w;
    fb_height = eyeres[0].h > eyeres[1].h ? eyeres[0].h : eyeres[1].h;
    update_rtarg(fb_width,fb_height);

    // Fill in the ovrGLTexture structures that describe our render target texture
    for( i=0; (i < 2); i++ )
    {
        fb_ovr_tex[i].OGL.Header.API = ovrRenderAPI_OpenGL;
        fb_ovr_tex[i].OGL.Header.TextureSize.w = fb_tex_width;
        fb_ovr_tex[i].OGL.Header.TextureSize.h = fb_tex_height;

        // This next field is the only one that differs between the two eyes
        fb_ovr_tex[i].OGL.Header.RenderViewport.Pos.x = i == 0 ? 0 : fb_width / 2.0;
        fb_ovr_tex[i].OGL.Header.RenderViewport.Pos.y = fb_tex_height - fb_height;
        fb_ovr_tex[i].OGL.Header.RenderViewport.Size.w = fb_width / 2.0;
        fb_ovr_tex[i].OGL.Header.RenderViewport.Size.h = fb_height;

        // Both eyes will use the same texture id
        fb_ovr_tex[i].OGL.TexId = fb_tex;
    }

    // Fill in the ovrGLConfig structure needed by the SDK to draw our stereo pair
    // to the actual HMD display (SDK-distortion mode)
    memset(&glcfg, 0, sizeof glcfg);
    glcfg.OGL.Header.API = ovrRenderAPI_OpenGL;
    glcfg.OGL.Header.RTSize = hmd->Resolution;
    glcfg.OGL.Header.Multisample = 1;


    if( hmd->HmdCaps & ovrHmdCap_ExtendDesktop )
    {
        printf("Running in \"extended desktop\" mode\n");
    }
    else
    {
        // To use the HMD display in "direct-hmd" mode, we have to call ovrHmd_AttachToWindow
        // This doesn't work properly yet due to bugs in the oculus 0.4.1 sdk/driver
#ifdef WIN32
        HWND sys_win = GetActiveWindow();
        glcfg.OGL.Window = sys_win;
        glcfg.OGL.DC = wglGetCurrentDC();
        ovrHmd_AttachToWindow(hmd, sys_win, 0, 0);
#endif
        printf("Running in \"direct-hmd\" mode\n");
    }

    // Enable low-persistence display and dynamic prediction for lattency compensation
    ovrHmd_SetEnabledCaps(hmd,ovrHmdCap_LowPersistence | ovrHmdCap_DynamicPrediction);

    // Configure SDK-rendering and enable chromatic abberation correction, vignetting, and
    // timewrap, which shifts the image before drawing to counter any lattency between the call
    // to ovrHmd_GetEyePose and ovrHmd_EndFrame.


    dcaps = ovrDistortionCap_Chromatic | ovrDistortionCap_Vignette | ovrDistortionCap_TimeWarp | ovrDistortionCap_Overdrive;

    if( !ovrHmd_ConfigureRendering(hmd,&glcfg.Config,dcaps,hmd->DefaultEyeFov,eye_rdesc) )
    {
        printf("Failed to configure distortion renderer\n");
    }


    for( int eye=0; (eye < 2); eye++ )
    {
        printf("Eye[%d]: x=%.2lf,y=%.2lf,z=%.2lf\n",eye,eye_rdesc[eye].ViewAdjust.x, eye_rdesc[eye].ViewAdjust.y, eye_rdesc[eye].ViewAdjust.z);
    }


    //eye_rdesc[0].ViewAdjust.x = 0.035;
    //eye_rdesc[1].ViewAdjust.x = -0.035;

    //* Disable the retarded "health and safety warning"
    ovrhmd_EnableHSWDisplaySDKRender(hmd,0);

    return(TRUE);
}

/******************************************************************************/

void GraphicsStart( void )
{
//int glut=GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE;
int glut=GLUT_RGB | GLUT_DEPTH | GLUT_SINGLE;
int ID;
char *name="HMD";

    if( !OCULUS_Init() )
    {
        return;
    }

    //GRAPHICS_GlutInit(glut,hmd->Resolution.w,hmd->Resolution.h);
    GRAPHICS_GlutInit(glut,0,0);
    ID = glutCreateWindow(name);

    OCULUS_DisplayPrimary();

    glewInit();

    if( !OCULUS_Start() )
    {
        return;
    }

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHT1);
    glEnable(GL_NORMALIZE);
//    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE,GL_TRUE);
//    glShadeModel(GL_FLAT);

    glClearColor(0.1,0.1,0.1,1.0);

    GRAPHICS_WidthX = GRAPHICS_MaxX - GRAPHICS_MinX;
    GRAPHICS_WidthY = GRAPHICS_MaxY - GRAPHICS_MinY;
    GRAPHICS_WidthZ = GRAPHICS_MaxZ - GRAPHICS_MinZ;

    GRAPHICS_CentreX = GRAPHICS_MinX + (GRAPHICS_WidthX / 2.0);
    GRAPHICS_CentreY = GRAPHICS_MinY + (GRAPHICS_WidthY / 2.0);
    GRAPHICS_CentreZ = GRAPHICS_MinZ + (GRAPHICS_WidthZ / 2.0);

    GRAPHICS_Centre(1,1) = GRAPHICS_CentreX;
    GRAPHICS_Centre(2,1) = GRAPHICS_CentreY;
    GRAPHICS_Centre(3,1) = GRAPHICS_CentreZ;

    disp(GRAPHICS_Centre);

    CALIBRATE_GridBuild();

    SpherePosition = GRAPHICS_Centre;
    SpherePosition(2,1) -= 0.10; // Lower...

    glutKeyboardFunc(KB_GLUT_KeyboardFuncInstall(GraphicsKeyboard));
    glutDisplayFunc(GraphicsDisplay);
    glutIdleFunc(GraphicsIdle);

    // Reset frequency timing objects.
    DisplayFrequency.Reset();
    IdleFrequency.Reset();

    // Give control to GLUT's main loop.
    glutMainLoop();
}

/******************************************************************************/

void RobotForcesFunction( matrix &P, matrix &V, matrix &F )
{
int i;
static matrix D(3,1);
static double d,e;

    if( LOOPTASK_First )
    {
        RobotControlLoopFrequency.Reset();
    }

    RobotControlLoopFrequency.Loop();

    RobotControlLoopLatency.Before();

    RobotPositionRaw = P;
    RobotPosition = P;// - RobotPositionOffset;
    RobotVelocity = V;
    RobotForces.zeros();

    // Calculate robot position in graphics co-orindate system.
    for( i=1; (i <= 3); i++ )
    {
        RobotPositionGraphics(i,1) = (RobotPosition(i,1) / 100.0) - RobotPositionOffset(i,1) + GRAPHICS_Centre(i,1);
    }

    /*if( TIMER_EveryHz(2.0) )
    {
        printf("RobotPosition(xyz): %.2lf,%.2lf,%.2lf\n",RobotPosition(1,1),RobotPosition(2,1),RobotPosition(3,1));
        printf("RobotPositionGraphics(xyz): %.2lf,%2.lf,%.2lf\n",RobotPositionGraphics(1,1),RobotPositionGraphics(2,1),RobotPositionGraphics(3,1));
    }*/

    D = RobotPositionGraphics - SpherePosition;
    d = norm(D);
    e = d - SphereRadius; // Scalar encroachment into surface of sphere.

    if( ROBOT_JustActivated(RobotID) )
    {
        SphereInsideFlag = (e < 0);
    }

    if( SphereInsideFlag )
    {
        e = d - (SphereRadius-CursorRadius);
    }
    else
    {
        e = d - (SphereRadius+CursorRadius);
    }

    if( (SphereInsideFlag && (e > 0.0)) || (!SphereInsideFlag && (e < 0.0)) )
    {
        RobotForces = RobotSpringConstant * e * (D/d);
    }

    RobotForces.clampnorm(RobotForceMax);

    F = RobotForces; 

    RobotControlLoopLatency.After();
}

/******************************************************************************/

BOOL RobotStart( void )
{
BOOL ok=FALSE;

    // Offset to get robot centred in graphics workspace.
    RobotPositionOffset(1,1) = 0.0;
    RobotPositionOffset(2,1) = 0.30;
    RobotPositionOffset(3,1) = 0.70;

    if( (RobotID=ROBOT_Open(RobotName)) == ROBOT_INVALID )
    {
        printf("Cannot open: \n",RobotName);
    }
    else
    if( !ROBOT_Start(RobotID,RobotForcesFunction) )
    {
        printf("Cannot start: %s\n",RobotName);
    }
    else
    {
        ok = TRUE;
        printf("Robot started: %s\n",RobotName);
    }

    return(ok);
}

/******************************************************************************/

void RobotStop( void )
{
BOOL flag;

    flag = ROBOT_Started(RobotID);

    ROBOT_Stop(RobotID);
    ROBOT_Close(RobotID);

    if( flag )
    {
        RobotControlLoopFrequency.Results();
        RobotControlLoopLatency.Results();
    }
}

/******************************************************************************/

void ProgramExit( void )
{
    RobotStop();
    OCULUS_Stop();

    // Print results...
    TimingResults();

    // Exit program...
    exit(0);
}

/******************************************************************************/

void Usage( void )
{
    printf("Usage:\n\n");
    printf("%s /R:RobotName\n",MODULE_NAME);

    exit(0);
}

/******************************************************************************/

BOOL Parameters( int argc, char *argv[] )
{
BOOL ok;
char *data;
int i;

    for( ok=TRUE,i=1; ((i < argc) && ok); i++ )
    {
        switch( CMDARG_code(argv[i],&data) )
        {
            case 'R' :
               ok = CMDARG_data(RobotName,data,STRLEN);
               break;

            case '?' :
               Usage();
               break;

            default :
               ok = FALSE;
               break;
        }

        if( !ok )
        {
            printf("Invalid argument: %s\n",argv[i]);
        }
    }

    if( ok && STR_null(RobotName) )
    {
        printf("Robot name not specified.\n");
        ok = FALSE;
    }

    if( !ok )
    {
        Usage();
    }

    return(ok);
}

/******************************************************************************/

void main( int argc, char *argv[] )
{
    // Process command-line parameteres.
    if( !Parameters(argc,argv) )
    {
        exit(0);
    }

    if( !RobotStart() )
    {
        exit(0);
    }

    // Start graphics.
    GraphicsStart();
}

/******************************************************************************/

void GraphicsSphere( matrix &posn, double radius, int color )
{
    GRAPHICS_ColorSet(color);
    glPushMatrix();
    glTranslatef(posn(1,1),posn(2,1),posn(3,1));
    glScalef(radius,radius,radius);
    GraphicsSphereDraw();
    glPopMatrix();
}

/******************************************************************************/

void GraphicsWireSphere( matrix &posn, double radius, int color )
{
    GRAPHICS_ColorSet(color);
    glDisable(GL_LIGHTING);
    glPushMatrix();
    glTranslatef(posn(1,1),posn(2,1),posn(3,1));
    glRotatef(90,1.0,0.0,0.0);
    glScalef(radius,radius,radius);
    GraphicsWireSphereDraw();
    glPopMatrix();
    glEnable(GL_LIGHTING);
}

/******************************************************************************/

int GraphicsSphereSegments=20;
int GraphicsSolidSphereList;
int GraphicsWireSphereList;
GLUquadricObj *GraphicsSphereObject;

void GraphicsSphereDraw( void )
{
    if( !glIsList(GraphicsSolidSphereList) )
    {
        GraphicsSolidSphereList = glGenLists(1);
        glNewList(GraphicsSolidSphereList,GL_COMPILE);
        GraphicsSphereObject = gluNewQuadric();
        gluQuadricNormals(GraphicsSphereObject,GLU_SMOOTH);
        gluQuadricTexture(GraphicsSphereObject,GL_TRUE);
        gluSphere(GraphicsSphereObject,1.0,GraphicsSphereSegments,GraphicsSphereSegments);
        gluDeleteQuadric(GraphicsSphereObject);          
        glEndList();
    }
 
    if( glIsList(GraphicsSolidSphereList) )
    {
        glCallList(GraphicsSolidSphereList);
    }
}

/******************************************************************************/

void GraphicsWireSphereDraw( void )
{
    if( !glIsList(GraphicsWireSphereList) )
    {
        GraphicsWireSphereList = glGenLists(1);
        glNewList(GraphicsWireSphereList,GL_COMPILE);
        GraphicsSphereObject = gluNewQuadric();
        gluQuadricDrawStyle(GraphicsSphereObject,GLU_LINE);
        gluSphere(GraphicsSphereObject,1.0,GraphicsSphereSegments,GraphicsSphereSegments);
        gluDeleteQuadric(GraphicsSphereObject);          
        glEndList();
    }
 
    if( glIsList(GraphicsWireSphereList) )
    {
        glCallList(GraphicsWireSphereList);
    }
}

/******************************************************************************/

