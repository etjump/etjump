// contents flags are seperate bits
// a given brush can contribute multiple content bits

constexpr int CONTENTS_SOLID        = 0x00000001;
//                                  = 0x00000002 unused
constexpr int CONTENTS_LIGHTGRID    = 0x00000004;
constexpr int CONTENTS_LAVA         = 0x00000008;
constexpr int CONTENTS_SLIME        = 0x00000010;
constexpr int CONTENTS_WATER        = 0x00000020;
constexpr int CONTENTS_FOG          = 0x00000040;
constexpr int CONTENTS_MISSILECLIP  = 0x00000080;
constexpr int CONTENTS_ITEM         = 0x00000100;
//                                  = 0x00000200 unused
//                                  = 0x00000400 unused
//                                  = 0x00000800 unused
//                                  = 0x00001000 unused, AI_NOSIGHT in q3map2
//                                  = 0x00002000 unused, CLIPSHOT in q3map2, OBS! compiles to solid
constexpr int CONTENTS_MOVER        = 0x00004000;
constexpr int CONTENTS_AREAPORTAL   = 0x00008000;
constexpr int CONTENTS_PLAYERCLIP   = 0x00010000;
constexpr int CONTENTS_MONSTERCLIP  = 0x00020000;
//                                  = 0x00040000 unused, was CONTENTS_TELEPORTER
constexpr int CONTENTS_PORTALCLIP   = 0x00080000; // etjump, was JUMPPAD, can't fire portalgun through this
constexpr int CONTENTS_NOSAVE       = 0x00100000; // etjump, was CLUSTERPORTAL.
constexpr int CONTENTS_NOPRONE      = 0x00200000; // etjump, was DONOTENTER
constexpr int CONTENTS_NONOCLIP     = 0x00400000; // etjump, was DONOTENTER_LARGE, BOTCLIP in q3map2
//                                  = 0x00800000 unused
constexpr int CONTENTS_ORIGIN       = 0x01000000; // removed before bsping an entity
constexpr int CONTENTS_BODY         = 0x02000000; // should never be on a brush, only in game
constexpr int CONTENTS_CORPSE       = 0x04000000;
constexpr int CONTENTS_DETAIL       = 0x08000000; // brushes not used for the bsp
constexpr int CONTENTS_STRUCTURAL   = 0x10000000; // brushes used for the bsp
constexpr int CONTENTS_TRANSLUCENT  = 0x20000000; // don't consume surface fragments inside
constexpr int CONTENTS_TRIGGER      = 0x40000000;
constexpr int CONTENTS_NODROP       = 0x80000000; // don't leave bodies or items (death fog, lava)

#define SURF_NODAMAGE 0x00000001 // never give falling damage
#define SURF_SLICK 0x00000002    // effects game physics
#define SURF_SKY 0x00000004      // lighting from environment map
#define SURF_LADDER 0x00000008
#define SURF_NOIMPACT 0x00000010 // don't make missile explosions
#define SURF_NOMARKS 0x00000020  // don't leave missile marks
#define SURF_SPLASH                                                            \
  0x00000040 // out of surf's, so replacing unused 'SURF_FLESH' - and as
             // SURF_CERAMIC wasn't used, it's now SURF_SPLASH
#define SURF_NODRAW 0x00000080 // don't generate a drawsurface at all
#define SURF_HINT 0x00000100   // make a primary bsp splitter
#define SURF_SKIP 0x00000200   // completely ignore, allowing non-closed brushes
#define SURF_NOLIGHTMAP 0x00000400 // surface doesn't need a lightmap
#define SURF_POINTLIGHT 0x00000800 // generate lighting info at vertexes
#define SURF_METAL 0x00001000      // clanking footsteps
#define SURF_NOSTEPS 0x00002000    // no footstep sounds
#define SURF_NONSOLID 0x00004000   // don't collide against curves with this set
#define SURF_LIGHTFILTER 0x00008000 // act as a light filter during q3map -light
#define SURF_ALPHASHADOW                                                       \
  0x00010000 // do per-pixel light shadow casting in q3map
#define SURF_NODLIGHT                                                          \
  0x00020000 // don't dlight even if solid (solid lava, skies)
#define SURF_WOOD 0x00040000
#define SURF_GRASS 0x00080000
#define SURF_GRAVEL 0x00100000
#define SURF_GLASS                                                             \
  0x00200000 // out of surf's, so replacing unused 'SURF_SMGROUP'
#define SURF_SNOW 0x00400000
#define SURF_ROOF 0x00800000
#define SURF_RUBBLE                                                            \
  0x01000000 // Zero: UNUSED. Only used in a piece of code that's never
             // reached
#define SURF_CARPET 0x02000000
#define SURF_MONSTERSLICK                                                      \
  0x04000000 // slick surf that only affects ai's // Zero: this could be
             // refactored to be usable for ETJump purposes
/*
#define SURF_MONSLICK_W			0x08000000  //Feen: MONSLICK_W ->
MONSLICK_S don't appear to be in use..
#define SURF_MONSLICK_N			0x10000000  //
However, MONSTERSLICK does... #define SURF_MONSLICK_E 0x20000000 #define
SURF_MONSLICK_S			0x40000000
*/

// Feen: New ETJump Surfaces...
#define SURF_PORTALGATE                                                        \
  0x08000000 // Feen: I hereby declare these's SURF's in the name of
             // ETJump!
#define SURF_MONSLICK_N 0x10000000 // ETJump: SURF_NOJUMPDELAY
#define SURF_MONSLICK_E                                                        \
  0x20000000 // Zero: I hereby declare this SURF as a
             // portalable/unportalable surface (depends on
             // level.portalSurfaces)
#define SURF_MONSLICK_S 0x40000000 // Zero: see SURF_OVERBOUNCE
const int SURF_OVERBOUNCE = 0x40000000;
const int SURF_NOJUMPDELAY = 0x10000000;
// END New ETJump Surfaces

#define SURF_LANDMINE 0x80000000 // ydnar: ok to place landmines on this surface
