// Copyright (c) Jared Taylor


#include "Filtering/GraspFilter_Interface.h"

#include "GraspInteractable.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GraspFilter_Interface)


UGraspFilter_Interface::UGraspFilter_Interface(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{}

bool UGraspFilter_Interface::ShouldFilterTarget(const FTargetingRequestHandle& TargetingHandle,
	const FTargetingDefaultResultData& TargetData) const
{
	TRACE_CPUPROFILER_EVENT_SCOPE(GraspFilter_Interface::ShouldFilterTarget);

	const AActor* TargetActor = TargetData.HitResult.GetActor();

	// Check if the target actor is valid
	if (!IsValid(TargetActor))
	{
		return true;
	}
	
	// Check if pending kill or torn off
	if (TargetActor->IsPendingKillPending() || TargetActor->GetTearOff())
	{
		return true;
	}
	
	// Check if the target actor implements the IGraspInteractable interface
	if (!TargetActor->Implements<UGraspInteractable>())
	{
		return true;
	}

	// Check if the target actor is dead
	if (IGraspInteractable::Execute_IsGraspInteractableDead(TargetActor))
	{
		return true;
	}
	
	return true;
}
