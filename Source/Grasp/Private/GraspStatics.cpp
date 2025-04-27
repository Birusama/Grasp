﻿// Copyright (c) Jared Taylor


#include "GraspStatics.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "Engine/Engine.h"
#include "DrawDebugHelpers.h"
#include "Graspable.h"
#include "GraspComponent.h"
#include "GraspData.h"
#include "GraspDeveloper.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/HUD.h"
#include "Components/PrimitiveComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GraspStatics)


FGameplayAbilitySpec* UGraspStatics::FindGraspAbilitySpec(const UAbilitySystemComponent* ASC,
	const UPrimitiveComponent* GraspableComponent)
{
	const IGraspable* Graspable = GraspableComponent ? CastChecked<IGraspable>(GraspableComponent) : nullptr;
	const TSubclassOf<UGameplayAbility>& GraspAbility = Graspable->GetGraspData()->GraspAbility;
	return ASC->FindAbilitySpecFromClass(GraspAbility);
}

bool UGraspStatics::PrepareGraspAbilityDataPayload(const UPrimitiveComponent* GraspableComponent,
	FGameplayEventData& Payload, EGraspAbilityComponentSource Source)
{
	Payload = {};
	if (Source == EGraspAbilityComponentSource::Custom)
	{
		return false;
	}
	
	const IGraspable* Graspable = GraspableComponent ? CastChecked<IGraspable>(GraspableComponent) : nullptr;
	const TArray<FGameplayAbilityTargetData*> OptionalTargetData = Graspable->GatherOptionalGraspTargetData();
	if (OptionalTargetData.Num() == 0 && Source == EGraspAbilityComponentSource::Automatic)
	{
		return false;
	}
	Payload.OptionalObject = GraspableComponent;
	for (FGameplayAbilityTargetData* TargetData : OptionalTargetData)
	{
		Payload.TargetData.Add(TargetData);
	}
	return true;
}

bool UGraspStatics::CanGraspActivateAbility(const AActor* SourceActor, const UPrimitiveComponent* GraspableComponent,
	EGraspAbilityComponentSource Source)
{
	// Find the GraspComponent from the SourceActor
	const UGraspComponent* GraspComponent = FindGraspComponentForActor(SourceActor);
	if (!GraspComponent)
	{
		return false;
	}

	// Get the ASC from the GraspComponent
	const UAbilitySystemComponent* ASC = GraspComponent->GetASC();
	if (!ASC)
	{
		return false;
	}
	
	// Retrieve the ability spec
	const FGameplayAbilitySpec* Spec = FindGraspAbilitySpec(ASC, GraspableComponent);
	if (!Spec || !Spec->Ability)
	{
		return false;
	}
	
	// Check if we can activate the ability
	const FGameplayAbilityActorInfo* ActorInfo = ASC->AbilityActorInfo.Get();
	FGameplayTagContainer RelevantTags;
	if (Spec->Ability->CanActivateAbility(Spec->Handle, ActorInfo, nullptr, nullptr, &RelevantTags))
	{
		FGameplayEventData Payload;
		if (PrepareGraspAbilityDataPayload(GraspableComponent, Payload, Source))
		{
			return Spec->Ability->ShouldAbilityRespondToEvent(ASC->AbilityActorInfo.Get(), &Payload);
		}
		return true;
	}
	return false;
}

bool UGraspStatics::TryActivateGraspAbility(const AActor* SourceActor, UPrimitiveComponent* GraspableComponent,
	EGraspAbilityComponentSource Source)
{
	// Find the grasp component (from the SourceActor's Controller)
	UGraspComponent* GraspComponent = FindGraspComponentForActor(SourceActor);
	if (!GraspComponent)
	{
		return false;
	}

	// Get the ASC from the GraspComponent
	UAbilitySystemComponent* ASC = GraspComponent->GetASC();
	if (!ASC)
	{
		return false;
	}

	// Get the target component from the target data -- we have already thoroughly validated this elsewhere
	const IGraspable* Graspable = GraspableComponent ? CastChecked<IGraspable>(GraspableComponent) : nullptr;

	// Retrieve the ability spec
	FGameplayAbilitySpec* Spec = FindGraspAbilitySpec(ASC, GraspableComponent);
	if (!Spec || !Spec->Ability)
	{
		return false;
	}

	// Notify
	GraspComponent->PreTryActivateGraspAbility(SourceActor, GraspableComponent, Source, Spec);
	
	// Optional target data
	const TArray<FGameplayAbilityTargetData*> OptionalTargetData = Graspable->GatherOptionalGraspTargetData();
	FGameplayEventData Payload;

	// Prepare the payload
	if (PrepareGraspAbilityDataPayload(GraspableComponent, Payload, Source))
	{
		FGameplayAbilityActorInfo* ActorInfo = ASC->AbilityActorInfo.Get();
		if (ASC->TriggerAbilityFromGameplayEvent(Spec->Handle, ActorInfo,
			FGraspTags::Grasp_Interact_Activate, &Payload, *ASC))
		{
			GraspComponent->PostActivateGraspAbility(SourceActor, GraspableComponent, Source, Spec, ActorInfo);
			return true;
		}
		else
		{
			GraspComponent->PostFailedActivateGraspAbility(SourceActor, GraspableComponent, Source, Spec, ActorInfo);
			return false;
		}
	}

	// Assign the component as the source object -- not required with event data because we send as optional object
	Spec->SourceObject = GraspableComponent;
	ASC->MarkAbilitySpecDirty(*Spec);

	if (ASC->TryActivateAbility(Spec->Handle, true))
	{
		GraspComponent->PostActivateGraspAbility(SourceActor, GraspableComponent, Source, Spec);
		return true;
	}
	else
	{
		GraspComponent->PostFailedActivateGraspAbility(SourceActor, GraspableComponent, Source, Spec);
		return false;
	}
}

