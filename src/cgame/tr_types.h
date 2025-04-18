#ifndef __TR_TYPES_H
#define __TR_TYPES_H

// renderer limit for a scene
inline constexpr int MAX_CORONAS = 32;
// renderer limit for a scene
inline constexpr int MAX_DLIGHTS = 32;

// can't be increased without changing drawsurf bit packing
inline constexpr int MAX_ENTITIES = 1023;

// renderfx flags

// always have some light (viewmodel, some items)
inline constexpr int RF_MINLIGHT = 0x000001;
// don't draw through eyes, only mirrors (player bodies, chat sprites)
inline constexpr int RF_THIRD_PERSON = 0x000002;
// only draw through eyes (view weapon, damage blood blob)
inline constexpr int RF_FIRST_PERSON = 0x000004;
// for view weapon Z crunching
inline constexpr int RF_DEPTHHACK = 0x000008;
// don't add stencil shadows
inline constexpr int RF_NOSHADOW = 0x000010;
// use refEntity->lightingOrigin instead of refEntity->origin for lighting.
// This allows entities to sink into the floor with their origin going solid,
// and allows all parts of a player to get the same lighting
inline constexpr int RF_LIGHTING_ORIGIN = 0x000020;
// use refEntity->shadowPlane
inline constexpr int RF_SHADOW_PLANE = 0x000040;
// mod the model frames by the maxframes to allow continuous animation
// without needing to know the frame count
inline constexpr int RF_WRAP_FRAMES = 0x000080;
// more than RF_MINLIGHT, for when an object is "highlighted"
// (looked at/training identification/etc)
inline constexpr int RF_HILIGHT = 0x000100;
// eyes in 'blink' state
inline constexpr int RF_BLINK = 0x000200;
inline constexpr int RF_FORCENOLOD = 0x000400;

// refdef flags

// used for player configuration screen
inline constexpr int RDF_NOWORLDMODEL = 1;
// teleportation effect (unused in ET)
inline constexpr int RDF_HYPERSPACE = 4;
inline constexpr int RDF_SKYBOXPORTAL = 8;
// so the renderer knows to use underwater fog when the player is underwater
inline constexpr int RDF_UNDERWATER = 1 << 4;
inline constexpr int RDF_DRAWINGSKY = 1 << 5;
inline constexpr int RDF_SNOOPERVIEW = 1 << 6;

typedef struct {
  vec3_t xyz;
  float st[2];
  byte modulate[4];
} polyVert_t;

typedef struct poly_s {
  qhandle_t hShader;
  int numVerts;
  polyVert_t *verts;
} poly_t;

typedef enum {
  RT_MODEL,
  RT_POLY,
  RT_SPRITE,
  RT_SPLASH, // ripple effect
  RT_BEAM,
  RT_RAIL_CORE,
  RT_RAIL_CORE_TAPER, // a modified core that creates a properly texture
                      // mapped core that's wider at one end
  RT_RAIL_RINGS,
  RT_LIGHTNING,
  RT_PORTALSURFACE, // doesn't draw anything, just info for portals

  RT_MAX_REF_ENTITY_TYPE
} refEntityType_t;

// only draw hand surfaces
inline constexpr int REFLAG_ONLYHAND = 1;
// force a low lod
inline constexpr int REFLAG_FORCE_LOD = 8;
// on LOD switch, align the model to the player's camera
inline constexpr int REFLAG_ORIENT_LOD = 16;
// allow the LOD to go lower than recommended
inline constexpr int REFLAG_DEAD_LOD = 32;

