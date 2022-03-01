// Fill out your copyright notice in the Description page of Project Settings.


#include "Main.h"
#include "FirstSaveGame.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Engine/World.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Weapon.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Kismet/KismetMathLibrary.h"
#include "Enemy.h"
#include "MainPlayerController.h"
#include "ItemStorage.h"

// Sets default values
AMain::AMain()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Create CameraBoom (Pulls towards the player if there's a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetRootComponent());
	CameraBoom->TargetArmLength = 600.f; // Camera follows at this distance
	CameraBoom->bUsePawnControlRotation = true; // Rotate arm based on controller
	// Controller is a class that is attached to a pawn, moves along with it, and can also provide input to the Controller
	// SpringArm class has this bool to allow it to be attached to the controller so if you're providing input to rotate the controller around...
	// ... this bool will cause the Camera spring arm to follow the controllers rotation, allowing it to look around in the world

	// Hard Code the Capsule Size
	GetCapsuleComponent()->SetCapsuleSize(48.f, 105.f);

	// Create Follow Camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); //Camera attached to Camera boom
	// SetupAttachment also takes a socket, that we're putting as USpringArmComponent to attach it to the CameraBoom
	// Sockets are something that can be put onto an asset, like a skeleton, allowing you to attach things to it, like a character picking up a sword
	// So now with this our Camera's set up and attached to the boom

	// Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; 
	// Want the camera to just be attached to the CameraBoom and follow along with that, and not be dependent on the controller

	// Set our turn rates for input
	BaseTurnRate = 65.f;
	BaseLookUpRate = 65.f;

	// Stop the character from rotating with the camera
	// Let that just affect the camera
	bUseControllerRotationYaw = false;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;

	// Configure Character Movement
	// Characters automatically inherit a CharacterMovementComponent, which contains a lot of useful parameters to affect the characters movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // When this set to true the character will turn towards the direction they're moving
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.f, 0.0f); // Middle is the yaw
	// This is a rotation rate that we want the character to use when orienting its rotation to the movement, only want to orient its yaw to the movement
	// So if it's jumping up or down we don't want to orient it's movement in those directions
	GetCharacterMovement()->JumpZVelocity = 500.f; // Sets how high character will jump
	GetCharacterMovement()->AirControl =  0.2f; // A little control, not a lot really
	// AirControl is when characters in the air, still allows you to still move around in space slightly while in the air, good for platformers

	// Initializing Player Stats
	MaxHealth = 100.f;
	Health = 65.f;
	MaxStamina = 150.f;
	Stamina = 120.f;
	Coins = 0;

	RunningSpeed = 650.f;
	SprintingSpeed = 950.f;

	bShiftKeyDown = false;
	bLMBDown = false;
	bESCDown = false;

	// Initialize Enums
	MovementStatus = EMovementStatus::EMS_Normal;
	StaminaStatus = EStaminaStatus::ESS_Normal;

	StaminaDrainRate = 25.f;
	MinSprintStamina = 50.f;

	InterpSpeed = 15.f;
	bInterpToEnemy = false;

	bHasCombatTarget = false;

	bMovingForward = false;
	bMovingRight = false;
}

// Called when the game starts or when spawned
void AMain::BeginPlay()
{
	Super::BeginPlay();

	MainPlayerController = Cast<AMainPlayerController>(GetController()); // Returns an AControllerObject and stores it
	// Call this as soon as we need to display a health bar

	FString Map = GetWorld()->GetMapName();
	Map.RemoveFromStart(GetWorld()->StreamingLevelsPrefix);

	if (Map != "SunTemple")
	{
		// Problem with this check is that we can't save on the SunTemple
		// So if we're on the sun temple and we save, we'll just start from the beginning
		// Better to not use this in the future

		LoadGameNoSwitch(); // When the level loads we'll get all the stuff we should have
		// This is mainly for switching between levels, so that we keep all our inventory

		if (MainPlayerController)
		{
			MainPlayerController->GameModeOnly();
			// Every time we load a new level from the pause menu, this will make sure we can move around
		}
	}
	
	
}

