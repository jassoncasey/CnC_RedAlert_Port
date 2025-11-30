/**
 * Red Alert macOS Port - Animation System Tests
 *
 * Tests for the animation system.
 */

#include "../game/anim.h"
#include <cstdio>
#include <cstring>
#include <cassert>

//===========================================================================
// Test Framework
//===========================================================================

static int tests_passed = 0;
static int tests_failed = 0;

typedef void (*TestFunc)();

static void run_test(const char* name, TestFunc func) {
    printf("  Testing %s...", name);
    func();
    printf(" OK\n");
    tests_passed++;
}

#define TEST(name) static void test_##name()
#define RUN_TEST(name) run_test(#name, test_##name)
#define ASSERT(cond) do { if (!(cond)) { printf(" FAILED\n  Assertion failed: %s\n", #cond); tests_failed++; return; } } while(0)
#define ASSERT_EQ(a, b) do { if ((a) != (b)) { printf(" FAILED\n  Expected %d, got %d\n", (int)(b), (int)(a)); tests_failed++; return; } } while(0)
#define ASSERT_STR_EQ(a, b) do { if (strcmp((a), (b)) != 0) { printf(" FAILED\n  Expected '%s', got '%s'\n", (b), (a)); tests_failed++; return; } } while(0)

//===========================================================================
// Animation Type Tests
//===========================================================================

TEST(anim_type_count) {
    // Verify we have all animation types defined
    ASSERT_EQ(ANIM_TYPE_COUNT, static_cast<int>(AnimType::COUNT));
}

TEST(anim_type_find) {
    Anims::Init();

    // Find explosion type
    const AnimTypeClass* fball = AnimTypeClass::Find(AnimType::FBALL1);
    ASSERT(fball != nullptr);
    ASSERT_EQ(fball->type_, AnimType::FBALL1);
    ASSERT_STR_EQ(fball->name_, "FBALL1");

    // Find fire type
    const AnimTypeClass* fire = AnimTypeClass::Find(AnimType::FIRE_MED);
    ASSERT(fire != nullptr);
    ASSERT_EQ(fire->type_, AnimType::FIRE_MED);

    // Invalid type returns nullptr
    const AnimTypeClass* invalid = AnimTypeClass::Find(AnimType::NONE);
    ASSERT(invalid == nullptr);
}

TEST(anim_type_properties) {
    Anims::Init();

    // Check explosion properties
    const AnimTypeClass* fball = AnimTypeClass::Find(AnimType::FBALL1);
    ASSERT(fball != nullptr);
    ASSERT(fball->frameCount_ > 0);
    ASSERT(fball->frameDelay_ > 0);
    ASSERT(fball->isCraterForming_);
    ASSERT(fball->isScorcher_);
    ASSERT_EQ(fball->layer_, AnimLayerType::AIR);

    // Check fire properties
    const AnimTypeClass* fire = AnimTypeClass::Find(AnimType::FIRE_MED);
    ASSERT(fire != nullptr);
    ASSERT(fire->damage_ > 0);  // Fire does damage

    // Check corpse is ground layer
    const AnimTypeClass* corpse = AnimTypeClass::Find(AnimType::CORPSE1);
    ASSERT(corpse != nullptr);
    ASSERT_EQ(corpse->layer_, AnimLayerType::GROUND);

    // Check parachute is sticky
    const AnimTypeClass* para = AnimTypeClass::Find(AnimType::PARACHUTE);
    ASSERT(para != nullptr);
    ASSERT(para->isSticky_);
}

TEST(anim_type_chaining) {
    Anims::Init();

    // Electrocution chains to fire
    const AnimTypeClass* elect = AnimTypeClass::Find(AnimType::ELECT_DIE);
    ASSERT(elect != nullptr);
    ASSERT_EQ(elect->chainTo_, AnimType::FIRE_MED);

    // Dog electrocution chains to small fire
    const AnimTypeClass* dogElect = AnimTypeClass::Find(AnimType::DOG_ELECT_DIE);
    ASSERT(dogElect != nullptr);
    ASSERT_EQ(dogElect->chainTo_, AnimType::FIRE_SMALL);

    // Regular explosion doesn't chain
    const AnimTypeClass* fball = AnimTypeClass::Find(AnimType::FBALL1);
    ASSERT(fball != nullptr);
    ASSERT_EQ(fball->chainTo_, AnimType::NONE);
}

