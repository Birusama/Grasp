// Copyright (c) Jared Taylor

#pragma once

#include "CoreMinimal.h"
#include "GameplayAbilitySpecHandle.h"
#include "Abilities/GameplayAbility.h"
#include "GraspAbilityData.generated.h"

/**
 * Granted ability data
 */
USTRUCT(BlueprintType)
struct GRASP_API FGraspAbilityData
{
	GENERATED_BODY()

	FGraspAbilityData()
		: bPersistent(false)
		, Handle(FGameplayAbilitySpecHandle())
		, Ability(nullptr)
	{}

	UPROPERTY(BlueprintReadOnly, Category=Grasp)
	bool bPersistent;

	UPROPERTY(BlueprintReadOnly, Category=Grasp)
	FGameplayAbilitySpecHandle Handle;

	UPROPERTY(BlueprintReadOnly, Category=Grasp)
	FGameplayAbilitySpec Spec;

	UPROPERTY(BlueprintReadOnly, Category=Grasp)
	TSubclassOf<UGameplayAbility> Ability;

	/** Interactables that have an ability lock, preventing the ability from being cleared */
	UPROPERTY()
	TArray<TWeakObjectPtr<const UPrimitiveComponent>> LockedGraspables;

	/** Interactables that are in range and require this ability remain active */
	UPROPERTY()
	TArray<TWeakObjectPtr<const UPrimitiveComponent>> Graspables;
};