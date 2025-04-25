// Copyright (c) Jared Taylor


#include "Targeting/GraspTargetSelection.h"

#include "GraspDeveloper.h"
#include "TargetingSystem/TargetingSubsystem.h"
#include "Engine/OverlapResult.h"
#include "Engine/World.h"
#include "Components/PrimitiveComponent.h"
#include "System/GraspVersioning.h"
#include "Targeting/GraspTargetingStatics.h"

#if UE_ENABLE_DEBUG_DRAWING
#if WITH_EDITORONLY_DATA
#include "Engine/Canvas.h"
#endif
#endif

DEFINE_LOG_CATEGORY_STATIC(LogGraspTargeting, Log, All);

#include UE_INLINE_GENERATED_CPP_BY_NAME(GraspTargetSelection)


namespace FGraspCVars
{
#if UE_ENABLE_DEBUG_DRAWING
	static bool bGraspSelectionDebug = false;
	FAutoConsoleVariableRef CVarGraspSelectionDebug(
		TEXT("p.Grasp.Selection.Debug"),
		bGraspSelectionDebug,
		TEXT("Optionally draw debug for Grasp AOE Selection Task.\n")
		TEXT("If true draw debug for Grasp AOE Selection Task"),
		ECVF_Default);
#endif
}

UGraspTargetSelection::UGraspTargetSelection(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	const TEnumAsByte<enum ECollisionChannel>& DefaultObjectType = GetDefault<UGraspDeveloper>()->GraspDefaultObjectType;
	const TEnumAsByte<EObjectTypeQuery> ObjectType = UCollisionProfile::Get()->ConvertToObjectType(DefaultObjectType);
	CollisionObjectTypes.Add(ObjectType);

	CollisionChannel = ECC_Visibility;
	bUseRelativeLocationOffset = true;
	bIgnoreSourceActor = true;
	bIgnoreInstigatorActor = false;
	bTraceComplex = false;

	LocationSource = EGraspTargetLocationSource::Actor;
	RotationSource = EGraspTargetRotationSource::Actor;

	HalfExtent = FVector(1000.f, 750.f, 250.f);
	HalfHeight = 500.f;
	Radius = 300.f;

	GraspAbilityRadius = 0.f;
	UpdateGraspAbilityRadius();
}

FVector UGraspTargetSelection::GetSourceLocation_Implementation(
	const FTargetingRequestHandle& TargetingHandle) const
{
	return UGraspTargetingStatics::GetSourceLocation(TargetingHandle, LocationSource);
}

FVector UGraspTargetSelection::GetSourceOffset_Implementation(
	const FTargetingRequestHandle& TargetingHandle) const
{
	return UGraspTargetingStatics::GetSourceOffset(TargetingHandle, LocationSource, DefaultSourceLocationOffset,
		bUseRelativeLocationOffset);
}

FQuat UGraspTargetSelection::GetSourceRotation_Implementation(
	const FTargetingRequestHandle& TargetingHandle) const
{
	return UGraspTargetingStatics::GetSourceRotation(TargetingHandle, RotationSource);
}

FQuat UGraspTargetSelection::GetSourceRotationOffset_Implementation(
	const FTargetingRequestHandle& TargetingHandle) const
{
	return DefaultSourceRotationOffset.Quaternion();
}

void UGraspTargetSelection::UpdateGraspAbilityRadius()
{
	// Average the dimensions of the shape to get a radius
	switch (ShapeType)
	{
	case EGraspTargetingShape::Box:
	case EGraspTargetingShape::Cylinder:
		GraspAbilityRadius = 0.5f * (HalfExtent.X + HalfExtent.Y);  // Ignore Z (height)
		break;
	case EGraspTargetingShape::Sphere:
		GraspAbilityRadius = Radius.GetValue();
		break;
	case EGraspTargetingShape::Capsule:
		GraspAbilityRadius = 0.5f * (Radius.GetValue() + HalfHeight.GetValue());
		break;
	}
}

void UGraspTargetSelection::PostLoad()
{
	Super::PostLoad();

	UpdateGraspAbilityRadius();
}

