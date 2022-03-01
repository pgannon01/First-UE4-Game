// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Collider.generated.h"

UCLASS()
class MYPROJECT_API ACollider : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ACollider();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UPROPERTY(VisibleAnywhere, Category = "Mesh")
	class UStaticMeshComponent* MeshComponent;

	UPROPERTY(VisibleAnywhere, Category = "Mesh")
	class USphereComponent* SphereComponent; // Will be a sphere for collision and physics and the like

	UPROPERTY(VisibleAnywhere, Category = "Mesh")
	class UCameraComponent* Camera; // Set up a camera

	UPROPERTY(VisibleAnywhere, Category = "Mesh")
	class USpringArmComponent* SpringArm; // Controls the cameras motion

	UPROPERTY(VisibleAnywhere, Category = "Movement")
	class UColliderMovementComponent* OurMovementComponent; // Naming it like this so we can remember that we made it and it's something we made for the lesson

	virtual UPawnMovementComponent* GetMovementComponent() const override;

	// GETTER
	// Complete function Declaration and definition all in one, gets the MeshComponent we declar and returns it
	// Rather than access the variable directly, good programming practice to create functions that will get or set(change or alter) a particular member variable
	// Which is what we do below
	FORCEINLINE UStaticMeshComponent* GetMeshComponent() { return MeshComponent; }
	// FORCEINLINE as the name suggets, makes a particular function inline
	// meaning that anywhere you call that function in your code, it will treat it like a macro and inline it
	// Essentially it will take the functionality inside the function and paste it inwherever you call it. 
	// If you don't it'll have to jump to wherever it is in your code everywhere it encounters a call to it
	// with FORCEINLINE when it compiles, { return MeshComponent } is pasted anywhere GetMeshComponent is called, and makes it a bit faster
	// Optimization!

	// SETTER
	// Good way to set the MeshComponent if we want to change it later
	FORCEINLINE void SetMeshComponent(UStaticMeshComponent* Mesh) { MeshComponent = Mesh; }

	FORCEINLINE USphereComponent* GetSphereComponent() { return SphereComponent; }
	FORCEINLINE void SetSphereComponent(USphereComponent* Sphere) { SphereComponent = Sphere; }

	FORCEINLINE UCameraComponent* GetCameraComponent() { return Camera; }
	FORCEINLINE void SetCameraComponent(UCameraComponent* InCamera) { Camera = InCamera; }

	FORCEINLINE USpringArmComponent* GetSpringArmComponent() { return SpringArm; }
	FORCEINLINE void SetSpringArmComponent(USpringArmComponent* InSpringArm) { SpringArm = InSpringArm; }

private:
	void MoveForward(float Input);
	void MoveRight(float Input);

	void PitchCamera(float AxisValue);
	void YawCamera(float AxisValue);

	FVector2D CameraInput;

};
