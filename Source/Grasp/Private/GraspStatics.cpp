﻿// Copyright (c) Jared Taylor


#include "GraspStatics.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "Engine/Engine.h"
#include "GraspableComponent.h"
#include "GraspComponent.h"
#include "GraspData.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Components/PrimitiveComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Misc/UObjectToken.h"

#if UE_ENABLE_DEBUG_DRAWING
#include "DrawDebugHelpers.h"
#endif

#include "GraspableOwner.h"
#include "Blueprint/SlateBlueprintLibrary.h"
#include "Components/Widget.h"
#include "Kismet/GameplayStatics.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GraspStatics)


FGameplayAbilitySpec* UGraspStatics::FindGraspAbilitySpec(const UAbilitySystemComponent* ASC,
	const UPrimitiveComponent* GraspableComponent)
{
	const IGraspableComponent* Graspable = GraspableComponent ? CastChecked<IGraspableComponent>(GraspableComponent) : nullptr;
	const TSubclassOf<UGameplayAbility>& GraspAbility = Graspable->GetGraspData()->GraspAbility;
	return ASC->FindAbilitySpecFromClass(GraspAbility);
}

bool UGraspStatics::PrepareGraspAbilityDataPayload(const UPrimitiveComponent* GraspableComponent,
	FGameplayEventData& Payload, const AActor* SourceActor, const FGameplayAbilityActorInfo* ActorInfo,
	EGraspAbilityComponentSource Source)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(GraspStatics::PrepareGraspAbilityDataPayload);
	
	Payload = {};

	// User might handle this in a custom way
	if (Source == EGraspAbilityComponentSource::Custom)
	{
		return false;
	}

	// We would have filtered if the type was invalid
	const IGraspableComponent* Graspable = GraspableComponent ? CastChecked<IGraspableComponent>(GraspableComponent) : nullptr;

	// Gather target data
	TArray<FGameplayAbilityTargetData*> OptionalTargetData = Graspable->GatherOptionalGraspTargetData(ActorInfo);

	// Gather from owner, if implemented
	if (const IGraspableOwner* GraspableOwner = Cast<IGraspableOwner>(GraspableComponent->GetOwner()))
	{
		const TArray<FGameplayAbilityTargetData*> OwnerTargetData = GraspableOwner->GatherOptionalGraspTargetData(ActorInfo);
		if (OwnerTargetData.Num() > 0)
		{
			OptionalTargetData.Append(OwnerTargetData);
		}
	}

	// We may only want to send the target data if we have it
	if (OptionalTargetData.Num() == 0 && Source == EGraspAbilityComponentSource::Automatic)
	{
		return false;
	}

	// Send the component along with the event data
	Payload.OptionalObject = GraspableComponent;

	// Send the target data along with the event data
	for (FGameplayAbilityTargetData* TargetData : OptionalTargetData)
	{
		Payload.TargetData.Add(TargetData);
	}

	return true;
}

bool UGraspStatics::CanGraspActivateAbility(const AActor* SourceActor, const UPrimitiveComponent* GraspableComponent,
	EGraspAbilityComponentSource Source)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(GraspStatics::CanGraspActivateAbility);
	
	if (!GraspableComponent)
	{
		return false;
	}

	// Is this a graspable component?
	if (!GraspableComponent->Implements<UGraspableComponent>())
	{
		UE_LOG(LogGrasp, Error, TEXT("CanGraspActivateAbility: Attempting to interact with an invalid component: %s belonging to %s"),
			*GetNameSafe(GraspableComponent), *GetNameSafe(SourceActor));

#if WITH_EDITOR
		FMessageLog("PIE").Error()
			->AddToken(FUObjectToken::Create(GraspableComponent))
			->AddToken(FUObjectToken::Create(GraspableComponent->GetOwner()->GetClass()))
			->AddToken(FTextToken::Create(FText::FromString(TEXT("Invalid Setup: Attempting to interact with an invalid component"))));
#endif
		
		return false;
	}
	
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
		if (PrepareGraspAbilityDataPayload(GraspableComponent, Payload, SourceActor, ActorInfo, Source))
		{
			return Spec->Ability->ShouldAbilityRespondToEvent(ActorInfo, &Payload);
		}
		return true;
	}
	return false;
}