typedef struct {
  refEntityType_t reType;
  int renderfx;

  qhandle_t hModel; // opaque type outside refresh

  // most recent data
  vec3_t lightingOrigin; // so multi-part models can be lit identically
                         // (RF_LIGHTING_ORIGIN)
  float shadowPlane;     // projection shadows go here, stencils go slightly
                         // lower

  vec3_t axis[3];             // rotation vectors
  vec3_t torsoAxis[3];        // rotation vectors for torso section of skeletal
                              // animation
  qboolean nonNormalizedAxes; // axis are not normalized, i.e. they have
                              // scale
  float origin[3];            // also used as MODEL_BEAM's "from"
  int frame;                  // also used as MODEL_BEAM's diameter
  qhandle_t frameModel;
  int torsoFrame; // skeletal torso can have frame independant of legs
                  // frame
  qhandle_t torsoFrameModel;

  // previous data for frame interpolation
  float oldorigin[3]; // also used as MODEL_BEAM's "to"
  int oldframe;
  qhandle_t oldframeModel;
  int oldTorsoFrame;
  qhandle_t oldTorsoFrameModel;
  float backlerp; // 0.0 = current, 1.0 = old
  float torsoBacklerp;

  // texturing
  int skinNum;            // inline skin index
  qhandle_t customSkin;   // NULL for default skin
  qhandle_t customShader; // use one image for the entire thing

  // misc
  byte shaderRGBA[4];      // colors used by rgbgen entity shaders
  float shaderTexCoord[2]; // texture coordinates used by tcMod entity
                           // modifiers
  float shaderTime;        // subtracted from refdef time to control effect
                           // start times

  // extra sprite information
  float radius;
  float rotation;

  // Ridah
  vec3_t fireRiseDir;

  // Ridah, entity fading (gibs, debris, etc)
  int fadeStartTime, fadeEndTime;

  float hilightIntensity; //----(SA)	added

  int reFlags;

  int entityNum; // currentState.number, so we can attach rendering
                 // effects to specific entities (Zombie)

} refEntity_t;

//----(SA)

//                                                                  //
// WARNING:: synch FOG_SERVER in sv_ccmds.c if you change anything	//
//                                                                  //
typedef enum {
  FOG_NONE, //	0

  FOG_SKY,        //	1	fog values to apply to the sky when using density
                  // fog
                  // for the world (non-distance clipping fog) (only used
                  // if(glfogsettings[FOG_MAP].registered) or
                  // if(glfogsettings[FOG_MAP].registered))
  FOG_PORTALVIEW, //	2	used by the portal sky scene
  FOG_HUD,        //	3	used by the 3D hud scene

  //		The result of these for a given frame is copied to the
  // scene.glFog when the scene is rendered

  // the following are fogs applied to the main world scene
  FOG_MAP,     //	4	use fog parameter specified using the "fogvars" in
               // the
               // sky shader
  FOG_WATER,   //	5	used when underwater
  FOG_SERVER,  //	6	the server has set my fog (probably a
               // target_fog) (keep synch in sv_ccmds.c !!!)
  FOG_CURRENT, //	7	stores the current values when a
               // transition
               // starts
  FOG_LAST,    //	8	stores the current values when a transition
               // starts
  FOG_TARGET,  //	9	the values it's transitioning to.

  FOG_CMD_SWITCHFOG, // 10	transition to the fog specified in the
                     // second parameter of R_SetFog(...) (keep synch in
                     // sv_ccmds.c
                     // !!!)

  NUM_FOGS
} glfogType_t;

typedef struct {
  int mode;       // GL_LINEAR, GL_EXP
  int hint;       // GL_DONT_CARE
  int startTime;  // in ms
  int finishTime; // in ms
  float color[4];
  float start;            // near
  float end;              // far
  qboolean useEndForClip; // use the 'far' value for the far clipping plane
  float density;          // 0.0-1.0
  qboolean registered;    // has this fog been set up?
  qboolean drawsky;       // draw skybox
  qboolean clearscreen;   // clear the GL color buffer
} glfog_t;

//----(SA)	end

inline constexpr int MAX_RENDER_STRINGS = 8;
inline constexpr int MAX_RENDER_STRING_LENGTH = 32;

