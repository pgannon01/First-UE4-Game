// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FloorSwitch.generated.h"

UCLASS()
class MYPROJECT_API AFloorSwitch : public AActor
{
	GENERATED_BODY()

public:
	/** Overlap volume for functionality to be triggered */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "FloorSwitch")
	class UBoxComponent* TriggerBox; // Basically a box, like our old USphereComponent
	// Calling it TriggerBox because whenever we enter it we want something to happen to it

	// Switch for character to step on
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "FloorSwitch")
	class UStaticMeshComponent* FloorSwitch;

	// Door to move when the floor switch is stepped on
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "FloorSwitch")
	UStaticMeshComponent* Door;

	// Below will be to do in code what we did in the blueprints
	/** Initial Location for the door */
	UPROPERTY(BlueprintReadWrite, Category = "FloorSwitch")
	FVector InitialDoorLocation;

	/** Initial location for the floor switch */
	UPROPERTY(BlueprintReadWrite, Category = "FloorSwitch")
	FVector InitialSwitchLocation;

	FTimerHandle SwitchHandle; // This will be a timer so the door stays up for however long we set it for

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FloorSwitch")
	float SwitchTime;

	bool bCharacterOnSwitch;

public:	
	// Sets default values for this actor's properties
	AFloorSwitch();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);
	// Above filled with parameters it will be passed when we call it in the cpp file by the AddDynamic macro

	UFUNCTION()
	void OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION(BlueprintImplementableEvent, Category = "FloorSwitch") 
	void RaiseDoor();
	// What "BlueprintImplementableEvent" means is that we don't have to provide implementation in C++, we can implement the functionality in blueprints
	// So that's what we're going to do

	UFUNCTION(BlueprintImplementableEvent, Category = "FloorSwitch") 
	void LowerDoor();

	UFUNCTION(BlueprintImplementableEvent, Category = "FloorSwitch") 
	void RaiseFloorSwitch();

	UFUNCTION(BlueprintImplementableEvent, Category = "FloorSwitch") 
	void LowerFloorSwitch();

	UFUNCTION(BlueprintCallable, Category = "FloorSwitch") 
	void UpdateDoorLocation(float Z);

	UFUNCTION(BlueprintCallable, Category = "FloorSwitch") 
	void UpdateFloorSwitchLocation(float Z);

	void CloseDoor(); // When our above timer is done it will call this function to close the door

};
