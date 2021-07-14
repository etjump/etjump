// This file must be identical in the quake and utils directories






// contents flags are seperate bits
// a given brush can contribute multiple content bits

// these definitions also need to be in q_shared.h!

// CONTENTS_SOLID              0x00000001 - vanilla
//                             0x00000002 - unknown
// CONTENTS_LIGHTGRID          0x00000004 - compiler
// CONTENTS_LAVA               0x00000008 - vanilla
// CONTENTS_SLIME              0x00000010 - vanilla
// CONTENTS_WATER              0x00000020 - vanilla
// CONTENTS_FOG                0x00000040 - vanilla
// CONTENTS_MISSILECLIP        0x00000080 - vanilla
// CONTENTS_ITEM               0x00000100 - vanilla
//                             0x00000200 - unknown
//                             0x00000400 - unknown
//                             0x00000800 - unknown
//                             0x00001000 - unknown
//                             0x00002000 - unknown
// CONTENTS_MOVER              0x00004000 - vanilla
// CONTENTS_AREAPORTAL         0x00008000 - vanilla
// CONTENTS_PLAYERCLIP         0x00010000 - vanilla
// CONTENTS_MONSTERCLIP        0x00020000 - etjump: phase brush b
// CONTENTS_TELEPORTER         0x00040000 - UNUSED (not recognized by q3map2)
// CONTENTS_NOPORTAL           0x00080000 - etjump: portalgun
// CONTENTS_NOSAVE             0x00100000 - etjump: nosave
// CONTENTS_DONOTENTER         0x00200000 - etjump: noprone
// CONTENTS_DONOTENTER_LARGE   0x00400000 - etjump: phase brush a
//                             0x00800000 - unknown
// CONTENTS_ORIGIN             0x01000000 - compiler
// CONTENTS_BODY               0x02000000 - vanilla
// CONTENTS_CORPSE             0x04000000 - vanilla
// CONTENTS_DETAIL             0x08000000 - compiler
// CONTENTS_STRUCTURAL         0x10000000 - compiler
// CONTENTS_TRANSLUCENT        0x20000000 - compiler
// CONTENTS_TRIGGER            0x40000000 - vanilla
// CONTENTS_NODROP             0x80000000 - vanilla

// unknown and compiler contents could probably be used if they are applied during for brush entities.

#define CONTENTS_SOLID              0x00000001
#define CONTENTS_LIGHTGRID          0x00000004
#define CONTENTS_LAVA               0x00000008
#define CONTENTS_SLIME              0x00000010
#define CONTENTS_WATER              0x00000020
#define CONTENTS_FOG                0x00000040
#define CONTENTS_MISSILECLIP        0x00000080
#define CONTENTS_ITEM               0x00000100
#define CONTENTS_MOVER              0x00004000
#define CONTENTS_AREAPORTAL         0x00008000
#define CONTENTS_PLAYERCLIP         0x00010000
#define CONTENTS_MONSTERCLIP        0x00020000
const int CONTENTS_PHASE_B =        0x00020000;
#define CONTENTS_TELEPORTER         0x00040000  //NOT USED EITHER....
#define CONTENTS_NOPORTAL           0x00080000  // Feen: PGM - Contents formerly known as CONTENTS_JUMPPAD - Used for 'emancipation grid'
#define CONTENTS_NOSAVE             0x00100000  // CONTENTS_NOSAVE
#define CONTENTS_DONOTENTER         0x00200000	// Aciz: renamed back to original, was renamed to CONTENTS_NOSAVERESET but never used anywhere.
const int CONTENTS_NOPRONE =        0x00200000;
#define CONTENTS_DONOTENTER_LARGE   0x00400000  
const int CONTENTS_PHASE_A =        0x00400000;
#define CONTENTS_ORIGIN             0x01000000  // removed before bsping an entity
#define CONTENTS_BODY               0x02000000  // should never be on a brush, only in game
#define CONTENTS_CORPSE             0x04000000
#define CONTENTS_DETAIL             0x08000000  // brushes not used for the bsp

#define CONTENTS_STRUCTURAL     0x10000000  // brushes used for the bsp
#define CONTENTS_TRANSLUCENT    0x20000000  // don't consume surface fragments inside
#define CONTENTS_TRIGGER        0x40000000
#define CONTENTS_NODROP         0x80000000  // don't leave bodies or items (death fog, lava)

#define SURF_NODAMAGE           0x00000001  // never give falling damage
#define SURF_SLICK              0x00000002  // effects game physics
#define SURF_SKY                0x00000004  // lighting from environment map
#define SURF_LADDER             0x00000008
#define SURF_NOIMPACT           0x00000010  // don't make missile explosions
#define SURF_NOMARKS            0x00000020  // don't leave missile marks
#define SURF_SPLASH             0x00000040  // out of surf's, so replacing unused 'SURF_FLESH' - and as SURF_CERAMIC wasn't used, it's now SURF_SPLASH
#define SURF_NODRAW             0x00000080  // don't generate a drawsurface at all
#define SURF_HINT               0x00000100  // make a primary bsp splitter
#define SURF_SKIP               0x00000200  // completely ignore, allowing non-closed brushes
#define SURF_NOLIGHTMAP         0x00000400  // surface doesn't need a lightmap
#define SURF_POINTLIGHT         0x00000800  // generate lighting info at vertexes
#define SURF_METAL              0x00001000  // clanking footsteps
#define SURF_NOSTEPS            0x00002000  // no footstep sounds
#define SURF_NONSOLID           0x00004000  // don't collide against curves with this set
#define SURF_LIGHTFILTER        0x00008000  // act as a light filter during q3map -light
#define SURF_ALPHASHADOW        0x00010000  // do per-pixel light shadow casting in q3map
#define SURF_NODLIGHT           0x00020000  // don't dlight even if solid (solid lava, skies)
#define SURF_WOOD               0x00040000
#define SURF_GRASS              0x00080000
#define SURF_GRAVEL             0x00100000
#define SURF_GLASS              0x00200000  // out of surf's, so replacing unused 'SURF_SMGROUP'
#define SURF_SNOW               0x00400000
#define SURF_ROOF               0x00800000
#define SURF_RUBBLE             0x01000000 //Zero: UNUSED. Only used in a piece of code that's never reached
#define SURF_CARPET             0x02000000
#define SURF_MONSTERSLICK       0x04000000  // slick surf that only affects ai's // Zero: this could be refactored to be usable for ETJump purposes
/*
#define SURF_MONSLICK_W			0x08000000  //Feen: MONSLICK_W -> MONSLICK_S don't appear to be in use..
#define SURF_MONSLICK_N			0x10000000  //		However, MONSTERSLICK does...
#define SURF_MONSLICK_E			0x20000000
#define SURF_MONSLICK_S			0x40000000
*/

//Feen: New ETJump Surfaces...   200000000
#define SURF_PORTALGATE         0x08000000  //Feen: I hereby declare these's SURF's in the name of ETJump!
#define SURF_MONSLICK_N         0x10000000  //ETJump: SURF_NOJUMPDELAY
#define SURF_MONSLICK_E         0x20000000  //Zero: I hereby declare this SURF as a portalable/unportalable surface (depends on level.portalSurfaces)
#define SURF_MONSLICK_S         0x40000000	//Zero: see SURF_OVERBOUNCE
const int SURF_OVERBOUNCE =		0x40000000;
const int SURF_NOJUMPDELAY =    0x10000000;
//END New ETJump Surfaces

#define SURF_LANDMINE           0x80000000  // ydnar: ok to place landmines on this surface