typedef struct {
  int x, y, width, height;
  float fov_x, fov_y;
  vec3_t vieworg;
  vec3_t viewaxis[3]; // transformation matrix

  int time;    // time in milliseconds for shader effects and other time
               // dependent rendering issues
  int rdflags; // RDF_NOWORLDMODEL, etc

  // 1 bits will prevent the associated area from rendering at all
  byte areamask[MAX_MAP_AREA_BYTES];

  // text messages for deform text shaders
  char text[MAX_RENDER_STRINGS][MAX_RENDER_STRING_LENGTH];

  //----(SA)	added (needed to pass fog infos into the portal sky
  // scene)
  glfog_t glfog;
  //----(SA)	end

} refdef_t;

typedef enum { STEREO_CENTER, STEREO_LEFT, STEREO_RIGHT } stereoFrame_t;

/*
** glconfig_t
**
** Contains variables specific to the OpenGL configuration
** being run right now.  These are constant once the OpenGL
** subsystem is initialized.
*/
typedef enum { TC_NONE, TC_S3TC, TC_EXT_COMP_S3TC } textureCompression_t;

typedef enum {
  GLDRV_ICD,        // driver is integrated with window system
                    // WARNING: there are tests that check for
                    // > GLDRV_ICD for minidriverness, so this
                    // should always be the lowest value in this
                    // enum set
  GLDRV_STANDALONE, // driver is a non-3Dfx standalone driver
  GLDRV_VOODOO      // driver is a 3Dfx standalone driver
} glDriverType_t;

typedef enum {
  GLHW_GENERIC,   // where everthing works the way it should
  GLHW_3DFX_2D3D, // Voodoo Banshee or Voodoo3, relevant since if this
                  // is the hardware type then there can NOT exist a
                  // secondary display adapter
  GLHW_RIVA128,   // where you can't interpolate alpha
  GLHW_RAGEPRO,   // where you can't modulate alpha on alpha textures
  GLHW_PERMEDIA2  // where you don't have src*dst
} glHardwareType_t;

typedef struct {
  char renderer_string[MAX_STRING_CHARS];
  char vendor_string[MAX_STRING_CHARS];
  char version_string[MAX_STRING_CHARS];
  char extensions_string[MAX_STRING_CHARS * 4]; // TTimo - bumping, some cards
                                                // have a big extension string

  int maxTextureSize;    // queried from GL
  int maxActiveTextures; // multitexture ability

  int colorBits, depthBits, stencilBits;

  glDriverType_t driverType;
  glHardwareType_t hardwareType;

  qboolean deviceSupportsGamma;
  textureCompression_t textureCompression;
  qboolean textureEnvAddAvailable;
  qboolean anisotropicAvailable; //----(SA)	added
  float maxAnisotropy;           //----(SA)	added

  // vendor-specific support
  // NVidia
  qboolean NVFogAvailable; //----(SA)	added
  int NVFogMode;           //----(SA)	added
  // ATI
  int ATIMaxTruformTess; // for truform support
  int ATINormalMode;     // for truform support
  int ATIPointMode;      // for truform support

  int vidWidth, vidHeight;
  // aspect is the screen's physical width / height, which may be
  // different than scrWidth / scrHeight if the pixels are non-square
  // normal screens should be 4/3, but wide aspect monitors may be 16/9
  float windowAspect;

  int displayFrequency;

  // synonymous with "does rendering consume the entire screen?",
  // therefore a Voodoo or Voodoo2 will have this set to TRUE, as will a
  // Win32 ICD that used CDS.
  qboolean isFullscreen;
  qboolean stereoEnabled;
  qboolean smpActive; // dual processor
} glconfig_t;

// =========================================
// Gordon, these MUST NOT exceed the values for
// SHADER_MAX_VERTEXES/SHADER_MAX_INDEXES
inline constexpr int MAX_PB_VERTS = 1025;
inline constexpr int MAX_PB_INDICIES = MAX_PB_VERTS * 6;

typedef struct polyBuffer_s {
  vec4_t xyz[MAX_PB_VERTS];
  vec2_t st[MAX_PB_VERTS];
  byte color[MAX_PB_VERTS][4];
  int numVerts;

  int indicies[MAX_PB_INDICIES];
  int numIndicies;

  qhandle_t shader;
} polyBuffer_t;
// =========================================

#endif // __TR_TYPES_H
