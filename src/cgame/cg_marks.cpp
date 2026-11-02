// cg_marks.c -- wall marks

#include "cg_local.h"

/*
CG_ImpactMark()
projection is a normal and distance (not a plane, but rather how far to project)
it MUST be normalized!
if lifeTime < 0, then generate a temporary mark
*/
void CG_ImpactMark(qhandle_t markShader, const vec3_t origin, vec4_t projection,
                   float radius, float orientation, float r, float g, float b,
                   float a, int lifeTime) {
  /* early out */
  if (lifeTime == 0) {
    return;
  }

  /* make rotated polygon axis */
  vec3_t axis[3];
  VectorCopy(projection, axis[0]);
  PerpendicularVector(axis[1], axis[0]);
  RotatePointAroundVector(axis[2], axis[0], axis[1], -orientation);
  CrossProduct(axis[0], axis[2], axis[1]);

  /* push the origin out a bit */
  vec3_t pushedOrigin;
  VectorMA(origin, -1.0f, axis[0], pushedOrigin);

  /* create the full polygon */
  vec3_t points[4];
  for (int32_t i = 0; i < 3; i++) {
    points[0][i] =
        pushedOrigin[i] - (radius * axis[1][i]) - (radius * axis[2][i]);
    points[1][i] =
        pushedOrigin[i] - (radius * axis[1][i]) + (radius * axis[2][i]);
    points[2][i] =
        pushedOrigin[i] + (radius * axis[1][i]) + (radius * axis[2][i]);
    points[3][i] =
        pushedOrigin[i] + (radius * axis[1][i]) - (radius * axis[2][i]);
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
  vec4_t color;
  color[0] = r;
  color[1] = g;
  color[2] = b;
  color[3] = a;

  /* set decal times (in seconds) */
  const int32_t fadeTime = lifeTime >> 4;

  /* add the decal */
  trap_R_ProjectDecal(markShader, 4, points, projection, color, lifeTime,
                      fadeTime);
}
