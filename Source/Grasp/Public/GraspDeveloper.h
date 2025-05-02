// Copyright (c) Jared Taylor

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "Engine/CollisionProfile.h"
#include "GraspDeveloper.generated.h"

UENUM(BlueprintType)
enum class EGraspDefaultCollisionMode : uint8
{
	Profile,
	ObjectType,
	Disabled,
};

/**
 * Set defaults for Grasp
 */
UCLASS(Config=Game, meta=(DisplayName="Grasp Developer Settings"))
class GRASP_API UGraspDeveloper : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Config, Category=Grasp)
	EGraspDefaultCollisionMode GraspDefaultCollisionMode = EGraspDefaultCollisionMode::Profile;
	
	UPROPERTY(EditAnywhere, Config, Category=Grasp, meta=(EditCondition="GraspDefaultCollisionMode==EGraspDefaultCollisionMode::Profile", EditConditionHides))
	FCollisionProfileName GraspDefaultCollisionProfile = FCollisionProfileName(TEXT("NoCollision"));
	
	/** When adding new Grasp Components they will default to this Object Type */
	UPROPERTY(EditAnywhere, Config, Category=Grasp, meta=(EditCondition="GraspDefaultCollisionMode==EGraspDefaultCollisionMode::ObjectType", EditConditionHides))
	TEnumAsByte<enum ECollisionChannel> GraspDefaultObjectType = ECC_WorldDynamic;

	UPROPERTY(EditAnywhere, Config, Category=Grasp, meta=(EditCondition="GraspDefaultCollisionMode==EGraspDefaultCollisionMode::ObjectType", EditConditionHides))
	bool bSetDefaultOverlapChannel = false;
	
	/** When adding new Grasp Components they will default to overlapping this Trace Channel */
	UPROPERTY(EditAnywhere, Config, Category=Grasp, meta=(EditCondition="GraspDefaultCollisionMode==EGraspDefaultCollisionMode::ObjectType&&bSetDefaultOverlapChannel", EditConditionHides))
	TEnumAsByte<enum ECollisionChannel> GraspDefaultOverlapChannel = ECC_Visibility;
	
	/**
	 * If true, the UGraspScanTask will not check the owner Ability's settings. Disable this if you have abnormal settings for good reason.
	 * Shipping builds are never checked regardless
	 */
	UPROPERTY(EditAnywhere, Config, AdvancedDisplay, Category=Grasp)
	bool bDisableScanTaskAbilityErrorChecking = false;
};
