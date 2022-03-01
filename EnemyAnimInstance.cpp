// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyAnimInstance.h"
#include "Enemy.h"

void UEnemyAnimInstance::NativeInitializeAnimation()
{
    if (Pawn == nullptr)
    {
        Pawn = TryGetPawnOwner(); // Try to get the pawn that owns this class
        // If that's valid, set it equal to Pawn
        if (Pawn)
        {
            Enemy = Cast<AEnemy>(Pawn); // A reference to the enemy and a pawn initialization
        }
    }
}

void UEnemyAnimInstance::UpdateAnimationProperties()
{
    // CHeck to see if the above values are valid and if they're not, try to set them here as well
    if (Pawn == nullptr)
    {
        Pawn = TryGetPawnOwner(); // Try to get the pawn that owns this class
        // If that's valid, set it equal to Pawn
        if (Pawn)
        { 
            Enemy = Cast<AEnemy>(Pawn); // A reference to the enemy and a pawn initialization
        }
    } 

    if (Pawn)
    {
        // Update our speed before we Cast to our Enemy
        FVector Speed = Pawn->GetVelocity(); // Get from the Pawn how fast it's going at that time/frame
        FVector LateralSpeed = FVector(Speed.X, Speed.Y, 0.f);
        MovementSpeed = LateralSpeed.Size(); // Will give us the magnitude of that vector as a float

        // This way our speed is set and ready to be accessed in our AnimBlueprint

        // Need to move our speed down to here so that way movement speed will actually update, because otherwise the check would fail every time
    }

    // This way we'll be sure that we'll have our Pawn and our Enemy as well
}