bool UGraspStatics::TryActivateGraspAbility(const AActor* SourceActor, UPrimitiveComponent* GraspableComponent,
	EGraspAbilityComponentSource Source)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(GraspStatics::TryActivateGraspAbility);
	
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
	const IGraspableComponent* Graspable = GraspableComponent ? CastChecked<IGraspableComponent>(GraspableComponent) : nullptr;

	// Retrieve the ability spec
	FGameplayAbilitySpec* Spec = FindGraspAbilitySpec(ASC, GraspableComponent);
	if (!Spec || !Spec->Ability)
	{
		return false;
	}

	// Notify
	GraspComponent->PreTryActivateGraspAbility(SourceActor, GraspableComponent, Source, Spec);
	
	// Optional target data
	FGameplayAbilityActorInfo* ActorInfo = ASC->AbilityActorInfo.Get();
	const TArray<FGameplayAbilityTargetData*> OptionalTargetData = Graspable->GatherOptionalGraspTargetData(ActorInfo);
	FGameplayEventData Payload;

	// Prepare the payload
	if (PrepareGraspAbilityDataPayload(GraspableComponent, Payload, SourceActor, ActorInfo, Source))
	{
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
	TRACE_CPUPROFILER_EVENT_SCOPE(GraspStatics::GetGraspSourceObject);
	
	// Don't use the built-in GetSourceObject() -- it expects we're instantiated, but we're not, we manually built this into the spec
	const UAbilitySystemComponent* const ASC = Ability->GetAbilitySystemComponentFromActorInfo_Ensured();
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
	TRACE_CPUPROFILER_EVENT_SCOPE(GraspStatics::K2_GetGraspableComponent);
	
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
	TRACE_CPUPROFILER_EVENT_SCOPE(GraspStatics::K2_GetGraspablePrimitive);
	
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
	TRACE_CPUPROFILER_EVENT_SCOPE(GraspStatics::FlushServerMovesForActor);
	
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
	TRACE_CPUPROFILER_EVENT_SCOPE(GraspStatics::FlushServerMoves);
	
	if (IsValid(Character))
	{
		if (UCharacterMovementComponent* CharacterMovement = Character->GetCharacterMovement())
		{
			CharacterMovement->FlushServerMoves();
		}
	}
}

EGraspCardinal_4Way UGraspStatics::GetCardinalDirectionFromAngle_4Way(float Angle)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(GraspStatics::GetCardinalDirectionFromAngle_4Way);
	
	const float AngleAbs = FMath::Abs(Angle);

	// Forward
	if (AngleAbs <= 45.f)
	{
		return EGraspCardinal_4Way::Forward;
	}

	// Backward
	if (AngleAbs >= 135.f)
	{
		return EGraspCardinal_4Way::Backward;
	}

	// Right
	if (Angle > 0.f)
	{
		return EGraspCardinal_4Way::Right;
	}

	// Left
	return EGraspCardinal_4Way::Left;
}

EGraspCardinal_8Way UGraspStatics::GetCardinalDirectionFromAngle_8Way(float Angle)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(GraspStatics::GetCardinalDirectionFromAngle_8Way);
	
	const float AngleAbs = FMath::Abs(Angle);

	// Forward
	if (AngleAbs <= 22.5f)
	{
		return EGraspCardinal_8Way::Forward;
	}

	// Backward
	if (AngleAbs >= 157.5f)
	{
		return EGraspCardinal_8Way::Backward;
	}

	// Diagonal Fwd
	if (AngleAbs <= 67.5f)
	{
		return Angle > 0.f ? EGraspCardinal_8Way::ForwardRight : EGraspCardinal_8Way::ForwardLeft;
	}

	// Diagonal Bwd
	if (AngleAbs >= 112.5f)
	{
		return Angle > 0.f ? EGraspCardinal_8Way::BackwardRight : EGraspCardinal_8Way::BackwardLeft;
	}

	// Right
	if (Angle > 0.f)
	{
		return EGraspCardinal_8Way::Right;
	}

	// Left
	return EGraspCardinal_8Way::Left;
}

