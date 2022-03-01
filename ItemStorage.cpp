// Fill out your copyright notice in the Description page of Project Settings.


#include "ItemStorage.h"

// Sets default values
AItemStorage::AItemStorage()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false; // This doesn't need to tick
	// Should also keep in mind to go through classes, see which ones actually need tick, and for those that don't set this to off to save resources
	// If we don't need tick, then there's no reason to leave it to true

}

// Called when the game starts or when spawned
void AItemStorage::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AItemStorage::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

