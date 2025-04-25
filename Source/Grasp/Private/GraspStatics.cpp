// Copyright (c) Jared Taylor


#include "GraspStatics.h"

#include "Engine/Engine.h"
#include "DrawDebugHelpers.h"
#include "GraspComponent.h"
#include "GraspData.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/HUD.h"
#include "Components/PrimitiveComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GraspStatics)


UGraspComponent* UGraspStatics::FindGraspComponentForActor(AActor* Actor)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(GraspStatics::FindGraspComponentForActor);

	if (!IsValid(Actor))
	{
		return nullptr;
	}

	// Only Local and Authority has a Controller
	if (Actor->GetLocalRole() == ROLE_SimulatedProxy)
	{
		return nullptr;
	}

	// Maybe the actor is a controller
	if (const AController* Controller = Cast<AController>(Actor))
	{
		return Controller->FindComponentByClass<UGraspComponent>();
	}

	// Maybe the actor is a pawn
	if (const APawn* Pawn = Cast<APawn>(Actor))
	{
		if (const AController* Controller = Pawn->GetController())
		{
			return Controller->FindComponentByClass<UGraspComponent>();
		}
	}

	// Maybe the actor is a player state
	if (const APlayerState* PlayerState = Cast<APlayerState>(Actor))
	{
		if (const AController* Controller = PlayerState->GetOwningController())
		{
			return Controller->FindComponentByClass<UGraspComponent>();
		}
	}

	// Unsupported actor
	return nullptr;
}

UGraspComponent* UGraspStatics::FindGraspComponentForPawn(APawn* Pawn)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(GraspStatics::FindGraspComponentForPawn);
	
	if (!IsValid(Pawn))
	{
		return nullptr;
	}
	
	// Only Local and Authority has a Controller
	if (Pawn->GetLocalRole() == ROLE_SimulatedProxy)
	{
		return nullptr;
	}
	
	if (const AController* Controller = Pawn->GetController())
	{
		return Controller->FindComponentByClass<UGraspComponent>();
	}
	return nullptr;
}

UGraspComponent* UGraspStatics::FindGraspComponentForController(AController* Controller)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(GraspStatics::FindGraspComponentForController);
	
	if (!IsValid(Controller))
	{
		return nullptr;
	}
	
	// Only Local and Authority has a Controller
	if (Controller->GetLocalRole() == ROLE_SimulatedProxy)
	{
		return nullptr;
	}
	
	return Controller->FindComponentByClass<UGraspComponent>();
}

UGraspComponent* UGraspStatics::FindGraspComponentForPlayerState(APlayerState* PlayerState)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(GraspStatics::FindGraspComponentForPlayerState);
	
	if (!IsValid(PlayerState))
	{
		return nullptr;
	}
	
	// Only Local and Authority has a Controller
	if (PlayerState->GetLocalRole() == ROLE_SimulatedProxy)
	{
		return nullptr;
	}
	
	if (const AController* Controller = PlayerState->GetOwningController())
	{
		return Controller->FindComponentByClass<UGraspComponent>();
	}
	return nullptr;
}

bool UGraspStatics::IsWithinInteractAngle(const FVector& SourceLocation, const FVector& TargetLocation, const FVector& FacingVector, float Degrees, bool bCheck2D, bool
	bHalfCircle)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(GraspStatics::IsWithinInteractAngle);
	
	const FVector Diff = TargetLocation - SourceLocation;
	const FVector Dir = bCheck2D ? Diff.GetSafeNormal2D() : Diff.GetSafeNormal();
	const float Radians = FMath::DegreesToRadians(Degrees * (bHalfCircle ? 1.f : 0.5f));
	const float Acos = FMath::Acos(FacingVector | Dir);
	return Acos <= Radians;
}

bool UGraspStatics::IsInteractableWithinAngle(const FVector& InteractableLocation, const FVector& InteractorLocation,
	const FVector& InteractorFacingVector, float Degrees)
{
	return IsWithinInteractAngle(InteractorLocation, InteractableLocation,
		InteractorFacingVector, Degrees, true, false);
}

bool UGraspStatics::CanInteractWithinAngle(const AActor* Interactor, const FVector& InteractableLocation, float Degrees)
{
	if (!IsValid(Interactor))
	{
		return false;
	}
	return IsInteractableWithinAngle(InteractableLocation, Interactor->GetActorLocation(),
		Interactor->GetActorForwardVector(), Degrees);
}

bool UGraspStatics::IsWithinInteractDistance(const FVector& SourceLocation, const FVector& TargetLocation,
	float Distance, bool bCheck2D)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(GraspStatics::IsWithinInteractDistance);
	
	const float DistSquared = bCheck2D ?
		FVector::DistSquared2D(SourceLocation, TargetLocation) : FVector::DistSquared(SourceLocation, TargetLocation);
	return DistSquared <= FMath::Square(Distance);
}

bool UGraspStatics::IsInteractableWithinDistance(const FVector& InteractableLocation, const FVector& InteractorLocation,
	float Distance, bool bCheck2D)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(GraspStatics::IsInteractableWithinDistance);
	
	return IsWithinInteractDistance(InteractorLocation, InteractableLocation,
		Distance, bCheck2D);
}

