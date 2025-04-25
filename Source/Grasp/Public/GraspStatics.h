// Copyright (c) Jared Taylor

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GraspTypes.h"

#include "GraspStatics.generated.h"

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
	/**
	 * Attempts to find a Grasp Component for the given Actor
	 * Must be Pawn, Controller, or PlayerState or will return nullptr
	 * Will return nullptr on SimulatedProxy
	 */
	UFUNCTION(BlueprintCallable, Category=Grasp)
	static UGraspComponent* FindGraspComponentForActor(AActor* Actor);
	
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
	 * @param FacingVector The facing vector of the source
	 * @param Degrees The angle in degrees
	 * @param bCheck2D If true, only the X and Y components of the vectors will be used
	 * @param bHalfCircle If true, the angle will be halved (i.e. 360 degrees becomes 180 degrees)
	 */
	UFUNCTION(BlueprintCallable, Category=Grasp)
	static bool IsWithinInteractAngle(const FVector& SourceLocation, const FVector& TargetLocation,
		const FVector& FacingVector, float Degrees, bool bCheck2D = true, bool bHalfCircle = false);

	/** 
	 * Check if the Interactor is within the angle of the Interactable
	 * @param InteractableLocation The location of the interactable
	 * @param InteractorLocation The location of the interactor
	 * @param InteractorFacingVector The facing vector of the interactor
	 * @param Degrees The angle in degrees
	 */
	UFUNCTION(BlueprintCallable, Category=Grasp)
	static bool IsInteractableWithinAngle(const FVector& InteractableLocation, const FVector& InteractorLocation,
		const FVector& InteractorFacingVector, float Degrees);

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
	static EGraspInteractQuery CanInteractWith(const AActor* Interactor, UGraspData* GraspData,
		UPARAM(ref)TArray<FGraspMarker>& Markers, float& NormalizedAngleDiff, float& NormalizedDistance,
		float& NormalizedHighlightDistance, FGraspMarker& BestMarker);

	/** Convenience getter for blueprint debugging -- used for the development of Grasp */
	UFUNCTION(BlueprintCallable, Category=Grasp, meta=(Keywords="debug"))
	static FTransform GetGraspMarkerWorldTransform(const FGraspMarker& Marker);
};
