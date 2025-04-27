// Copyright (c) Jared Taylor

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GraspTypes.h"

#include "GraspStatics.generated.h"

struct FGameplayEventData;
class UAbilitySystemComponent;
class UGraspComponent;
struct FScalableFloat;

/**
 * Helper functions for Grasp
 */
UCLASS()
class GRASP_API UGraspStatics : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	static FGameplayAbilitySpec* FindGraspAbilitySpec(const UAbilitySystemComponent* ASC, const UPrimitiveComponent* GraspableComponent);

	/**
	 * Required prior to CanGraspActivateAbility() or TryActivateGraspAbility()
	 * if checking ShouldAbilityRespondToEvent() or ActivateAbilityFromEvent()
	 *
	 * Payload will only be prepared if bAlwaysTriggerEvent is true or IGraspable::GatherOptionalGraspTargetData() returns any target data
	 * 
	 * @return True if a Payload was prepared, true if IGraspable::GatherOptionalGraspTargetData() returns any target data
	 */
	UFUNCTION(BlueprintCallable, Category=Grasp)
	static bool PrepareGraspAbilityDataPayload(const UPrimitiveComponent* GraspableComponent,
		FGameplayEventData& Payload, EGraspAbilityComponentSource Source = EGraspAbilityComponentSource::EventData);

	/** 
	 * Check CanActivateAbility()
	 * Optionally check ShouldAbilityRespondToEvent() if we have valid target data and bCheckTargetData is true
	 * bCheckTargetData will be ignored unless UGraspDeveloper::AbilityActivationHandling is set to Full
	 * @return True if the ability can be activated
	 */
	UFUNCTION(BlueprintCallable, Category=Grasp)
	static bool CanGraspActivateAbility(const AActor* SourceActor, const UPrimitiveComponent* GraspableComponent,
		EGraspAbilityComponentSource Source = EGraspAbilityComponentSource::EventData);

	/**
	 * Use instead of TryActivateAbility, will set the SourceObject to the GraspableComponent
	 * Optionally gathers target data from IGraspable::GatherOptionalGraspTargetData() and sends it to the ability
	 * Use bAlwaysTriggerEvent if you want to check ShouldAbilityRespondToEvent() even if the target data is empty
	 */
	UFUNCTION(BlueprintCallable, Category=Grasp)
	static bool TryActivateGraspAbility(const AActor* SourceActor, UPrimitiveComponent* GraspableComponent,
		EGraspAbilityComponentSource Source = EGraspAbilityComponentSource::EventData);

	static UObject* GetGraspSourceObject(const UGameplayAbility* Ability);
	static const UObject* GetGraspObjectFromPayload(const FGameplayEventData& Payload);
	
	/**
	 * Retrieve the graspable component from the ability and payload if its available ( ShouldAbilityRespondToEvent() and ActivateAbilityFromEvent() )
	 */
	template<typename T>
	static T* GetGraspableComponent(const UGameplayAbility* Ability, const FGameplayEventData& Payload);

	/**
	 * Retrieve the graspable component from the ability and payload if its available ( ShouldAbilityRespondToEvent() and ActivateAbilityFromEvent() )
	 */
	UFUNCTION(BlueprintCallable, Category=Grasp, meta=(DefaultToSelf="Ability", DeterminesOutputType="ComponentType", DisplayName="Get Graspable Component"))
	static const UPrimitiveComponent* K2_GetGraspableComponent(const UGameplayAbility* Ability,
		FGameplayEventData MaybePayload, TSubclassOf<UPrimitiveComponent> ComponentType);

	/**
	 * Retrieve the graspable primitive component from the ability and payload if its available ( ShouldAbilityRespondToEvent() and ActivateAbilityFromEvent() )
	 */
	UFUNCTION(BlueprintCallable, Category=Grasp, meta=(DefaultToSelf="Ability", DeterminesOutputType="ComponentType", DisplayName="Get Graspable Primitive"))
	static const UPrimitiveComponent* K2_GetGraspablePrimitive(const UGameplayAbility* Ability,
		FGameplayEventData MaybePayload);
	
	UFUNCTION(BlueprintCallable, Category=Grasp)
	static UAbilitySystemComponent* GraspFindAbilitySystemComponentForActor(const AActor* Actor);

	UFUNCTION(BlueprintCallable, Category=Grasp)
	static UGraspComponent* FindGraspComponentForActor(const AActor* Actor);

	/**
	 * Attempts to find a Grasp Component for the given Pawn
	 * Will return nullptr on SimulatedProxy
	 */
	UFUNCTION(BlueprintCallable, Category=Grasp)
	static UGraspComponent* FindGraspComponentForPawn(APawn* Pawn);

	/**
	 * Attempts to find a Grasp Component for the given Controller
	 * Will return nullptr on SimulatedProxy
	 */
	UFUNCTION(BlueprintCallable, Category=Grasp)
	static UGraspComponent* FindGraspComponentForController(AController* Controller);

	/**
	 * Attempts to find a Grasp Component for the given PlayerState
	 * Will return nullptr on SimulatedProxy
	 */
	UFUNCTION(BlueprintCallable, Category=Grasp)
	static UGraspComponent* FindGraspComponentForPlayerState(APlayerState* PlayerState);