#if WITH_EDITOR
void UGraspTargetSelection::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	static const TArray<FName> ShapePropNames = {
		GET_MEMBER_NAME_CHECKED(ThisClass, HalfExtent),
		GET_MEMBER_NAME_CHECKED(ThisClass, Radius),
		GET_MEMBER_NAME_CHECKED(ThisClass, HalfHeight),
		GET_MEMBER_NAME_CHECKED(ThisClass, ShapeType)
	};

	// Average the dimensions of the shape to get a radius
	if (PropertyChangedEvent.ChangeType == EPropertyChangeType::ValueSet)
	{
		if (ShapePropNames.Contains(PropertyChangedEvent.GetMemberPropertyName()))
		{
			UpdateGraspAbilityRadius();
		}
	}
}
#endif

void UGraspTargetSelection::Execute(const FTargetingRequestHandle& TargetingHandle) const
{
	TRACE_CPUPROFILER_EVENT_SCOPE(GraspTargetSelection::Execute);

	SetTaskAsyncState(TargetingHandle, ETargetingTaskAsyncState::Executing);

	// @note: There isn't Async Overlap support based on Primitive Component, so even if using async targeting, it will
	// run this task in "immediate" mode.
	if (IsAsyncTargetingRequest(TargetingHandle))
	{
		ExecuteAsyncTrace(TargetingHandle);
	}
	else
	{
		ExecuteImmediateTrace(TargetingHandle);
	}
}

void UGraspTargetSelection::ExecuteImmediateTrace(const FTargetingRequestHandle& TargetingHandle) const
{
#if UE_ENABLE_DEBUG_DRAWING
	ResetDebugString(TargetingHandle);
#endif // UE_ENABLE_DEBUG_DRAWING

	TRACE_CPUPROFILER_EVENT_SCOPE(GraspTargetSelection::ExecuteImmediateTrace);

	const UWorld* World = GetSourceContextWorld(TargetingHandle);
	if (World && TargetingHandle.IsValid())
	{
		const FVector SourceLocation = GetSourceLocation(TargetingHandle) + GetSourceOffset(TargetingHandle);
		const FQuat SourceRotation = (GetSourceRotation(TargetingHandle) * GetSourceRotationOffset(TargetingHandle)).GetNormalized();

		TArray<FOverlapResult> OverlapResults;
		const FCollisionShape CollisionShape = GetCollisionShape();
		FCollisionQueryParams OverlapParams(TEXT("UGraspTargetSelection_AOE"), SCENE_QUERY_STAT_ONLY(UGraspTargetSelection_AOE), false);
		InitCollisionParams(TargetingHandle, OverlapParams);

		if (CollisionObjectTypes.Num() > 0)
		{
			FCollisionObjectQueryParams ObjectParams;
			for (auto Iter = CollisionObjectTypes.CreateConstIterator(); Iter; ++Iter)
			{
				const ECollisionChannel& Channel = UCollisionProfile::Get()->ConvertToCollisionChannel(false, *Iter);
				ObjectParams.AddObjectTypesToQuery(Channel);
			}

			World->OverlapMultiByObjectType(OverlapResults, SourceLocation, SourceRotation, ObjectParams, CollisionShape, OverlapParams);
		}
		else if (CollisionProfileName.Name != TEXT("NoCollision"))
		{
			World->OverlapMultiByProfile(OverlapResults, SourceLocation, SourceRotation, CollisionProfileName.Name, CollisionShape, OverlapParams);
		}
		else
		{
			World->OverlapMultiByChannel(OverlapResults, SourceLocation, SourceRotation, CollisionChannel, CollisionShape, OverlapParams);
		}

		const int32 NumValidResults = ProcessOverlapResults(TargetingHandle, OverlapResults);
		
#if UE_ENABLE_DEBUG_DRAWING
		if (FGraspCVars::bGraspSelectionDebug)
		{
			const FColor& DebugColor = NumValidResults > 0 ? FColor::Red : FColor::Green;
			DebugDrawBoundingVolume(TargetingHandle, DebugColor);
		}
#endif
	}

	SetTaskAsyncState(TargetingHandle, ETargetingTaskAsyncState::Completed);
}