// Called every frame
void AMain::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (MovementStatus == EMovementStatus::EMS_Dead) return;

	// How much the stamina should change in this particular frame for drain or recovery
	float DeltaStamina = StaminaDrainRate * DeltaTime;
	// Could have different floats for recover and drain, but the process is the same anyway so no real need unless you want one to be faster than the other

	// Because of so many different statuses for Sprinting, Normal, BelowMinimum, Exhausted, etc., rather than do a bunch of if else statements...
	// And we'll only be in one at any time, we should use a switch statement
	switch (StaminaStatus)
	{
	case EStaminaStatus::ESS_Normal:
		if (bShiftKeyDown)
		{
			// First going to check to see if we're in the normal state and draining stamina with the key down
			// But before we drain stamina, we need to see if such a small change would put us BelowMinimum
			if (Stamina - DeltaStamina <= MinSprintStamina)
			{
				// If this is the case we'll cross over to the BelowMinimum state
				SetStaminaStatus(EStaminaStatus::ESS_BelowMinimum); // Change stamina bar color
				Stamina -= DeltaStamina;
			}
			// However, if above isn't the case we're good to be in the normal state
			else 
			{
				Stamina -= DeltaStamina; // Don't need to change the state, we're fine to just decrement stamina
			}
			// If the shift key is being held down, we want to be in sprinting, so we want to set that status
			if (bMovingForward || bMovingRight)
			{
				SetMovementStatus(EMovementStatus::EMS_Sprinting);
			}
			else
			{
				SetMovementStatus(EMovementStatus::EMS_Normal);
			}
		}
		else // Shift key up
		{
			// If we're in the Normal state and shift key is not down
			// At this point our stamina should be replenishing
			// But, we don't want it to regen past the maximum
			if (Stamina + DeltaStamina >= MaxStamina)
			{
				Stamina = MaxStamina;
			}
			else
			{
				Stamina += DeltaStamina;
			}
			// If in normal state and shift key up, we shouldn't be sprinting
			SetMovementStatus(EMovementStatus::EMS_Normal);
		}
		break;

	case EStaminaStatus::ESS_BelowMinimum:
		if (bShiftKeyDown)
		{
			if (Stamina - DeltaStamina <= 0.f)
			{
				// Set status to Exhausted
				SetStaminaStatus(EStaminaStatus::ESS_Exhausted); // Change stamina bar color
				Stamina = 0;
				SetMovementStatus(EMovementStatus::EMS_Normal);
				// If you're exhausted you can't sprint anymore and you run out of stamina
			}
			else
			{
				Stamina -= DeltaStamina;
				// If we won't get to 0 stamina, we don't need to stop running, can still decrement
				if (bMovingForward || bMovingRight)
				{
					SetMovementStatus(EMovementStatus::EMS_Sprinting);
				}
				else
				{
					SetMovementStatus(EMovementStatus::EMS_Normal);
				}
			}
		}
		else // Shift key up
		{
			if (Stamina + DeltaStamina >= MinSprintStamina)
			{
				// If recovery pushes past MinSprintStamina, set StaminaStatus back to normal again, and out of MinSprintStamina status
				SetStaminaStatus(EStaminaStatus::ESS_Normal);
				Stamina += DeltaStamina; // But also still recovering so can keep recovering
			}
			else
			{
				Stamina += DeltaStamina;
			}
			SetMovementStatus(EMovementStatus::EMS_Normal);
		}
		break;
		
	case EStaminaStatus::ESS_Exhausted:
		if (bShiftKeyDown)
		{
			Stamina = 0.f;
			// Reached the exhaustion point, and don't want to be running and draining more stamina
			// Want the Player to let go of the key so stamina can regenerate
		}
		else // Shift key up
		{
			SetStaminaStatus(EStaminaStatus::ESS_ExhaustedRecovering); // Change stamina bar color
			Stamina += DeltaStamina;
		}
		SetMovementStatus(EMovementStatus::EMS_Normal);
		break;

	case EStaminaStatus::ESS_ExhaustedRecovering:
		if (Stamina + DeltaStamina >= MinSprintStamina)
		{
			// Should not be able to sprint until recovered, and check to see if recharge rate will put us back to MinSprintStamina level
			SetStaminaStatus(EStaminaStatus::ESS_Normal);
			Stamina += DeltaStamina;
		}
		else // If recovery doesn't put us above the MinSprintStamina level, keep recovering
		{
			Stamina += DeltaStamina;
		}
		SetMovementStatus(EMovementStatus::EMS_Normal); // Not in the sprinting status
		break;

	default:
		break;
	}

	if (bInterpToEnemy && CombatTarget)
	{
		// In order to interpolate we need to know what we're interpolating and how to get there
		// Going to involve rotators because we need to smoothly rotate the character to the enemy
		// Find our LookAtRotationyaw
		FRotator LookAtYaw = GetLookAtRotationYaw(CombatTarget->GetActorLocation());
		// This is the rotation we want to be looking directly at the enemy
		// If we set our rotation to this right away the PC would just snap to that location, but we don't want that we want a smooth transition
		// That's what interpolation is for
		// At any given frame we need to find the interpolation rotation appropriate for a smooth transition
		FRotator InterpRotation = FMath::RInterpTo(GetActorRotation(), LookAtYaw, DeltaTime, InterpSpeed); 
		// This will return us a rotator that is at the correct rotation position at this frame to do a smooth transition

		// Finally we can set our actor rotation to the interp rotation
		SetActorRotation(InterpRotation);
	}

	if (CombatTarget)
	{
		CombatTargetLocation = CombatTarget->GetActorLocation();
		// Check to see if we have our controller
		if (MainPlayerController)
		{
			MainPlayerController->EnemyLocation = CombatTargetLocation; // Update the Controller with CombatTargetLocation
		}
	}

}

