// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon.h"
#include "Components/SkeletalMeshComponent.h"
#include "Main.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Sound/SoundCue.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Components/BoxComponent.h"
#include "Enemy.h"
#include "Engine/SkeletalMeshSocket.h"

AWeapon::AWeapon()
{
    SkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMesh"));
    SkeletalMesh->SetupAttachment(GetRootComponent());

    CombatCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("Combat Collision"));
    CombatCollision->SetupAttachment(GetRootComponent());

    bWeaponParticles = false;

    WeaponState = EWeaponState::EWS_Pickup;

    Damage = 25.f; // Initial damage inflicted by our weapon
}

void AWeapon::BeginPlay()
{
    Super::BeginPlay();

    CombatCollision->OnComponentBeginOverlap.AddDynamic(this, &AWeapon::CombatOnOverlapBegin);
    CombatCollision->OnComponentEndOverlap.AddDynamic(this, &AWeapon::CombatOnOverlapEnd);   
    // AddDynamic is a helper macro to bind a UObject instance and a member UFUNCTION to a dynamic mult-cast delegate
    // With the above, we have working Begin and EndOverlap functions we can use for whenever something overlaps our weapon hitbox

    CombatCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision); // By default no collision will happen so we can wait for ActiveCollision to be called
    CombatCollision->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic); // Has some automatic overlap parameters set for it
    CombatCollision->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore); // By default set it to be ignored
    CombatCollision->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap); // Our enemy is a pawn
    // Essentially we tell it to ignore everything else, but for pawns we want it to overlap
}

void AWeapon::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
    Super::OnOverlapBegin(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

    // Equip with our OtherActor
    // On overlapping we can equip ourselves to the main character
    UE_LOG(LogTemp, Warning, TEXT("Warning Outside If Check"));
    if ((WeaponState == EWeaponState::EWS_Pickup) && OtherActor)
    {
        UE_LOG(LogTemp, Warning, TEXT("Warning Inside If Check"));
        AMain* Main = Cast<AMain>(OtherActor);
        if (Main)
        {
            // We don't want to equip right as we overlap, so we get rid of: Equip(Main);
            // Instead, we set THIS as the ActiveOverlappingItem so that we can have the player choose if they want to pick it up
            Main->SetActiveOverlappingItem(this);
        }
    }
}

void AWeapon::OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    Super::OnOverlapEnd(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex);
    if (OtherActor)
    {
        AMain* Main = Cast<AMain>(OtherActor);
        if (Main)
        {
            Main->SetActiveOverlappingItem(nullptr);
            // As soon as we stop overlapping if we haven't equipped it, we won't have any overlapping item on the character
        }
    }
}

void AWeapon::Equip(AMain* Char)
{
    // Attach the mesh to the socket of the character
    if (Char)
    {
        // Set damage type
        SetInstigator(Char->GetController());

        // If character is valid
        // Need to set CollisionResponse to ignore for the camera so the camera won't zoom in on the player if the sword gets between the camera and the player
        SkeletalMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
        // Now camera won't zoom in if sword is in the way
        // Also don't want any collision with the pawn itself
        SkeletalMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);

        // Need to stop simulating physics to attach to player
        SkeletalMesh->SetSimulatePhysics(false);

        const USkeletalMeshSocket* RightHandSocket = Char->GetMesh()->GetSocketByName("RightHandSocket"); // get the socket by name and assign it to RightHandSocket
        // If successful this RighHandSocket variable should be a reference to the RightHandSocket on our character's skeleton

        if (RightHandSocket)
        {
            // If the RightHandSocket isn't null, attach the thing calling this to the socket
            RightHandSocket->AttachActor(this, Char->GetMesh());
            // Calling Equip, will attach whatever is calling it to the RightHandSocket to the skeleton
            // Also helped to program some physics and collision settings that we need upon equipping
            
            // As soon as we attach to the actor, set bRotate to false
            bRotate = false;

            Char->SetEquippedWeapon(this); // Sets the equipped weapon to this particular weapon instance
            Char->SetActiveOverlappingItem(nullptr);
        }
        // Play sound on pickup
        if (OnEquipSound) UGameplayStatics::PlaySound2D(this, OnEquipSound);

        if (!bWeaponParticles)
        {
            IdleParticlesComponent->Deactivate();
        }
    }
}

void AWeapon::CombatOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
    if (OtherActor)
    {
        // Once we have the other actor we need to cast it to an enemy
        AEnemy* Enemy = Cast<AEnemy>(OtherActor);
        if (Enemy)
        {
            // Attempt to play the HitParticles
            // So we need to check to make sure the HitParticles are working
            if (Enemy->HitParticles)
            {
                // reference to the socket we created on the weapon
                const USkeletalMeshSocket* WeaponSocket = SkeletalMesh->GetSocketByName("WeaponSocket");
                if (WeaponSocket)
                {
                    // Spawn the emitter at the location of our socket
                    FVector SocketLocation = WeaponSocket->GetSocketLocation(SkeletalMesh);
                    UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), Enemy->HitParticles, SocketLocation, FRotator(0.f), false);
                    // This will spawn the particle system at our swords blade instead of the entire sword
                }
            }
            if (Enemy->HitSound)
            {
                UGameplayStatics::PlaySound2D(this, Enemy->HitSound); // As long as the enemy has a hit sound on the blueprint, this will work
            }
            if (DamageTypeClass)
            {
                UGameplayStatics::ApplyDamage(Enemy, Damage, WeaponInstigator, this, DamageTypeClass);
            }
        }
    }
}

void AWeapon::CombatOnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{

}

void AWeapon::ActivateCollision()
{
    CombatCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly); // Turn collision on for attacks
}

void AWeapon::DeactivateCollision()
{
    CombatCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision); // turn it off when attack is done
    // Both Activate and Deactivate need to be created so we're not constantly colliding our weapons hitbox with the enemies
    // This way, only when we are attacking will collision need to be enabled
}