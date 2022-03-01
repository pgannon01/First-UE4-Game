// Fill out your copyright notice in the Description page of Project Settings.


#include "LevelTransitionVolume.h"
#include "Components/BoxComponent.h"
#include "Components/BillboardComponent.h"
#include "Main.h"

// Sets default values
ALevelTransitionVolume::ALevelTransitionVolume()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false; // This doesn't need to tick so we can set it to false, it's just gonna be a box

	TransitionVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("TransitionVolume"));
	RootComponent = TransitionVolume;

	Billboard = CreateDefaultSubobject<UBillboardComponent>(TEXT("Billboard")); // How to hard code in a billboard component
	Billboard->SetupAttachment(GetRootComponent());

	TransitionLevelName = "SunTemple";
	// By setting it to default to SunTemple, we can ensure that we don't switch levels until we give it an actual different level name
	// With this, we won't change levels unless we're not in the SunTemple level anymore
	// This way we can test it in blueprints to make sure it works

}

// Called when the game starts or when spawned
void ALevelTransitionVolume::BeginPlay()
{
	Super::BeginPlay();

	TransitionVolume->OnComponentBeginOverlap.AddDynamic(this, &ALevelTransitionVolume::OnOverlapBegin);
}

// Called every frame
void ALevelTransitionVolume::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ALevelTransitionVolume::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
	// Don't need an OnOverlapEnd because as soon as we enter this box we'll transition to the next level, so no need to look for when the overlap ends
	// This function will do most of the hard work of actually coding in the level change
	// Want to create our level transition functionality in the main character, so we'll cast to the main character and call it from there
	UE_LOG(LogTemp, Warning, TEXT("Overlap Begins"));

	if (OtherActor)
	{
		UE_LOG(LogTemp, Warning, TEXT("OtherActor Valid"));
		AMain* Main = Cast<AMain>(OtherActor);
		if (Main)
		{
			Main->SwitchLevel(TransitionLevelName);
		}
	}
}