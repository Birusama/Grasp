// Copyright (c) Jared Taylor


#include "Filtering/GraspFilter_IsWithinGraspableRange.h"

#include "GraspComponent.h"
#include "GraspStatics.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GraspFilter_IsWithinGraspableRange)


UGraspFilter_IsWithinGraspableRange::UGraspFilter_IsWithinGraspableRange(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{}

bool UGraspFilter_IsWithinGraspableRange::ShouldFilterTarget(const FTargetingRequestHandle& TargetingHandle,
	const FTargetingDefaultResultData& TargetData) const
{
	TRACE_CPUPROFILER_EVENT_SCOPE(GraspFilter_IsWithinGraspableData::ShouldFilterTarget);

	// Find the source actor
	const FTargetingSourceContext* SourceContext = FTargetingSourceContext::Find(TargetingHandle);
	if (!SourceContext || !IsValid(SourceContext->SourceActor))
	{
		return true;
	}

	// Get the grasp component from the source actor
	const TObjectPtr<AActor> SourceActor = SourceContext->SourceActor;
	const UPrimitiveComponent* TargetComponent = TargetData.HitResult.GetComponent();

	// Query if we can interact with the target based on angle and distance
	float NormalizedDistance, NormalizedHighlightDistance = 0.f;
	const EGraspQueryResult Result = UGraspStatics::CanInteractWithRange(SourceActor, TargetComponent,
		NormalizedDistance, NormalizedHighlightDistance);

	// Return the result based on the threshold
	bool bCanInteract;
	switch (Result)
	{
	case EGraspQueryResult::Highlight:
		bCanInteract = Threshold == EGraspQueryResult::Highlight;
		break;
	case EGraspQueryResult::Interact:
		bCanInteract = true;
		break;
	default:
		return true;
	}
	
	return !bCanInteract;
}
