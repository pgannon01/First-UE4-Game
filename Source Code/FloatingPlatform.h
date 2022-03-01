// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FloatingPlatform.generated.h"

UCLASS()
class MYPROJECT_API AFloatingPlatform : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AFloatingPlatform();

	/** Mesh for the platform */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Platform")
	class UStaticMeshComponent* Mesh;

	UPROPERTY(EditAnywhere, Category = "Platform")
	FVector StartPoint;

	UPROPERTY(EditAnywhere, meta = (MakeEditWidget = "true")) // use the meta here so we can make a visualization widget that we can manipulate in the editor
	FVector EndPoint; 

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Platform")
	float InterpSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Platform")
	float InterpTime;

	FTimerHandle InterpTimer;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Platform")
	bool bInterping;

	float Distance;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void ToggleInterping();

	void SwapVectors(FVector& VecOne, FVector& VecTwo);

};
