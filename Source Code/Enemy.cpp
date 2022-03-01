// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy.h"
#include "Components/SphereComponent.h"
#include "AIController.h"
#include "Main.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Components/BoxComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Main.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Sound/SoundCue.h"
#include "Animation/AnimInstance.h"
#include "TimerManager.h"
#include "Components/CapsuleComponent.h"
#include "MainPlayerController.h"

// Sets default values
AEnemy::AEnemy()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	AggroSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AggroSphere"));
	AggroSphere->SetupAttachment(GetRootComponent()); // All character classes have a capsule component as their root, you cannot make another component equal to the root
	// Can only set things attached TO the root which is the capsule
	AggroSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Ignore); 
	// Aggro Sphere will ignore any object that's set to WorldDynamic, which at this point is mainly our explosive objects in the world
	AggroSphere->InitSphereRadius(600.f);

	CombatSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CombatSphere"));
	CombatSphere->SetupAttachment(GetRootComponent());
	CombatSphere->InitSphereRadius(85.f);
	// We'll need an OverlapBegin and End function for these so we can know when a player enters and leaves the sphere

	CombatCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("CombatCollision"));
	// Instead of attaching this to the root component, we're gonna want to attach it to a specific location on the mesh itself
	// namely its claws, so we put a socket on that part of the body
	CombatCollision->SetupAttachment(GetMesh(), FName("EnemySocket"));
	// With the above we're effectively doing the same thing as we were doing in AttachToComponent, in that we're attaching to the mesh
	// Ouside of the constructor can use AttachToComponent, but inside the constructor better to use SetupAttachment

	// CombatCollision->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, FName("EnemySocket"));
	// Above AttachToComponent will cause some errors, but won't crash, better to use the SetupAttachment version

	bOverlappingCombatSphere = false;

	// Initial enemy stats
	Health = 75.f;
	MaxHealth = 100.f;
	Damage = 10.f;

	AttackMinTime = 0.5f;
	AttackMaxTime = 3.5f;

	EnemyMovementStatus = EEnemyMovementStatus::EMS_Idle; // Set this to ensure our MovementStatus is correctly set from the start

	DeathDelay = 3.f; // 3 seconds

	bHasValidTarget = false;
}

// Called when the game starts or when spawned
void AEnemy::BeginPlay()
{
	Super::BeginPlay();

	AIController = Cast<AAIController>(GetController()); // GetController will return an AController, and AIController is derived from Controller so this is very specific
	// Right on BeginPlay we cast GetController which returns an AController and cast it to an AIController and store it in AIController so we have a reference to our...
	// ... AIController

	// Without below, when we enter the Aggro or Combat Sphere's, nothing will actually happen
	// Need to bind an Overlap Event to our Overlap Components
	AggroSphere->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::AggroSphereOnOverlapBegin);
	AggroSphere->OnComponentEndOverlap.AddDynamic(this, &AEnemy::AggroSphereOnOverlapEnd);

	CombatSphere->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::CombatSphereOnOverlapBegin);
	CombatSphere->OnComponentEndOverlap.AddDynamic(this, &AEnemy::CombatSphereOnOverlapEnd);

	// Also need to bind the CombatCollision like we do for the CombatSphere above, otherwise the collision won't work
	CombatCollision->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::CombatOnOverlapBegin);
	CombatCollision->OnComponentEndOverlap.AddDynamic(this, &AEnemy::CombatOnOverlapEnd);

	// Setting the collision parameters, same for Weapon.cpp
	CombatCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision); // By default no collision will happen so we can wait for ActiveCollision to be called
    CombatCollision->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic); // Has some automatic overlap parameters set for it
    CombatCollision->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore); // By default set it to be ignored
    CombatCollision->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap); // Our enemy is a pawn

	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore); // Collision with the camera won't happen
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore); // Same thing as above
	
}

// Called every frame
void AEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AEnemy::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void AEnemy::AggroSphereOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
	// When the player enters this sphere, the enemy will move to the player
	if (OtherActor && Alive()) 
	{
		// If an actor enters this sphere, cast that to the Main
		AMain* Main = Cast<AMain>(OtherActor);
		if (Main)
		{
			// If that cast was successful, move to the target
			MoveToTarget(Main);
		}
	}
}