FRotator AMain::GetLookAtRotationYaw(FVector Target)
{
	// Take the location of our target and find what rotation we need to orient ourselves to this target
	FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), Target);
	// This will give us exactly how MUCH we need to rotate to the target, but we only need the yaw from this
	FRotator LookAtRotationYaw(0.f, LookAtRotation.Yaw, 0.f);

	return LookAtRotationYaw;
}

// Called to bind functionality to input
void AMain::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	// Setting up input functions
	check(PlayerInputComponent); // Checks to see if this is valid, and if it's halt it'll stop execution of the code here, like a break

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &AMain::Jump);
	// For Bind Action it's a bit different, we have to specify if it's on button press or release, so the IE_Pressed indicates that we want to Jump when we press the button
	// Also, just like Turn and LookUp below, we can call a function we inherit to do the actual jumping rather than building our own function
	// For a function call when the button is released it would look like this: 
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);
	// Binding this action on Jump and StopJumping on Pressed and Released respectively is a little more optimized than the BindAxis 
	// BindAxis is sending a value every frame and calling the function every frame, but BindAction is only calling these functions on button press or release

	PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &AMain::ShiftKeyDown);
	PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &AMain::ShiftKeyUp);

	PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &AMain::ShiftKeyDown);
	PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &AMain::ShiftKeyUp);

	PlayerInputComponent->BindAction("ESC", IE_Pressed, this, &AMain::ESCDown);
	PlayerInputComponent->BindAction("ESC", IE_Pressed, this, &AMain::ESCUp);

	// To pause the whole game and not just open the pause menu would also need to do this:
	// PlayerInputComponent->BindAction("ESC", IE_Pressed, this, &AMain::ESCDown).bExecuteWhenPaused = true;
	// PlayerInputComponent->BindAction("ESC", IE_Pressed, this, &AMain::ESCUp).bExecuteWhenPaused = true;

	PlayerInputComponent->BindAxis("MoveForward", this, &AMain::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AMain::MoveRight);

	// PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput); 
	// // For mouse movement we don't have to define our own function, can just call the above function that we already inherit to do the work for us
	// PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);

	// However, to keep our code all organized and ensuring that we're checking to see if the pause menu is up, we need to make our own functions
	// These functions, below, simply call the above two Pawn functions, but before that do a check to see if we're paused
	// If we're not, then we can continue to move around
	PlayerInputComponent->BindAxis("Turn", this, &AMain::Turn); 
	PlayerInputComponent->BindAxis("LookUp", this, &AMain::LookUp);

	PlayerInputComponent->BindAxis("TurnRate", this, &AMain::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AMain::LookUpAtRate);

}

bool AMain::CanMove(float Value)
{
	if (MainPlayerController)
	{
		return (Value != 0.0f) && 
			(!bAttacking) && 
			(MovementStatus != EMovementStatus::EMS_Dead) &&
			(!MainPlayerController->bPauseMenuVisible);
	}
	return false;
}

void AMain::Turn(float Value)
{
	if (CanMove(Value))
	{
		AddControllerYawInput(Value);
	}
}

void AMain::LookUp(float Value)
{
if (CanMove(Value))
	{
		AddControllerPitchInput(Value);
	}
}

