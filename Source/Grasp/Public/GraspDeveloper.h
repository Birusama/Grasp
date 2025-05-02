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

	/**
	 * If true, the UGraspScanTask will not check the owner Ability's settings. Disable this if you have abnormal settings for good reason.
	 * Shipping builds are never checked regardless
	 */
	UPROPERTY(EditAnywhere, Config, AdvancedDisplay, Category=Grasp)
	bool bDisableScanTaskAbilityErrorChecking;
};
