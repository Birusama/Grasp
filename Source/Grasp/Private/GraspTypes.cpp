// Copyright (c) Jared Taylor. All Rights Reserved


#include "GraspTypes.h"

#include "GraspInteractComponent.h"

DEFINE_LOG_CATEGORY(LogGrasp);

FGraspInteractPoint::FGraspInteractPoint(const USceneComponent* InComponent)
	: Location(FVector::ZeroVector)
	, Forward(FVector::ZeroVector)
	, ContextTag(FGameplayTag::EmptyTag)
{
	if (IsValid(InComponent))
	{
		Location = InComponent->GetComponentLocation();
		Forward = InComponent->GetForwardVector();
	}
}

FGraspInteractPoint::FGraspInteractPoint(const UGraspInteractComponent* InComponent)
	: Location(FVector::ZeroVector)
	, Forward(FVector::ZeroVector)
	, ContextTag(FGameplayTag::EmptyTag)
{
	if (IsValid(InComponent))
	{
		Location = InComponent->GetComponentLocation();
		Forward = InComponent->GetForwardVector();
		ContextTag = InComponent->InteractContextTag;
	}
}
