// Copyright (c) Jared Taylor

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GraspAbilityData.h"
#include "GraspTypes.h"
#include "GraspDelegates.generated.h"

class UGraspComponent;
class UGraspData;

DECLARE_DELEGATE_OneParam(FOnPauseGrasp, bool /* bIsPaused */);
DECLARE_DELEGATE(FOnRequestGrasp);

UDELEGATE()
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FiveParams(FOnPostGiveGraspAbility, UGraspComponent*, GraspComponent,
	TSubclassOf<UGameplayAbility>, Ability, const UPrimitiveComponent*, GraspableComponent,
	const UGraspData*, GraspData, const FGraspAbilityData&, AbilityData);

UDELEGATE()
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnPostGiveCommonGraspAbility, UGraspComponent*, GraspComponent,
	TSubclassOf<UGameplayAbility>, Ability, const FGraspAbilityData&, AbilityData);

UDELEGATE()
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnPreClearGraspAbility, UGraspComponent*, GraspComponent,
	TSubclassOf<UGameplayAbility>, Ability, const UGraspData*, GraspData, const FGraspAbilityData&, AbilityData);

UDELEGATE()
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FiveParams(FOnPreTryActivateGraspAbility, UGraspComponent*, GraspComponent,
	const AActor*, SourceActor, UPrimitiveComponent*, GraspableComponent,EGraspAbilityComponentSource, Source,
	const FGameplayAbilitySpec&, AbilitySpec);

UDELEGATE()
DECLARE_DYNAMIC_MULTICAST_DELEGATE_SixParams(FOnPostActivateGraspAbility, UGraspComponent*, GraspComponent,
	const AActor*, SourceActor, UPrimitiveComponent*, GraspableComponent, EGraspAbilityComponentSource, Source,
	const FGameplayAbilitySpec&, AbilitySpec, const FGameplayAbilityActorInfo&, ActorInfo);

UDELEGATE()
DECLARE_DYNAMIC_MULTICAST_DELEGATE_SixParams(FOnPostFailedActivateGraspAbility, UGraspComponent*, GraspComponent,
	const AActor*, SourceActor, UPrimitiveComponent*, GraspableComponent, EGraspAbilityComponentSource, Source,
	const FGameplayAbilitySpec&, AbilitySpec, const FGameplayAbilityActorInfo&, ActorInfo);