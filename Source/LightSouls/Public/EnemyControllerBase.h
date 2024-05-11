// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionComponent.h"
#include "EnemyControllerBase.generated.h"
/**
 * 
 */
UCLASS()
class LIGHTSOULS_API AEnemyControllerBase : public AAIController
{
	GENERATED_BODY()

public:
	AEnemyControllerBase();

protected:
	virtual void BeginPlay() override;

private:
	UFUNCTION()
	void TargetPerceptionUpdated(AActor* const SpottedActor, const FAIStimulus& Stimulus);

protected:
	UPROPERTY(EditDefaultsOnly)
	UBlackboardData* BlackboardToUse;

	UPROPERTY(EditDefaultsOnly)
	UBehaviorTree* DefaultBehaviorTree;

	UPROPERTY(EditDefaultsOnly)
	UAIPerceptionComponent* AIPerception;
	
private:
	UBlackboardComponent* BlackboardComponent;
};