void AEnemy::AggroSphereOnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor)
	{
		AMain* Main = Cast<AMain>(OtherActor);
		if (Main)
		{
			bHasValidTarget = false;
			if (Main->CombatTarget == this)
			{
				// Check to see if the combat target is this particular enemy
				// If there's multiple enemies we don't want to remove all targets, just the current one
				// So we put this check here
				Main->SetCombatTarget(nullptr); // When we're not close to the enemy we won't be interpolating 
			}
			Main->SetHasCombatTarget(false);
			Main->UpdateCombatTarget();

			// With UpdateCombatTarget() we don't need below to remove the enemies health bar anymore
			// if (Main->MainPlayerController)
			// {
			// 	// If that's valid
			// 	Main->MainPlayerController->RemoveEnemyHealthBar(); // Hide the enemy health bar when you leave the aggro sphere
			// }

			SetEnemyMovementStatus(EEnemyMovementStatus::EMS_Idle);
			if (AIController)
			{
				AIController->StopMovement(); // Will stop the movement if we leave the Aggro sphere
			}
		}
	}
}

void AEnemy::CombatSphereOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
	if (OtherActor && Alive())
	{
		AMain* Main = Cast<AMain>(OtherActor);
		{
			if (Main)
			{
				bHasValidTarget = true;

				Main->SetCombatTarget(this); // Set the PC's combat target to this instance
				Main->SetHasCombatTarget(true);

				// Display enemy health bar
				Main->UpdateCombatTarget(); // Instead of calling DisplayHealthBar we call this since it calls that anyway

				CombatTarget = Main;
				bOverlappingCombatSphere = true;
				// Attack(); // Instead of calling attack we're going to put the AttackEnd timer here to ensure the player doesn't get spammed by enemy attacks
				float AttackTime = FMath::FRandRange(AttackMinTime, AttackMaxTime);
				GetWorldTimerManager().SetTimer(AttackTimer, this, &AEnemy::Attack, AttackTime);
			}
		}
	}
}

void AEnemy::CombatSphereOnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor && OtherComp)
	{
		AMain* Main = Cast<AMain>(OtherActor);
		if (Main)
		{
			bOverlappingCombatSphere = false;
			MoveToTarget(Main);
			CombatTarget = nullptr;

			if (Main->CombatTarget == this)
			{
				// Only if THIS enemy is the PC's target should these be called
				// Otherwise this will call with every enemy, and we don't want that we just want to update the individual combat target, not all enemies
				Main->SetCombatTarget(nullptr); // Set CombatTarget to null for the main character when the combat sphere stops overlapping
				Main->bHasCombatTarget = false;
				Main->UpdateCombatTarget(); // As soon as overlap ends we call this to reupdate the combat target
			}

			if(Main->MainPlayerController)
			{
				USkeletalMeshComponent* MainMesh = Cast<USkeletalMeshComponent>(OtherComp);
				if(MainMesh) Main->MainPlayerController->RemoveEnemyHealthBar();
			}
		
			GetWorldTimerManager().ClearTimer(AttackTimer); // When the PC leaves the combat sphere, this will reset the timer to ensure it doesn't resume where it left off
			// Or keep counting
		}
	}
}

void AEnemy::MoveToTarget(class AMain* Target)
{
	// When we call this, we want to set our MovementStatus to "MoveToTarget"
	SetEnemyMovementStatus(EEnemyMovementStatus::EMS_MoveToTarget);

	// Now we actually need to move to the target
	if (AIController)
	{
		// If it's valid and we have a reference to our controller, we can actually give it some functionality
		FAIMoveRequest MoveRequest; // A struct that we can set specific properties to get the AI to actually move
		MoveRequest.SetGoalActor(Target);
		MoveRequest.SetAcceptanceRadius(10.0f); // Hard coding for now, but can change later

		// Now that we have that, we need the other parameter, which is the path
		FNavPathSharedPtr NavPath;

		// Now we have the two things we need to pass in to get our AI to actually move
		// Now we get our AIController
		AIController->MoveTo(MoveRequest, &NavPath); //&NavPath is passing in the address to a NavPath which has the same type as a pointer to NavPath

		/** MARKING NAVPOINTS TO SEE AI PATHING
		

		// TArray<FNavPathPoint> PathPoints = NavPath->GetPathPoints(); // Will return a TArray of FNavPath Point
		// We want to make a TArray of FNavPathPoint and initialize it with these GetPathPoints
		// Above, with the TArray declaration, is one way to do it, but below is the easier one
		auto PathPoints = NavPath->GetPathPoints();
		// the "auto" keyword will automatically set the type for you
		
		// Now we're going to draw debug spheres everywhere there's a path point to see where they are
		for (auto Point : PathPoints) // This basically says: for every Point in PathPoints
		{
			// What this will do is give us a Point which is an element in the array called PathPoints
			FVector Location = Point.Location; // For each Point in PathPoints we can access that location
			// Then finally we can create a debug sphere for every point
			UKismetSystemLibrary::DrawDebugSphere(this, Location, 25.f, 8, FLinearColor::Red, 10.f, 1.5f);
		}


		*/
	}
}