float UGraspStatics::CalculateCardinalAngle(const FVector& Direction, const FRotator& SourceRotation)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(GraspStatics::CalculateCardinalAngle);
	
	// Copied from UKismetAnimationLibrary::CalculateDirection
	
	if (!Direction.IsNearlyZero())
	{
		const FMatrix RotMatrix = FRotationMatrix(SourceRotation);
		const FVector ForwardVector = RotMatrix.GetScaledAxis(EAxis::X);
		const FVector RightVector = RotMatrix.GetScaledAxis(EAxis::Y);
		const FVector Normalize = Direction.GetSafeNormal2D();

		// get a cos(alpha) of forward vector vs velocity
		const float ForwardCosAngle = static_cast<float>(FVector::DotProduct(ForwardVector, Normalize));
		// now get the alpha and convert to degree
		float ForwardDeltaDegree = FMath::RadiansToDegrees(FMath::Acos(ForwardCosAngle));

		// depending on where right vector is, flip it
		const float RightCosAngle = static_cast<float>(FVector::DotProduct(RightVector, Normalize));
		if (RightCosAngle < 0.f)
		{
			ForwardDeltaDegree *= -1.f;
		}

		return ForwardDeltaDegree;
	}

	return 0.f;
}

EGraspCardinal_4Way UGraspStatics::CalculateCardinalDirection_4Way(const FVector& SourceLocation,
	const FRotator& SourceRotation,	const FVector& TargetLocation)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(GraspStatics::CalculateCardinalDirection_4Way);
	
	const FVector Direction = TargetLocation - SourceLocation;
	const float Angle = CalculateCardinalAngle(Direction, SourceRotation);
	return GetCardinalDirectionFromAngle_4Way(Angle);
}

EGraspCardinal_8Way UGraspStatics::CalculateCardinalDirection_8Way(const FVector& SourceLocation,
	const FRotator& SourceRotation,	const FVector& TargetLocation)
{ 
	TRACE_CPUPROFILER_EVENT_SCOPE(GraspStatics::CalculateCardinalDirection_8Way);
	
	const FVector Direction = TargetLocation - SourceLocation;
	const float Angle = CalculateCardinalAngle(Direction, SourceRotation);
	return GetCardinalDirectionFromAngle_8Way(Angle);
}

EGraspCardinal_4Way UGraspStatics::GetOppositeCardinalDirection_4Way(EGraspCardinal_4Way Cardinal)
{
	switch (Cardinal)
	{
	case EGraspCardinal_4Way::Forward: return EGraspCardinal_4Way::Backward;
	case EGraspCardinal_4Way::Backward: return EGraspCardinal_4Way::Forward;
	case EGraspCardinal_4Way::Left: return EGraspCardinal_4Way::Right;
	case EGraspCardinal_4Way::Right: return EGraspCardinal_4Way::Left;
	default: return Cardinal;
	}
}

EGraspCardinal_8Way UGraspStatics::GetOppositeCardinalDirection_8Way(EGraspCardinal_8Way Cardinal)
{
	switch (Cardinal)
	{
	case EGraspCardinal_8Way::Forward: return EGraspCardinal_8Way::Backward;
	case EGraspCardinal_8Way::Backward: return EGraspCardinal_8Way::Forward;
	case EGraspCardinal_8Way::Left: return EGraspCardinal_8Way::Right;
	case EGraspCardinal_8Way::Right: return EGraspCardinal_8Way::Left;
	case EGraspCardinal_8Way::ForwardLeft: return EGraspCardinal_8Way::BackwardRight;
	case EGraspCardinal_8Way::ForwardRight: return EGraspCardinal_8Way::BackwardLeft;
	case EGraspCardinal_8Way::BackwardLeft: return EGraspCardinal_8Way::ForwardRight;
	case EGraspCardinal_8Way::BackwardRight: return EGraspCardinal_8Way::ForwardLeft;
	default: return Cardinal;
	}
}

FVector UGraspStatics::GetDirectionFromCardinal_4Way(EGraspCardinal_4Way Cardinal, const FRotator& SourceRotation)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(GraspStatics::GetDirectionFromCardinal_4Way);
	return SourceRotation.RotateVector(GetSnappedDirectionFromCardinal_4Way(Cardinal));
}

FVector UGraspStatics::GetDirectionFromCardinal_8Way(EGraspCardinal_8Way Cardinal, const FRotator& SourceRotation)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(GraspStatics::GetDirectionFromCardinal_8Way);
	return SourceRotation.RotateVector(GetSnappedDirectionFromCardinal_8Way(Cardinal));
}

