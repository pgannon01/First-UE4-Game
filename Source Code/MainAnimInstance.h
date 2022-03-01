// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "MainAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class MYPROJECT_API UMainAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
public: 
	// UAnimInstance is not an actor, hence the U at the beginning, so it doesn't have a BeginPlay()
	virtual void NativeInitializeAnimation() override; // This will be similar to BeginPlay for us

	UFUNCTION(BlueprintCallable, Category = AnimationProperties) // Can give functions a UFUNCTION tag and, just like UPROPERTY, can have them work with blueprints
	void UpdateAnimationProperties();

	// Going to want the movement speed so we can update our BlendSpace to know if we're idle, walking or running
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Movement)
	float MovementSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Movement)
	bool bIsInAir; // true when in air; false when on ground
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Movement)
	class APawn* Pawn; // for our character pawn

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Movement)
	class AMain* Main;

};
