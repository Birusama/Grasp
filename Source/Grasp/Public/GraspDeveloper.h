// Copyright (c) Jared Taylor

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "GraspDeveloper.generated.h"


/**
 * Set defaults for Grasp
 */
UCLASS(Config=Game, meta=(DisplayName="Grasp Developer Settings"))
class GRASP_API UGraspDeveloper : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	/** When adding new Grasp Components they will default to this Object Type */
	UPROPERTY(EditAnywhere, Config, Category=Grasp)
	TEnumAsByte<enum ECollisionChannel> GraspDefaultObjectType;
};
