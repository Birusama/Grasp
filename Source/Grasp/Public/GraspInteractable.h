// Copyright (c) Jared Taylor

#pragma once

#include "CoreMinimal.h"
#include "GraspTypes.h"

#include "GraspInteractable.generated.h"

class UGraspData;

UINTERFACE()
class UGraspInteractable : public UInterface
{
	GENERATED_BODY()
};

/**
 * Grasp Interface
 * Used to retrieve GraspData from an interactable and query the interactable's state
 */
class GRASP_API IGraspInteractable
{
	GENERATED_BODY()

public:
	/**
	 * Retrieve the GraspData for this interactable
	 * This defines how we (the Pawn/Player/etc.) interact with the interactable actor,
	 * as well as how the interactable actor behaves when interacted with.
	 * Also includes parameters for adjusting the interaction distance and angle
	 *
	 * Optionally retrieve InteractPoints by using UGraspStatics::GetGraspInteractPoints()
	 * Otherwise the location and direction of the actor will be used instead
	 * 
	 * @return The GraspData for this interactable
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category=Grasp)
	UGraspData* GetGraspData(TArray<FGraspInteractPoint>& InteractPoints) const;
	virtual UGraspData* GetGraspData_Implementation(TArray<FGraspInteractPoint>& InteractPoints) const PURE_VIRTUAL(, return nullptr;)
	
	/**
	 * Dead actors have their abilities removed from the Pawn that they were granted to
	 * If the actor becomes available again in the future and is interacted with immediately after before the ability
	 * is re-granted, there will be de-sync.
	 *
	 * You do not need to check IsPendingKillPending() or IsTornOff(), this is done for you regardless
	 * 
	 * @return True if this actor is no longer available, e.g. a Barrel that is exploding, a Pawn who is dying
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category=Grasp)
	bool IsGraspInteractableDead() const;
	virtual bool IsGraspInteractableDead_Implementation() const { return false; }
};
