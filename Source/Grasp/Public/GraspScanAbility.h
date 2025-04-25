// Copyright (c) Jared Taylor

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"

#include "GraspScanAbility.generated.h"

/**
 * Passive ability that Grasp uses to scan for interactables to grant abilities for
 * Only runs on the server
 * Call UGraspScanTask from ActivateAbility from your derived ability - that's it!
 */
UCLASS(BlueprintType, Blueprintable, Abstract)
class GRASP_API UGraspScanAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:
	/** Automatically activate this ability after being granted */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, AdvancedDisplay, Category=Grasp)
	bool bAutoActivateOnGrantAbility = true;
	
public:
	UGraspScanAbility(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void OnAvatarSet(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;
};
