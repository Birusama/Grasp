// Copyright (c) Jared Taylor

#pragma once

#include "CoreMinimal.h"
#include "GraspTypes.h"
#include "Tasks/TargetingFilterTask_BasicFilterTemplate.h"
#include "GraspFilter_IsWithinGraspableData.generated.h"


/**
 * Filter targets by whether they are within the parameters defined in UGraspData such as the angle and distance
 */
UCLASS(Blueprintable, DisplayName="Grasp Filter (Is Within Graspable Data)")
class GRASP_API UGraspFilter_IsWithinGraspableData : public UTargetingFilterTask_BasicFilterTemplate
{
	GENERATED_BODY()

public:
	/** What result we must pass to not be filtered out */
	UPROPERTY(EditAnywhere, Category="Grasp Filter", meta=(InvalidEnumValues="None"))
	EGraspQueryResult Threshold = EGraspQueryResult::Interact;

public:
	UGraspFilter_IsWithinGraspableData(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
	virtual bool ShouldFilterTarget(const FTargetingRequestHandle& TargetingHandle, const FTargetingDefaultResultData& TargetData) const override;
};
