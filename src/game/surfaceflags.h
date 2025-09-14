// contents flags are separate bits
// a given brush can contribute multiple content bits

#pragma once

// clang-format off

/* content bits */

inline constexpr int32_t CONTENTS_SOLID        = 0x00000001;
//                                              = 0x00000002 unused, no built-in surfaceparm
inline constexpr int32_t CONTENTS_LIGHTGRID    = 0x00000004;
inline constexpr int32_t CONTENTS_LAVA         = 0x00000008;
inline constexpr int32_t CONTENTS_SLIME        = 0x00000010;
inline constexpr int32_t CONTENTS_WATER        = 0x00000020;
inline constexpr int32_t CONTENTS_FOG          = 0x00000040;
inline constexpr int32_t CONTENTS_MISSILECLIP  = 0x00000080;
inline constexpr int32_t CONTENTS_ITEM         = 0x00000100;
//                                              = 0x00000200 unused, no built-in surfaceparm
//                                              = 0x00000400 unused, no built-in surfaceparm
//                                              = 0x00000800 unused, no built-in surfaceparm
//                                              = 0x00001000 unused, surfaceparm "ai_nosight"
//                                              = 0x00002000 unused, surfaceparm "clipshot", compiles to solid
inline constexpr int32_t CONTENTS_MOVER        = 0x00004000;
inline constexpr int32_t CONTENTS_AREAPORTAL   = 0x00008000;
inline constexpr int32_t CONTENTS_PLAYERCLIP   = 0x00010000;
inline constexpr int32_t CONTENTS_MONSTERCLIP  = 0x00020000;
//                                              = 0x00040000 unused, was CONTENTS_TELEPORTER, no built-in surfaceparm
inline constexpr int32_t CONTENTS_PORTALCLIP   = 0x00080000; // etjump, was JUMPPAD, no built-in surfaceparm
inline constexpr int32_t CONTENTS_NOSAVE       = 0x00100000; // etjump, was CLUSTERPORTAL, surfaceparm "clusterportal"
inline constexpr int32_t CONTENTS_NOPRONE      = 0x00200000; // etjump, was DONOTENTER, surfaceparm "donotenter"
inline constexpr int32_t CONTENTS_NONOCLIP     = 0x00400000; // etjump, was DONOTENTER_LARGE, BOTCLIP in q3map2, surfaceparm "donotenterlarge"
//                                              = 0x00800000 unused, no built-in surfaceparm
inline constexpr int32_t CONTENTS_ORIGIN       = 0x01000000; // removed before bsping an entity
inline constexpr int32_t CONTENTS_BODY         = 0x02000000; // should never be on a brush, only in game
inline constexpr int32_t CONTENTS_CORPSE       = 0x04000000;
inline constexpr int32_t CONTENTS_DETAIL       = 0x08000000; // brushes not used for the bsp
inline constexpr int32_t CONTENTS_STRUCTURAL   = 0x10000000; // brushes used for the bsp
inline constexpr int32_t CONTENTS_TRANSLUCENT  = 0x20000000; // don't consume surface fragments inside
inline constexpr int32_t CONTENTS_TRIGGER      = 0x40000000;
inline constexpr int32_t CONTENTS_NODROP       = 0x80000000; // don't leave bodies or items (death fog, lava)

/* surface bits */

inline constexpr int32_t SURF_NODAMAGE         = 0x00000001; // never give falling damage
inline constexpr int32_t SURF_SLICK            = 0x00000002; // effects game physics
inline constexpr int32_t SURF_SKY              = 0x00000004; // lighting from environment map
inline constexpr int32_t SURF_LADDER           = 0x00000008;
inline constexpr int32_t SURF_NOIMPACT         = 0x00000010; // don't make missile explosions
inline constexpr int32_t SURF_NOMARKS          = 0x00000020; // don't leave missile marks
inline constexpr int32_t SURF_SPLASH           = 0x00000040; // SURF_FLESH in quake
inline constexpr int32_t SURF_NODRAW           = 0x00000080; // don't generate a drawsurface at all
inline constexpr int32_t SURF_HINT             = 0x00000100; // make a primary bsp splitter
inline constexpr int32_t SURF_SKIP             = 0x00000200; // completely ignore, allowing non-closed brushes
inline constexpr int32_t SURF_NOLIGHTMAP       = 0x00000400; // surface doesn't need a lightmap
inline constexpr int32_t SURF_POINTLIGHT       = 0x00000800; // generate lighting info at vertexes
inline constexpr int32_t SURF_METAL            = 0x00001000; // clanking footsteps
inline constexpr int32_t SURF_NOSTEPS          = 0x00002000; // no footstep sounds
inline constexpr int32_t SURF_NONSOLID         = 0x00004000; // don't collide against curves with this set
inline constexpr int32_t SURF_LIGHTFILTER      = 0x00008000; // act as a light filter during q3map -light
inline constexpr int32_t SURF_ALPHASHADOW      = 0x00010000; // do per-pixel light shadow casting in q3map
inline constexpr int32_t SURF_NODLIGHT         = 0x00020000; // don't dlight even if solid (solid lava, skies)
inline constexpr int32_t SURF_WOOD             = 0x00040000;
inline constexpr int32_t SURF_GRASS            = 0x00080000;
inline constexpr int32_t SURF_GRAVEL           = 0x00100000;
inline constexpr int32_t SURF_GLASS            = 0x00200000;
inline constexpr int32_t SURF_SNOW             = 0x00400000;
inline constexpr int32_t SURF_ROOF             = 0x00800000;
//                                              = 0x01000000 unused, was RUBBLE, surfaceparm "rubble"
inline constexpr int32_t SURF_CARPET           = 0x02000000;
//                                              = 0x04000000 unused, was MONSTERSLICK, surfaceparm "monsterslick"
inline constexpr int32_t SURF_PORTALGATE       = 0x08000000; // etjump, was SURF_MONSLICK_W, surfaceparm "monsterslickwest"
inline constexpr int32_t SURF_NOJUMPDELAY      = 0x10000000; // etjump, was SURF_MONSLICK_N, surfaceparm "monsterslicknorth"
inline constexpr int32_t SURF_PORTALSURFACE    = 0x20000000; // etjump, was SURF_MONSLICK_E, surfaceparm "monsterslickeast"
inline constexpr int32_t SURF_OVERBOUNCE       = 0x40000000; // etjump, was SURF_MONSLICK_S, surfaceparm "monsterslicksouth"
inline constexpr int32_t SURF_LANDMINE         = 0x80000000; // ydnar: ok to place landmines on this surface
// clang-format on
