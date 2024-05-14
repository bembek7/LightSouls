// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/BoxComponent.h"
#include "Combatable.generated.h"

UCLASS()
class LIGHTSOULS_API ACombatable : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ACombatable();

	float GetCurrentHealth() const;
	float GetMaxHealth() const;
	float StartLightAttack();
	float StartHeavyAttack();

	bool IsDead() const;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintPure)
	bool IsBlocking() const;

	UFUNCTION()
	void AttackFinished();

	UFUNCTION()
	void BlockingStarted();

	UFUNCTION()
	void BlockingEnded();

	virtual void Die();
	virtual void Damage(const float Damage, const FVector& HitterLocation);

private:
	float StartAttack(UAnimSequence* const AttackAnimSequence);
	void SpawnBlood(const FVector& HitterLocation);

	UFUNCTION()
	void OnSwordHit(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp);

public:

protected:
	UPROPERTY(EditDefaultsOnly)
	UAnimSequence* LightAttackAnimSequence;
	UPROPERTY(EditDefaultsOnly)
	UAnimSequence* HeavyAttackAnimSequence;

	UPROPERTY(EditDefaultsOnly)
	UAnimSequence* BlockingStartAnimSequence;

	UPROPERTY(EditDefaultsOnly)
	UAnimSequence* BlockingEndAnimSequence;

	UPROPERTY(EditDefaultsOnly)
	UAnimSequence* HitImpactAnimSequence;

	UPROPERTY(EditDefaultsOnly)
	UAnimSequence* DeathAnimSequence;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AActor>OnHitParticleEffectClass;

	UPROPERTY(EditDefaultsOnly)
	class UBoxComponent* SwordCollider;

	UPROPERTY(EditDefaultsOnly)
	float MaxHealth = 150.f;
	UPROPERTY(EditDefaultsOnly)
	float LightAttackDamage = 30.f;
	UPROPERTY(EditDefaultsOnly)
	float HeavyAttackDamage = 50.f;
	UPROPERTY(EditDefaultsOnly)
	float BlockValue = 30.f;

	bool bInAttack = false;
	bool bIsDead = false;

private:
	TArray<AActor*>EnemiesHit;

	float CurrentHealth;

	bool bInLightAttack = false;
	bool bInHeavyAttack = false;
	bool bIsBlocking = false;
};
