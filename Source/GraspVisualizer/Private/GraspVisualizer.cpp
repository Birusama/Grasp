// Copyright (c) Jared Taylor

#include "GraspVisualizer.h"

#include "Components/GraspableBoxComponent.h"
#include "Components/GraspableCapsuleComponent.h"
#include "Components/GraspableSkeletalMeshComponent.h"
#include "Components/GraspableSphereComponent.h"
#include "Components/GraspableStaticMeshComponent.h"
#include "GraspableVisualizer.h"
#include "Editor/UnrealEdEngine.h"
#include "UnrealEdGlobals.h"

#define LOCTEXT_NAMESPACE "FGraspVisualizerModule"

void FGraspVisualizerModule::StartupModule()
{
	GUnrealEd->RegisterComponentVisualizer(UGraspableBoxComponent::StaticClass()->GetFName(), MakeShareable(new FGraspableVisualizer()));
	GUnrealEd->RegisterComponentVisualizer(UGraspableCapsuleComponent::StaticClass()->GetFName(), MakeShareable(new FGraspableVisualizer()));
	GUnrealEd->RegisterComponentVisualizer(UGraspableSkeletalMeshComponent::StaticClass()->GetFName(), MakeShareable(new FGraspableVisualizer()));
	GUnrealEd->RegisterComponentVisualizer(UGraspableSphereComponent::StaticClass()->GetFName(), MakeShareable(new FGraspableVisualizer()));
	GUnrealEd->RegisterComponentVisualizer(UGraspableStaticMeshComponent::StaticClass()->GetFName(), MakeShareable(new FGraspableVisualizer()));
}

void FGraspVisualizerModule::ShutdownModule()
{
	if (!UObjectInitialized() || IsEngineExitRequested())
	{
		return;
	}
	
	GUnrealEd->UnregisterComponentVisualizer(UGraspableBoxComponent::StaticClass()->GetFName());
	GUnrealEd->UnregisterComponentVisualizer(UGraspableCapsuleComponent::StaticClass()->GetFName());
	GUnrealEd->UnregisterComponentVisualizer(UGraspableSkeletalMeshComponent::StaticClass()->GetFName());
	GUnrealEd->UnregisterComponentVisualizer(UGraspableSphereComponent::StaticClass()->GetFName());
	GUnrealEd->UnregisterComponentVisualizer(UGraspableStaticMeshComponent::StaticClass()->GetFName());
}

#undef LOCTEXT_NAMESPACE
    
IMPLEMENT_MODULE(FGraspVisualizerModule, GraspVisualizer)