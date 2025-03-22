// cg_marks.c -- wall marks

#include "cg_local.h"

/*
===================================================================

MARK POLYS

===================================================================
*/

markPoly_t cg_activeMarkPolys; // double linked list
markPoly_t *cg_freeMarkPolys;  // single linked list
markPoly_t cg_markPolys[MAX_MARK_POLYS];

/*
===================
CG_InitMarkPolys

This is called at startup and for tournement restarts
===================
*/
void CG_InitMarkPolys(void) {
  int i;
  markPoly_t *trav, *lasttrav;

  memset(cg_markPolys, 0, sizeof(cg_markPolys));

  cg_activeMarkPolys.nextMark = &cg_activeMarkPolys;
  cg_activeMarkPolys.prevMark = &cg_activeMarkPolys;
  cg_freeMarkPolys = cg_markPolys;
  for (i = 0, trav = cg_markPolys + 1, lasttrav = cg_markPolys;
       i < MAX_MARK_POLYS - 1; i++, trav++) {
    lasttrav->nextMark = trav;
    lasttrav = trav;
  }
}

/*
==================
CG_FreeMarkPoly
==================
*/
void CG_FreeMarkPoly(markPoly_t *le) {
  if (!le->prevMark) {
    CG_Error("CG_FreeLocalEntity: not active");
  }

  // remove from the doubly linked active list
  le->prevMark->nextMark = le->nextMark;
  le->nextMark->prevMark = le->prevMark;

  // the free list is only singly linked
  le->nextMark = cg_freeMarkPolys;
  cg_freeMarkPolys = le;
}

/*
===================
CG_AllocMark

Will allways succeed, even if it requires freeing an old active mark
===================
*/
markPoly_t *CG_AllocMark(int endTime) {
  markPoly_t *le; //, *trav, *lastTrav;
  int time;

  if (!cg_freeMarkPolys) {
    // no free entities, so free the one at the end of the chain
    // remove the oldest active entity
    time = cg_activeMarkPolys.prevMark->time;
    while (cg_activeMarkPolys.prevMark &&
           time == cg_activeMarkPolys.prevMark->time) {
      CG_FreeMarkPoly(cg_activeMarkPolys.prevMark);
    }
  }

  le = cg_freeMarkPolys;
  cg_freeMarkPolys = cg_freeMarkPolys->nextMark;

  memset(le, 0, sizeof(*le));

  // Ridah, TODO: sort this, so the list is always sorted by longest
  // duration -> shortest duration, this way the shortest duration mark
  // will always get overwritten first for (trav =
  // cg_activeMarkPolys.nextMark; (trav->duration + trav->time >
  // endTime) && (trav != cg_activeMarkPolys.prevMark) ; lastTrav =
  // trav, trav++ ) { Respect the FOR loop
  //}

  // link into the active list
  le->nextMark = cg_activeMarkPolys.nextMark;
  le->prevMark = &cg_activeMarkPolys;
  cg_activeMarkPolys.nextMark->prevMark = le;
  cg_activeMarkPolys.nextMark = le;
  return le;
}

/*
CG_ImpactMark()
projection is a normal and distance (not a plane, but rather how far to project)
it MUST be normalized!
if lifeTime < 0, then generate a temporary mark
*/

inline constexpr int MAX_MARK_FRAGMENTS = 384;
inline constexpr int MAX_MARK_POINTS = 1024;
inline constexpr int MARK_FADE_TIME = 10000;

void CG_ImpactMark(qhandle_t markShader, vec3_t origin, vec4_t projection,
                   float radius, float orientation, float r, float g, float b,
                   float a, int lifeTime) {
  int i;
  vec3_t pushedOrigin, axis[3];
  vec4_t color;
  int fadeTime;
  vec3_t points[4];

  /* early out */
  if (lifeTime == 0) {
    return;
  }

  /* make rotated polygon axis */
  VectorCopy(projection, axis[0]);
  PerpendicularVector(axis[1], axis[0]);
  RotatePointAroundVector(axis[2], axis[0], axis[1], -orientation);
  CrossProduct(axis[0], axis[2], axis[1]);

  /* push the origin out a bit */
  VectorMA(origin, -1.0f, axis[0], pushedOrigin);

  /* create the full polygon */
  for (i = 0; i < 3; i++) {
    points[0][i] = pushedOrigin[i] - radius * axis[1][i] - radius * axis[2][i];
    points[1][i] = pushedOrigin[i] - radius * axis[1][i] + radius * axis[2][i];
    points[2][i] = pushedOrigin[i] + radius * axis[1][i] + radius * axis[2][i];
    points[3][i] = pushedOrigin[i] + radius * axis[1][i] - radius * axis[2][i];
  }

/* debug code */
#if 0
	VectorSet(points[0], origin[0] - radius, origin[1] - radius, origin[2]);
	VectorSet(points[1], origin[0] - radius, origin[1] + radius, origin[2]);
	VectorSet(points[2], origin[0] + radius, origin[1] + radius, origin[2]);
	VectorSet(points[3], origin[0] + radius, origin[1] - radius, origin[2]);
	CG_Printf("Dir: %f %f %f\n", dir[0], dir[1], dir[2]);
#endif

  /* set color */
  color[0] = r;
  color[1] = g;
  color[2] = b;
  color[3] = a;

  /* set decal times (in seconds) */
  fadeTime = lifeTime >> 4;

  /* add the decal */
  trap_R_ProjectDecal(markShader, 4, points, projection, color, lifeTime,
                      fadeTime);
}

/*
===============
CG_AddMarks
===============
*/

void CG_AddMarks(void) {
  int j;
  markPoly_t *mp, *next;
  int t;
  int fade;

  if (!cg_markTime.integer) {
    return;
  }

  mp = cg_activeMarkPolys.nextMark;
  for (; mp != &cg_activeMarkPolys; mp = next) {
    // grab next now, so if the local entity is freed we
    // still have it
    next = mp->nextMark;

    // see if it is time to completely remove it
    if (cg.time > mp->time + mp->duration) {
      CG_FreeMarkPoly(mp);
      continue;
    }

    // fade all marks out with time
    t = mp->time + mp->duration - cg.time;
    if (t < (float)mp->duration / 2.0) {
      fade = (int)(255.0 * (float)t / ((float)mp->duration / 2.0));
      if (mp->alphaFade) {
        for (j = 0; j < mp->poly.numVerts; j++) {
          mp->verts[j].modulate[3] = fade;
        }
      } else {
        for (j = 0; j < mp->poly.numVerts; j++) {
          mp->verts[j].modulate[0] = mp->color[0] * fade;
          mp->verts[j].modulate[1] = mp->color[1] * fade;
          mp->verts[j].modulate[2] = mp->color[2] * fade;
        }
      }
    }

    trap_R_AddPolyToScene(mp->markShader, mp->poly.numVerts, mp->verts);
  }
}
