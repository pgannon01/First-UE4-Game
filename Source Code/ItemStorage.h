// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ItemStorage.generated.h"

UCLASS()
class MYPROJECT_API AItemStorage : public AActor
{
	GENERATED_BODY()

	// Purpose of this class is to use this as a storage container for things we want to save and load for things we can't save, namely blueprints
	// This way things like armor and weapons can be saved and trasferred across saves, otherwise without it we won't have a way to save what weapons the character has in
	// their inventory
	
public:	
	// Sets default values for this actor's properties
	AItemStorage();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Save a variable for each different type of weapon there is
	// Create the ability to store UClasses, in our case some blueprints
	// What we're going to use to store them is a map. It's a lot like an array
	// But unlike an array a map works more like an object
	// You have a key, which can be any kind of type, and a value for that key
	// Whereas in array's the keys are all ints, and you can index the item by their value
	UPROPERTY(EditDefaultsOnly, Category = "SaveData")
	TMap<FString, TSubclassOf<class AWeapon>> WeaponMap; // How we declare a TMap, TMap<(Key Type we want to store), (Value Type we want to store)>
	// So now we have WeaponMap, which is like an array only we can add things in by specifying the key and value
	// And we can also get things out of it by specifying the key and it will return the value
	// Also has speicific functions like Add(), in which we'd have to supply an FString for the Name and then an AWeapon (In our case)
	// Ex: WeaponMap.Add("Key", MyWeapon);
	// This would store MyWeapon in the WeaponMap associated with its set key
	// Could then access it like: WeaponMap["Key"]
	// That would return us the Item stored with that key

};
