// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Item.generated.h"

UCLASS()
class MYPROJECT_API AItem : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AItem();

	/** Base shape collision */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Item | Collision") // Category | SubCategory
	class USphereComponent* CollisionVolume;

	/** Base mesh component */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Item | Mesh") // Category | SubCategory
	class UStaticMeshComponent* Mesh;

	// For particles
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item | Particles")
	class UParticleSystemComponent* IdleParticlesComponent;
	// A reminder: a UParticleSystem would NOT need to be constructed in the constructor to work in blueprints, but a UParticleSystemComponent does
	// Same with UStaticMesh and USphere

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item | Particles")
	class UParticleSystem* OverlapParticles;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item | Sounds")
	class USoundCue* OverlapSound;

	// Rotational functionality
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item | Properties")
	bool bRotate; // Don't want everything to rotate, only things we set so we make it a bool

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item | Properties")
	float RotationRate;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION()
	virtual void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);
	
	UFUNCTION()
	virtual void OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

};
