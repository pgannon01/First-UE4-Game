// Fill out your copyright notice in the Description page of Project Settings.


#include "Collider.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Components/InputComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "ColliderMovementComponent.h"

// Sets default values
ACollider::ACollider()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

	SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComponent"));
	// Need to attach SphereComponent to the root like we did Critter
	//SphereComponent->SetupAttachment(GetRootComponent());

	// For our movement we need to set the SphereComponent as the RootComponent instead of making a RootComponent
	SetRootComponent(SphereComponent); // This way SphereComponent is actually the root

	SphereComponent->InitSphereRadius(40.f); // Set Sphere radius
	SphereComponent->SetCollisionProfileName(TEXT("Pawn")); // set collision profile name

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetupAttachment(GetRootComponent());
	// This function, below, will create a UStaticMesh, so we're finding/creating a StaticMesh 
	static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshComponentAsset(TEXT("StaticMesh'/Game/StarterContent/Shapes/Shape_Sphere.Shape_Sphere'"));
	// So here we're creating a new variable of this type called MeshComponentAsset
	// This will create a MeshComponentAsset, with some variables we can access to initialize our MeshComponent

	if (MeshComponentAsset.Succeeded()) // If this Object Constructor helper succeeded and found a static mesh at the set path
	{
		MeshComponent->SetStaticMesh(MeshComponentAsset.Object);
		MeshComponent->SetRelativeLocation(FVector(0.f, 0.f, -40.f));
		MeshComponent->SetWorldScale3D(FVector(0.8f, 0.8f, 0.8f));
	}
	// Above is essentially the way to hardcode static meshes rather than selecting them in blueprints

	// SpringArm is essentially where we WANT the camera to be positioned, but not the actual camera
	// It also controls some of the effects of the camera, like camera lag
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(GetRootComponent());
	SpringArm->SetRelativeRotation(FRotator(-45.f, 0.f, 0.f)); // set camera angle looking at pawn(down)
	SpringArm->TargetArmLength = 400.f; // Camera distance from pawn
	SpringArm->bEnableCameraLag = true; // Camera lag is basically smooth trailing behind our pawn, lags behind slightly according to our camera lag speed
	SpringArm->CameraLagSpeed = 3.0f; // Setting the camera lag speed

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera")); // Add the actual camera
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName); // Actually attach it to our SpringArm

	OurMovementComponent = CreateDefaultSubobject<UColliderMovementComponent>(TEXT("OurMovementComponent"));
	OurMovementComponent->UpdatedComponent = RootComponent; // Updated Component on a movement component is the component on a pawn that this movement component is associated with...
	// ... that all the movement will be applied to
	// If this pawn is going to be moved by the movement component, the movement component is going to apply that movement to a component
	// Essentially a movement component needs a designated update component that it can update
	// So we're going to give it the RootComponent of our collider
	// When our movement component needs to apply movement, it's going to apply it to the colliders root, which we make the SphereComponent above

	CameraInput = FVector2D(0.f, 0.f);

	AutoPossessPlayer = EAutoReceiveInput::Player0;

}

// Called when the game starts or when spawned
void ACollider::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ACollider::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	FRotator NewRotation = GetActorRotation();
	NewRotation.Yaw += CameraInput.X;
	SetActorRotation(NewRotation);

	FRotator NewSpringArmRotation = SpringArm->GetComponentRotation();
	NewSpringArmRotation.Pitch = FMath::Clamp(NewSpringArmRotation.Pitch += CameraInput.Y, -80.f, -15.f);

	SpringArm->SetWorldRotation(NewSpringArmRotation);

}

// Called to bind functionality to input
void ACollider::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis(TEXT("MoveForward"), this, &ACollider::MoveForward);
	PlayerInputComponent->BindAxis(TEXT("MoveRight"), this, &ACollider::MoveRight);

	PlayerInputComponent->BindAxis(TEXT("CameraPitch"), this, &ACollider::PitchCamera);
	PlayerInputComponent->BindAxis(TEXT("CameraYaw"), this, &ACollider::YawCamera);

}

void ACollider::MoveForward(float Input) 
{
	FVector Forward = GetActorForwardVector();

	if (OurMovementComponent)
	{
		OurMovementComponent->AddInputVector(Forward * Input);
	}
}

void ACollider::MoveRight(float Input)
{
	FVector Right = GetActorRightVector();
	
	if (OurMovementComponent)
	{
		OurMovementComponent->AddInputVector(Right * Input);
	}
}

void ACollider::YawCamera(float AxisValue)
{
	CameraInput.X = AxisValue;
}

void ACollider::PitchCamera(float AxisValue)
{
	CameraInput.Y = AxisValue;
}

UPawnMovementComponent* ACollider::GetMovementComponent() const
{
	return OurMovementComponent;
	// OurMovementComponent derives from UPawnMovementComponent so we can return OurMovementComponent
	// Just keep in mind it's going to be returned in the form of a UPawnMovementComponent
}