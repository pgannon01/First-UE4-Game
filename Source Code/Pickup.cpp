// Fill out your copyright notice in the Description page of Project Settings.


#include "Pickup.h"
#include "Main.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Sound/SoundCue.h"

APickup::APickup()
{

}

void APickup::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
    // Because this is called in the child class, if there's any inherited functionality from Item we want that to be called as well
    // So to do that we'll call Super
    Super::OnOverlapBegin(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

    if (OtherActor) // Doing the same thing as in Explosive, only we're adding coins
    {
        AMain* Main = Cast<AMain>(OtherActor);
        if (Main)
        {
            OnPickupBP(Main);
            // If the pickup we get is a coin, the BP can increment coins, if it's a potion the BP can increment health
            
            Main->PickupLocations.Add(GetActorLocation()); // Get the location of the pickup as an FVector and add it to PickupLocations

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

            Destroy();
        }
    }

}

void APickup::OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    Super::OnOverlapEnd(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex);

}