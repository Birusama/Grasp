// Copyright (c) Jared Taylor

#include "GraspVisualizer.h"
#include "Editor/UnrealEdEngine.h"
#include "UnrealEdGlobals.h"

#define LOCTEXT_NAMESPACE "FGraspVisualizerModule"

void FGraspVisualizerModule::StartupModule()
{
	GUnrealEd->RegisterComponentVisualizer(UGraspInteractComponent::StaticClass()->GetFName(), MakeShareable(new FGraspInteractComponentVisualizer()));
}

void FGraspVisualizerModule::ShutdownModule()
{
    
}

#undef LOCTEXT_NAMESPACE
    
IMPLEMENT_MODULE(FGraspVisualizerModule, GraspVisualizer)