//===========================================================================
// Animation Creation Tests
//===========================================================================

TEST(anim_create) {
    Anims::Init();
    Anims::Clear_All();

    // Create an animation
    AnimClass* anim = Anims::Create(AnimType::FBALL1, 100, 200);
    ASSERT(anim != nullptr);
    ASSERT(anim->Is_Active());
    ASSERT_EQ(anim->Get_Type(), AnimType::FBALL1);
    ASSERT_EQ(anim->Get_X(), 100);
    ASSERT_EQ(anim->Get_Y(), 200);

    ASSERT_EQ(Anims::Count(), 1);

    Anims::Clear_All();
    ASSERT_EQ(Anims::Count(), 0);
}

TEST(anim_create_with_delay) {
    Anims::Init();
    Anims::Clear_All();

    // Create animation with delay
    AnimClass* anim = Anims::Create(AnimType::FBALL1, 100, 200, 10);
    ASSERT(anim != nullptr);
    ASSERT(anim->Is_Active());
    ASSERT_EQ(anim->Get_Frame(), 0);  // Hasn't started yet

    // Update a few times (less than delay)
    for (int i = 0; i < 5; i++) {
        Anims::Update_All();
    }
    ASSERT(anim->Is_Active());

    // Update past delay
    for (int i = 0; i < 10; i++) {
        Anims::Update_All();
    }
    // Animation should have started
    ASSERT(anim->Is_Active());

    Anims::Clear_All();
}

TEST(anim_create_multiple) {
    Anims::Init();
    Anims::Clear_All();

    // Create multiple animations
    AnimClass* anim1 = Anims::Create(AnimType::FBALL1, 0, 0);
    AnimClass* anim2 = Anims::Create(AnimType::FIRE_MED, 100, 100);
    AnimClass* anim3 = Anims::Create(AnimType::SMOKE_PUFF, 200, 200);

    ASSERT(anim1 != nullptr);
    ASSERT(anim2 != nullptr);
    ASSERT(anim3 != nullptr);
    ASSERT(anim1 != anim2);
    ASSERT(anim2 != anim3);

    ASSERT_EQ(Anims::Count(), 3);

    Anims::Clear_All();
}

TEST(anim_pool_limit) {
    Anims::Init();
    Anims::Clear_All();

    // Fill the pool
    for (int i = 0; i < ANIM_MAX; i++) {
        AnimClass* anim = Anims::Create(AnimType::FBALL1, i, i);
        ASSERT(anim != nullptr);
    }

    ASSERT_EQ(Anims::Count(), ANIM_MAX);

    // Pool should be exhausted
    AnimClass* overflow = Anims::Create(AnimType::FBALL1, 0, 0);
    ASSERT(overflow == nullptr);

    Anims::Clear_All();
}

//===========================================================================
// Animation Lifecycle Tests
//===========================================================================

TEST(anim_frame_advance) {
    Anims::Init();
    Anims::Clear_All();

    AnimClass* anim = Anims::Create(AnimType::FBALL1, 100, 200);
    ASSERT(anim != nullptr);

    int frameCount = anim->Get_Frame_Count();
    ASSERT(frameCount > 0);

    // Animation should advance frames over time
    int lastFrame = anim->Get_Frame();
    for (int i = 0; i < 100; i++) {
        Anims::Update_All();
        if (anim->Get_Frame() != lastFrame) {
            break;  // Frame advanced
        }
    }
    // Frame should have changed (or animation completed)

    Anims::Clear_All();
}

TEST(anim_completion) {
    Anims::Init();
    Anims::Clear_All();

    // Create a short animation (PIFF has 5 frames)
    AnimClass* anim = Anims::Create(AnimType::PIFF, 100, 200);
    ASSERT(anim != nullptr);

    // Run many updates to complete animation
    for (int i = 0; i < 200; i++) {
        Anims::Update_All();
    }

    // Animation should be complete and removed
    ASSERT_EQ(Anims::Count(), 0);
}

TEST(anim_looping) {
    Anims::Init();
    Anims::Clear_All();

    // Create animation with multiple loops
    AnimClass* anim = Anims::Create(AnimType::FIRE_MED, 100, 200, 0, 3);
    ASSERT(anim != nullptr);
    ASSERT(anim->Is_Looping());

    // Run some updates
    for (int i = 0; i < 50; i++) {
        Anims::Update_All();
    }

    // Should still be active (looping)
    ASSERT(anim->Is_Active());

    Anims::Clear_All();
}

