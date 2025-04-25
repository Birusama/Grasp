// Copyright (c) Jared Taylor

#pragma once

#include "CoreMinimal.h"
#include "GraspTargetingTypes.generated.h"


UENUM(BlueprintType)
enum class EGraspTargetingShape : uint8
{
	Box,
	Cylinder,
	Sphere,
	Capsule,
};

UENUM(BlueprintType)
enum class EGraspTargetLocationSource : uint8
{
	Actor,
	ViewLocation,
	Camera,
};

UENUM(BlueprintType)
enum class EGraspTargetRotationSource : uint8
{
	Actor,
	ControlRotation,
	ViewRotation,
};