FVector UGraspStatics::GetSnappedDirectionFromCardinal_4Way(EGraspCardinal_4Way Cardinal)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(GraspStatics::GetSnappedDirectionFromCardinal_4Way);
	switch (Cardinal)
	{
	case EGraspCardinal_4Way::Forward:
		return FVector(1.f, 0.f, 0.f);
	case EGraspCardinal_4Way::Backward:
		return FVector(-1.f, 0.f, 0.f);
	case EGraspCardinal_4Way::Left:
		return FVector(0.f, -1.f, 0.f);
	case EGraspCardinal_4Way::Right:
		return FVector(0.f, 1.f, 0.f);
	default:
		return FVector::ZeroVector;
	}
}

FVector UGraspStatics::GetSnappedDirectionFromCardinal_8Way(EGraspCardinal_8Way Cardinal)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(GraspStatics::GetSnappedDirectionFromCardinal_8Way);
	switch (Cardinal)
	{
	case EGraspCardinal_8Way::Forward:
		return FVector(1.f, 0.f, 0.f);
	case EGraspCardinal_8Way::Backward:
		return FVector(-1.f, 0.f, 0.f);
	case EGraspCardinal_8Way::Left:
		return FVector(0.f, -1.f, 0.f);
	case EGraspCardinal_8Way::Right:
		return FVector(0.f, 1.f, 0.f);
	case EGraspCardinal_8Way::ForwardLeft:
		return FVector(1.f, -1.f, 0.f).GetSafeNormal2D();
	case EGraspCardinal_8Way::ForwardRight:
		return FVector(1.f, 1.f, 0.f).GetSafeNormal2D();
	case EGraspCardinal_8Way::BackwardLeft:
		return FVector(-1.f, -1.f, 0.f).GetSafeNormal2D();
	case EGraspCardinal_8Way::BackwardRight:
		return FVector(-1.f, 1.f, 0.f).GetSafeNormal2D();
	default:
		return FVector::ZeroVector;
	}
}

FVector UGraspStatics::GetDirectionSnappedToCardinal(const FVector& SourceLocation, const FRotator& SourceRotation,
	const FVector& TargetLocation, EGraspCardinalType CardinalType, bool bFlipDirection)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(GraspStatics::GetDirectionSnappedToCardinal);
	
	const FVector Direction = TargetLocation - SourceLocation;
	const float Angle = CalculateCardinalAngle(Direction, SourceRotation);

	switch (CardinalType)
	{
	case EGraspCardinalType::Cardinal_8Way:
		{
			EGraspCardinal_8Way Cardinal = GetCardinalDirectionFromAngle_8Way(Angle);
			if (bFlipDirection)
			{
				Cardinal = GetOppositeCardinalDirection_8Way(Cardinal);
			}
			return GetDirectionFromCardinal_8Way(Cardinal, SourceRotation);
		}
	default:
		{
			EGraspCardinal_4Way Cardinal = GetCardinalDirectionFromAngle_4Way(Angle);
			if (bFlipDirection)
			{
				Cardinal = GetOppositeCardinalDirection_4Way(Cardinal);
			}
			return GetDirectionFromCardinal_4Way(Cardinal, SourceRotation);
		}
	}
}

bool UGraspStatics::IsWithinInteractAngle(const FVector& InteractorLocation, const FVector& InteractableLocation, const FVector& Forward, float Degrees, bool bCheck2D, bool
	bHalfCircle)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(GraspStatics::IsWithinInteractAngle);
	
	const FVector Diff = InteractableLocation - InteractorLocation;
	const FVector Dir = bCheck2D ? Diff.GetSafeNormal2D() : Diff.GetSafeNormal();
	const float Radians = FMath::DegreesToRadians(Degrees * (bHalfCircle ? 1.f : 0.5f));
	const float Acos = FMath::Acos(Forward | Dir);
	return Acos <= Radians;
}

