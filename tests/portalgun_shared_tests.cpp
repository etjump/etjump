#include <gtest/gtest.h>
#include "../src/game/etj_portalgun_shared.h"

namespace ETJump {
class PortalgunSharedTests : public testing::Test {
public:
  void SetUp() override {}
  void TearDown() override {}

  // trace hitting a wall facing +X at the given point
  static trace_t createWallTrace(const float x, const float y, const float z) {
    trace_t tr{};
    tr.fraction = 0.5f;
    VectorSet(tr.endpos, x, y, z);
    VectorSet(tr.plane.normal, 1.0f, 0.0f, 0.0f);
    tr.entityNum = ENTITYNUM_WORLD;
    return tr;
  }

  // trace hitting a floor at the given point
  static trace_t createFloorTrace(const float x, const float y, const float z) {
    trace_t tr{};
    tr.fraction = 0.5f;
    VectorSet(tr.endpos, x, y, z);
    VectorSet(tr.plane.normal, 0.0f, 0.0f, 1.0f);
    tr.entityNum = ENTITYNUM_WORLD;
    return tr;
  }

  static void expectVec3Eq(const vec3_t a, const vec3_t b) {
    EXPECT_FLOAT_EQ(a[0], b[0]);
    EXPECT_FLOAT_EQ(a[1], b[1]);
    EXPECT_FLOAT_EQ(a[2], b[2]);
  }
};

TEST_F(PortalgunSharedTests, SurfaceAllowsPortal_NoHit) {
  trace_t tr = createWallTrace(0, 0, 0);
  tr.fraction = 1.0f;
  EXPECT_FALSE(PortalgunShared::surfaceAllowsPortal(tr, true));
}

TEST_F(PortalgunSharedTests, SurfaceAllowsPortal_NoImpact) {
  trace_t tr = createWallTrace(0, 0, 0);
  tr.surfaceFlags = SURF_NOIMPACT;
  EXPECT_FALSE(PortalgunShared::surfaceAllowsPortal(tr, true));
}

TEST_F(PortalgunSharedTests, SurfaceAllowsPortal_PortalClipAndBody) {
  trace_t tr = createWallTrace(0, 0, 0);
  tr.contents = CONTENTS_PORTALCLIP;
  EXPECT_FALSE(PortalgunShared::surfaceAllowsPortal(tr, true));

  tr.contents = CONTENTS_BODY;
  EXPECT_FALSE(PortalgunShared::surfaceAllowsPortal(tr, true));

  tr.contents = CONTENTS_SOLID;
  EXPECT_TRUE(PortalgunShared::surfaceAllowsPortal(tr, true));
}

TEST_F(PortalgunSharedTests, SurfaceAllowsPortal_PortalSurfacesEnabled) {
  trace_t tr = createWallTrace(0, 0, 0);
  EXPECT_TRUE(PortalgunShared::surfaceAllowsPortal(tr, true));

  tr.surfaceFlags = SURF_PORTALSURFACE;
  EXPECT_FALSE(PortalgunShared::surfaceAllowsPortal(tr, true));
}

TEST_F(PortalgunSharedTests, SurfaceAllowsPortal_PortalSurfacesDisabled) {
  trace_t tr = createWallTrace(0, 0, 0);
  EXPECT_FALSE(PortalgunShared::surfaceAllowsPortal(tr, false));

  tr.surfaceFlags = SURF_PORTALSURFACE;
  EXPECT_TRUE(PortalgunShared::surfaceAllowsPortal(tr, false));
}

TEST_F(PortalgunSharedTests, ComputePlacement_Wall) {
  const trace_t tr = createWallTrace(100, 50, 25);
  PortalgunShared::Placement placement{};
  PortalgunShared::computePlacement(tr, nullptr, placement);

  const vec3_t expectedSurface = {100, 50, 25};
  const vec3_t expectedOrigin = {100 + PORTAL_SURFACE_OFFSET, 50, 25};
  const vec3_t expectedAngles = {0, 0, 0};

  expectVec3Eq(placement.surfacePoint, expectedSurface);
  expectVec3Eq(placement.origin, expectedOrigin);
  expectVec3Eq(placement.angles, expectedAngles);
  EXPECT_FLOAT_EQ(placement.scale, 1.0f);
}

TEST_F(PortalgunSharedTests, ComputePlacement_Floor) {
  const trace_t tr = createFloorTrace(10, 20, 30);
  PortalgunShared::Placement placement{};
  PortalgunShared::computePlacement(tr, nullptr, placement);

  const vec3_t expectedOrigin = {10, 20, 30 + PORTAL_SURFACE_OFFSET};

  expectVec3Eq(placement.origin, expectedOrigin);
  EXPECT_FLOAT_EQ(placement.angles[PITCH], -90.0f);
}

TEST_F(PortalgunSharedTests, ComputePlacement_TargetIsCentered) {
  // hit the wall off-center, target center is behind the wall surface
  const trace_t tr = createWallTrace(100, 70, 10);
  PortalgunShared::PortalTarget target{};
  VectorSet(target.center, 90, 50, 25);

  PortalgunShared::Placement placement{};
  PortalgunShared::computePlacement(tr, &target, placement);

  // center projected onto the wall plane
  const vec3_t expectedSurface = {100, 50, 25};
  const vec3_t expectedOrigin = {100 + PORTAL_SURFACE_OFFSET, 50, 25};

  expectVec3Eq(placement.surfacePoint, expectedSurface);
  expectVec3Eq(placement.origin, expectedOrigin);
  EXPECT_FLOAT_EQ(placement.scale, 1.0f);
}

TEST_F(PortalgunSharedTests, ComputePlacement_TargetSize) {
  const trace_t tr = createWallTrace(100, 50, 25);
  PortalgunShared::PortalTarget target{};
  VectorSet(target.center, 100, 50, 25);
  target.size = 120;

  PortalgunShared::Placement placement{};
  PortalgunShared::computePlacement(tr, &target, placement);

  EXPECT_FLOAT_EQ(placement.scale, 2.0f);
  EXPECT_EQ(PortalgunShared::sizeFromScale(placement.scale), 120);
}

TEST_F(PortalgunSharedTests, PortalsOverlap) {
  const trace_t tr = createWallTrace(100, 50, 25);
  PortalgunShared::Placement placement{};
  PortalgunShared::computePlacement(tr, nullptr, placement);

  const int32_t defaultSize = PortalgunShared::sizeFromScale(1.0f);
  const vec3_t wallAngles = {0, 0, 0};
  const vec3_t oppositeWallAngles = {0, 180, 0};

  // same spot on the same wall
  EXPECT_TRUE(PortalgunShared::portalsOverlap(placement, placement.origin,
                                              wallAngles, defaultSize));

  // close, but not overlapping
  // NOTE: keep a margin from the exact threshold, results exactly at the
  // boundary vary between platforms (e.g. '-ffast-math' + x87 on 32-bit Linux)
  vec3_t farOrigin;
  VectorCopy(placement.origin, farOrigin);
  farOrigin[1] += MIN_PORTALS_DIST * 2 + 1.0f;
  EXPECT_FALSE(PortalgunShared::portalsOverlap(placement, farOrigin,
                                               wallAngles, defaultSize));

  // larger portal overlaps from further away
  EXPECT_TRUE(PortalgunShared::portalsOverlap(placement, farOrigin, wallAngles,
                                              defaultSize * 2));

  // same spot, but facing the other way (e.g. other side of a thin wall)
  EXPECT_FALSE(PortalgunShared::portalsOverlap(
      placement, placement.origin, oppositeWallAngles, defaultSize));
}

TEST_F(PortalgunSharedTests, PortalTargetEncoding) {
  entityState_t es{};
  es.eType = ET_MOVER;
  EXPECT_FALSE(PortalgunShared::isPortalTarget(es));

  // 'trigger_heal' charge written to the targeted entity
  es.onFireEnd = 100;
  EXPECT_FALSE(PortalgunShared::isPortalTarget(es));
  es.onFireEnd = -1;
  EXPECT_FALSE(PortalgunShared::isPortalTarget(es));

  es.onFireEnd = PortalgunShared::encodePortalTarget(0);
  EXPECT_TRUE(PortalgunShared::isPortalTarget(es));
  EXPECT_EQ(PortalgunShared::portalTargetSize(es), 0);

  es.onFireEnd = PortalgunShared::encodePortalTarget(256);
  EXPECT_EQ(PortalgunShared::portalTargetSize(es), 256);

  es.onFireEnd = PortalgunShared::encodePortalTarget(10000);
  EXPECT_EQ(PortalgunShared::portalTargetSize(es), MAX_PORTAL_TARGET_SIZE);

  es.eType = ET_STATIC_CLIENT;
  EXPECT_TRUE(PortalgunShared::isPortalTarget(es));

  // other entity types never count as portal targets
  es.eType = ET_GENERAL;
  EXPECT_FALSE(PortalgunShared::isPortalTarget(es));
}

TEST_F(PortalgunSharedTests, PortalTargetState) {
  entityState_t es{};
  es.eType = ET_STATIC_CLIENT;

  const vec3_t center = {10, 20, 30};
  PortalgunShared::setPortalTarget(es, 64, center);
  EXPECT_TRUE(PortalgunShared::isPortalTarget(es));

  PortalgunShared::PortalTarget target{};
  PortalgunShared::getPortalTarget(es, target);
  expectVec3Eq(target.center, center);
  EXPECT_EQ(target.size, 64);
}

TEST_F(PortalgunSharedTests, TraceEndpoints) {
  const vec3_t origin = {0, 0, 0};
  const vec3_t viewangles = {0, 90, 0};
  vec3_t start;
  vec3_t end;

  PortalgunShared::traceEndpoints(origin, 40, viewangles, start, end);

  const vec3_t expectedStart = {0, 0, 40};
  expectVec3Eq(start, expectedStart);
  EXPECT_NEAR(end[0], 0.0f, 0.1f);
  EXPECT_FLOAT_EQ(end[1], MAX_PORTAL_RANGE);
  EXPECT_FLOAT_EQ(end[2], 40.0f);
}
} // namespace ETJump
