// Fill out your copyright notice in the Description page of Project Settings.


#include "Explosive.h"
#include "Main.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Sound/SoundCue.h"
#include "Enemy.h"

AExplosive::AExplosive()
{
    Damage = 15.f;   
}

void AExplosive::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
    // Because this is called in the child class, if there's any inherited functionality from Item we want that to be called as well
    // So to do that we'll call Super
    Super::OnOverlapBegin(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

    // Damage the player when the player overlaps with the Explosive
    if (OtherActor) 
    {
        // If the OtherActor is valid, we can do a quick cast to the main character
        // Casting basically converts from one type to another, and we want to cast to the main character to get access to their functions
        // When we want to access another types functions or check if, for instance, if an actor IS a character, we can do a Cast
        AMain* Main = Cast<AMain>(OtherActor);
        AEnemy* Enemy = Cast<AEnemy>(OtherActor); // Casting the OtherActor to both
        // If this OtherActor is not AMain, Main will be null
        if (Main || Enemy)
        {
            if (OverlapParticles)
            {
                UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), OverlapParticles, GetActorLocation(), FRotator(0.f), true);
                // With this we'll spawn a specific emitter that we set and then, with below Destroy() function, will remove this item
                // Basically, when we walk over it we'll pick it up/activate it, and then it'll be deleted from the game
            }

            if (OverlapSound)
            {
                // Play a sound on overlap
                UGameplayStatics::PlaySound2D(this, OverlapSound);
            }
            // If Main isn't null, we can access things inside AMain
            // Main->DecrementHealth(Damage);
            // Instead of calling DecrementHealth we can just use UE's own ApplyDamage function
            UGameplayStatics::ApplyDamage(OtherActor, Damage, nullptr, this, DamageTypeClass);
            
            Destroy();
        }
    }

}

void AExplosive::OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    Super::OnOverlapEnd(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex);

}