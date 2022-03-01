// Fill out your copyright notice in the Description page of Project Settings.


#include "Critter.h"
#include "Components/SkeletalMeshComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/InputComponent.h"

// Sets default values
ACritter::ACritter() // constructor
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	MeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetupAttachment(GetRootComponent()); // Creating the static mesh component and adding it to the root

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera")); // creating a camera
	Camera->SetupAttachment(GetRootComponent()); // attaching camera to the root component
	Camera->SetRelativeLocation(FVector(-300.f, 0.f, 300.f));
	Camera->SetRelativeRotation(FRotator(-45.f, 0.f, 0.f));

	// Here we're creating an Enum that allows us to possess and play as our critter (the default pawn we've selected)
	// AutoPossessPlayer = EAutoReceiveInput::Player0; 
	// Player 0 refers to the first player in multiplayer, and the first and only player in single player
	// This plus everything else we did to create the pawn and the game mode, will set the player start as the pawn we've selected
	// And thus when we hit play we'll spawn in as the pawn. However, as of this point we won't have any actual controls to move around
	// Need to go set up inputs in Project Settings (Edit option along the top)

	CurrentVelocity = FVector(0.f);
	MaxSpeed = 100.f;

}

// Called when the game starts or when spawned
void ACritter::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ACritter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Take our current location and add our velocity times the DeltaTime(the frames)
	// How this equation works is the location + some distance * time
	FVector NewLocation = GetActorLocation() + (CurrentVelocity * DeltaTime);
	SetActorLocation(NewLocation);

}

// Called to bind functionality to input
void ACritter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) // Where keybinding happens
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Here we're gonna customize the keybindings we set up in the Project Settings
	// Basic premise of below call:
	// BindAxis(Axis in string you're calling, what is the object doing all this, in this case "this", a reference to a function)
	// Passing the function in as we've done will bind it to that event, that keybind, so when we hit the W, it will call that function
	// And it will pass into value the axis, depending on whether the key is pressed or not
	// And when "s" or the down key is pressed it will pass in the negative value we set, thus meaning we don't have to set up a MoveBackwards function
	PlayerInputComponent->BindAxis(TEXT("MoveForward"), this, &ACritter::MoveForward);
	PlayerInputComponent->BindAxis(TEXT("MoveRight"), this, &ACritter::MoveRight);

}

// Functions below set up how the movement will actually go
void ACritter::MoveForward(float Value)
{
	// FMath takes a min and a max and ensures it never goes below the min or above the max
	CurrentVelocity.Y = FMath::Clamp(Value, -1.f, 1.f) * MaxSpeed;
}

void ACritter::MoveRight(float Value)
{
	CurrentVelocity.X = FMath::Clamp(Value, -1.f, 1.f) * MaxSpeed;
	// BUUUUT, these won't actually move us, these will jut update CurrentVelocity, that's when we go into the Tick function!
}