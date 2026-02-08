
#include "AbilitySystem/ExecutionCalculations/GEC_RPSDamage.h"
#include "AbilitySystem/PartyAttributeSet.h"
#include "Game/GameState/PartyGameStateBase.h"

// Struct to hold attribute capture definitions
struct RPSDamageStatics
{
	DECLARE_ATTRIBUTE_CAPTUREDEF(IncomingDamage);
	
	RPSDamageStatics()
	{
		// Snapshot is set to false to capture the value at execution time
		DEFINE_ATTRIBUTE_CAPTUREDEF(UPartyAttributeSet,IncomingDamage,Source,false);
	}
	
};

// Hold the struct in a static pointer to look up once for performance
static const RPSDamageStatics& DamageStatics()
{
	static RPSDamageStatics Infos;
	return Infos;
}


UGEC_RPSDamage::UGEC_RPSDamage()
{
	RelevantAttributesToCapture.Add(DamageStatics().IncomingDamageDef);
}

void UGEC_RPSDamage::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams,
	FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	UAbilitySystemComponent* SourceASC = ExecutionParams.GetSourceAbilitySystemComponent();
	UAbilitySystemComponent* TargetASC = ExecutionParams.GetTargetAbilitySystemComponent();
	// Check if ASCs are null, abort.
	if (!SourceASC || !TargetASC) return;

	// --- 1. GATHER DATA ---

	//Define Team Tags
	static const FGameplayTag RockTag = FGameplayTag::RequestGameplayTag(FName("Team.Type.Rock"));
	static const FGameplayTag PaperTag = FGameplayTag::RequestGameplayTag(FName("Team.Type.Paper"));
	static const FGameplayTag ScissorsTag = FGameplayTag::RequestGameplayTag(FName("Team.Type.Scissors"));

	//SetByCaller Tag of the Gameplay Effect
	static const FGameplayTag CallerTag = FGameplayTag::RequestGameplayTag(FName("Caller.GameplayEffect.Damage"));

	// Get Base Damage from SetByCaller
	const float BaseDamage = ExecutionParams.GetOwningSpec().GetSetByCallerMagnitude(CallerTag);

	//Get Percents from Curve Tables
	const float DamagePercent = DisadvantageDamagePercentCurve.Eval(0.f,FString());
	const float SameTeamPercent = SameTeamPercentCurve.Eval(0.f,FString());

	//Get Source and Target tags
	FGameplayTagContainer SourceContainer = SourceASC->GetOwnedGameplayTags();
	FGameplayTagContainer TargetContainer = TargetASC->GetOwnedGameplayTags();

	// --- 2. FRIENDLY FIRE LOGIC ---

	UWorld* World = SourceASC->GetWorld();
	//Check Game Rules via GameState
	const APartyGameStateBase* PartyGameState = Cast<APartyGameStateBase>(World->GetGameState());
	if (!PartyGameState) return;
	bool bCanAttackSameTeam = PartyGameState->bCanAttackSameTeam;
	bool bIsSameTeam = false;

	//Check if both actors belong the same team
	if (bCanAttackSameTeam)
	{
		bIsSameTeam = SourceContainer.HasTag(RockTag) && TargetContainer.HasTag(RockTag) ||
	    SourceContainer.HasTag(PaperTag) && TargetContainer.HasTag(PaperTag) ||
		SourceContainer.HasTag(ScissorsTag) && TargetContainer.HasTag(ScissorsTag);
	}
	//Handle Same Team Damage
	if (bCanAttackSameTeam && bIsSameTeam)
	{
		OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(DamageStatics().IncomingDamageProperty,EGameplayModOp::Additive,BaseDamage * (SameTeamPercent / 100)));
		return;
	}

	// --- 3. RPS LOGIC (Rock-Paper-Scissors) ---
	
	bool bIsAdvantage = false;
	
	//Determine Advantage/Disadvantage based on RPS Rules
	if (SourceContainer.HasTag(RockTag))
	{
		if (TargetContainer.HasTag(PaperTag)) bIsAdvantage = false;
		else if (TargetContainer.HasTag(ScissorsTag)) bIsAdvantage = true;
	}

	if (SourceContainer.HasTag(PaperTag))
	{
		if (TargetContainer.HasTag(RockTag)) bIsAdvantage = true;
		else if (TargetContainer.HasTag(ScissorsTag)) bIsAdvantage = false;	
	}

	if (SourceContainer.HasTag(ScissorsTag))
	{
		if (TargetContainer.HasTag(PaperTag)) bIsAdvantage = true;
		else if (TargetContainer.HasTag(RockTag)) bIsAdvantage = false;
	}

	
	if (bIsAdvantage)
	{
		//If the source actor has advantage, it applies full damage
		OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(DamageStatics().IncomingDamageProperty,EGameplayModOp::Additive,BaseDamage));
	}
	else
	{
		//Applies reduced damage if the source actor has disadvantage (e.g. 40% -> 0.4)
		float EvaluatedDamage = BaseDamage * (DamagePercent / 100);
		OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(DamageStatics().IncomingDamageProperty,EGameplayModOp::Additive,EvaluatedDamage));
	}

}
