// Copyright (c) Jared Taylor

#pragma once

#include "CoreMinimal.h"
#include "GraspTypes.h"
#include "Tasks/TargetingFilterTask_BasicFilterTemplate.h"
#include "GraspFilter_IsWithinGraspableAngle.generated.h"


/**
 * Filter targets by whether they are within the parameters defined in UGraspData such as the angle and distance
 */
UCLASS(Blueprintable, DisplayName="Grasp Filter (Graspable Angle)")
class GRASP_API UGraspFilter_IsWithinGraspableAngle : public UTargetingFilterTask_BasicFilterTemplate
{
	GENERATED_BODY()

public:
	/**
	 * What result we must pass to not be filtered out
	 * This filter can be used to find targets that can be interacted with only, or targets that can be highlighted
	 */
	UPROPERTY(EditAnywhere, Category="Grasp Filter", meta=(InvalidEnumValues="None"))
	EGraspQueryResult Threshold = EGraspQueryResult::Interact;

public:
	UGraspFilter_IsWithinGraspableAngle(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	/** Called against every target data to determine if the target should be filtered out */
	virtual bool ShouldFilterTarget(const FTargetingRequestHandle& TargetingHandle, const FTargetingDefaultResultData& TargetData) const override;
};