bool UGraspStatics::IsInteractableWithinAngle(const FVector& InteractorLocation, const FVector& InteractableLocation,
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

bool UGraspStatics::IsWithinInteractDistance(const FVector& InteractorLocation, const FVector& InteractableLocation,
	float Distance, bool bCheck2D)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(GraspStatics::IsWithinInteractDistance);
	
	const float DistSquared = bCheck2D ?
		FVector::DistSquared2D(InteractorLocation, InteractableLocation) : FVector::DistSquared(InteractorLocation, InteractableLocation);
	return DistSquared <= FMath::Square(Distance);
}

bool UGraspStatics::IsInteractableWithinDistance(const FVector& InteractorLocation, const FVector& InteractableLocation,
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

bool UGraspStatics::IsInteractableWithinHeight(const FVector& InteractorLocation, const FVector& InteractableLocation,
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
	TRACE_CPUPROFILER_EVENT_SCOPE(GraspStatics::CanInteractWith);
	
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
	const IGraspableComponent* Graspable = CastChecked<IGraspableComponent>(Component);
	if (!ensure(Graspable->GetGraspData() != nullptr))
	{
		return EGraspQueryResult::None;
	}

	const FVector InteractorLocation = Interactor->GetActorLocation();
	const FVector Location = Component->GetComponentLocation();
	const FVector Forward = Component->GetForwardVector();
	const UGraspData* Data = Graspable->GetGraspData();

	const float AuthNetToleranceAngleScalar = Data->GetAuthNetToleranceAngleScalar();
	const float AuthNetToleranceDistanceScalar = Data->GetAuthNetToleranceDistanceScalar();
	
	const float Angle = Interactor->HasAuthority() && Interactor->GetNetMode() != NM_Standalone ?
		Data->MaxGraspAngle * AuthNetToleranceAngleScalar : Data->MaxGraspAngle;

	const float Distance = Interactor->HasAuthority() && Interactor->GetNetMode() != NM_Standalone ?
		Data->MaxGraspDistance * AuthNetToleranceDistanceScalar : Data->MaxGraspDistance;

	const float HighlightDistance = Interactor->HasAuthority() && Interactor->GetNetMode() != NM_Standalone ?
		Data->MaxHighlightDistance * AuthNetToleranceDistanceScalar : Data->MaxHighlightDistance;

	const float MaxHeightAbove = Interactor->HasAuthority() && Interactor->GetNetMode() != NM_Standalone ?
		Data->MaxHeightAbove * AuthNetToleranceDistanceScalar : Data->MaxHeightAbove;

	const float MaxHeightBelow = Interactor->HasAuthority() && Interactor->GetNetMode() != NM_Standalone ?
		Data->MaxHeightBelow * AuthNetToleranceDistanceScalar : Data->MaxHeightBelow;
	
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

EGraspQueryResult UGraspStatics::CanInteractWithRange(const AActor* Interactor, const UPrimitiveComponent* Graspable,
	float& NormalizedDistance, float& NormalizedHighlightDistance)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(GraspStatics::CanInteractWithRange);
	
	NormalizedDistance = 0.f;
	NormalizedHighlightDistance = 0.f;

	// Validate the interactor
	if (!IsValid(Interactor))
	{
		return EGraspQueryResult::None;
	}

	// Validate the graspable
	if (!Graspable)
	{
		return EGraspQueryResult::None;
	}

	const FVector InteractorLocation = Interactor->GetActorLocation();
	const FVector Location = Graspable->GetComponentLocation();
	const UGraspData* Data = CastChecked<IGraspableComponent>(Graspable)->GetGraspData();

	const float AuthNetToleranceDistanceScalar = Data->GetAuthNetToleranceDistanceScalar();

	const float Distance = Interactor->HasAuthority() && Interactor->GetNetMode() != NM_Standalone ?
		Data->MaxGraspDistance * AuthNetToleranceDistanceScalar : Data->MaxGraspDistance;

	const float HighlightDistance = Interactor->HasAuthority() && Interactor->GetNetMode() != NM_Standalone ?
		Data->MaxHighlightDistance * AuthNetToleranceDistanceScalar : Data->MaxHighlightDistance;

	// Check if within distance
	if (!IsInteractableWithinDistance(Location, InteractorLocation, Distance))
	{
		// Check if highlight is enabled and within distance
		if (HighlightDistance > 0.f && IsInteractableWithinDistance(Location, InteractorLocation, HighlightDistance))
		{
			NormalizedHighlightDistance = FMath::Clamp(
				FVector::Dist2D(Location, InteractorLocation) / HighlightDistance, 0.f, 1.f);

			return EGraspQueryResult::Highlight;
		}

		return EGraspQueryResult::None;
	}

	const float DistNormalized = Data->bGraspDistance2D ? FVector::Dist2D(Location, InteractorLocation) :
		FVector::Dist(Location, InteractorLocation);
	
	NormalizedDistance = FMath::Clamp(DistNormalized / Distance, 0.f, 1.f);

	return EGraspQueryResult::Interact;
}

bool UGraspStatics::CanInteractWithAngle(const AActor* Interactor, const UPrimitiveComponent* Graspable,
	float& NormalizedAngleDiff)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(GraspStatics::CanInteractWithAngle);
	
	NormalizedAngleDiff = 0.f;

	// Validate the interactor
	if (!IsValid(Interactor))
	{
		return false;
	}

	// Validate the graspable
	if (!Graspable)
	{
		return false;
	}

	const FVector InteractorLocation = Interactor->GetActorLocation();
	const FVector Location = Graspable->GetComponentLocation();
	const FVector Forward = Graspable->GetForwardVector();
	const UGraspData* Data = CastChecked<IGraspableComponent>(Graspable)->GetGraspData();

	const float AuthNetToleranceAngleScalar = Data->GetAuthNetToleranceAngleScalar();

	const float Angle = Interactor->HasAuthority() && Interactor->GetNetMode() != NM_Standalone ?
		Data->MaxGraspAngle * AuthNetToleranceAngleScalar : Data->MaxGraspAngle;

	// Check if within angle
	if (!IsInteractableWithinAngle(Location, InteractorLocation, Forward, Angle))
	{
		return false;
	}

	const float AngleNormalized = FMath::Clamp(
		FVector::Dist2D(Location, InteractorLocation) / Angle, 0.f, 1.f);
	
	NormalizedAngleDiff = AngleNormalized;

	return true;
}

