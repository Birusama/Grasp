// Copyright (c) Jared Taylor


#include "GraspInteractComponentVisualizer.h"

#include "GraspInteractComponent.h"


void FGraspInteractComponentVisualizer::DrawVisualization(const UActorComponent* InComponent, const FSceneView* View,
	FPrimitiveDrawInterface* PDI)
{
	const UGraspInteractComponent* Component = InComponent ? Cast<const UGraspInteractComponent>(InComponent) : nullptr;
	if (!Component || !IsValid(Component->GetOwner()))
	{
		return;
	}

	
}