UObject* UGraspStatics::GetGraspSourceObject(const UGameplayAbility* Ability)
{
	// Don't use the built-in GetSourceObject() -- it expects we're instantiated, but we're not, we manually built this into the spec
	const UAbilitySystemComponent* const ASC = Ability->GetAbilitySystemComponentFromActorInfo_Checked();
	if (const FGameplayAbilitySpec* Spec = ASC->FindAbilitySpecFromHandle(Ability->GetCurrentAbilitySpecHandle()))
	{
		return Spec->SourceObject.IsValid() ? Spec->SourceObject.Get() : nullptr;
	}
	return nullptr;
}

const UObject* UGraspStatics::GetGraspObjectFromPayload(const FGameplayEventData& Payload)
{
	return Payload.OptionalObject;
}

const UPrimitiveComponent* UGraspStatics::K2_GetGraspableComponent(const UGameplayAbility* Ability,
	FGameplayEventData Payload, TSubclassOf<UPrimitiveComponent> ComponentType)
{
	const UObject* Object = GetGraspObjectFromPayload(Payload);
	if (!Object)
	{
		Object = GetGraspSourceObject(Ability);
	}
	if (Object)
	{
		if (Object->IsA(ComponentType))
		{
			return Cast<UPrimitiveComponent>(Object);
		}
	}
	return nullptr;
}

const UPrimitiveComponent* UGraspStatics::K2_GetGraspablePrimitive(const UGameplayAbility* Ability,
	FGameplayEventData Payload)
{
	const UObject* Object = GetGraspObjectFromPayload(Payload);
	if (!Object)
	{
		Object = GetGraspSourceObject(Ability);
	}
	if (Object)
	{
		return Cast<UPrimitiveComponent>(Object);
	}
	return nullptr;
}

UAbilitySystemComponent* UGraspStatics::GraspFindAbilitySystemComponentForActor(const AActor* Actor)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(GraspStatics::GraspFindAbilitySystemComponentForActor);

	if (!IsValid(Actor))
	{
		return nullptr;
	}

	// CPP only
	const IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(Actor);
	if (ASI)
	{
		return ASI->GetAbilitySystemComponent();
	}

	// Maybe the actor is a pawn
	if (const APawn* Pawn = Cast<APawn>(Actor))
	{
		// And maybe the ASC is on the pawn
		if (UAbilitySystemComponent* ASC = Pawn->FindComponentByClass<UAbilitySystemComponent>())
		{
			return ASC;
		}

		// Maybe the ASC is on the player state
		if (const APlayerState* PlayerState = Pawn->GetPlayerState())
		{
			return PlayerState->FindComponentByClass<UAbilitySystemComponent>();
		}
	}

	return nullptr;
}

UGraspComponent* UGraspStatics::FindGraspComponentForActor(const AActor* Actor)
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

void UGraspStatics::FlushServerMovesForActor(AActor* CharacterActor)
{
	if (IsValid(CharacterActor))
	{
		if (const ACharacter* Character = Cast<ACharacter>(CharacterActor))
		{
			if (UCharacterMovementComponent* CharacterMovement = Character->GetCharacterMovement())
			{
				CharacterMovement->FlushServerMoves();
			}
		}
	}
}

void UGraspStatics::FlushServerMoves(ACharacter* Character)
{
	if (IsValid(Character))
	{
		if (UCharacterMovementComponent* CharacterMovement = Character->GetCharacterMovement())
		{
			CharacterMovement->FlushServerMoves();
		}
	}
}

