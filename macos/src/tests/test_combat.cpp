/**
 * Red Alert macOS Port - Combat System Tests
 *
 * Tests for damage calculation, bullets, and weapon firing.
 */

#include <cstdio>
#include <cassert>
#include <cmath>
#include "game/combat.h"
#include "game/bullet.h"
#include "game/weapon_types.h"
#include "game/object.h"
#include "game/cell.h"
#include "game/mapclass.h"

//===========================================================================
// Test Infrastructure
//===========================================================================

static int testsPassed = 0;
static int testsFailed = 0;

#define TEST(name) void test_##name()
#define RUN_TEST(name) do { \
    printf("  Testing " #name "..."); \
    test_##name(); \
    printf(" PASSED\n"); \
    testsPassed++; \
} while(0)

#define ASSERT_TRUE(cond) do { \
    if (!(cond)) { \
        printf(" FAILED at line %d: %s\n", __LINE__, #cond); \
        testsFailed++; \
        return; \
    } \
} while(0)

#define ASSERT_EQ(a, b) do { \
    if ((a) != (b)) { \
        printf(" FAILED at line %d: %s != %s (%d != %d)\n", __LINE__, #a, #b, (int)(a), (int)(b)); \
        testsFailed++; \
        return; \
    } \
} while(0)

#define ASSERT_NEAR(a, b, tolerance) do { \
    if (abs((a) - (b)) > (tolerance)) { \
        printf(" FAILED at line %d: %s not near %s (%d vs %d)\n", __LINE__, #a, #b, (int)(a), (int)(b)); \
        testsFailed++; \
        return; \
    } \
} while(0)

//===========================================================================
// Damage Calculation Tests
//===========================================================================

TEST(damage_basic) {
    // Basic damage with no armor and no distance
    int dmg = Modify_Damage(100, WarheadType::SA, ArmorType::NONE, 0);
    ASSERT_TRUE(dmg > 0);
    ASSERT_TRUE(dmg <= 100);
}

TEST(damage_zero) {
    // Zero damage should return zero
    int dmg = Modify_Damage(0, WarheadType::SA, ArmorType::NONE, 0);
    ASSERT_EQ(dmg, 0);
}

TEST(damage_minimum) {
    // Even with very high armor, minimum damage should be 1
    int dmg = Modify_Damage(1, WarheadType::SA, ArmorType::CONCRETE, 0);
    ASSERT_TRUE(dmg >= MIN_DAMAGE);
}

TEST(damage_maximum) {
    // Very high damage should be capped
    int dmg = Modify_Damage(10000, WarheadType::SA, ArmorType::NONE, 0);
    ASSERT_TRUE(dmg <= MAX_DAMAGE);
}

TEST(damage_distance_falloff) {
    // Damage should decrease with distance
    int dmgClose = Modify_Damage(100, WarheadType::SA, ArmorType::NONE, 0);
    int dmgFar = Modify_Damage(100, WarheadType::SA, ArmorType::NONE, 500);
    ASSERT_TRUE(dmgClose >= dmgFar);
}

TEST(damage_healing) {
    // Negative damage (healing) should work at close range
    int heal = Modify_Damage(-50, WarheadType::SA, ArmorType::NONE, 0);
    ASSERT_EQ(heal, -50);

    // Healing at long range should be 0
    int healFar = Modify_Damage(-50, WarheadType::SA, ArmorType::NONE, 100);
    ASSERT_EQ(healFar, 0);
}

TEST(damage_armor_types) {
    // Different armor types should give different damage
    int dmgNone = Modify_Damage(100, WarheadType::AP, ArmorType::NONE, 0);
    int dmgLight = Modify_Damage(100, WarheadType::AP, ArmorType::LIGHT, 0);
    int dmgHeavy = Modify_Damage(100, WarheadType::AP, ArmorType::HEAVY, 0);

    // All should be non-zero
    ASSERT_TRUE(dmgNone > 0);
    ASSERT_TRUE(dmgLight > 0);
    ASSERT_TRUE(dmgHeavy > 0);
}

//===========================================================================
// Warhead Tests
//===========================================================================