bool UGraspStatics::CanInteractWithHeight(const AActor* Interactor, const UPrimitiveComponent* Graspable)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(GraspStatics::CanInteractWithHeight);
	
	// Validate the interactor
	if (!IsValid(Interactor))
	{
		return false;
	}

	// Validate the graspable
	if (!Graspable)
	{
		return false;
	}

	const FVector InteractorLocation = Interactor->GetActorLocation();
	const FVector Location = Graspable->GetComponentLocation();
	const UGraspData* Data = CastChecked<IGraspableComponent>(Graspable)->GetGraspData();

	const float AuthNetToleranceDistanceScalar = Data->GetAuthNetToleranceDistanceScalar();

	const float MaxHeightAbove = Interactor->HasAuthority() && Interactor->GetNetMode() != NM_Standalone ?
		Data->MaxHeightAbove * AuthNetToleranceDistanceScalar : Data->MaxHeightAbove;

	const float MaxHeightBelow = Interactor->HasAuthority() && Interactor->GetNetMode() != NM_Standalone ?
		Data->MaxHeightBelow * AuthNetToleranceDistanceScalar : Data->MaxHeightBelow;

	return IsInteractableWithinHeight(Location, InteractorLocation, MaxHeightAbove, MaxHeightBelow);
}

FVector2D UGraspStatics::GetScreenPositionForGraspableComponent(const UPrimitiveComponent* GraspableComponent,
	APlayerController* PlayerController, const UWidget* Widget, bool& bSuccess)
{
	bSuccess = false;
	FVector2D ScreenPosition;
	if (!IsValid(PlayerController) || !IsValid(GraspableComponent) && Widget)
	{
		return ScreenPosition;
	}

	// Get the screen position of the graspable component
	if (UGameplayStatics::ProjectWorldToScreen(PlayerController, GraspableComponent->GetComponentLocation(),
		ScreenPosition, true))
	{
		bSuccess = true;

		// Convert to viewport coordinates
		USlateBlueprintLibrary::ScreenToViewport(PlayerController, ScreenPosition, ScreenPosition);

		// Adjust for the widget half size to center the widget
		const FVector2D DesiredSize = Widget->GetDesiredSize() * 0.5f;
		ScreenPosition -= DesiredSize;
	}

	return ScreenPosition;
}
