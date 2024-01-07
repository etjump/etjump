// contents flags are seperate bits
// a given brush can contribute multiple content bits
// clang-format off

/* content bits */
constexpr int CONTENTS_SOLID        = 0x00000001;
//                                  = 0x00000002 unused, no built-in surfaceparm
constexpr int CONTENTS_LIGHTGRID    = 0x00000004;
constexpr int CONTENTS_LAVA         = 0x00000008;
constexpr int CONTENTS_SLIME        = 0x00000010;
constexpr int CONTENTS_WATER        = 0x00000020;
constexpr int CONTENTS_FOG          = 0x00000040;
constexpr int CONTENTS_MISSILECLIP  = 0x00000080;
constexpr int CONTENTS_ITEM         = 0x00000100;
//                                  = 0x00000200 unused, no built-in surfaceparm
//                                  = 0x00000400 unused, no built-in surfaceparm
//                                  = 0x00000800 unused, no built-in surfaceparm
//                                  = 0x00001000 unused, surfaceparm "ai_nosight"
//                                  = 0x00002000 unused, surfaceparm "clipshot", compiles to solid
constexpr int CONTENTS_MOVER        = 0x00004000;
constexpr int CONTENTS_AREAPORTAL   = 0x00008000;
constexpr int CONTENTS_PLAYERCLIP   = 0x00010000;
constexpr int CONTENTS_MONSTERCLIP  = 0x00020000;
//                                  = 0x00040000 unused, was CONTENTS_TELEPORTER, no built-in surfaceparm
constexpr int CONTENTS_PORTALCLIP   = 0x00080000; // etjump, was JUMPPAD, no built-in surfaceparm
constexpr int CONTENTS_NOSAVE       = 0x00100000; // etjump, was CLUSTERPORTAL, surfaceparm "clusterportal"
constexpr int CONTENTS_NOPRONE      = 0x00200000; // etjump, was DONOTENTE, surfaceparm "donotenter"
constexpr int CONTENTS_NONOCLIP     = 0x00400000; // etjump, was DONOTENTER_LARGE, BOTCLIP in q3map2, surfaceparm "donotenterlarge"
//                                  = 0x00800000 unused, no built-in surfaceparm
constexpr int CONTENTS_ORIGIN       = 0x01000000; // removed before bsping an entity
constexpr int CONTENTS_BODY         = 0x02000000; // should never be on a brush, only in game
constexpr int CONTENTS_CORPSE       = 0x04000000;
constexpr int CONTENTS_DETAIL       = 0x08000000; // brushes not used for the bsp
constexpr int CONTENTS_STRUCTURAL   = 0x10000000; // brushes used for the bsp
constexpr int CONTENTS_TRANSLUCENT  = 0x20000000; // don't consume surface fragments inside
constexpr int CONTENTS_TRIGGER      = 0x40000000;
constexpr int CONTENTS_NODROP       = 0x80000000; // don't leave bodies or items (death fog, lava)

/* surface bits */
constexpr int SURF_NODAMAGE         = 0x00000001; // never give falling damage
constexpr int SURF_SLICK            = 0x00000002; // effects game physics
constexpr int SURF_SKY              = 0x00000004; // lighting from environment map
constexpr int SURF_LADDER           = 0x00000008;
constexpr int SURF_NOIMPACT         = 0x00000010; // don't make missile explosions
constexpr int SURF_NOMARKS          = 0x00000020; // don't leave missile marks
constexpr int SURF_SPLASH           = 0x00000040; // SURF_FLESH in quake
constexpr int SURF_NODRAW           = 0x00000080; // don't generate a drawsurface at all
constexpr int SURF_HINT             = 0x00000100; // make a primary bsp splitter
constexpr int SURF_SKIP             = 0x00000200; // completely ignore, allowing non-closed brushes
constexpr int SURF_NOLIGHTMAP       = 0x00000400; // surface doesn't need a lightmap
constexpr int SURF_POINTLIGHT       = 0x00000800; // generate lighting info at vertexes
constexpr int SURF_METAL            = 0x00001000; // clanking footsteps
constexpr int SURF_NOSTEPS          = 0x00002000; // no footstep sounds
constexpr int SURF_NONSOLID         = 0x00004000; // don't collide against curves with this set
constexpr int SURF_LIGHTFILTER      = 0x00008000; // act as a light filter during q3map -light
constexpr int SURF_ALPHASHADOW      = 0x00010000; // do per-pixel light shadow casting in q3map
constexpr int SURF_NODLIGHT         = 0x00020000; // don't dlight even if solid (solid lava, skies)
constexpr int SURF_WOOD             = 0x00040000;
constexpr int SURF_GRASS            = 0x00080000;
constexpr int SURF_GRAVEL           = 0x00100000;
constexpr int SURF_GLASS            = 0x00200000;
constexpr int SURF_SNOW             = 0x00400000;
constexpr int SURF_ROOF             = 0x00800000;
//                                  = 0x01000000 unused, was RUBBLE, surfaceparm "rubble"
constexpr int SURF_CARPET           = 0x02000000;
//                                  = 0x04000000 unused, was MONSTERSLICK, surfaceparm "monsterslick"
constexpr int SURF_PORTALGATE       = 0x08000000; // etjump, was SURF_MONSLICK_W, surfaceparm "monsterslickwest"
constexpr int SURF_NOJUMPDELAY      = 0x10000000; // etjump, was SURF_MONSLICK_N, surfaceparm "monsterslicknorth"
constexpr int SURF_PORTALSURFACE    = 0x20000000; // etjump, was SURF_MONSLICK_E, surfaceparm "monsterslickeast"
constexpr int SURF_OVERBOUNCE       = 0x40000000; // etjump, was SURF_MONSLICK_S, surfaceparm "monsterslicksouth"
constexpr int SURF_LANDMINE         = 0x80000000; // ydnar: ok to place landmines on this surface
// clang-format on
