// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "MoveToLightAttackRange.generated.h"

/**
 * 
 */
UCLASS()
class LIGHTSOULS_API UMoveToLightAttackRange : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UMoveToLightAttackRange(const FObjectInitializer& objectInitializer);

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

protected:
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

private:
	UFUNCTION()
	void FinishMove();
private:
	bool bTaskFinished = false;
};