void UGraspTargetSelection::ExecuteAsyncTrace(const FTargetingRequestHandle& TargetingHandle) const
{
	TRACE_CPUPROFILER_EVENT_SCOPE(GraspTargetSelection::ExecuteAsyncTrace);
	
	UWorld* World = GetSourceContextWorld(TargetingHandle);
	if (World && TargetingHandle.IsValid())
	{
		const FVector SourceLocation = GetSourceLocation(TargetingHandle) + GetSourceOffset(TargetingHandle);
		const FQuat SourceRotation = (GetSourceRotation(TargetingHandle) * GetSourceRotationOffset(TargetingHandle)).GetNormalized();

		const FCollisionShape CollisionShape = GetCollisionShape();
		FCollisionQueryParams OverlapParams(TEXT("UGraspTargetSelection_AOE"), SCENE_QUERY_STAT_ONLY(UGraspTargetSelection_AOE_Shape), false);
		InitCollisionParams(TargetingHandle, OverlapParams);

		const FOverlapDelegate Delegate = FOverlapDelegate::CreateUObject(this, &UGraspTargetSelection::HandleAsyncOverlapComplete, TargetingHandle);
		if (CollisionObjectTypes.Num() > 0)
		{
			FCollisionObjectQueryParams ObjectParams;
			for (auto Iter = CollisionObjectTypes.CreateConstIterator(); Iter; ++Iter)
			{
				const ECollisionChannel& Channel = UCollisionProfile::Get()->ConvertToCollisionChannel(false, *Iter);
				ObjectParams.AddObjectTypesToQuery(Channel);
			}

			World->AsyncOverlapByObjectType(SourceLocation, SourceRotation, ObjectParams, CollisionShape, OverlapParams, &Delegate);
		}
		else if (CollisionProfileName.Name != TEXT("NoCollision"))
		{
			World->AsyncOverlapByProfile(SourceLocation, SourceRotation, CollisionProfileName.Name, CollisionShape, OverlapParams, &Delegate);
		}
		else
		{
			World->AsyncOverlapByChannel(SourceLocation, SourceRotation, CollisionChannel, CollisionShape, OverlapParams, FCollisionResponseParams::DefaultResponseParam, &Delegate);
		}
	}
	else
	{
		SetTaskAsyncState(TargetingHandle, ETargetingTaskAsyncState::Completed);
	}
}

void UGraspTargetSelection::HandleAsyncOverlapComplete(const FTraceHandle& InTraceHandle,
	FOverlapDatum& InOverlapDatum, FTargetingRequestHandle TargetingHandle) const
{
	TRACE_CPUPROFILER_EVENT_SCOPE(GraspTargetSelection::HandleAsyncOverlapComplete);
	
	if (TargetingHandle.IsValid())
	{
#if UE_ENABLE_DEBUG_DRAWING
		ResetDebugString(TargetingHandle);
#endif

		const int32 NumValidResults = ProcessOverlapResults(TargetingHandle, InOverlapDatum.OutOverlaps);
		
#if UE_ENABLE_DEBUG_DRAWING
		if (FGraspCVars::bGraspSelectionDebug)
		{
			const FColor& DebugColor = NumValidResults > 0 ? FColor::Red : FColor::Green;
			DebugDrawBoundingVolume(TargetingHandle, DebugColor, &InOverlapDatum);
		}
#endif
	}

	SetTaskAsyncState(TargetingHandle, ETargetingTaskAsyncState::Completed);
}

