#include "pedestal.h"

#include "../scene/dynamic_scene.h"
#include "../defs.h"
#include "../models/models.h"
#include "../util/time.h"

#include "../build/assets/materials/static.h"
#include "../build/assets/models/pedestal.h"

void pedestalRender(void* data, struct RenderScene* renderScene) {
    struct Pedestal* pedestal = (struct Pedestal*)data;

    Mtx* matrix = renderStateRequestMatrices(renderScene->renderState, 1);
    transformToMatrixL(&pedestal->transform, matrix, SCENE_SCALE);

    Mtx* armature = renderStateRequestMatrices(renderScene->renderState, pedestal->armature.numberOfBones);

    skCalculateTransforms(&pedestal->armature, armature);

    Gfx* attachments = skBuildAttachments(&pedestal->armature, (pedestal->flags & PedstalFlagsDown) ? NULL : &w_portal_gun_gfx, renderScene->renderState);

    Gfx* objectRender = renderStateAllocateDLChunk(renderScene->renderState, 4);
    Gfx* dl = objectRender;

    if (attachments) {
        gSPSegment(dl++, BONE_ATTACHMENT_SEGMENT,  osVirtualToPhysical(attachments));
    }
    gSPSegment(dl++, MATRIX_TRANSFORM_SEGMENT,  osVirtualToPhysical(armature));
    gSPDisplayList(dl++, pedestal_model_gfx);
    gSPEndDisplayList(dl++);

    renderSceneAdd(
        renderScene,
        objectRender,
        matrix,
        DEFAULT_INDEX,
        &pedestal->transform.position,
        NULL
    );
}

void pedestalInit(struct Pedestal* pedestal, struct PedestalDefinition* definition) {
    transformInitIdentity(&pedestal->transform);

    pedestal->transform.position = definition->position;
    pedestal->roomIndex = definition->roomIndex;

    skArmatureInit(
        &pedestal->armature,
        pedestal_model_gfx,
        PEDESTAL_DEFAULT_BONES_COUNT,
        pedestal_default_bones,
        pedestal_bone_parent,
        PEDESTAL_ATTACHMENT_COUNT
    );

    skAnimatorInit(&pedestal->animator, PEDESTAL_DEFAULT_BONES_COUNT, NULL, NULL);

    pedestal->dynamicId = dynamicSceneAdd(pedestal, pedestalRender, &pedestal->transform, 0.8f);

    pedestal->flags = 0;
}

void pedestalUpdate(struct Pedestal* pedestal) {
    skAnimatorUpdate(&pedestal->animator, pedestal->armature.boneTransforms, FIXED_DELTA_TIME);

    quatAxisAngle(&gUp, gTimePassed, &pedestal->armature.boneTransforms[PEDESTAL_HOLDER_BONE].rotation);
}

void pedestalHide(struct Pedestal* pedestal) {
    pedestal->flags |= PedstalFlagsDown;

    skAnimatorRunClip(&pedestal->animator, &pedestal_animations[0], 0);
}