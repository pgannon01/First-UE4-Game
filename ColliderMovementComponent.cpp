// Fill out your copyright notice in the Description page of Project Settings.


#include "ColliderMovementComponent.h"

void UColliderMovementComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction); // Super calls the parent version
    // So anything in TickComponent that happens in the parent version is going to want to be called
    // Need to make sure to pass in the parameters so that whenever anything happens in the parent version this will be called

    // Next we need to see if a few things are valid
    if (!PawnOwner || !UpdatedComponent || ShouldSkipUpdate(DeltaTime)) // The MovementComponent has a variable called PawnOwner and this is basically the owning pawn that owns this MovementComponent
    {
        // So in our case since we're going to own the MovementComponent from the collider, we're checking to see if that's valid with this check 
        // Then we also are checking the UpdatedComponent that we have designated in our check
        // Remember in Collider.CPP we set the UpdatedComponent to the RootComponent of the Collider so we're checking here to see if that's valid
        // Finally we're going to check using a function we inherit called ShouldSkipUpdate to check if we should skip updating and have TickComponent do nothing
        // All of these are conditions to see if we should do nothing, so we can simply return
        return;
        // Anything after this will not run and we'll exit out of this function early
    }

    // If we get past that check and something in those parameters is valid then we'll do the below
    FVector DesiredMovementThisFrame = ConsumeInputVector().GetClampedToMaxSize(1.0f); // This will represent how much movement we to move in any given frame
    // Remember: MovementComponent has something called an InputVector that will change frame to frame to tell us how to move
    // So what we're doing here is we'd like to see what that InputVector is this frame and ConsumeInputVector will return it
    // But then Consume means that it will reset it to 0. First it'll return the value of InputVector and then clear it out
    // GetClampedToMaxSize will take our given vector and it scales it down to a set size, so we're clamping it down to a magnitude of 1 and returning it
    // So now we have a vector that takes the MovementComponents InputVector, stores it and squashes it down to 1, then clearing it

    if(!DesiredMovementThisFrame.IsNearlyZero()) // If Vector is close to 0 it will return to true
    {
        FHitResult Hit; // Declaring an empy FHitResult struct and passing it, but if it hits something then Hit will get filled in by SafeMoveUpdatedComponent
        // Since, if we're in here, the vector isn't nearly zero, we can move the pawn
        SafeMoveUpdatedComponent(DesiredMovementThisFrame, UpdatedComponent->GetComponentRotation(), true, Hit);
        // This takes the updated component and moves it
        // Need to also pass in the UpdatedComponents rotation
        // Finally also need to pass in an FHitResult
        // FHitResult stores information about when things hit each other, for collision
        // So when two objects collide it will get that hit information
        // Last input parameter we can get away with not passing it, it already has a default value

        // But what happens if we bump into something? We can slide along its side
        if (Hit.IsValidBlockingHit())
        {
            SlideAlongSurface(DesiredMovementThisFrame, 1.f - Hit.Time, Hit.Normal, Hit);
            // Takes the amount of movement we would like to move
            // Takes the amount of time since we hit the surface
            // Takes a normal vector which describes the orientation of the face we hit
            // And takes the hit result
            // Then performs a caluculation that tells us where to move the pawn if we were to slide along the surface
        }
    }

}