﻿// Copyright (c) Jared Taylor

#pragma once

#include "CoreMinimal.h"
#include "GameplayAbilitySpecHandle.h"
#include "GameplayTagContainer.h"
#include "GraspAbilityData.h"
#include "GraspTags.h"
#include "GraspTypes.h"
#include "Components/ActorComponent.h"
#include "Types/TargetingSystemTypes.h"
#include "GraspComponent.generated.h"

struct FGameplayAbilitySpec;
struct FGameplayAbilityActorInfo;
struct FGameplayAbilitySpecHandle;
class UGraspData;
class UGameplayAbility;
class UAbilitySystemComponent;
class AController;

/**
 * Add to your Controller
 * Interfaces with the passive GraspScanAbility and handles resulting data
 * Subclass this to add custom functionality
 */
UCLASS(Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GRASP_API UGraspComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	/**
	 * These abilities are pre-granted and never removed
	 * Generally this is preferable if it is a grasp ability that is used frequently
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Grasp)
	TArray<TSubclassOf<UGameplayAbility>> CommonGraspAbilities;

	/** Targeting presets for finding graspables to interact with, used unless overriding GetTargetingPresets() */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Grasp)
	TMap<FGameplayTag, UTargetingPreset*> DefaultTargetingPresets = { { FGraspTags::Grasp_Interact, nullptr } };

	/**
	 * Determines which actor to use as the source for the targeting request
	 * Unless overridden in GetTargetingSource()
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Grasp)
	EGraspTargetingSource DefaultTargetingSource = EGraspTargetingSource::Pawn;

	/** If true, each Grasp request will update targeting presets before proceeding */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Grasp)
	bool bUpdateTargetingPresetsOnUpdate = false;

	/** If true, will update targeting presets when the owning controller's possessed pawn changes */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Grasp)
	bool bUpdateTargetingPresetsOnPawnChange = false;
	
	/** If true, any change in pawn possession will end existing targeting requests */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Grasp)
	bool bEndTargetingRequestsOnPawnChange = false;

public:
	/** Track any change in preset update mode so we can rebind delegates as required */
	UPROPERTY(Transient)
	bool bLastUpdateTargetingPresetsOnPawnChange = false;

	/** Used to throttle the update rate for optimization purposes */
	UPROPERTY(Transient)
	float LastGraspScanTime = -1.f;

	/** Current targeting presets that will be used to perform targeting requests */
	UPROPERTY(Transient, DuplicateTransient)
	TMap<FGameplayTag, UTargetingPreset*> CurrentTargetingPresets;

	/** Existing targeting request handles that are in-progress */
	UPROPERTY(Transient)
	TMap<FGameplayTag, FTargetingRequestHandle> TargetingRequests;

	/** Handle for the GraspScanAbility */
	FGameplayAbilitySpecHandle ScanAbilityHandle;
	
protected:
	/** Owning controller */
	UPROPERTY(Transient, DuplicateTransient)
	TObjectPtr<AController> Controller = nullptr;

public:
	/** Delegate called when a targeting request is completed, populated with targeting results */
	FOnGraspTargetsReady OnGraspTargetsReady;

	/** GraspScanTask binds to this to pause itself when executed */
	FOnPauseGrasp OnPauseGrasp;

	/**
	 * GraspScanTask binds to this to be notified of when a Grasp is requested
	 * This is a prerequisite for us to be able to end our own targeting requests
	 * Otherwise, the Grasp task would not ever receive the callback and know to continue
	 */
	FOnRequestGrasp OnRequestGrasp;

protected:
	/** Last results of Grasp Focusing update, these are the current focus targets */
	UPROPERTY()
	TArray<FGraspScanResult> CurrentScanResults;

	/** Data for granted abilities, mapped to the ability class */
	UPROPERTY()
	TMap<TSubclassOf<UGameplayAbility>, FGraspAbilityData> AbilityData;

	/** The ASC we were initialized with */
	TWeakObjectPtr<UAbilitySystemComponent> ASC;

public:
	UAbilitySystemComponent* GetASC() { return ASC.IsValid() ? ASC.Get() : nullptr; }
	const UAbilitySystemComponent* GetASC() const { return ASC.IsValid() ? ASC.Get() : nullptr; }

public:
	/** Extension point called after giving grasp ability */
	UPROPERTY(BlueprintAssignable, Category=Grasp)
	FOnPostGiveGraspAbility OnPostGiveGraspAbility;
	
	/** Extension point called after giving common grasp ability */
	UPROPERTY(BlueprintAssignable, Category=Grasp)
	FOnPostGiveCommonGraspAbility OnPostGiveCommonGraspAbility;

	/** Extension point called before clearing grasp ability */
	UPROPERTY(BlueprintAssignable, Category=Grasp)
	FOnPreClearGraspAbility OnPreClearGraspAbility;

	/** Extension point called before trying to activate the grasp ability */
	UPROPERTY(BlueprintAssignable, Category=Grasp)
	FOnPreTryActivateGraspAbility OnPreTryActivateGraspAbility;

	/** Extension point called after successfully activating the grasp ability */
	UPROPERTY(BlueprintAssignable, Category=Grasp)
	FOnPostActivateGraspAbility OnPostActivateGraspAbility;

	/** Extension point called after failing to activate the grasp ability */
	UPROPERTY(BlueprintAssignable, Category=Grasp)
	FOnPostFailedActivateGraspAbility OnPostFailedActivateGraspAbility;
	
