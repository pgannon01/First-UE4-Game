// Fill out your copyright notice in the Description page of Project Settings.


#include "MainAnimInstance.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Main.h"

void UMainAnimInstance::NativeInitializeAnimation() 
{
    // First things we want to do here is check to see if the pawn is null
    if (Pawn == nullptr)
    {
        Pawn = TryGetPawnOwner(); // This will try to get the owner of this animation instance
        // Nice function that AnimInstance inherits. Checks to see what is the pawn that owns this animation instance, if it has one, and return it if it has one...
        // ... and store it in Pawn
        if (Pawn)
        {
            Main = Cast<AMain>(Pawn);
        }
    }

    // Next we want some sort of function that can get called every frame, like a Tick, but Actors have a Tick, AnimInstance doesn't
    // We want something that can update the Speed and IsInAir variables for us so that we can update what our animation is doing at any given frame

}

void UMainAnimInstance::UpdateAnimationProperties() // Call this function every frame
{
    // First thing we need to do is check the Pawn is valid
    if (Pawn == nullptr)
    {
        Pawn = TryGetPawnOwner(); // Just do the same thing we do above to make sure there's an owner
    }

    if (Pawn)
    {
        // Every frame the most important thing we wanna get is the speed and whether or not the character is in the air
        FVector Speed = Pawn->GetVelocity(); // Get from the Pawn how fast it's going at that time/frame
        FVector LateralSpeed = FVector(Speed.X, Speed.Y, 0.f);
        MovementSpeed = LateralSpeed.Size(); // Will give us the magnitude of that vector as a float

        bIsInAir = Pawn->GetMovementComponent()->IsFalling(); // Get that information from the CharacterMovementComponent

        if (Main == nullptr)
        {
            Main = Cast<AMain>(Pawn);
        }
    }
}