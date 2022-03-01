// Fill out your copyright notice in the Description page of Project Settings.


#include "FloorSwitch.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "TimerManager.h"

// Sets default values
AFloorSwitch::AFloorSwitch()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	RootComponent = TriggerBox;

	// Set collision parameters
	// You can set collision parameters in the blueprints under the collisions tab, but we can also set that information here, which is what we're doing below
	TriggerBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly); // QueryOnly will not collide with anything, it will overlap and only do overlap events, and no physics 
	// This allows you to specify the type of collision enabled for this actor
	TriggerBox->SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic); // Set what the specific object type is
	TriggerBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore); // This will not fire off ANY events when it collides or overlaps with anything
	// Change/Set the response to all collision channels, all collision object types
	// Reason why we set it to Ignore is so we can specify individually collision responses to each channel
	TriggerBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap); 
	// Here we set that it will ONLY respond to collisions with Pawns, like our character, and set it to Overlap

	TriggerBox->SetBoxExtent(FVector(62.f, 62.f, 32.f)); // Give us a shape for our box

	// FloorSwitch
	FloorSwitch = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FloorSwitch"));
	FloorSwitch->SetupAttachment(GetRootComponent());

	Door = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Door"));
	Door->SetupAttachment(GetRootComponent());

	SwitchTime = 2.f; // This will amount for 2 seconds since we're using it for our time
	// So our switch will be active for 2 seconds before calling the CloseDoor() function
	bCharacterOnSwitch = false; // This is the way to combat the problems we have with the timer
	// By just using the timer, once we step on it it will lower the door, even if we're still on the switch, which we don't want
	// So we'll use this to check if we're still on the switch while the timers going and make sure to not lower the door unless we're off the switch

}

// Called when the game starts or when spawned
void AFloorSwitch::BeginPlay()
{
	Super::BeginPlay();

	// Below will be the functionality that will allow for, when the character overlaps with this TriggerBox, the door to open
	TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &AFloorSwitch::OnOverlapBegin);
	// AddDynamic takes what's using it, in our case "this", and what it does is it will bind a function to the overlap event
	// As soon as something overlaps with the triggerbox, call the overlap function
	// When OnOverlapBegin is called in the above code, it passes in some event parameters, so we need to put the same parameter inputs
	TriggerBox->OnComponentEndOverlap.AddDynamic(this, &AFloorSwitch::OnOverlapEnd);
	// These need to be in BeginPlay and not in the constructor because if they happen in the constructor, it may be too early in the game start process for them to work
	// These may happen before the object they're attached to is constructed, so they won't work

	InitialDoorLocation = Door->GetComponentLocation();
	InitialSwitchLocation = FloorSwitch->GetComponentLocation();
	
}

// Called every frame
void AFloorSwitch::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AFloorSwitch::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
	UE_LOG(LogTemp, Warning, TEXT("Overlap Begin"));
	if (!bCharacterOnSwitch) bCharacterOnSwitch = true;
	RaiseDoor();
	LowerFloorSwitch();
}

void AFloorSwitch::OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	UE_LOG(LogTemp, Warning, TEXT("Overlap End"));
	if (bCharacterOnSwitch) bCharacterOnSwitch = false;
	GetWorldTimerManager().SetTimer(SwitchHandle, this, &AFloorSwitch::CloseDoor, SwitchTime);
	// GetWorldTimerManager is the function that can actually set timers in game
}

void AFloorSwitch::UpdateDoorLocation(float Z)
{
	// This is going to Update the location of the door, as the name says
	FVector NewLocation = InitialDoorLocation;
	NewLocation.Z += Z;
	// Take the inital location, then add the Z we're getting from the Timeline in the BP
	Door->SetWorldLocation(NewLocation);
}

void AFloorSwitch::UpdateFloorSwitchLocation(float Z)
{
	// Do the same thing we do for the door, only this time for the floor switch and set it to move down
	FVector NewLocation = InitialSwitchLocation;
	NewLocation.Z += Z;
	FloorSwitch->SetWorldLocation(NewLocation);
}

void AFloorSwitch::CloseDoor()
{
	if (!bCharacterOnSwitch)
	{
		LowerDoor();
		RaiseFloorSwitch();
	}
	
}

// void AFloorSwitch::RaiseDoor()
// {
	// Don't need this here since we made it so we can implement it in blueprints!
// }