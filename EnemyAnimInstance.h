// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "EnemyAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class MYPROJECT_API UEnemyAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
public:
	virtual void NativeInitializeAnimation() override; // This will be similar to BeginPlay for us

	UFUNCTION(BlueprintCallable, Category = "AnimationProperties") // Can give functions a UFUNCTION tag and, just like UPROPERTY, can have them work with blueprints
	void UpdateAnimationProperties();

	// Going to want the movement speed so we can update our BlendSpace to know if we're idle, walking or running
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	float MovementSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	class APawn* Pawn; // for our character pawn

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	class AEnemy* Enemy; 
	// Above two will be to reference the pawn and the main Enemy instance that owns this AnimInstance
	
};
