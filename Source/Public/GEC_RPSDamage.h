/*
  *@brief A Custom Calculation class for Rock-Paper-Scissors based damage logic
  *Handles advantage/disadvantage percents via Curve Tables

*/
#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectExecutionCalculation.h"
#include "GEC_RPSDamage.generated.h"

UCLASS()
class MULTIPLAYERARGE_API UGEC_RPSDamage : public UGameplayEffectExecutionCalculation
{
	GENERATED_BODY()

public:
	UGEC_RPSDamage();
	virtual void Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const override;

	// Curve Table Row for Disadvantage percent value (e.g., 30 for Disadvantage)
	UPROPERTY(EditDefaultsOnly)
	FCurveTableRowHandle DisadvantageDamagePercentCurve;

	// Curve Table Row for Same Team percent value (e.g., 50 for Same Team)
	UPROPERTY(EditDefaultsOnly)
	FCurveTableRowHandle SameTeamPercentCurve;

	
};