void AMain::MoveForward(float Value)
{
	bMovingForward = false;
	// Controller is what we're going to use for our forward movement
	if (CanMove(Value)) // This is what we're going to bind to our move forward input
	{
		// Find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation(); // Returns a rotator that gives you a direction the controller is facing this frame
		// Now that we've got our controller rotation we can get the yaw of that rotation 
		const FRotator YawRotation(0.f, Rotation.Yaw, 0.f); // Pitch and roll are zero'd and we only have the yaw from the controller

		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X); // Get the forward vector based off our YawRotation Rotator
		// Creates an FRotationMatrix that we can then use to get an axis from RotationMatrix
		// So what above does is creates a RotationMatrix from YawRotation which has UnitAxes that we can axis, and one of those that we can access is the X axis
		// So this way we can get a rotation and get a forward vector from that rotation
		// This will be the direction we'll be looking at any point in time in our game
		AddMovementInput(Direction, Value); // This should result in adding movement input for our character in the direction that we just calculated

		bMovingForward = true;
	}
}

void AMain::MoveRight(float Value)
{
	bMovingRight = false;
	if (CanMove(Value))
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);

		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// Whereas the EAsix::X is the forward Vector, the Y is the right/left vector
		AddMovementInput(Direction, Value);

		bMovingRight = true;
	}
}

void AMain::TurnAtRate(float Rate)
{
	// Going to add input to our controller
	// Take input from key presses, left and right keys, and add input to the controller rotation
	// Below function will take input and rotate the controller in the yaw axis specifically and it will take in a float for the value to rotate it (called every frame)
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
	// GetWorld returns a UWorldObject
}

