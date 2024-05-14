// Fill out your copyright notice in the Description page of Project Settings.

#include "LightAttackTask.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "EnemyBase.h"

ULightAttackTask::ULightAttackTask(const FObjectInitializer& objectInitializer)
	: Super(objectInitializer)
{
	INIT_TASK_NODE_NOTIFY_FLAGS();
}

EBTNodeResult::Type ULightAttackTask::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::ExecuteTask(OwnerComp, NodeMemory);
	AAIController* const OwningController = Cast<AAIController>(OwnerComp.GetOwner());
	if (OwningController)
	{
		if (AEnemyBase* const ControlledPawn = Cast<AEnemyBase>(OwningController->GetPawn()))
		{
			PerformLightAttack(ControlledPawn);
		}
	}

	return EBTNodeResult::InProgress;
}

void ULightAttackTask::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);

	// I have to use such a stupid workaurond cause there is no way to pass Owner comp to some function so that it would work, it's pretty weird
	if (bTaskFinished)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		bTaskFinished = false;
	}
}

void ULightAttackTask::PerformLightAttack(AEnemyBase* const ControlledPawn)
{
	if (ControlledPawn)
	{
		const float AttackTime = ControlledPawn->StartLightAttack();
		FTimerHandle FinishTaskTimerHandle;
		ControlledPawn->GetWorldTimerManager().SetTimer(FinishTaskTimerHandle, [this]() {bTaskFinished = true; }, AttackTime, false);
	}
}