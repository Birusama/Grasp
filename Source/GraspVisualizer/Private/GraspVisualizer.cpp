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
}

#undef LOCTEXT_NAMESPACE
    
IMPLEMENT_MODULE(FGraspVisualizerModule, GraspVisualizer)