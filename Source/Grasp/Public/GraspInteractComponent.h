// Copyright (c) Jared Taylor

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Components/SceneComponent.h"
#include "GraspInteractComponent.generated.h"


/**
 * This component can optionally be placed on the interactable actor
 * It defines a point from which interaction can occur
 * Actors with no interactable component will use the root component as the interaction point
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GRASP_API UGraspInteractComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	/**
	 * Gets fed to the ability if valid
	 * Used for contextual abilities based on which point we interacted from
	 * @warning This incurs replication cost
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Grasp)
	FGameplayTag InteractContextTag = FGameplayTag::EmptyTag;
};
