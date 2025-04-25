// Copyright (c) Jared Taylor

#pragma once

#include "CoreMinimal.h"
#include "Components/ArrowComponent.h"
#include "GraspInteractMarker.generated.h"


/**
 * Editor-only component used to define the locations and rotations of the interactable
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GRASP_API UGraspInteractMarker : public UArrowComponent
{
	GENERATED_BODY()

public:
	UGraspInteractMarker();

#if WITH_EDITOR
	void PropagateMarkers();
	virtual void OnComponentCreated() override;
	virtual void PostEditComponentMove(bool bFinished) override;
#endif
};