bool UGraspStatics::CanInteractWithinDistance(const AActor* Interactor, const FVector& InteractableLocation,
	float Distance, bool bCheck2D)
{
	if (!IsValid(Interactor))
	{
		return false;
	}
	return IsInteractableWithinDistance(InteractableLocation, Interactor->GetActorLocation(), Distance, bCheck2D);
}

bool UGraspStatics::CanInteractWithinAngleAndDistance(const AActor* Interactor, const FVector& InteractableLocation,
	float Degrees, float Distance)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(GraspStatics::CanInteractWithinAngleAndDistance);
	
	if (!IsValid(Interactor))
	{
		return false;
	}

	const bool bWithinAngle = IsInteractableWithinAngle(InteractableLocation,
		Interactor->GetActorLocation(), Interactor->GetActorForwardVector(), Degrees);

	const bool bWithinDistance = IsInteractableWithinDistance(InteractableLocation,
		Interactor->GetActorLocation(), Distance);

	return bWithinAngle && bWithinDistance;
}

bool UGraspStatics::IsInteractableWithinHeight(const FVector& InteractableLocation, const FVector& InteractorLocation,
	float MaxHeightAbove, float MaxHeightBelow)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(GraspStatics::IsInteractableWithinHeight);
	
	const float Height = InteractableLocation.Z - InteractorLocation.Z;
	return Height >= -MaxHeightBelow && Height <= MaxHeightAbove;
}

bool UGraspStatics::CanInteractWithinHeight(const AActor* Interactor, const FVector& InteractableLocation,
	float MaxHeightAbove, float MaxHeightBelow)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(GraspStatics::CanInteractWithinHeight);
	
	if (!IsValid(Interactor))
	{
		return false;
	}
	return IsInteractableWithinHeight(InteractableLocation, Interactor->GetActorLocation(),
		MaxHeightAbove, MaxHeightBelow);
}

EGraspInteractQuery UGraspStatics::CanInteractWith(const AActor* Interactor, UGraspData* Data,
	TArray<FGraspMarker>& Markers, float& NormalizedAngleDiff, float& NormalizedDistance,
	float& NormalizedHighlightDistance, FGraspMarker& BestMarker)
{
	NormalizedAngleDiff = 0.f;
	NormalizedDistance = 0.f;
	NormalizedHighlightDistance = 0.f;
	
	if (!IsValid(Interactor))
	{
		return EGraspInteractQuery::None;
	}

	if (!ensure(Data != nullptr))
	{
		return EGraspInteractQuery::None;
	}

	const bool b2D = Data->bGraspDistance2D;

	// Find the closest marker -- test the closest first
	const FVector InteractorLoc = Interactor->GetActorLocation();
	Markers.Sort([&InteractorLoc, &b2D](const FGraspMarker& A, const FGraspMarker& B)
	{
		const float DistA = b2D ? FVector::Dist2D(InteractorLoc, A.GetLocation()) : FVector::Dist(InteractorLoc, A.GetLocation());
		const float DistB = b2D ? FVector::Dist2D(InteractorLoc, B.GetLocation()) : FVector::Dist(InteractorLoc, B.GetLocation());
		return DistA < DistB;
	});

	// Test each marker
	for (const FGraspMarker& Marker : Markers)
	{
		// Check if within distance
		if (!IsInteractableWithinDistance(Marker.GetLocation(), InteractorLoc, Data->MaxGraspDistance))
		{
			// Check if highlight is enabled and within distance
			if (Data->MaxHighlightDistance > 0.f && IsInteractableWithinDistance(Marker.GetLocation(), InteractorLoc, Data->MaxHighlightDistance))
			{
				NormalizedHighlightDistance = FMath::Clamp(FVector::Dist2D(Marker.GetLocation(), InteractorLoc) / Data->MaxHighlightDistance, 0.f, 1.f);
				BestMarker = Marker;

				// We sorted by distance, if this one is too far, the rest are too
				return EGraspInteractQuery::Highlight;
			}

			// We sorted by distance, if this one is too far, the rest are too
			return EGraspInteractQuery::None;
		}

		const float DistNormalized = b2D ? FVector::Dist2D(Marker.GetLocation(), InteractorLoc) :
			FVector::Dist(Marker.GetLocation(), InteractorLoc);
		NormalizedDistance = FMath::Clamp(DistNormalized / Data->MaxGraspDistance, 0.f, 1.f);

		// Check if within angle
		if (!IsInteractableWithinAngle(Marker.GetLocation(), InteractorLoc,
			Interactor->GetActorForwardVector(), Data->MaxGraspAngle))
		{
			return EGraspInteractQuery::None;
		}
		
		NormalizedAngleDiff = FMath::Clamp(FVector::Dist2D(Marker.GetLocation(), InteractorLoc) / Data->MaxGraspAngle, 0.f, 1.f);

		// Check if within height
		if (!IsInteractableWithinHeight(Marker.GetLocation(), InteractorLoc, Data->MaxHeightAbove, Data->MaxHeightBelow))
		{
			return EGraspInteractQuery::None;
		}

		BestMarker = Marker;
		return EGraspInteractQuery::Interact;
	}

	return EGraspInteractQuery::None;
}

FTransform UGraspStatics::GetGraspMarkerWorldTransform(const FGraspMarker& Marker)
{
	return Marker.GetTransform();
}
