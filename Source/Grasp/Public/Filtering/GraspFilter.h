// Copyright (c) Jared Taylor

#pragma once

#include "CoreMinimal.h"
#include "Tasks/TargetingFilterTask_BasicFilterTemplate.h"
#include "GraspFilter.generated.h"

/**
 * Filter targets that don't implement IGraspInteractable
 * Mandatory for Grasp interaction -- no other implement checks are performed
 */
UCLASS(Blueprintable, DisplayName="Grasp Filter (Interactable)")
class GRASP_API UGraspFilter : public UTargetingFilterTask_BasicFilterTemplate
{
	GENERATED_BODY()

public:
	UGraspFilter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
	virtual bool ShouldFilterTarget(const FTargetingRequestHandle& TargetingHandle, const FTargetingDefaultResultData& TargetData) const override;
};
