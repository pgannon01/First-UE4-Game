// Fill out your copyright notice in the Description page of Project Settings.


#include "SpawnVolume.h"
#include "Components/BoxComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/World.h"
#include "Enemy.h"
#include "AIController.h"

// Sets default values
ASpawnVolume::ASpawnVolume()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	// Should only need to spawn on game beginning, so shouldn't need to tick

	// Set size of spawning box
	SpawningBox = CreateDefaultSubobject<UBoxComponent>(TEXT("SpawningBox"));


}

// Called when the game starts or when spawned
void ASpawnVolume::BeginPlay()
{
	Super::BeginPlay();

	if(Actor_1 && Actor_2 && Actor_3 && Actor_4)
	{
		SpawnArray.Add(Actor_1);
		SpawnArray.Add(Actor_2);
		SpawnArray.Add(Actor_3);
		SpawnArray.Add(Actor_4);
	}
	
}

// Called every frame
void ASpawnVolume::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

FVector ASpawnVolume::GetSpawnPoint() // Will pass this FVector into SpawnOurActor in blueprints to give it the Location param
{
	// Get a random point in our spawn volume and return that
	// We can do that by using a function that is available in something called UKismetMathLibrary, which has the function called RandomPointInBoundingBox
	// That will get a random point in a box
	FVector Extent = SpawningBox->GetScaledBoxExtent(); // Will return the scale of our box and store it in an FVector
	FVector Origin = SpawningBox->GetComponentLocation(); // Will return the origin of the box. Need both of these to get a random point
	
	FVector Point = UKismetMathLibrary::RandomPointInBoundingBox(Origin, Extent);
	// With this we have a spawning point blueprint pure function
	return Point;
}

void ASpawnVolume::SpawnOurActor_Implementation(UClass* ToSpawn, const FVector& Location)
{
	// When we make a BlueprintNative event, our C++ implementation has to be called the above, our function name with _Implementation
	// This way UE knows this is the implementation we scripted out in C++ so that part of it will also be carried out in blueprints
	if (ToSpawn)
	{
		UWorld* World = GetWorld();
		FActorSpawnParameters SpawnParams;

		if (World)
		{
			AActor* Actor = World->SpawnActor<AActor>(ToSpawn, Location, FRotator(0.f), SpawnParams); 
			// This constructs and spawns an actor that we set, and then returns that actor (set the actor to spawn in blueprints on the GetSpawnActor Array)

			// Check to see if we're spawning enemies and not just actors
			// Our spawn volume can be used to spawn potions, coins, pretty much anything
			// So we need to see if we're spawning an enemy
			AEnemy* Enemy = Cast<AEnemy>(Actor);
			if (Enemy)
			{
				Enemy->SpawnDefaultController(); // Will spawn a AI Controller for this and set it for our pawn

				// That takes care of our AI controller, but our Enemy already should have an AI Controller
				// So what we need to do if we're spawning it here, is to set that variable already
				// So need to first create a AIController variable
				AAIController* AICont = Cast<AAIController>(Enemy->GetController()); // Will cast to an AIController
				if (AICont)
				{
					Enemy->AIController = AICont; // Set the AIController on the enemy to our already set AIController
				}
			}
		}
	}
}

TSubclassOf<AActor> ASpawnVolume::GetSpawnActor()
{
	if (SpawnArray.Num() > 0)
	{
		int32 Selection = FMath::RandRange(0, SpawnArray.Num() - 1); // Element numbers will be 4, but we can't index the 4th number so we have to start at 0 instead of 1

		return SpawnArray[Selection];
		// Take the random int, pass it into the array, and get that element
	}
	else
	{
		// If spawn array is empty, return null
		return nullptr;
	}
}