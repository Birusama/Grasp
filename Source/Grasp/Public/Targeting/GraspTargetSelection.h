// Copyright (c) Jared Taylor

#pragma once

#include "CoreMinimal.h"
#include "GraspTargetingTypes.h"
#include "Tasks/TargetingSelectionTask_AOE.h"
#include "GraspTargetSelection.generated.h"

/**
 * Extend targeting for interaction selection
 * Adds location and rotation sources
 */
UCLASS(Blueprintable, DisplayName="Grasp Target Selection")
class GRASP_API UGraspTargetSelection : public UTargetingTask
{
	GENERATED_BODY()

protected:
	/** The collision channel to use for the overlap check (as long as Collision Profile Name is not set) */
	UPROPERTY(EditAnywhere, Category="Grasp Selection")
	TEnumAsByte<ECollisionChannel> CollisionChannel;
	
	/** The collision profile name to use for the overlap check */
	UPROPERTY(EditAnywhere, Category="Grasp Selection")
	FCollisionProfileName CollisionProfileName;

	/** The object types to use for the overlap check */
	UPROPERTY(EditAnywhere, Category="Grasp Selection")
	TArray<TEnumAsByte<EObjectTypeQuery>> CollisionObjectTypes;

	/** Location to trace from */
	UPROPERTY(EditAnywhere, Category="Grasp Selection")
	EGraspTargetLocationSource LocationSource;

	/** Rotation to trace from */
	UPROPERTY(EditAnywhere, Category="Grasp Selection")
	EGraspTargetRotationSource RotationSource;

	/** The default source location offset used by GetSourceOffset */
	UPROPERTY(EditAnywhere, Category="Grasp Selection")
	FVector DefaultSourceLocationOffset = FVector::ZeroVector;

	/** Should we offset based on world or relative Source object transform? */
	UPROPERTY(EditAnywhere, Category="Grasp Selection")
	uint8 bUseRelativeLocationOffset : 1;

	/** The default source rotation offset used by GetSourceOffset */
	UPROPERTY(EditAnywhere, Category="Grasp Selection")
	FRotator DefaultSourceRotationOffset = FRotator::ZeroRotator;

protected:
	/** When enabled, the trace will be performed against complex collision. */
	UPROPERTY(EditAnywhere, Category="Grasp Selection")
	uint8 bTraceComplex : 1 = false;
	
protected:
	/** Indicates the trace should ignore the source actor */
	UPROPERTY(EditAnywhere, Category="Grasp Selection")
	uint8 bIgnoreSourceActor : 1;

	/** Indicates the trace should ignore the source actor */
	UPROPERTY(EditAnywhere, Category="Grasp Selection")
	uint8 bIgnoreInstigatorActor : 1;
	
protected:
	/** The shape type to use for the AOE */
	UPROPERTY(EditAnywhere, Category="Grasp Selection Shape")
	EGraspTargetingShape ShapeType;

	/** The half extent to use for box and cylinder */
	UPROPERTY(EditAnywhere, Category="Grasp Selection Shape", meta=(EditCondition="ShapeType==EGraspTargetingShape::Box||ShapeType==EGraspTargetingShape::Cylinder", EditConditionHides))
	FVector HalfExtent;

	/** The radius to use for sphere and capsule overlaps */
	UPROPERTY(EditAnywhere, Category="Grasp Selection Shape", meta=(EditCondition="ShapeType==EGraspTargetingShape::Sphere||ShapeType==EGraspTargetingShape::Capsule", EditConditionHides))
	FScalableFloat Radius;

	/** The half height to use for capsule overlap checks */
	UPROPERTY(EditAnywhere, Category="Grasp Selection Shape", meta=(EditCondition="ShapeType==EGraspTargetingShape::Capsule", EditConditionHides))
	FScalableFloat HalfHeight;

	/**
	 * Radius used by Grasp for granting abilities
	 * Calculated based on the shape dimensions
	 * @see UGraspData::NormalizedGrantAbilityDistance
	 */
	UPROPERTY(VisibleAnywhere, Category="Grasp Selection Shape")
	float GraspAbilityRadius;

public:
	UGraspTargetSelection(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	/** Native Event to get the source location for the AOE */
	UFUNCTION(BlueprintNativeEvent, Category="Grasp Selection")
	FVector GetSourceLocation(const FTargetingRequestHandle& TargetingHandle) const;

	/** Native Event to get a source location offset for the AOE */
	UFUNCTION(BlueprintNativeEvent, Category="Grasp Selection")
	FVector GetSourceOffset(const FTargetingRequestHandle& TargetingHandle) const;

	/** Native event to get the source rotation for the AOE  */
	UFUNCTION(BlueprintNativeEvent, Category="Grasp Selection")
	FQuat GetSourceRotation(const FTargetingRequestHandle& TargetingHandle) const;

	/** Native event to get the source rotation for the AOE  */
	UFUNCTION(BlueprintNativeEvent, Category="Grasp Selection")
	FQuat GetSourceRotationOffset(const FTargetingRequestHandle& TargetingHandle) const;

public:
	void UpdateGraspAbilityRadius();

	virtual void PostLoad() override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	
	/** Evaluation function called by derived classes to process the targeting request */
	virtual void Execute(const FTargetingRequestHandle& TargetingHandle) const override;

protected:
	/** Method to process the trace task immediately */
	void ExecuteImmediateTrace(const FTargetingRequestHandle& TargetingHandle) const;

	/** Method to process the trace task asynchronously */
	void ExecuteAsyncTrace(const FTargetingRequestHandle& TargetingHandle) const;

	/** Callback for an async overlap */
	void HandleAsyncOverlapComplete(const FTraceHandle& InTraceHandle, FOverlapDatum& InOverlapDatum,
		FTargetingRequestHandle TargetingHandle) const;

	/**
	 * Method to take the overlap results and store them in the targeting result data
	 * @return Num valid results
	 */
	int32 ProcessOverlapResults(const FTargetingRequestHandle& TargetingHandle, const TArray<FOverlapResult>& Overlaps) const;
	
protected:
	/** Helper method to build the Collision Shape */
	FCollisionShape GetCollisionShape() const;
	
	/** Setup CollisionQueryParams for the AOE */
	void InitCollisionParams(const FTargetingRequestHandle& TargetingHandle, FCollisionQueryParams& OutParams) const;
	
public:
	/** Debug draws the outlines of the set shape type. */
	void DebugDrawBoundingVolume(const FTargetingRequestHandle& TargetingHandle, const FColor& Color,
		const FOverlapDatum* OverlapDatum = nullptr) const;

#if UE_ENABLE_DEBUG_DRAWING
protected:
	virtual void DrawDebug(UTargetingSubsystem* TargetingSubsystem, FTargetingDebugInfo& Info,
		const FTargetingRequestHandle& TargetingHandle, float XOffset, float YOffset, int32 MinTextRowsToAdvance) const override;
	void BuildDebugString(const FTargetingRequestHandle& TargetingHandle, const TArray<FTargetingDefaultResultData>& TargetResults) const;
	void ResetDebugString(const FTargetingRequestHandle& TargetingHandle) const;
#endif
};