bool UGraspStatics::IsWithinInteractAngle(const FVector& SourceLocation, const FVector& TargetLocation, const FVector& Forward, float Degrees, bool bCheck2D, bool
	bHalfCircle)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(GraspStatics::IsWithinInteractAngle);
	
	const FVector Diff = TargetLocation - SourceLocation;
	const FVector Dir = bCheck2D ? Diff.GetSafeNormal2D() : Diff.GetSafeNormal();
	const float Radians = FMath::DegreesToRadians(Degrees * (bHalfCircle ? 1.f : 0.5f));
	const float Acos = FMath::Acos(Forward | Dir);
	return Acos <= Radians;
}

bool UGraspStatics::IsInteractableWithinAngle(const FVector& InteractableLocation, const FVector& InteractorLocation,
	const FVector& Forward, float Degrees)
{
	return IsWithinInteractAngle(InteractorLocation, InteractableLocation,
		Forward, Degrees, true, false);
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

EGraspQueryResult UGraspStatics::CanInteractWith(const AActor* Interactor, const UPrimitiveComponent* Component,
	float& NormalizedAngleDiff, float& NormalizedDistance, float& NormalizedHighlightDistance)
{
	NormalizedAngleDiff = 0.f;
	NormalizedDistance = 0.f;
	NormalizedHighlightDistance = 0.f;

	// Validate the interactor
	if (!IsValid(Interactor))
	{
		return EGraspQueryResult::None;
	}

	// Validate the graspable
	if (!Component)
	{
		return EGraspQueryResult::None;
	}

	// Validate the grasp data
	const IGraspable* Graspable = CastChecked<IGraspable>(Component);
	if (!ensure(Graspable->GetGraspData() != nullptr))
	{
		return EGraspQueryResult::None;
	}

	const FVector InteractorLocation = Interactor->GetActorLocation();
	const FVector Location = Component->GetComponentLocation();
	const FVector Forward = Component->GetForwardVector();
	const UGraspData* Data = Graspable->GetGraspData();

	const float Angle = Interactor->HasAuthority() && Interactor->GetNetMode() != NM_Standalone ?
		Data->MaxGraspAngle * Data->AuthNetToleranceAngleScalar : Data->MaxGraspAngle;

	const float Distance = Interactor->HasAuthority() && Interactor->GetNetMode() != NM_Standalone ?
		Data->MaxGraspDistance * Data->AuthNetToleranceDistanceScalar : Data->MaxGraspDistance;

	const float HighlightDistance = Interactor->HasAuthority() && Interactor->GetNetMode() != NM_Standalone ?
		Data->MaxHighlightDistance * Data->AuthNetToleranceDistanceScalar : Data->MaxHighlightDistance;

	const float MaxHeightAbove = Interactor->HasAuthority() && Interactor->GetNetMode() != NM_Standalone ?
		Data->MaxHeightAbove * Data->AuthNetToleranceDistanceScalar : Data->MaxHeightAbove;

	const float MaxHeightBelow = Interactor->HasAuthority() && Interactor->GetNetMode() != NM_Standalone ?
		Data->MaxHeightBelow * Data->AuthNetToleranceDistanceScalar : Data->MaxHeightBelow;
	
	// Check if within distance
	if (!IsInteractableWithinDistance(Location, InteractorLocation, Distance))
	{
		// Check if highlight is enabled and within distance
		if (HighlightDistance > 0.f && IsInteractableWithinDistance(Location, InteractorLocation, HighlightDistance))
		{
			NormalizedHighlightDistance = FMath::Clamp(
				FVector::Dist2D(Location, InteractorLocation) / HighlightDistance, 0.f, 1.f);

			// We sorted by distance, if this one is too far, the rest are too
			return EGraspQueryResult::Highlight;
		}

		// We sorted by distance, if this one is too far, the rest are too
		return EGraspQueryResult::None;
	}

	const float DistNormalized = Data->bGraspDistance2D ? FVector::Dist2D(Location, InteractorLocation) :
		FVector::Dist(Location, InteractorLocation);
	NormalizedDistance = FMath::Clamp(DistNormalized / Distance, 0.f, 1.f);

	// Check if within angle
	if (!IsInteractableWithinAngle(Location, InteractorLocation, Forward, Angle))
	{
		return EGraspQueryResult::None;
	}
	
	NormalizedAngleDiff = FMath::Clamp(
		FVector::Dist2D(Location, InteractorLocation) / Angle, 0.f, 1.f);

	// Check if within height
	if (!IsInteractableWithinHeight(Location, InteractorLocation, MaxHeightAbove, MaxHeightBelow))
	{
		return EGraspQueryResult::None;
	}

	return EGraspQueryResult::Interact;
}
