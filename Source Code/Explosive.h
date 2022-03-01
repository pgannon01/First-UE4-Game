// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item.h"
#include "Explosive.generated.h"

/**
 * 
 */
UCLASS()
class MYPROJECT_API AExplosive : public AItem
{
	GENERATED_BODY()

public:

	AExplosive();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	float Damage;

	virtual void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult) override;
	
	virtual void OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex) override;
	// We're overriding these two because they exist in the Item class, which we're deriving from
	// Because on Item there are already UFUNCTION macros, we can't mark them with UFUNCTION macros
	// So the way this works is if we're inheriting from a parent class, and on that parent there's a UFUNCTION macro with specifiers, what will happen is...
	// ... you will inherit those UFUNCTION properties, so whatever properties the parent has the child will inherit
	// If we don't take them off we'll get a compiler error

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	TSubclassOf<UDamageType> DamageTypeClass;

};
