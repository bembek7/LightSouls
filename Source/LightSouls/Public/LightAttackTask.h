// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "LightAttackTask.generated.h"

/**
 *
 */
UCLASS()
class LIGHTSOULS_API ULightAttackTask : public UBTTaskNode
{
	GENERATED_BODY()

public:
	ULightAttackTask(const FObjectInitializer& objectInitializer);

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

protected:
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

private:
	UFUNCTION()
	void PerformLightAttack(AEnemyBase* const ControlledPawn);
private:
	bool bTaskFinished = false;
};