void AMain::LookUpAtRate(float Rate)
{
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void AMain::LMBDown()
{
	bLMBDown = true;
	// Also going to check to see if we're overlapping

	if (MovementStatus == EMovementStatus::EMS_Dead) return; // If the character is dead, return out of this function
	// Early end, just cancel out of the function and don't run any of the rest of the code in this function

	// Check to see if the menu is open so we can't attack
	if (MainPlayerController) if (MainPlayerController->bPauseMenuVisible) return;
	UE_LOG(LogTemp, Warning, TEXT("MainPlayerController valid"));

	if (ActiveOverlappingItem) // Check if we have an equipped weapon, if not then allows left click will only pick up and not attack
	{
		UE_LOG(LogTemp, Warning, TEXT("ActiveOverlappingItem valid"));
		// Cast this to a weapon
		AWeapon* Weapon = Cast<AWeapon>(ActiveOverlappingItem);
		// If that cast is successfull..
		if (Weapon)
		{
			// Go to the weapon and call equip
			Weapon->Equip(this);
			// This should allow us to attach the weapon to our hand
			SetActiveOverlappingItem(nullptr);
		}
	}
	else if (EquippedWeapon) // If we have a weapon and we're not overlapping another weapon, then attack
	{
		Attack();
	}

}
void AMain::LMBUp()
{
	bLMBDown = false;
}

void AMain::ESCDown()
{
	bESCDown = true;

	// Turn on visibility for our pause menu, need to see if we have a MainPlayerController
	if (MainPlayerController)
	{
		MainPlayerController->TogglePauseMenu();
	}

	// Below is a way to do it so that it pauses the state of the world, above will just load up the pause menu
	// if (MainPlayerController->bPauseMenuVisible)
	// {
	// 	MainPlayerController->SetPause(true);
	// }
	// else
	// {
	// 	MainPlayerController->SetPause(false);
	// }
	// See Axis Binds for what else we would have to do
}

void AMain::ESCUp()
{
	bESCDown = false;
}

void AMain::DecrementHealth(float Amount)
{
	// Check to see if character should die
	if (Health - Amount <= 0.f)
	{
		Health -= Amount;
		Die();
	}
	else
	{
		Health -= Amount;
	}
}

void AMain::IncrementCoins(int32 Amount)
{
	Coins += Amount;
}

void AMain::IncrementHealth(float Amount)
{
	if (Health + Amount >= MaxHealth)
	{
		Health = MaxHealth;
	}
	else 
	{
		Health += Amount;
	}
}

void AMain::Die()
{
	if (MovementStatus == EMovementStatus::EMS_Dead) return;
	// When the health reaches 0, game over
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance(); // grabbing the AnimInstance we set
	if (AnimInstance && CombatMontage)
	{
		AnimInstance->Montage_Play(CombatMontage, 1.0f);
		AnimInstance->Montage_JumpToSection(FName("Death"));
	}
	SetMovementStatus(EMovementStatus::EMS_Dead);
}

void AMain::Jump()
{
	if (MainPlayerController) if (MainPlayerController->bPauseMenuVisible) return;

	if (MovementStatus != EMovementStatus::EMS_Dead)
	{
		Super::Jump(); 
		// Forbid us from jumping if we're dead
	}
}

void AMain::DeathEnd()
{
	GetMesh()->bPauseAnims = true;
	GetMesh()->bNoSkeletonUpdate = true;
}

void AMain::SetMovementStatus(EMovementStatus Status)
{
	MovementStatus = Status;
	if (MovementStatus == EMovementStatus::EMS_Sprinting)
	{
		GetCharacterMovement()->MaxWalkSpeed = SprintingSpeed;
	}
	else
	{
		GetCharacterMovement()->MaxWalkSpeed = RunningSpeed;
	}
}

void AMain::ShiftKeyDown()
{
	bShiftKeyDown = true;
}

void AMain::ShiftKeyUp()
{
	bShiftKeyDown = false;
}

void AMain::ShowPickupLocations()
{
	// Debug Sphere
	// Debug objects, like Spheres are used to show things in the game that you don't have meshes for
	// Meant to show locations in the world that you otherwise don't have any visual representation of
	// What location corresponds to particular events, etc

	// Loop through our TArray and print out the locations of all the coins we pick up
	for (int32 i = 0; i < PickupLocations.Num(); i++)
	{
		UKismetSystemLibrary::DrawDebugSphere(this, PickupLocations[i], 25.f, 8, FLinearColor::Green, 10.f, .5f);
	}

	// range for loop
	// for (FVector Location : PickupLocations)
	// {
	// 	UKismetSystemLibrary::DrawDebugSphere(this, PickupLocations[i], 25.f, 8, FLinearColor::Green, 10.f, .5f);
	// }

}

void AMain::SetEquippedWeapon(AWeapon* WeaponToSet)
{
	// This is a way to not crash Unreal
	// Basically, when we pick up a weapon this will destroy the weapon 
	if (EquippedWeapon)
	{
		EquippedWeapon->Destroy();
	}
	EquippedWeapon = WeaponToSet;
}

void AMain::Attack()
{
	if (!bAttacking && MovementStatus != EMovementStatus::EMS_Dead)
	{
		bAttacking = true;
		SetInterpToEnemy(true);

		// Playing our animation montage
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		if (AnimInstance && CombatMontage)
		{
			int32 Section = FMath::RandRange(0, 1); // Random number between 0 and 1
			switch (Section)
			{
			case 0:
				AnimInstance->Montage_Play(CombatMontage, 2.f);
				AnimInstance->Montage_JumpToSection(FName("Attack_1"), CombatMontage);
				break;

			case 1:
				AnimInstance->Montage_Play(CombatMontage, 1.8f);
				AnimInstance->Montage_JumpToSection(FName("Attack_2"), CombatMontage);
				break;
			
			default:
				;
			}

		}
	}
}

void AMain::AttackEnd()
{
	bAttacking = false;
	SetInterpToEnemy(false);
	if (bLMBDown)
	{
		Attack();
	}
}

void AMain::PlaySwingSound()
{
	if (EquippedWeapon->SwingSound)
	{
		// If our weapon has a sound
		UGameplayStatics::PlaySound2D(this, EquippedWeapon->SwingSound);
	}
}

void AMain::SetInterpToEnemy(bool Interp)
{
	bInterpToEnemy = Interp;
}

float AMain::TakeDamage(float DamageAmount, struct FDamageEvent const & DamageEvent, class AController * EventInstigator, AActor * DamageCauser)
{
	if (Health - DamageAmount <= 0.f)
	{
		Health -= DamageAmount;
		Die();
		if (DamageCauser)
		{
			AEnemy* Enemy = Cast<AEnemy>(DamageCauser);
			if (Enemy)
			{
				Enemy->bHasValidTarget = false;
			}
		}
	}
	else
	{
		Health -= DamageAmount;
	}

	return DamageAmount;
}

void AMain::UpdateCombatTarget()
{
	// When our character kills an enemy we don't update our combat target to another enemy if we're facing multiple enemies
	// It just stays on the enemy we killed
	// So this function will be for updating the combat target
	TArray<AActor*> OverlappingActors; // This TArray will be what we pass in to GetOverlappingActors
	// It essentially just gives us an array of all overlapping actors, so we can see who we can target next

	GetOverlappingActors(OverlappingActors, EnemyFilter);

	// Iterate over the array we create and select the closest one to be the combat target
	// But don't want to do that if the array is empty obviously
	if (OverlappingActors.Num() == 0)
	{
		// After we kill an enemy, if there's no other enemies overlapping, we remove the enemy health bar widget from the viewport
		if(MainPlayerController)
		{
			MainPlayerController->RemoveEnemyHealthBar();
		}
		return;
	}

	// If there's at least one element in the array, we want to get the closest target
	AEnemy* ClosestEnemy = Cast<AEnemy>(OverlappingActors[0]);
	if(ClosestEnemy)
	{
		FVector Location = GetActorLocation(); // Rather than constantly declare GetActorLocation, like we were in MinDistance, just make a variable for it that we can use
		float MinDistance = (ClosestEnemy->GetActorLocation() - Location).Size(); // Gives us a float in distance from our PC to the enemy
		// The parenthesis gives us a vector, then Size() turns it to a float, so we'll have a float in distance
		// But this is just the first character in the array so we need to loop through the array and see if there are any more who are closer

		// range based for loop since we don't care about the index in this case
		for (auto Actor : OverlappingActors)
		{
			// This for loop will give us basically whatever Actor OverlappingActors contains, looping through the whole array
			// Each iteration through the loop, the "auto Actor" is going to be the Actor in OverlappingActors at that particular index
			AEnemy* Enemy = Cast<AEnemy>(Actor); // Cast Actor to Enemy so we know that it's an Enemy
			if (Enemy)
			{
				float DistanceToActor = (Enemy->GetActorLocation() - Location).Size();
				// See if this Distance is smaller than MinDistance
				if (DistanceToActor < MinDistance)
				{
					MinDistance = DistanceToActor;
					ClosestEnemy = Enemy;
					// Update the closest enemy and the MinDistance as well
				}
			}
		}
		if (MainPlayerController)
		{
			MainPlayerController->DisplayEnemyHealthBar(); // Display the closest enemy health bar
		}
		SetCombatTarget(ClosestEnemy);
		bHasCombatTarget = true;
	}
}

void AMain::SwitchLevel(FName LevelName)
{
	// First check to make sure the level name isn't what we're already at 
	// To do that, need to use a special function from World called GetMapName()
	UWorld* World = GetWorld();
	if (World)
	{
		// Call GetMapName() and set it to our CurrentLevel's name variable
		FString CurrentLevel = World->GetMapName();

		FName CurrentLevelName(*CurrentLevel); // Need to turn an FString into an FName, can do that with this
		// This will convert the CurrentLevel FString into the FName
		// But need the * at the front of CurrentLevel to dereference it, which allows it to be converted to an FName, otherwise it won't work correctly
		// * will make it a string literal (Only way to get the string literal from an FString is using the dereference operator, the *)
		if (CurrentLevelName != LevelName)
		{
			// If the level's name we're on is NOT the same as the level we want to transition to, we can run this code
			// To transition to a different level:
			UGameplayStatics::OpenLevel(World, LevelName);
			// Takes a world, so we'll pass in the world we're in, and an FName for a level's name, which will be what we want to change the level to
			// So we give it the World variable we created above, and the LevelName we want to go to
			// Want to call this function as soon as we overlap with our LevelTransitionVolume
		}
	}
}

void AMain::SaveGame()
{
	// To save the game, we need to create an instance of our SaveGame object, and UGameplayStatics has a function for that
	UFirstSaveGame* SaveGameInstance = Cast<UFirstSaveGame>(UGameplayStatics::CreateSaveGameObject(UFirstSaveGame::StaticClass()));
	// CreateSaveGameObject takes a TSubclassOf, but we haven't made one
	// That's where UFirstSaveGame::StaticClass() comes in, this will return a value that's compatible with TSubclassOf
	// It will return a UClass of type UFirstSaveGame
	// An example is our enemy class. If you needed a TSubclassOf for the Enemy we would want a variable that we would set in blueprints, because the enemy...
	// ... has a bunch of stuff we set in blueprints, so we wouldn't want to call StaticClass because none of those things would be set, they'd be in default values...
	// ... unless we hard coded them, so that's not a good situation
	// But in this case, for UFirstSaveGame, we can use StaticClass
	// And since CreateSaveGameObject creates a USaveGame object, we need to cast it to UFirstSaveGame
	
	// In short, what we're doing is we're passing in what's returned from StaticClass, which is a UClass, and that's being passed in for our CreateSaveGameObject function...
	// And that creates an object of type USaveGame, we're casting it to UFirstSaveGame, and then we're finally storing that in SaveGameInstance
	// So now we have an instance of the Save Game Object we created
	// Once we have one of those we can start setting its variables
	SaveGameInstance->CharacterStats.Health = Health; // Here we're storing the main characters current health in the struct we made
	SaveGameInstance->CharacterStats.MaxHealth = MaxHealth;
	SaveGameInstance->CharacterStats.Stamina = Stamina;
	SaveGameInstance->CharacterStats.MaxStamina = MaxStamina;
	SaveGameInstance->CharacterStats.Coins = Coins;

	// Save the level name
	FString MapName = GetWorld()->GetMapName();
	MapName.RemoveFromStart(GetWorld()->StreamingLevelsPrefix);
	// This will strip away the prefix prepended to every UE map name and just leave us the actual map name
	// This way we can actually save the map name and load it without any issues
	SaveGameInstance->CharacterStats.LevelName = MapName;

	// Save weapons (But first check to make sure we have an equipped weapon)
	if (EquippedWeapon)
	{
		SaveGameInstance->CharacterStats.WeaponName = EquippedWeapon->Name;
	}

	SaveGameInstance->CharacterStats.Location = GetActorLocation();
	SaveGameInstance->CharacterStats.Rotation = GetActorRotation();

	// Now that we have the struct populated with relevant data from the time we save our game, we can call another UGameplayStatics function to save it
	UGameplayStatics::SaveGameToSlot(SaveGameInstance, SaveGameInstance->PlayerName, SaveGameInstance->UserIndex);
	// Once we call this our SaveGameInstance will have all of its data saved to your computers memory

}

void AMain::LoadGame(bool bSetPosition)
{
	// This will work, at least at the start, similar to SaveGame()
	// First we'll want to create an instance of the UFirstSaveGame class
	UFirstSaveGame* LoadGameInstance = Cast<UFirstSaveGame>(UGameplayStatics::CreateSaveGameObject(UFirstSaveGame::StaticClass()));
	// as you can see, this is exactly the same steps we need for saving so far
	// But now we need to actually load our saved data, which is another GameplayStatics function

	LoadGameInstance = Cast<UFirstSaveGame>(UGameplayStatics::LoadGameFromSlot(LoadGameInstance->PlayerName, LoadGameInstance->UserIndex));
	// In order to access the CharacterStats struct, we need to cast this to a UFirstSaveGame 
	// This will take what's returned from LoadGameFromSlot, which is a USaveGame object, and cast it to a UFirstSaveGame and saving it back into LoadGameInstance
	// Can overwrite it in this manner because the compiler will read it from right to left, so it'll use the data that's already saved, and then overwrite it

	// So now that we have a LoadGameInstance that is filled with the data from when we last saved it, we can access that data
	// What we're gonna do with that data is set those variables in main to what we saved, so basically the opposite of what we did in SaveGameInstance
	Health = LoadGameInstance->CharacterStats.Health;
	MaxHealth = LoadGameInstance->CharacterStats.MaxHealth;
	Stamina = LoadGameInstance->CharacterStats.Stamina;
	MaxStamina = LoadGameInstance->CharacterStats.MaxStamina;
	Coins = LoadGameInstance->CharacterStats.Coins;

	// Before we can load our weapon, we need to do a few things
	// We need to create an instance of our WeaponStorage and use that Actor
	// First check to see if WeaponStorage is valid, to ensure we don't crash
	if (WeaponStorage)
	{
		AItemStorage* Weapons = GetWorld()->SpawnActor<AItemStorage>(WeaponStorage);
		// This way we have what's returned from SpawnActor stored in Weapons
		if (Weapons)
		{
			// In Weapons we have access to that Map we created that are linked to actual weapon BP's, so we can use their names to spawn those weapons
			// This will load all the particle systems, sounds, and everything linked to those blueprints
			// So we're gonna access that map
			FString WeaponName = LoadGameInstance->CharacterStats.WeaponName;

			// Make sure the weapon name isn't empty
			if (WeaponName != TEXT(""))
			{
				if (Weapons->WeaponMap.Contains(WeaponName)) // This check is to make sure we don't try to look for something that isn't there, and crash the engine
				{
					AWeapon* WeaponToEquip = GetWorld()->SpawnActor<AWeapon>(Weapons->WeaponMap[WeaponName]);
					WeaponToEquip->Equip(this);
				}
				// So with this we can spawn our weapon
				// Now with the weapon spawned, we need to equip it
				// Weapon class has the functionality to equip it to our character!
			}			
		}
	}

	// We can't set actor location and rotation the same way we saved it, however, because any time we change to a new level the game essentially launches itself again
	// BeginPlay will run again, so we essentially have to load all the stats we saved. In doing so, we'll attempt to load the actors location and rotation
	// So in moving to a new level, we may end up spawning the player in some random location not at all relevant to where they're supposed to spawn, which would be bad!
	// So instead we create a boolean to check if we're changing levels or not
	if (bSetPosition)
	{
		SetActorLocation(LoadGameInstance->CharacterStats.Location);
		SetActorRotation(LoadGameInstance->CharacterStats.Rotation);
	}

	// Need to set our movement status to what it was when we saved, or at least resetting it, so that if we load when we die, our character won't be stuck in the dead state
	SetMovementStatus(EMovementStatus::EMS_Normal); // Should no longer be stuck in the dead status
	GetMesh()->bPauseAnims = false;
	GetMesh()->bNoSkeletonUpdate = false;

	if (LoadGameInstance->CharacterStats.LevelName != TEXT(""))
	{
		// If the game hasn't been saved, and the name of the world hasn't been loaded, we don't want to load anything
		// However, if it's not then we can continue and switch the level
		FName LevelName(*LoadGameInstance->CharacterStats.LevelName);
		// SwitchLevel() takes an FString not an FName, so we have to convert it using this method, AND REMEMBER THE DEREFERENCING!
		SwitchLevel(LevelName);
	}
}

void AMain::LoadGameNoSwitch() // Ideal for loading the level as we switch to a new level
{
	// This will do all the same stuff as LoadGame, but NOT set the position
	UFirstSaveGame* LoadGameInstance = Cast<UFirstSaveGame>(UGameplayStatics::CreateSaveGameObject(UFirstSaveGame::StaticClass()));

	LoadGameInstance = Cast<UFirstSaveGame>(UGameplayStatics::LoadGameFromSlot(LoadGameInstance->PlayerName, LoadGameInstance->UserIndex));

	Health = LoadGameInstance->CharacterStats.Health;
	MaxHealth = LoadGameInstance->CharacterStats.MaxHealth;
	Stamina = LoadGameInstance->CharacterStats.Stamina;
	MaxStamina = LoadGameInstance->CharacterStats.MaxStamina;
	Coins = LoadGameInstance->CharacterStats.Coins;

	if (WeaponStorage)
	{
		AItemStorage* Weapons = GetWorld()->SpawnActor<AItemStorage>(WeaponStorage);
		if (Weapons)
		{
			FString WeaponName = LoadGameInstance->CharacterStats.WeaponName;

			if (WeaponName != TEXT(""))
			{
				if (Weapons->WeaponMap.Contains(WeaponName))
				{
					AWeapon* WeaponToEquip = GetWorld()->SpawnActor<AWeapon>(Weapons->WeaponMap[WeaponName]);
					WeaponToEquip->Equip(this);
				}
			}
		}
	}
	SetMovementStatus(EMovementStatus::EMS_Normal); 
	GetMesh()->bPauseAnims = false;
	GetMesh()->bNoSkeletonUpdate = false;
}

// Another quick way to pause the entire game:
// Would actually need to create this function and use it in MainPlayerController, just putting it here for reference sake
// void AMainPlayerController::ShowPauseMenu_Implementation()
// {
//     if (PauseMenu)
//     {
//     	bPauseMenuVisible = true;
//     	PauseMenu->SetVisibility(ESlateVisibility::Visible);
//     	SetInputMode(FInputModeGameAndUI());
//     	bShowMouseCursor = true;
//     	UGameplayStatics::SetGamePaused(GetWorld(), true);
//     }
// }
// void AMainPlayerController::HidePauseMenu_Implementation()
// {
//     bPauseMenuVisible = false;
//     PauseMenu->SetVisibility(ESlateVisibility::Hidden);
//     SetInputMode(FInputModeGameOnly());
//     bShowMouseCursor = false;
//     UGameplayStatics::SetGamePaused(GetWorld(), false);
// }

// Since I use SetGamePaused(), this blocks all action and axis mappings. In order to allow...
// some mappings to be executed when the game is paused, we can set the bool variable...
// bExecuteWhenPaused in BindAction for example, to true. You may want to do this if...
// you would like to press your pause menu button again to unpause.

// FInputActionBinding& PauseButton = PlayerInputComponent->BindAction("PauseGame", IE_Pressed, this, &AMainCharacter::PauseGameButtonDown);
	
// PauseButton.bExecuteWhenPaused = true;

// In blueprints, we can find it in the details panel of our mapped input(the bExecuteWhenPaused bool)