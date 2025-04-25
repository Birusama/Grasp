// Copyright (c) Jared Taylor


#include "GraspInteractMarker.h"

#if WITH_EDITOR
#include "GraspInteractable.h"
#include "Misc/UObjectToken.h"
#include "Logging/MessageLog.h"
#endif

#include UE_INLINE_GENERATED_CPP_BY_NAME(GraspInteractMarker)


UGraspInteractMarker::UGraspInteractMarker()
{
#if WITH_EDITOR
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	SetIsReplicatedByDefault(false);
#endif

	bIsEditorOnly = true;
}

#if WITH_EDITOR
void UGraspInteractMarker::PropagateMarkers()
{
	if (!IsValid(GetOwner()))
	{
		return;
	}

	// Check if the owner implements IGraspInteractable
	if (!ensure(GetOwner()->Implements<UGraspInteractable>()))
	{
		FMessageLog("AssetCheck").Error()
			->AddToken(FUObjectToken::Create(this))
			->AddToken(FTextToken::Create(NSLOCTEXT("GraspInteractMarker", "InvalidGraspInteractable",
				"Grasp Interactable does not implement IGraspInteractable")));
		
		FMessageLog("AssetCheck").Open(EMessageSeverity::Error);
		
		return;
	}

	// Get a copy of the interact markers from the owner
	TMap<UGraspInteractMarker*, FGraspMarker> Markers = IGraspInteractable::Execute_GetEditorInteractMarkers(GetOwner());

	// Add or update this marker in the map
	const auto AP = GetAttachParent();
	FGraspMarker& Marker = Markers.FindOrAdd(this);
	Marker = FGraspMarker(this);

	// Propagate the copy back up to the owner
	IGraspInteractable::Execute_PropagateEditorInteractMarkers(GetOwner(), Markers);
}

void UGraspInteractMarker::OnComponentCreated()
{
	Super::OnComponentCreated();
	
	PropagateMarkers();
}

void UGraspInteractMarker::PostEditComponentMove(bool bFinished)
{
	Super::PostEditComponentMove(bFinished);

	PropagateMarkers();
}
#endif