public:
	UGraspComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	/**
	 * Call when your Pawn receives a controller
	 * Must be called on both authority and local client
	 * Providing ScanAbility is optional, without it there will be no scanning and only common grasp abilities will be used
	 */
	UFUNCTION(BlueprintCallable, Category=Grasp)
	void InitializeGrasp(UAbilitySystemComponent* InAbilitySystemComponent, TSubclassOf<UGameplayAbility> ScanAbility);
	
	/**
	 * Grasp will scan for focus options at this rate, if it can keep up
	 * The scan rate may be lower if the async targeting request has not completed
	 * This can be used to throttle the number of scans per second for performance reasons
	 * Set to 0 to disable throttling
	 */
	UFUNCTION(BlueprintNativeEvent, Category=Grasp)
	float GetMaxGraspScanRate() const;
	virtual float GetMaxGraspScanRate_Implementation() const { return 0.f; }

	/** Get the Targeting Source passed to the targeting system */
	UFUNCTION(BlueprintNativeEvent, Category=Grasp)
	AActor* GetTargetingSource() const;
	
	/** Retrieve presets used for Grasp's targeting and cache to CurrentTargetingPresets for next update */
	UFUNCTION(BlueprintNativeEvent, Category=Grasp)
	TMap<FGameplayTag, UTargetingPreset*> GetTargetingPresets() const;

	/** Find the ability data mapped to the ability class */
	const FGraspAbilityData* GetGraspAbilityData(const TSubclassOf<UGameplayAbility>& Ability) const;
	