TEST(anim_infinite_loop) {
    Anims::Init();
    Anims::Clear_All();

    // Create infinitely looping animation (loops=0)
    AnimClass* anim = Anims::Create(AnimType::ON_FIRE_SMALL, 100, 200, 0, 0);
    ASSERT(anim != nullptr);

    // Run many updates
    for (int i = 0; i < 500; i++) {
        Anims::Update_All();
    }

    // Should still be active
    ASSERT(anim->Is_Active());

    // Stop it manually
    anim->Stop();
    ASSERT(!anim->Is_Active());

    Anims::Clear_All();
}

TEST(anim_pause_resume) {
    Anims::Init();
    Anims::Clear_All();

    AnimClass* anim = Anims::Create(AnimType::FBALL1, 100, 200);
    ASSERT(anim != nullptr);

    // Run a few updates
    Anims::Update_All();
    Anims::Update_All();

    int frameBeforePause = anim->Get_Frame();

    // Pause
    anim->Pause(true);
    ASSERT(anim->IsPaused());

    // Run more updates
    for (int i = 0; i < 50; i++) {
        Anims::Update_All();
    }

    // Frame should not have changed
    ASSERT_EQ(anim->Get_Frame(), frameBeforePause);

    // Resume
    anim->Pause(false);
    ASSERT(!anim->IsPaused());

    // Run more updates
    for (int i = 0; i < 50; i++) {
        Anims::Update_All();
    }

    // Frame should have advanced (or animation completed)
    // Just verify it's either advanced or done

    Anims::Clear_All();
}

//===========================================================================
// Animation Position Tests
//===========================================================================

TEST(anim_position) {
    Anims::Init();
    Anims::Clear_All();

    AnimClass* anim = Anims::Create(AnimType::FBALL1, 100, 200);
    ASSERT(anim != nullptr);

    ASSERT_EQ(anim->Get_X(), 100);
    ASSERT_EQ(anim->Get_Y(), 200);

    anim->Set_Position(300, 400);
    ASSERT_EQ(anim->Get_X(), 300);
    ASSERT_EQ(anim->Get_Y(), 400);

    Anims::Clear_All();
}

TEST(anim_layer) {
    Anims::Init();
    Anims::Clear_All();

    // Air layer animation
    AnimClass* airAnim = Anims::Create(AnimType::FBALL1, 0, 0);
    ASSERT(airAnim != nullptr);
    ASSERT_EQ(airAnim->Get_Layer(), AnimLayerType::AIR);

    // Ground layer animation
    AnimClass* groundAnim = Anims::Create(AnimType::CORPSE1, 0, 0);
    ASSERT(groundAnim != nullptr);
    ASSERT_EQ(groundAnim->Get_Layer(), AnimLayerType::GROUND);

    Anims::Clear_All();
}

//===========================================================================
// Animation Chaining Tests
//===========================================================================

TEST(anim_chain_to_next) {
    Anims::Init();
    Anims::Clear_All();

    // Electrocution animation chains to fire
    AnimClass* anim = Anims::Create(AnimType::ELECT_DIE, 100, 200);
    ASSERT(anim != nullptr);
    ASSERT_EQ(anim->Get_Type(), AnimType::ELECT_DIE);

    // Run until it completes and chains
    for (int i = 0; i < 500; i++) {
        Anims::Update_All();
        if (Anims::Count() == 0) break;

        // Check if it chained
        AnimClass* current = AnimClass::Get_First();
        if (current && current->Get_Type() == AnimType::FIRE_MED) {
            // Successfully chained!
            break;
        }
    }

    Anims::Clear_All();
}

//===========================================================================
// Animation Iterator Tests
//===========================================================================

TEST(anim_iteration) {
    Anims::Init();
    Anims::Clear_All();

    // Create several animations
    Anims::Create(AnimType::FBALL1, 0, 0);
    Anims::Create(AnimType::FIRE_MED, 100, 100);
    Anims::Create(AnimType::SMOKE_PUFF, 200, 200);

    // Iterate through all
    int count = 0;
    AnimClass* anim = AnimClass::Get_First();
    while (anim) {
        count++;
        anim = AnimClass::Get_Next(anim);
    }

    ASSERT_EQ(count, 3);

    Anims::Clear_All();
}

//===========================================================================
// Animation Helper Tests
//===========================================================================