int32 UGraspTargetSelection::ProcessOverlapResults(const FTargetingRequestHandle& TargetingHandle,
	const TArray<FOverlapResult>& Overlaps) const
{
	TRACE_CPUPROFILER_EVENT_SCOPE(GraspTargetSelection::ProcessOverlapResults);

#if WITH_EDITOR
	// During editor update this so we can modify properties during runtime
	UGraspTargetSelection* MutableThis = const_cast<UGraspTargetSelection*>(this);
	MutableThis->UpdateGraspAbilityRadius();
#endif
	
	// process the overlaps
	int32 NumValidResults = 0;
	if (Overlaps.Num() > 0)
	{
		FTargetingDefaultResultsSet& TargetingResults = FTargetingDefaultResultsSet::FindOrAdd(TargetingHandle);
		const FVector SourceLocation = GetSourceLocation(TargetingHandle) + GetSourceOffset(TargetingHandle);
		const FQuat SourceRotation = (GetSourceRotation(TargetingHandle) * GetSourceRotationOffset(TargetingHandle)).GetNormalized();

		for (const FOverlapResult& OverlapResult : Overlaps)
		{
			if (!OverlapResult.GetActor())
			{
				continue;
			}

			// cylinders use box overlaps, so a radius check is necessary to constrain it to the bounds of a cylinder
			if (ShapeType == EGraspTargetingShape::Cylinder)
			{
				const float RadiusSquared = (HalfExtent.X * HalfExtent.X);
				const float DistanceSquared = FVector::DistSquared2D(OverlapResult.GetActor()->GetActorLocation(), SourceLocation);
				if (DistanceSquared > RadiusSquared)
				{
					continue;
				}
			}

			bool bAddResult = true;
			for (const FTargetingDefaultResultData& ResultData : TargetingResults.TargetResults)
			{
				if (ResultData.HitResult.GetActor() == OverlapResult.GetActor())
				{
					bAddResult = false;
					break;
				}
			}

			if (bAddResult)
			{
				NumValidResults++;
				
				FTargetingDefaultResultData* ResultData = new(TargetingResults.TargetResults) FTargetingDefaultResultData();
				ResultData->HitResult.HitObjectHandle = OverlapResult.OverlapObjectHandle;
				ResultData->HitResult.Component = OverlapResult.GetComponent();
				ResultData->HitResult.ImpactPoint = OverlapResult.GetActor()->GetActorLocation();
				ResultData->HitResult.Location = OverlapResult.GetActor()->GetActorLocation();
				ResultData->HitResult.bBlockingHit = OverlapResult.bBlockingHit;
				ResultData->HitResult.TraceStart = SourceLocation;
				ResultData->HitResult.Item = OverlapResult.ItemIndex;

				// Store the normal based on where we are looking based on source rotation
				ResultData->HitResult.Normal = SourceRotation.Vector();

				// We need the normalized distance, which we calculate from GraspAbilityRadius
				ResultData->HitResult.Distance = GraspAbilityRadius;
			}
		}

#if UE_ENABLE_DEBUG_DRAWING
		BuildDebugString(TargetingHandle, TargetingResults.TargetResults);
#endif
	}

	return NumValidResults;
}

FCollisionShape UGraspTargetSelection::GetCollisionShape() const
{
	TRACE_CPUPROFILER_EVENT_SCOPE(GraspTargetSelection::GetCollisionShape);
	
	switch (ShapeType)
	{
	case EGraspTargetingShape::Box:	return FCollisionShape::MakeBox(HalfExtent);
	case EGraspTargetingShape::Cylinder: return FCollisionShape::MakeBox(HalfExtent);
	case EGraspTargetingShape::Sphere: return FCollisionShape::MakeSphere(Radius.GetValue());
	case EGraspTargetingShape::Capsule:
		return FCollisionShape::MakeCapsule(Radius.GetValue(), HalfHeight.GetValue());
	default: return {};
	}
}

void UGraspTargetSelection::InitCollisionParams(const FTargetingRequestHandle& TargetingHandle,
	FCollisionQueryParams& OutParams) const
{
	UGraspTargetingStatics::InitCollisionParams(TargetingHandle, OutParams, bIgnoreSourceActor,
		bIgnoreInstigatorActor, bTraceComplex);
}

