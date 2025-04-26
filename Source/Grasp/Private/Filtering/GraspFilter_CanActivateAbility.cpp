// Copyright (c) Jared Taylor


#include "Filtering/GraspFilter_CanActivateAbility.h"

#include "GraspComponent.h"
#include "GraspStatics.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GraspFilter_CanActivateAbility)


UGraspFilter_CanActivateAbility::UGraspFilter_CanActivateAbility(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{}

bool UGraspFilter_CanActivateAbility::ShouldFilterTarget(const FTargetingRequestHandle& TargetingHandle,
	const FTargetingDefaultResultData& TargetData) const
{
	TRACE_CPUPROFILER_EVENT_SCOPE(GraspFilter_CanActivateAbility::ShouldFilterTarget);

	// Find the source actor
	const FTargetingSourceContext* SourceContext = FTargetingSourceContext::Find(TargetingHandle);
	if (!SourceContext || !IsValid(SourceContext->SourceActor))
	{
		return true;
	}

	// Get the grasp component from the source actor
	const TObjectPtr<AActor> SourceActor = SourceContext->SourceActor;
	const UPrimitiveComponent* TargetComponent = TargetData.HitResult.GetComponent();

	// Filter out if we can't activate the ability
	const bool CanActivateAbility = UGraspStatics::CanGraspActivateAbility(SourceActor, TargetComponent, Source);
	return !CanActivateAbility;
}
