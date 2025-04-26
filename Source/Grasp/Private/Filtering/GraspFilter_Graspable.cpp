﻿// Copyright (c) Jared Taylor


#include "Filtering/GraspFilter_Graspable.h"

#include "Graspable.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GraspFilter_Graspable)


UGraspFilter_Graspable::UGraspFilter_Graspable(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{}

bool UGraspFilter_Graspable::ShouldFilterTarget(const FTargetingRequestHandle& TargetingHandle,
	const FTargetingDefaultResultData& TargetData) const
{
	TRACE_CPUPROFILER_EVENT_SCOPE(GraspFilter_Graspable::ShouldFilterTarget);

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

	const UPrimitiveComponent* TargetComponent = TargetData.HitResult.GetComponent();
	const IGraspable* Graspable = TargetComponent ? CastChecked<IGraspable>(TargetComponent) : nullptr;
	
	// Check if graspable is valid
	if (!Graspable)
	{
		return true;
	}

	// No data
	if (!Graspable->GetGraspData())
	{
		return true;
	}

	// Check if the target is dead
	if (Graspable->IsGraspableDead())
	{
		return true;
	}
	
	return false;
}
