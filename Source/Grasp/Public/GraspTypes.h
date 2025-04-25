// Copyright (c) Jared Taylor. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameplayAbilitySpecHandle.h"
#include "GameplayTagContainer.h"
#include "GraspTypes.generated.h"

class UGraspInteractComponent;
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
 * Cached properties from interactable component
 */
USTRUCT(BlueprintType)
struct GRASP_API FGraspInteractPoint
{
	GENERATED_BODY()

	FGraspInteractPoint()
		: Location(FVector::ZeroVector)
		, Forward(FVector::ZeroVector)
		, ContextTag(FGameplayTag::EmptyTag)
	{}
	
	FGraspInteractPoint(const USceneComponent* InComponent);
	FGraspInteractPoint(const UGraspInteractComponent* InComponent);

	UPROPERTY(BlueprintReadWrite, Category=Grasp)
	FVector Location;

	UPROPERTY(BlueprintReadWrite, Category=Grasp)
	FVector Forward;

	UPROPERTY(BlueprintReadWrite, Category=Grasp)
	FGameplayTag ContextTag;
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
		UGraspData* InData = nullptr, const TArray<FGraspInteractPoint>& InMarkerCache = {},
		float InNormalizedAvatarDistance = 0.f)
		: InteractTag(InInteractTag)
		, Interactable(InInteractable)
		, Data(InData)
		, InteractPoints(InMarkerCache)
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

	/** Cached locations and vectors, i.e. points from which we can interact */
	UPROPERTY()
	TArray<FGraspInteractPoint> InteractPoints;

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