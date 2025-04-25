// Copyright (c) Jared Taylor

#pragma once

#include "CoreMinimal.h"
#include "ComponentVisualizer.h"

/**
 * Draws editor visualizers for interactable components
 * This visualizes the interaction distance and angle and height
 */
class GRASPVISUALIZER_API FGraspInteractComponentVisualizer : public FComponentVisualizer
{
public:
	virtual void DrawVisualization(const UActorComponent* Component, const FSceneView* View,
		FPrimitiveDrawInterface* PDI) override;
};
