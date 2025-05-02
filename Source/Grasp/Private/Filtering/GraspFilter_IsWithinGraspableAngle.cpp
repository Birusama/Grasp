// Copyright (c) Jared Taylor


#include "Filtering/GraspFilter_IsWithinGraspableAngle.h"

#include "GraspComponent.h"
#include "GraspStatics.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GraspFilter_IsWithinGraspableAngle)


UGraspFilter_IsWithinGraspableAngle::UGraspFilter_IsWithinGraspableAngle(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{}

bool UGraspFilter_IsWithinGraspableAngle::ShouldFilterTarget(const FTargetingRequestHandle& TargetingHandle,
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
	float NormalizedAngle = 0.f;
	const bool bResult = UGraspStatics::CanInteractWithAngle(SourceActor, TargetComponent, NormalizedAngle);

	return !bResult;
}
