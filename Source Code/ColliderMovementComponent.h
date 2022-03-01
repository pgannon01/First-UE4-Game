// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PawnMovementComponent.h"
#include "ColliderMovementComponent.generated.h"

/**
 * 
 */
UCLASS()
class MYPROJECT_API UColliderMovementComponent : public UPawnMovementComponent
{
	GENERATED_BODY()
public:
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;

	// Why are we doing this? Normally you would not ever create your own movement component, you would allow UE's own functionality to work for you
	// Usually when you create a carrier it automatically spawns in its own movement component which already has its own functionailty inside it
	// These files are meant to be a more learning exercise where we can learn and understand how and why it works
	// So we're gonna make our own custom movement component so we can better understand and appreciate what's going on normally
	// In the future we'll let Unreal do the work for us
};
