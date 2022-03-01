// Fill out your copyright notice in the Description page of Project Settings.


#include "MainPlayerController.h"
#include "Blueprint/UserWidget.h"

void AMainPlayerController::BeginPlay()
{
    Super::BeginPlay(); // Whenever overriding BeginPlay this should be the first thing we do

    if (HUDOverlayAsset)
    {
        // If we selected a HUD Overlay asset
        HUDOverlay = CreateWidget<UUserWidget>(this, HUDOverlayAsset); // Similar to CreateDefaultSubobject but for Widgets
    }

    HUDOverlay->AddToViewport(); // Will add the HUD to the Viewport
    HUDOverlay->SetVisibility(ESlateVisibility::Visible); // Exists because in code we can call this and set it to invisible
    // So we can set the HUD to visible or invisible in code if we want

    // Check enemy health bar is valid and set in the blueprints
    if (WEnemyHealthBar)
    {
        EnemyHealthBar = CreateWidget<UUserWidget>(this, WEnemyHealthBar);
        if (EnemyHealthBar)
        {
            EnemyHealthBar->AddToViewport();

            // Set it to hidden
            EnemyHealthBar->SetVisibility(ESlateVisibility::Hidden); // On the viewport but not being shown
        }
        // One last thing we need to call to set the alignment of the widget (How it's gonna be aligned, we want it to be flat and facing the screen)
        FVector2D Alignment(0.f, 0.f);
        EnemyHealthBar->SetAlignmentInViewport(Alignment);
        // All should work fine, just need our Enemylocation to be updated, but that updating can happen in the Main
    }

    if (WPauseMenu)
    {
        PauseMenu = CreateWidget<UUserWidget>(this, WPauseMenu);
        if (PauseMenu)
        {
            PauseMenu->AddToViewport();

            // Set it to hidden
            PauseMenu->SetVisibility(ESlateVisibility::Hidden); // On the viewport but not being shown
        }
    }
}

void AMainPlayerController::DisplayEnemyHealthBar()
{
    if (EnemyHealthBar)
    {
        bEnemyHealthBarVisible = true;
        EnemyHealthBar->SetVisibility(ESlateVisibility::Visible);
    }
}

void AMainPlayerController::RemoveEnemyHealthBar()
{
    if (EnemyHealthBar)
    {
        bEnemyHealthBarVisible = false;
        EnemyHealthBar->SetVisibility(ESlateVisibility::Hidden);
    }
}

void AMainPlayerController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (EnemyHealthBar)
    {
        // Need to get our enemys location in 3d space and convert it a location on a 2d screen
        FVector2D PositionInViewport; // a vector for the position in the screen

        // Call a function that will allow us to take a world location and project it to the screen
        ProjectWorldLocationToScreen(EnemyLocation, PositionInViewport);
        PositionInViewport.Y -= 85.f;

        FVector2D SizeInViewport = FVector2D(150.f, 25.f); // for the size of our widget in the viewport (200x, 25y)
        // Changing above values can make it bigger, for the x, or smaller

        EnemyHealthBar->SetPositionInViewport(PositionInViewport); // Set the location in viewport
        EnemyHealthBar->SetDesiredSizeInViewport(SizeInViewport); // Set the size in viewport
        // This should effectively set the size and location to the FVector EnemyLocation
        // All we need is to get the EnemyLocation and use that to project a 2d position on the viewport
    }
}

void AMainPlayerController::DisplayPauseMenu_Implementation()
{
    // Displaying the pause menu when we press the ESC key

    if (PauseMenu)
    {
        bPauseMenuVisible = true;
        PauseMenu->SetVisibility(ESlateVisibility::Visible);

        // Display the mouse cursor
        // To setup our ability to use our mouse we need to define an FInputMode that we'll use, since this is what allows us to turn on our mose
        // There are 3 main types of FInputModes:
        // FInputModeUIOnly - Allows you to move your mouse around but not participate in game functionality
        // FInputModeGameAndUI - Allows you to move your mouse around AND participate in game functionality
        // FInputModeGameOnly - The default mode, doesn't allow you to interact with the hud with your mouse (?)

        // FInputModeUIOnly InputModeUIOnly; 
        // UIOnly also disables the use of our keyboard, so we can't use any of the keys to quit the pause menu, so this won't work

        // FInputModeGameOnly InputModeGameOnly;
        // This one SORTA works like what we imagine, only problem is we have to click on the menu to stop the camera from moving with the mouse still
        // Also still doesn't stop the camera from moving

        FInputModeGameAndUI InputModeGameAndUI;
        // This will do essentially what we want, we have access to the mouse and to the keyboard, but the problem is we can still move around the character with this one
        // Also, we won't have access to the mouse moving around the camera when we close the menu either.

        SetInputMode(InputModeGameAndUI); // This will do what we essentially want, but won't show mouse cursor off the bat so we have to set bShowMouseCursor to true
        bShowMouseCursor = true;
    }
}

void AMainPlayerController::RemovePauseMenu_Implementation()
{
    // Remove PauseMenu from the screen when we press ESC key
    if (PauseMenu)
    {
        GameModeOnly();
        
        bShowMouseCursor = false;
        // Doing it like this returns our full functionality to the game as we want it to (Though we still can move around in the pause menu)

        bPauseMenuVisible = false;
        // PauseMenu->SetVisibility(ESlateVisibility::Hidden);
        // Taking this out to set it in the blueprint, as without it when we close the pause menu we don't get the animation we made
    }
}

void AMainPlayerController::TogglePauseMenu()
{
    if (bPauseMenuVisible)
    {
        RemovePauseMenu(); // If we want to actually call the version that lets blueprints have some behavior, we DON'T call the version with _Implementation
    }
    else
    {
        DisplayPauseMenu();
    }
}

void AMainPlayerController::GameModeOnly()
{
    FInputModeGameOnly InputModeGameOnly;

    SetInputMode(InputModeGameOnly);
}