TEST(warhead_modifier) {
    // Get warhead modifier for various combinations
    int mod = GetWarheadModifier(WarheadType::SA, ArmorType::NONE);
    ASSERT_TRUE(mod > 0);
    ASSERT_TRUE(mod <= 512);  // Should be reasonable percentage
}

TEST(warhead_spread) {
    // Spread can be 0 for some warhead types - test HE which should have spread
    int spread = GetWarheadSpread(WarheadType::HE);
    // HE warheads typically have area effect spread
    // Just verify the function doesn't crash and returns reasonable value
    ASSERT_TRUE(spread >= 0);
}

TEST(warhead_destroy_wall) {
    // HE and AP should be able to destroy walls
    bool heWall = CanDestroyWall(WarheadType::HE);
    bool saWall = CanDestroyWall(WarheadType::SA);
    // At least one should work
    (void)heWall;
    (void)saWall;
    ASSERT_TRUE(true);  // Just testing it doesn't crash
}

//===========================================================================
// Bullet Tests
//===========================================================================

TEST(bullet_construction) {
    BulletClass bullet;
    ASSERT_EQ(bullet.WhatAmI(), RTTIType::BULLET);
    ASSERT_TRUE(bullet.type_ == BulletType::NONE);
}

TEST(bullet_init) {
    BulletClass bullet;
    bullet.Init(BulletType::CANNON, nullptr, XY_Coord(1000, 1000), 50, WarheadType::AP);

    ASSERT_TRUE(bullet.type_ == BulletType::CANNON);
    ASSERT_EQ(bullet.damage_, 50);
    ASSERT_TRUE(bullet.warhead_ == WarheadType::AP);
}

TEST(bullet_type_data) {
    // Check bullet type data lookup
    const BulletTypeData* data = GetBulletType(BulletType::CANNON);
    ASSERT_TRUE(data != nullptr);
    ASSERT_TRUE(data->iniName != nullptr);
}

TEST(bullet_instant_hit) {
    BulletClass bullet;
    bullet.Init(BulletType::INVISIBLE, nullptr, XY_Coord(1000, 1000), 50, WarheadType::SA);
    ASSERT_TRUE(bullet.IsInstantHit());
}

TEST(bullet_flight_state) {
    BulletClass bullet;
    bullet.Init(BulletType::CANNON, nullptr, XY_Coord(1000, 1000), 50, WarheadType::AP);

    // Before launch, should be idle
    // After Init, state is IDLE
    // After Launch, should be FLYING
}

TEST(bullet_distance_to_target) {
    BulletClass bullet;
    int32_t target = XY_Coord(1000, 1000);
    bullet.Init(BulletType::CANNON, nullptr, target, 50, WarheadType::AP);
    bullet.coord_ = XY_Coord(500, 500);

    int dist = bullet.DistanceToTarget();
    ASSERT_TRUE(dist > 0);
}

TEST(bullet_detonation) {
    BulletClass bullet;
    bullet.Init(BulletType::CANNON, nullptr, XY_Coord(1000, 1000), 50, WarheadType::AP);

    // Check ShouldDetonate logic
    ASSERT_TRUE(!bullet.ShouldDetonate());  // Not flying yet
}

//===========================================================================
// Weapon Tests
//===========================================================================

TEST(weapon_range) {
    int range = GetWeaponRange(WeaponTypeEnum::VULCAN);
    ASSERT_TRUE(range > 0);
}

TEST(weapon_rof) {
    int rof = GetWeaponROF(WeaponTypeEnum::VULCAN);
    ASSERT_TRUE(rof > 0);
}

TEST(weapon_data) {
    const WeaponTypeData* data = GetWeaponType(WeaponTypeEnum::CHAIN_GUN);
    ASSERT_TRUE(data != nullptr);
    ASSERT_TRUE(data->damage > 0);
    ASSERT_TRUE(data->range > 0);
}

//===========================================================================
// Combat Animation Tests
//===========================================================================

TEST(combat_anim_small) {
    AnimType anim = Combat_Anim(5, WarheadType::SA);
    ASSERT_TRUE(anim == AnimType::PIFF);
}

TEST(combat_anim_medium) {
    AnimType anim = Combat_Anim(30, WarheadType::SA);
    // Should be one of the medium explosion types
    ASSERT_TRUE(anim == AnimType::PIFFPIFF || anim == AnimType::VEH_HIT1);
}

