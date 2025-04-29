// Copyright (c) Jared Taylor

#include "GraspEditor.h"

#include "GraspComponent.h"
#include "GraspCustomization.h"
#include "Components/GraspableBoxComponent.h"
#include "Components/GraspableCapsuleComponent.h"
#include "Components/GraspableSkeletalMeshComponent.h"
#include "Components/GraspableSphereComponent.h"
#include "Components/GraspableStaticMeshComponent.h"

#define LOCTEXT_NAMESPACE "FGraspEditorModule"

void FGraspEditorModule::StartupModule()
{
	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	
	PropertyModule.RegisterCustomClassLayout(UGraspComponent::StaticClass()->GetFName(),
		FOnGetDetailCustomizationInstance::CreateStatic(&FGraspCustomization::MakeInstance));

	PropertyModule.RegisterCustomClassLayout(UGraspableBoxComponent::StaticClass()->GetFName(),
		FOnGetDetailCustomizationInstance::CreateStatic(&FGraspCustomization::MakeInstance));

	PropertyModule.RegisterCustomClassLayout(UGraspableCapsuleComponent::StaticClass()->GetFName(),
		FOnGetDetailCustomizationInstance::CreateStatic(&FGraspCustomization::MakeInstance));

	PropertyModule.RegisterCustomClassLayout(UGraspableSkeletalMeshComponent::StaticClass()->GetFName(),
		FOnGetDetailCustomizationInstance::CreateStatic(&FGraspCustomization::MakeInstance));

	PropertyModule.RegisterCustomClassLayout(UGraspableSphereComponent::StaticClass()->GetFName(),
		FOnGetDetailCustomizationInstance::CreateStatic(&FGraspCustomization::MakeInstance));

	PropertyModule.RegisterCustomClassLayout(UGraspableStaticMeshComponent::StaticClass()->GetFName(),
		FOnGetDetailCustomizationInstance::CreateStatic(&FGraspCustomization::MakeInstance));
}

void FGraspEditorModule::ShutdownModule()
{
	if (FModuleManager::Get().IsModuleLoaded("PropertyEditor"))
	{
		FPropertyEditorModule* PropertyModule = FModuleManager::Get().GetModulePtr<FPropertyEditorModule>("PropertyEditor");
		
		PropertyModule->UnregisterCustomClassLayout(UGraspComponent::StaticClass()->GetFName());
		PropertyModule->UnregisterCustomClassLayout(UGraspableBoxComponent::StaticClass()->GetFName());
		PropertyModule->UnregisterCustomClassLayout(UGraspableCapsuleComponent::StaticClass()->GetFName());
		PropertyModule->UnregisterCustomClassLayout(UGraspableSkeletalMeshComponent::StaticClass()->GetFName());
		PropertyModule->UnregisterCustomClassLayout(UGraspableSphereComponent::StaticClass()->GetFName());
		PropertyModule->UnregisterCustomClassLayout(UGraspableStaticMeshComponent::StaticClass()->GetFName());
	}
}

#undef LOCTEXT_NAMESPACE
    
IMPLEMENT_MODULE(FGraspEditorModule, GraspEditor)