public:
	/** Cast the CharacterActor to a character and flush server moves on its movement component */
	UFUNCTION(BlueprintCallable, Category=Grasp)
	static void FlushServerMovesForActor(AActor* CharacterActor);

	/** Flush server moves on the movement component */
	UFUNCTION(BlueprintCallable, Category=Grasp)
	static void FlushServerMoves(ACharacter* Character);
	
public:
	// /**
	//  * Determine if we can interact with the interactable
	//  */
	// UFUNCTION(BlueprintCallable, Category=Grasp)
	// static EGraspInteractQuery CanInteractWithGraspInteractable(const FVector& InteractorLocation, const FVector& InteractableLocation,
	// 	UGraspData* GraspData);

	/**
	 * Check if the TargetLocation is within the angle of the FacingVector
	 * @param SourceLocation The location of the source
	 * @param TargetLocation The location of the target
	 * @param Forward The facing vector of the target
	 * @param Degrees The angle in degrees
	 * @param bCheck2D If true, only the X and Y components of the vectors will be used
	 * @param bHalfCircle If true, the angle will be halved (i.e. 360 degrees becomes 180 degrees)
	 */
	UFUNCTION(BlueprintCallable, Category=Grasp)
	static bool IsWithinInteractAngle(const FVector& SourceLocation, const FVector& TargetLocation,
		const FVector& Forward, float Degrees, bool bCheck2D = true, bool bHalfCircle = false);

	/** 
	 * Check if the Interactor is within the angle of the Interactable
	 * @param InteractableLocation The location of the interactable
	 * @param InteractorLocation The location of the interactor
	 * @param Forward The facing vector of the interactable
	 * @param Degrees The angle in degrees
	 */
	UFUNCTION(BlueprintCallable, Category=Grasp)
	static bool IsInteractableWithinAngle(const FVector& InteractableLocation, const FVector& InteractorLocation,
		const FVector& Forward, float Degrees);

	/** 
	 * Check if the Interactor is within the angle of the Interactable
	 * @param Interactor The interactor actor
	 * @param InteractableLocation The location of the interactable
	 * @param Degrees The angle in degrees
	 */
	UFUNCTION(BlueprintCallable, Category=Grasp)
	static bool CanInteractWithinAngle(const AActor* Interactor, const FVector& InteractableLocation, float Degrees);

	/** 
	 * Check if the SourceLocation is within distance to the TargetLocation
	 * @param SourceLocation The location of the source
	 * @param TargetLocation The location of the target
	 * @param Distance The distance to check
	 * @param bCheck2D If true, only the X and Y components of the vectors will be used
	 */
	UFUNCTION(BlueprintCallable, Category=Grasp)
	static bool IsWithinInteractDistance(const FVector& SourceLocation, const FVector& TargetLocation,
		float Distance, bool bCheck2D = true);

	/** 
	 * Check if the Interactor is within distance to the Interactable
	 * @param InteractableLocation The location of the interactable
	 * @param InteractorLocation The location of the interactor
	 * @param Distance The distance to check
	 * @param bCheck2D If true, only the X and Y components of the vectors will be used
	 */
	UFUNCTION(BlueprintCallable, Category=Grasp)
	static bool IsInteractableWithinDistance(const FVector& InteractableLocation, const FVector& InteractorLocation,
		float Distance, bool bCheck2D = true);

	/** 
	 * Check if the Interactor is within distance to the Interactable
	 * @param Interactor The interactor actor
	 * @param InteractableLocation The location of the interactable
	 * @param Distance The distance to check
	 * @param bCheck2D If true, only the X and Y components of the vectors will be used
	 */
	UFUNCTION(BlueprintCallable, Category=Grasp)
	static bool CanInteractWithinDistance(const AActor* Interactor, const FVector& InteractableLocation, float Distance, bool bCheck2D = true);

	/**
	 * Check if the Interactor is within angle and distance to the Interactable
	 * @param Interactor The interactor actor
	 * @param InteractableLocation The location of the interactable
	 * @param Degrees The angle in degrees
	 * @param Distance The distance to check
	 */
	UFUNCTION(BlueprintCallable, Category=Grasp)
	static bool CanInteractWithinAngleAndDistance(const AActor* Interactor, const FVector& InteractableLocation,
		float Degrees, float Distance);

	UFUNCTION(BlueprintCallable, Category=Grasp)
	static bool IsInteractableWithinHeight(const FVector& InteractableLocation, const FVector& InteractorLocation,
		float MaxHeightAbove, float MaxHeightBelow);
	
	UFUNCTION(BlueprintCallable, Category=Grasp)
	static bool CanInteractWithinHeight(const AActor* Interactor, const FVector& InteractableLocation,
		float MaxHeightAbove, float MaxHeightBelow);
	
	UFUNCTION(BlueprintCallable, Category=Grasp)
	static EGraspQueryResult CanInteractWith(const AActor* Interactor, const UPrimitiveComponent* Graspable,
		float& NormalizedAngleDiff, float& NormalizedDistance, float& NormalizedHighlightDistance);
};

template <typename T>
T* UGraspStatics::GetGraspableComponent(const UGameplayAbility* Ability, const FGameplayEventData& Payload)
{
	if (const UObject* Obj = GetGraspObjectFromPayload(Payload))
	{
		return Cast<T>(Obj);
	}

	if (UObject* Source = GetGraspSourceObject(Ability))
	{
		return Cast<T>(Source);
	}

	return nullptr;
}