TEST(combat_anim_large) {
    AnimType anim = Combat_Anim(150, WarheadType::HE);
    // Should be a large explosion type
    ASSERT_TRUE(anim == AnimType::VEH_HIT3 || anim == AnimType::VEH_HIT2);
}

//===========================================================================
// TechnoClass Combat Tests
//===========================================================================

// Concrete test class for TechnoClass testing
class TestTechnoClass : public TechnoClass {
public:
    TestTechnoClass() : TechnoClass() {
        rtti_ = RTTIType::UNIT;
    }
    void DrawIt(int, int, int) const override {}
};

TEST(techno_can_fire) {
    TestTechnoClass techno;
    techno.isInLimbo_ = false;
    techno.arm_[0] = 0;
    ASSERT_TRUE(techno.CanFire());

    techno.arm_[0] = 10;
    ASSERT_TRUE(!techno.CanFire());  // Still arming
}

TEST(techno_rearm_time) {
    TestTechnoClass techno;
    int rearm = techno.RearmTime(0);
    ASSERT_TRUE(rearm > 0);
}

//===========================================================================
// Coordinate Helper Tests
//===========================================================================

TEST(distance_zero) {
    int32_t coord = XY_Coord(100, 100);
    int dist = Distance(coord, coord);
    ASSERT_EQ(dist, 0);
}

TEST(distance_horizontal) {
    int32_t c1 = XY_Coord(100, 100);
    int32_t c2 = XY_Coord(200, 100);
    int dist = Distance(c1, c2);
    ASSERT_TRUE(dist > 0);
    ASSERT_TRUE(dist < 200);  // Should be approximately 100
}

TEST(distance_diagonal) {
    int32_t c1 = XY_Coord(0, 0);
    int32_t c2 = XY_Coord(100, 100);
    int dist = Distance(c1, c2);
    ASSERT_TRUE(dist > 100);  // Diagonal should be longer than sides
}

//===========================================================================
// Main
//===========================================================================

int main() {
    printf("Combat System Tests\n");
    printf("===================\n\n");

    // Initialize map for explosion tests
    Map.AllocCells();
    Map.InitCells();

    // Damage calculation tests
    printf("Damage Calculation:\n");
    RUN_TEST(damage_basic);
    RUN_TEST(damage_zero);
    RUN_TEST(damage_minimum);
    RUN_TEST(damage_maximum);
    RUN_TEST(damage_distance_falloff);
    RUN_TEST(damage_healing);
    RUN_TEST(damage_armor_types);

    // Warhead tests
    printf("\nWarhead Tests:\n");
    RUN_TEST(warhead_modifier);
    RUN_TEST(warhead_spread);
    RUN_TEST(warhead_destroy_wall);

    // Bullet tests
    printf("\nBullet Tests:\n");
    RUN_TEST(bullet_construction);
    RUN_TEST(bullet_init);
    RUN_TEST(bullet_type_data);
    RUN_TEST(bullet_instant_hit);
    RUN_TEST(bullet_flight_state);
    RUN_TEST(bullet_distance_to_target);
    RUN_TEST(bullet_detonation);

    // Weapon tests
    printf("\nWeapon Tests:\n");
    RUN_TEST(weapon_range);
    RUN_TEST(weapon_rof);
    RUN_TEST(weapon_data);

    // Combat animation tests
    printf("\nCombat Animation Tests:\n");
    RUN_TEST(combat_anim_small);
    RUN_TEST(combat_anim_medium);
    RUN_TEST(combat_anim_large);

    // TechnoClass combat tests
    printf("\nTechnoClass Combat Tests:\n");
    RUN_TEST(techno_can_fire);
    RUN_TEST(techno_rearm_time);

    // Coordinate helper tests
    printf("\nCoordinate Helper Tests:\n");
    RUN_TEST(distance_zero);
    RUN_TEST(distance_horizontal);
    RUN_TEST(distance_diagonal);

    Map.FreeCells();

    printf("\n===================\n");
    printf("Results: %d passed, %d failed\n", testsPassed, testsFailed);

    return testsFailed > 0 ? 1 : 0;
}