void UGraspTargetSelection::DebugDrawBoundingVolume(const FTargetingRequestHandle& TargetingHandle,
	const FColor& Color, const FOverlapDatum* OverlapDatum) const
{
#if UE_ENABLE_DEBUG_DRAWING
	const UWorld* World = GetSourceContextWorld(TargetingHandle);
	const FVector SourceLocation = OverlapDatum ? OverlapDatum->Pos : GetSourceLocation(TargetingHandle) + GetSourceOffset(TargetingHandle);
	const FQuat SourceRotation = OverlapDatum ? OverlapDatum->Rot : (GetSourceRotation(TargetingHandle) * GetSourceRotationOffset(TargetingHandle)).GetNormalized();
	const FCollisionShape CollisionShape = GetCollisionShape();

	constexpr bool bPersistentLines = false;
#if UE_5_04_OR_LATER
	const float LifeTime = UTargetingSubsystem::GetOverrideTargetingLifeTime();
#else
	constexpr float LifeTime = 0.f;
#endif
	constexpr uint8 DepthPriority = 0;
	constexpr float Thickness = 2.0f;

	switch (ShapeType)
	{
	case EGraspTargetingShape::Box:
		DrawDebugBox(World, SourceLocation, CollisionShape.GetExtent(), SourceRotation,
			Color, bPersistentLines, LifeTime, DepthPriority, Thickness);
		break;
	case EGraspTargetingShape::Sphere:
		DrawDebugCapsule(World, SourceLocation, CollisionShape.GetSphereRadius(), CollisionShape.GetSphereRadius(), SourceRotation,
			Color, bPersistentLines, LifeTime, DepthPriority, Thickness);
		break;
	case EGraspTargetingShape::Capsule:
		DrawDebugCapsule(World, SourceLocation, CollisionShape.GetCapsuleHalfHeight(), CollisionShape.GetCapsuleRadius(), SourceRotation,
			Color, bPersistentLines, LifeTime, DepthPriority, Thickness);
		break;
	case EGraspTargetingShape::Cylinder:
		{
			const FVector RotatedExtent = SourceRotation * CollisionShape.GetExtent();
			DrawDebugCylinder(World, SourceLocation - RotatedExtent, SourceLocation + RotatedExtent, CollisionShape.GetExtent().X, 32,
				Color, bPersistentLines, LifeTime, DepthPriority, Thickness);
			break;
		}
	}
#endif
}

#if UE_ENABLE_DEBUG_DRAWING
void UGraspTargetSelection::DrawDebug(UTargetingSubsystem* TargetingSubsystem, FTargetingDebugInfo& Info,
	const FTargetingRequestHandle& TargetingHandle, float XOffset, float YOffset, int32 MinTextRowsToAdvance) const
{
#if WITH_EDITORONLY_DATA
	if (FGraspCVars::bGraspSelectionDebug)
	{
		FTargetingDebugData& DebugData = FTargetingDebugData::FindOrAdd(TargetingHandle);
		const FString& ScratchPadString = DebugData.DebugScratchPadStrings.FindOrAdd(GetNameSafe(this));
		if (!ScratchPadString.IsEmpty())
		{
			if (Info.Canvas)
			{
				Info.Canvas->SetDrawColor(FColor::Yellow);
			}

			const FString TaskString = FString::Printf(TEXT("Results : %s"), *ScratchPadString);
			TargetingSubsystem->DebugLine(Info, TaskString, XOffset, YOffset, MinTextRowsToAdvance);
		}
	}
#endif
}

void UGraspTargetSelection::BuildDebugString(const FTargetingRequestHandle& TargetingHandle,
	const TArray<FTargetingDefaultResultData>& TargetResults) const
{
#if WITH_EDITORONLY_DATA
	if (FGraspCVars::bGraspSelectionDebug)
	{
		FTargetingDebugData& DebugData = FTargetingDebugData::FindOrAdd(TargetingHandle);
		FString& ScratchPadString = DebugData.DebugScratchPadStrings.FindOrAdd(GetNameSafe(this));

		for (const FTargetingDefaultResultData& TargetData : TargetResults)
		{
			if (const AActor* Target = TargetData.HitResult.GetActor())
			{
				if (ScratchPadString.IsEmpty())
				{
					ScratchPadString = FString::Printf(TEXT("%s"), *GetNameSafe(Target));
				}
				else
				{
					ScratchPadString += FString::Printf(TEXT(", %s"), *GetNameSafe(Target));
				}
			}
		}
	}
#endif
}

void UGraspTargetSelection::ResetDebugString(const FTargetingRequestHandle& TargetingHandle) const
{
#if WITH_EDITORONLY_DATA
	FTargetingDebugData& DebugData = FTargetingDebugData::FindOrAdd(TargetingHandle);
	FString& ScratchPadString = DebugData.DebugScratchPadStrings.FindOrAdd(GetNameSafe(this));
	ScratchPadString.Reset();
#endif
}
#endif
