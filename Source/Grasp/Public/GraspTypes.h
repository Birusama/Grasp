// Copyright (c) Jared Taylor. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameplayAbilitySpecHandle.h"
#include "GameplayTagContainer.h"
#include "GraspTypes.generated.h"

class UGraspInteractMarker;
class UGameplayAbility;
class UGraspData;
class UGraspComponent;

DECLARE_LOG_CATEGORY_EXTERN(LogGrasp, Log, All);

DECLARE_DELEGATE_OneParam(FOnPauseGrasp, bool /* bIsPaused */);
DECLARE_DELEGATE(FOnRequestGrasp);

UENUM(BlueprintType)
enum class EGraspTargetingSource : uint8
{
	Pawn						UMETA(ToolTip="Use the controlled Pawn as the targeting source"),
	PawnIfValid					UMETA(ToolTip="Use the controlled Pawn as the targeting source, or the Controller if invalid"),
	Controller					UMETA(ToolTip="Use the Controller as the targeting source"),
};

UENUM(BlueprintType)
enum class EGraspInteractQuery : uint8
{
	None			UMETA(ToolTip="Cannot interact or highlight"),
	Highlight		UMETA(ToolTip="Highlight, but not interact -- typically for driving UI"),
	Interact		UMETA(ToolTip="Can interact"),
};

UENUM(BlueprintType)
enum class EGraspFocusMode : uint8
{
	None 			UMETA(ToolTip="Does not require focus to interact"),
	Focus			UMETA(ToolTip="Requires focus to interact"),
	FocusAlways		UMETA(ToolTip="Requires focus to interact and ability will end if focus is lost"),
};

/**
 * Stores data about one UGraspInteractMarker
 * Allows editor markers to update location and rotation and cache for
 * non-editor builds
 * This saves on the expensive perf cost of updating scene components
 */
USTRUCT(BlueprintType)
struct GRASP_API FGraspMarker
{
	GENERATED_BODY()

	FGraspMarker()
		: RelativeLocation(FVector::ZeroVector)
		, RelativeRotation(FQuat::Identity)
		, AttachParent(nullptr)
	{}

#if WITH_EDITOR
	FGraspMarker(const UGraspInteractMarker* InMarker);
#endif

protected:
	UPROPERTY()
	FVector RelativeLocation;

	UPROPERTY()
	FQuat RelativeRotation;

	UPROPERTY()
	USceneComponent* AttachParent;

public:
	/** This allows us to have doors that open on hinges updating the interact location */
	FTransform GetTransform() const
	{
		const FTransform RelativeTransform = { RelativeRotation, RelativeLocation };
		return AttachParent ? AttachParent->GetComponentTransform().GetRelativeTransform(RelativeTransform) : FTransform::Identity;
	}

	FVector GetLocation() const
	{
		return GetTransform().GetLocation();
	}

	FVector GetForwardVector() const
	{
		return GetTransform().GetUnitAxis(EAxis::X);
	}
};

/**
 * Grasp will scan for interactables to retrieve their data and ability
 */
USTRUCT()
struct GRASP_API FGraspScanResult
{
	GENERATED_BODY()

	FGraspScanResult(const FGameplayTag& InInteractTag = FGameplayTag::EmptyTag,
		const TWeakObjectPtr<AActor>& InInteractable = nullptr,
		UGraspData* InData = nullptr, const TArray<FGraspMarker>& InMarkerCache = {},
		float InNormalizedAvatarDistance = 0.f)
		: InteractTag(InInteractTag)
		, Interactable(InInteractable)
		, Data(InData)
		, Markers(InMarkerCache)
		, NormalizedScanDistance(InNormalizedAvatarDistance)
	{}

	/** Tag used for the targeting preset that discovered this interactable during Grasp scanning */
	UPROPERTY()
	FGameplayTag InteractTag;

	/** The actor being interacted with, e.g. a door or chest */
	UPROPERTY()
	TWeakObjectPtr<AActor> Interactable;

	/** Contains the ability and parameters for interaction */
	UPROPERTY()
	UGraspData* Data;

	/** Cached marker locations and vectors, i.e. points from which we can interact */
	UPROPERTY()
	TArray<FGraspMarker> Markers;

	/**
	 * Normalized Distance between Avatar and Interactable root location on a 0-1 scale
	 * Normalized between 0 and the max scan range
	 * Abilities are granted when we're closer and removed when we're further away
	 */
	UPROPERTY()
	float NormalizedScanDistance;

	bool operator==(const FGraspScanResult& Other) const
	{
		return Interactable == Other.Interactable;
	}

	bool operator!=(const FGraspScanResult& Other) const
	{
		return !(*this == Other);
	}
};
DECLARE_DELEGATE_TwoParams(FOnGraspTargetsReady, UGraspComponent* GraspComponent, const TArray<FGraspScanResult>& Results);

/**
 * Granted ability data
 */
USTRUCT()
struct GRASP_API FGraspAbilityData
{
	GENERATED_BODY()

	FGraspAbilityData()
		: bPersistent(false)
		, Handle(FGameplayAbilitySpecHandle())
		, Ability(nullptr)
	{}
	
	UPROPERTY()
	bool bPersistent;

	UPROPERTY()
	FGameplayAbilitySpecHandle Handle;

	UPROPERTY()
	TSubclassOf<UGameplayAbility> Ability;

	/** Interactables that are in range and require this ability remain active */
	UPROPERTY()
	TArray<TWeakObjectPtr<AActor>> Interactables;
};