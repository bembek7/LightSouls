// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/TimelineComponent.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "PlayerBase.generated.h"

UCLASS()
class LIGHTSOULS_API APlayerBase : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	APlayerBase();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	float GetCurrentStamina() const;
	float GetMaxStamina() const;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	UFUNCTION()
	void RegenerateStamina();

	bool HasEnoughStamina(const float StaminaCost) const;

	void PayStamina(const float StaminaCost);

	UFUNCTION(Category = "Input Response")
	void StartRoll();

	UFUNCTION(Category = "Input Response")
	void StartLightAttack();

	UFUNCTION(Category = "Input Response")
	void Walk(const FInputActionValue& IAValue);

	UFUNCTION(Category = "Input Response")
	void Look(const FInputActionValue& IAValue);

	UFUNCTION()
	void RollUpdate(const float RollForceMultiplier) const;

	UFUNCTION()
	void RollFinished();

	UFUNCTION()
	void AttackFinished();

	UFUNCTION()
	void LockCamera();

	AActor* EstablishTargetToLockOn();

	AActor* FindEnemyLookedAt();

	bool IsInputBlocked() const;

	UFUNCTION()
	void OnSwordHit(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp);

	void SnapCameraToTarget();

public:

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Enhanced Input")
	TSoftObjectPtr<UInputMappingContext> InputMappingContext;

	UPROPERTY(EditDefaultsOnly, Category = "Enhanced Input")
	UInputAction* IAWalk;

	UPROPERTY(EditDefaultsOnly, Category = "Enhanced Input")
	UInputAction* IALook;

	UPROPERTY(EditDefaultsOnly, Category = "Enhanced Input")
	UInputAction* IARoll;

	UPROPERTY(EditDefaultsOnly, Category = "Enhanced Input")
	UInputAction* IALightAttack;

	UPROPERTY(EditDefaultsOnly, Category = "Enhanced Input")
	UInputAction* IALockCamera;

	UPROPERTY(EditDefaultsOnly)
	class UCameraComponent* Camera;

	UPROPERTY(EditDefaultsOnly)
	class USpringArmComponent* SpringArm;

	UPROPERTY(EditDefaultsOnly)
	class UBoxComponent* SwordCollider;

	UPROPERTY(EditDefaultsOnly)
	UAnimMontage* RollAnimMontage;

	UPROPERTY(EditDefaultsOnly)
	UAnimMontage* LightAttackAnimMontage;

	UPROPERTY(EditDefaultsOnly)
	float RollAnimPlayRate = 1.5f;

	UPROPERTY(EditDefaultsOnly, Category = "Roll")
	float RollForce = 400.f;

	UPROPERTY(EditDefaultsOnly, Category = "Roll")
	UCurveFloat* RollForceCurve;

private:
	bool bInRoll = false;
	bool bInAttack = false;
	FVector2D MoveVector;
	FVector RollDirection;

	AActor* LockedTarget = nullptr;

	float MaxStamina = 100.f;
	float CurrentStamina;
	float RollStaminaCost = 30.f;

	float StaminaRegenerationPerSecond = 30.f;

	float LightAttackStaminaCost = 25.f;
	float LightAttackDamage = 30.f;
	TArray<AActor*>EnemiesHit;

	FOnTimelineFloat RollInterp;
	FOnTimelineEvent RollFinishedEvent;
	FTimerHandle StaminaRegenerationHandle;

	UTimelineComponent* RollTimeline;
};
