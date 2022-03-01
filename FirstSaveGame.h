// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "FirstSaveGame.generated.h"

USTRUCT(BlueprintType)
struct FCharacterStats
{
	// Rather than create a bunch of individual variables inside this file to keep track of all the players current stats like the gold they have, how much health they have, etc...
	// We'll use a struct to hold all that data for us
	// F before the Struct name is convention and like enums, we have to prefix it properly if we want to give it a macro that will expose it to the reflection system
	// And since these stats need to be saved across multiple levels, making sure it's visible in the reflection system is important
	// By giving it the UStruct macro and giving it the BlueprintType param, the struct we create can be a type in blueprints
	// Another thing you have to do is give it a GENERATED_BODY() macro, without it it won't be visible in the reflection system
	GENERATED_BODY() // Like this

	// Structs can have their own variables and macros
	UPROPERTY(VisibleAnywhere, Category = "SaveGameData")
	float Health; // This is just a member of this struct type 

	UPROPERTY(VisibleAnywhere, Category = "SaveGameData")
	float MaxHealth;

	UPROPERTY(VisibleAnywhere, Category = "SaveGameData")
	float Stamina;

	UPROPERTY(VisibleAnywhere, Category = "SaveGameData")
	float MaxStamina;

	UPROPERTY(VisibleAnywhere, Category = "SaveGameData")
	int32 Coins;

	UPROPERTY(VisibleAnywhere, Category = "SaveGameData")
	FVector Location; // save the characters location in the world

	UPROPERTY(VisibleAnywhere, Category = "SaveGameData")
	FRotator Rotation; // save the characters rotation in the world

	UPROPERTY(VisibleAnywhere, Category = "SaveGameData")
	FString WeaponName;

	UPROPERTY(VisibleAnywhere, Category = "SaveGameData")
	FString LevelName;
};

/**
 * 
 */
UCLASS()
class MYPROJECT_API UFirstSaveGame : public USaveGame
{
	// See that FirstSaveGame has a U in front of it, it's not an actor
	// It derives from UObject, and not something you can place in the level
	GENERATED_BODY()
	
public:
	UFirstSaveGame();

	// Default variables for Player's name and save slots
	// Have to be given UPROPERTY's to be recognized in the reflection system so we can save the variables
	UPROPERTY(VisibleAnywhere, Category = Basic) // Category MUST BE Basic, otherwise it won't be saved
	FString PlayerName; // Slot name
	
	UPROPERTY(VisibleAnywhere, Category = Basic) // Category MUST BE Basic, otherwise it won't be saved
	uint32 UserIndex; // Slot Index

	UPROPERTY(VisibleAnywhere, Category = Basic)
	FCharacterStats CharacterStats; // Once we've defined our struct we can come into the class and assign it to a variable


};
