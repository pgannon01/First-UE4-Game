// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item.h"
#include "Pickup.generated.h"

/**
 * 
 */
UCLASS()
class MYPROJECT_API APickup : public AItem
{
	GENERATED_BODY()

public: 

	APickup();

	virtual void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult) override;
	
	virtual void OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex) override;

	UFUNCTION(BlueprintImplementableEvent, Category = "Pickup")
	void OnPickupBP(class AMain* Target);
	// Above function is meant to replace coin count
	// This C++ class will no longer just be meant to represent Coins, but any and all possible pickups we could have
	// Instead of writing code out to check which kind of pickup it is in this C++ file, or copying this to a different C++ file specifically for potions...
	// ... we're going to make this a catchall for ANY pickup, but add in functionality to differentiate via blueprints
	// So the blueprints will be the Coins or Potions, and they'll inherit from this Pickup C++ class
	// That's what OnPickupBP() is for, to allow us a Blueprint usable function that can differentiate between the items
};