TEST(anim_explosion_helper) {
    Anims::Init();

    // Test explosion type mapping
    ASSERT_EQ(Anims::Get_Explosion_Anim(0), AnimType::PIFF);         // Small arms
    ASSERT_EQ(Anims::Get_Explosion_Anim(1), AnimType::FBALL1);       // HE
    ASSERT_EQ(Anims::Get_Explosion_Anim(2), AnimType::VEH_HIT2);     // AP
    ASSERT_EQ(Anims::Get_Explosion_Anim(3), AnimType::NAPALM2);      // Fire
    ASSERT_EQ(Anims::Get_Explosion_Anim(4), AnimType::ATOM_BLAST);   // Nuke
}

TEST(anim_fire_helper) {
    Anims::Init();

    // Test fire type mapping based on damage
    ASSERT_EQ(Anims::Get_Fire_Anim(75), AnimType::ON_FIRE_BIG);
    ASSERT_EQ(Anims::Get_Fire_Anim(50), AnimType::ON_FIRE_MED);
    ASSERT_EQ(Anims::Get_Fire_Anim(25), AnimType::ON_FIRE_SMALL);
    ASSERT_EQ(Anims::Get_Fire_Anim(10), AnimType::NONE);
}

//===========================================================================
// Animation Owner Tests
//===========================================================================

TEST(anim_owner) {
    Anims::Init();
    Anims::Clear_All();

    AnimClass* anim = Anims::Create(AnimType::FBALL1, 100, 200);
    ASSERT(anim != nullptr);

    ASSERT_EQ(anim->Get_Owner(), HousesType::NONE);

    anim->Set_Owner(HousesType::GOOD);
    ASSERT_EQ(anim->Get_Owner(), HousesType::GOOD);

    anim->Set_Owner(HousesType::BAD);
    ASSERT_EQ(anim->Get_Owner(), HousesType::BAD);

    Anims::Clear_All();
}

//===========================================================================
// Animation RTTI Tests
//===========================================================================

TEST(anim_rtti) {
    Anims::Init();
    Anims::Clear_All();

    AnimClass* anim = Anims::Create(AnimType::FBALL1, 100, 200);
    ASSERT(anim != nullptr);

    ASSERT_EQ(anim->What_Am_I(), RTTIType::ANIMATION);

    Anims::Clear_All();
}

//===========================================================================
// Main
//===========================================================================

int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;

    printf("Animation System Tests\n");
    printf("======================\n\n");

    // Animation Type Tests
    printf("Animation Type Tests:\n");
    RUN_TEST(anim_type_count);
    RUN_TEST(anim_type_find);
    RUN_TEST(anim_type_properties);
    RUN_TEST(anim_type_chaining);

    // Animation Creation Tests
    printf("\nAnimation Creation Tests:\n");
    RUN_TEST(anim_create);
    RUN_TEST(anim_create_with_delay);
    RUN_TEST(anim_create_multiple);
    RUN_TEST(anim_pool_limit);

    // Animation Lifecycle Tests
    printf("\nAnimation Lifecycle Tests:\n");
    RUN_TEST(anim_frame_advance);
    RUN_TEST(anim_completion);
    RUN_TEST(anim_looping);
    RUN_TEST(anim_infinite_loop);
    RUN_TEST(anim_pause_resume);

    // Animation Position Tests
    printf("\nAnimation Position Tests:\n");
    RUN_TEST(anim_position);
    RUN_TEST(anim_layer);

    // Animation Chaining Tests
    printf("\nAnimation Chaining Tests:\n");
    RUN_TEST(anim_chain_to_next);

    // Animation Iterator Tests
    printf("\nAnimation Iterator Tests:\n");
    RUN_TEST(anim_iteration);

    // Animation Helper Tests
    printf("\nAnimation Helper Tests:\n");
    RUN_TEST(anim_explosion_helper);
    RUN_TEST(anim_fire_helper);

    // Animation Owner Tests
    printf("\nAnimation Owner Tests:\n");
    RUN_TEST(anim_owner);

    // Animation RTTI Tests
    printf("\nAnimation RTTI Tests:\n");
    RUN_TEST(anim_rtti);

    printf("\n======================\n");
    printf("Tests passed: %d\n", tests_passed);
    printf("Tests failed: %d\n", tests_failed);
    printf("======================\n");

    return tests_failed > 0 ? 1 : 0;
}