public:
	/** Rebind the OnPossessedPawnChanged binding if the requirement changes */
	void UpdatePawnChangedBinding();

	/** Listen for a change in possessed Pawn to optionally clear targeting requests and optionally update targeting presets */
	UFUNCTION()
	void OnPawnChanged(APawn* OldPawn, APawn* NewPawn);

	/**
	 * Retrieves new CurrentTargetingPresets
	 * Ends any requests that are no longer current
	 */
	void UpdateTargetingPresets();

	void DrawDebugGrantAbilityLine(const UPrimitiveComponent* Component, const FColor& Color) const;
	void DrawDebugGrantAbilityBox(const UPrimitiveComponent* Component, const FString& Info, const FString& Ability,
		const FColor& Color) const;

	/**
	 * Notified by UGraspScanTask that our targets are ready
	 * Cache the results and notify any listeners
	 */
	void GraspTargetsReady(const TArray<FGraspScanResult>& Results);

	/** Extension point called after giving grasp ability */
	UFUNCTION(BlueprintNativeEvent, Category=Grasp)
	void PostGiveGraspAbility(TSubclassOf<UGameplayAbility> InAbility,
		const UPrimitiveComponent* GraspableComponent, const UGraspData* GraspData, FGraspAbilityData& InAbilityData);
	
	virtual void PostGiveGraspAbility_Implementation(TSubclassOf<UGameplayAbility> InAbility,
		const UPrimitiveComponent* GraspableComponent, const UGraspData* GraspData, FGraspAbilityData& InAbilityData);

	/** Extension point called after giving common grasp ability */
	UFUNCTION(BlueprintNativeEvent, Category=Grasp)
	void PostGiveCommonGraspAbility(TSubclassOf<UGameplayAbility> InAbility, FGraspAbilityData& InAbilityData);
	
	virtual void PostGiveCommonGraspAbility_Implementation(TSubclassOf<UGameplayAbility> InAbility, FGraspAbilityData& InAbilityData);

	/** Extension point called before clearing grasp ability */
	UFUNCTION(BlueprintNativeEvent, Category=Grasp)
	void PreClearGraspAbility(TSubclassOf<UGameplayAbility> InAbility, const UGraspData* GraspData,
		FGraspAbilityData& InAbilityData);
	
	virtual void PreClearGraspAbility_Implementation(TSubclassOf<UGameplayAbility> InAbility,
		const UGraspData* GraspData, FGraspAbilityData& InAbilityData);

	/** Extension point called before trying to activate the grasp ability */
	virtual void PreTryActivateGraspAbility(const AActor* SourceActor, UPrimitiveComponent* GraspableComponent,
		EGraspAbilityComponentSource Source, FGameplayAbilitySpec* InSpec);

	/** Extension point called after successfully activating the grasp ability */
	virtual void PostActivateGraspAbility(const AActor* SourceActor, UPrimitiveComponent* GraspableComponent,
		EGraspAbilityComponentSource Source, FGameplayAbilitySpec* InSpec, FGameplayAbilityActorInfo* ActorInfo = nullptr);

	/** Extension point called after failing to activate the grasp ability */
	virtual void PostFailedActivateGraspAbility(const AActor* SourceActor, UPrimitiveComponent* GraspableComponent,
		EGraspAbilityComponentSource Source, FGameplayAbilitySpec* InSpec, FGameplayAbilityActorInfo* ActorInfo = nullptr);

	/** Pause or resume Grasp */
	UFUNCTION(BlueprintCallable, Category=Grasp)
	void PauseGrasp(bool bPaused, bool bEndTargetingRequestsOnPause = true);
	
	/**
	 * End all targeting requests that match the InteractTag
	 * If InteractTag is empty then all targeting requests will be ended
	 * @param InteractTag The tag of the preset to end targeting requests for, or all if tag is none
	 * @param bNotifyGrasp If true, will notify the GraspScanTask to end its targeting requests
	 * @warning If Grasp is not notified, it could stop running and never resume, advanced use only!
	 */
	UFUNCTION(BlueprintCallable, Category=Grasp, meta=(AdvancedDisplay="bNotifyGrasp"))
	void EndTargetingRequests(const FGameplayTag& InteractTag, bool bNotifyGrasp = true);

	/**
	 * End all targeting requests
	 * @param bNotifyGrasp If true, will notify the GraspScanTask to end its targeting requests
	 * @warning If Grasp is not notified, it could stop running and never resume, advanced use only!
	 */
	UFUNCTION(BlueprintCallable, Category=Grasp)
	void EndAllTargetingRequests(bool bNotifyGrasp = true)
	{
		EndTargetingRequests(FGameplayTag::EmptyTag, bNotifyGrasp);
	}

	/**
	 * Check if the granted gameplay ability is currently in range
	 * This means if we clear the ability, it will be re-granted next frame
	 * 
	 * @param Ability The ability to check
	 * @return True if the ability is currently in range
	 */
	UFUNCTION(BlueprintCallable, Category=Grasp)
	bool IsGrantedGameplayAbilityInRange(TSubclassOf<UGameplayAbility> Ability) const;

	/**
	 * Clear the granted gameplay ability
	 *
	 * @param Ability The ability to clear
	 * @param bClearLockedAbilities If true, will include abilities that are currently locked by AddAbilityLock()
	 * @param bClearAbilitiesInRange If true, will ignore abilities that are currently in range of any Graspable components, i.e. they will typically be re-granted next frame
	 * @return True if the ability was cleared
	 */
	UFUNCTION(BlueprintCallable, Category=Grasp, meta=(AdvancedDisplay="bIgnoreInRange"))
	bool ClearGrantedGameplayAbility(TSubclassOf<UGameplayAbility> Ability, bool bClearAbilitiesInRange = false, bool bClearLockedAbilities = false);

	/**
	 * Clear the granted gameplay ability for a specific Graspable component
	 * 
	 * @param GraspableComponent The component to clear the ability for
	 * @param bClearLockedAbilities If true, will include abilities that are currently locked by AddAbilityLock()
	 * @param bClearAbilitiesInRange If true, will ignore abilities that are currently in range of the Graspable component, i.e. they will typically be re-granted next frame
	 * @return True if the ability was cleared
	 */
	UFUNCTION(BlueprintCallable, Category=Grasp, meta=(AdvancedDisplay="bIgnoreInRange"))
	bool ClearGrantedGameplayAbilityForComponent(const UPrimitiveComponent* GraspableComponent, bool bClearAbilitiesInRange = false, bool bClearLockedAbilities = false);
	
	/**
	 * Call to clear all granted gameplay abilities
	 * Might be worthwhile before pausing Grasp
	 * @note Data can only be emptied if bIncludeCommonAbilities is true
	 * @param bClearCommonAbilities If true, will include pre-granted interact abilities as well
	 * @param bClearAbilitiesInRange If true, will remove abilities that are currently in range of the Graspable component, i.e. they will typically be re-granted next frame
	 * @param bClearLockedAbilities If true, will include abilities that are currently locked by AddAbilityLock()
	 * @param bClearScanAbility If true, will include the scan ability as well, you will need to call InitializeGrasp() again to re-enable it
	 * @param bEmptyData If true, will empty the ability data instead of resetting it. Emptying may cause a frame drop but resetting it keeps the array size allocated in memory
	 */
	UFUNCTION(BlueprintCallable, Category=Grasp, meta=(AdvancedDisplay="bEmptyData"))
	void ClearAllGrantedGameplayAbilities(bool bClearCommonAbilities = false, bool bClearAbilitiesInRange = false,
		bool bClearLockedAbilities = false, bool bClearScanAbility = false, bool bEmptyData = false);

public:
	/**
	 * This component's ability cannot be cleared until ability lock is removed
	 * @return True if the ability lock was added
	 */
	UFUNCTION(BlueprintCallable, Category=Grasp)
	bool AddAbilityLock(const UPrimitiveComponent* GraspableComponent);

	/**
	 * Remove ability lock for this component so it's ability can now be cleared
	 * @return True if the ability lock was removed
	 */
	UFUNCTION(BlueprintCallable, Category=Grasp)
	bool RemoveAbilityLock(const UPrimitiveComponent* GraspableComponent);
	
protected:
	UFUNCTION(BlueprintCallable, Category=Grasp)
	bool HasValidData() const;
	
	FString GetRoleString() const;
};