void AEnemy::CombatOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
    if (OtherActor)
    {
        // Once we have the other actor we need to cast it to an enemy
        AMain* Main = Cast<AMain>(OtherActor);
        if (Main)
        {
            // Attempt to play the HitParticles
            // So we need to check to make sure the HitParticles are working
            if (Main->HitParticles)
            {
                // reference to the socket we created on the weapon
                const USkeletalMeshSocket* TipSocket = GetMesh()->GetSocketByName("TipSocket");
                if (TipSocket)
                {
                    // Spawn the emitter at the location of our socket
                    FVector SocketLocation = TipSocket->GetSocketLocation(GetMesh());
                    UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), Main->HitParticles, SocketLocation, FRotator(0.f), false);
                    // This will spawn the particle system at our swords blade instead of the entire sword
                }
            }
            if (Main->HitSound)
            {
                UGameplayStatics::PlaySound2D(this, Main->HitSound); // As long as the enemy has a hit sound on the blueprint, this will work
            }
			if (DamageTypeClass)
			{
				UGameplayStatics::ApplyDamage(Main, Damage, AIController, this, DamageTypeClass);
			}
        }
    }
}	

void AEnemy::CombatOnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{

}

void AEnemy::ActivateCollision()
{
    CombatCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly); // Turn collision on for attacks
}

void AEnemy::DeactivateCollision()
{
    CombatCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision); // turn it off when attack is done
    // Both Activate and Deactivate need to be created so we're not constantly colliding our weapons hitbox with the enemies
    // This way, only when we are attacking will collision need to be enabled
	if (SwingSound)
	{
		UGameplayStatics::PlaySound2D(this, SwingSound);
		// We'll have to set these on the blueprint, but we have the functionality working
	}
}

void AEnemy::Attack()
{
	// If we're not alive, don't attack
	if (Alive() && bHasValidTarget)
	{
		// Check to see if we should stop movement
		if (AIController)
		{
			AIController->StopMovement(); // Stop moving, focus on attacking
			SetEnemyMovementStatus(EEnemyMovementStatus::EMS_Attacking);
		}
		if (!bAttacking) // If not already attacking, start attack anim montage
		{
			bAttacking = true;
			// Play our combat montage animation
			UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance(); // Should give us our anim instance, but still need to check
			if (AnimInstance)
			{
				// Use montage play
				AnimInstance->Montage_Play(CombatMontage, 1.35f);
				AnimInstance->Montage_JumpToSection(FName("Attack"), CombatMontage); // Will play the Attack section from Montage
			}
		}
	}
}

void AEnemy::AttackEnd()
{
	bAttacking = false;
	if (bOverlappingCombatSphere)
	{
		// If PC is still inside of the CombatSphere, keep attacking
		float AttackTime = FMath::FRandRange(AttackMinTime, AttackMaxTime); // Set a timer based on an AttackTime to wait between attacks
		GetWorldTimerManager().SetTimer(AttackTimer, this, &AEnemy::Attack, AttackTime); // This will make it attack, wait, then attack again
		// Since it's already managed to turn OverlappingCombatSphere on and off in above functions we don't have to worry about it here
		// This will handle if the monster will keep attacking or if it will stop attacking
		// If we walk away and leave the sphere, this check will fail and the monster will not attack, the monsters will just continue to run towards the player
	}
}

float AEnemy::TakeDamage(float DamageAmount, struct FDamageEvent const & DamageEvent, class AController * EventInstigator, AActor * DamageCauser) 
{
	if (Health - DamageAmount <= 0.f)
	{
		Health -= DamageAmount;
		Die(DamageCauser);
	}
	else {
		Health -= DamageAmount;
	}

	return DamageAmount;
}

void AEnemy::Die(AActor* Causer)
{
	SetEnemyMovementStatus(EEnemyMovementStatus::EMS_Death);
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance)
	{
		AnimInstance->Montage_Play(CombatMontage, 1.0f);
		AnimInstance->Montage_JumpToSection(FName("Death"), CombatMontage);
	}
	
	// Remove all collision volumes
	CombatCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	AggroSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CombatSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	bAttacking = false;

	// Check to see if the Causer is the PC
	AMain* Main = Cast<AMain>(Causer);
	if (Main)
	{
		Main->UpdateCombatTarget();
	}
}

void AEnemy::DeathEnd()
{
	// When enemy health reaches 0, forbid it from just getting back up, make sure it stays dead
	GetMesh()->bPauseAnims = true;
	GetMesh()->bNoSkeletonUpdate = true;

	// As soon as the enemy is killed, set the timer for the body to be despawned
	GetWorldTimerManager().SetTimer(DeathTimer, this, &AEnemy::Despawn, DeathDelay);
}

bool AEnemy::Alive()
{
	return GetEnemyMovementStatus() != EEnemyMovementStatus::EMS_Death;
}

void AEnemy::Despawn()
{
	Destroy();
}