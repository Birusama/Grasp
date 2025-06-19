// Copyright (c) Jared Taylor

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "GraspEditorDeveloper.generated.h"

/**
 * Editor settings for Grasp
 */
UCLASS(Config=EditorPerProjectUserSettings, meta=(DisplayName="Grasp Editor Settings"))
class GRASP_API UGraspEditorDeveloper : public UDeveloperSettings
{
	GENERATED_BODY()

public:
#if WITH_EDITORONLY_DATA
	/** If true, the Grasp Editor will show a notification when the collision profile of a Graspable is automatically assigned */
	UPROPERTY(EditAnywhere, Config, Category=Grasp)
	bool bNotifyOnCollisionChanged = true;
#endif
};
