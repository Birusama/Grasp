// Copyright (c) Jared Taylor

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/RootMotionSource.h"
#include "RootMotionSource_GraspFaceTowards.generated.h"

/**
 * Face towards a target over time
 */
USTRUCT()
struct GRASP_API FRootMotionSource_GraspFaceTowards : public FRootMotionSource
{
	GENERATED_BODY()

	FRootMotionSource_GraspFaceTowards();

	virtual ~FRootMotionSource_GraspFaceTowards() override {}

	UPROPERTY()
	FVector StartDirection;
	
	UPROPERTY()
	FVector TargetDirection;
	
	virtual FRootMotionSource* Clone() const override;

	virtual bool Matches(const FRootMotionSource* Other) const override;

	virtual void PrepareRootMotion(
		float SimulationTime, 
		float MovementTickTime,
		const ACharacter& Character, 
		const UCharacterMovementComponent& MoveComponent
		) override;

	virtual bool NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess) override;

	virtual UScriptStruct* GetScriptStruct() const override
	{
		return StaticStruct();
	}

	virtual FString ToSimpleString() const override;
};

template<>
struct TStructOpsTypeTraits< FRootMotionSource_GraspFaceTowards > : public TStructOpsTypeTraitsBase2< FRootMotionSource_GraspFaceTowards >
{
	enum
	{
		WithNetSerializer = true,
		WithCopy = true
	};
};