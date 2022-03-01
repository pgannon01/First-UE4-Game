// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Main.generated.h"

// Making our own enum (and making it registered with the garbage collector)
// First declare enum and then usually a good idea to use what's called Scoped Enums, where we put a "class" call as well
// This will prevent our Enum constants from being accessible without having to fully qualify their names with the enum class
UENUM(BlueprintType) // Like with other U macros, we can make it so we can use these in blueprints
enum class EMovementStatus : uint8 // Meant to represent if the characters sprinting or not
{
	// Why a uint8? When you're not planning on having a lot of enum constants, when their value isn't going to exceed that which could fit into a uint8 then you can use it
	// Will be smaller than an int32
	// in Unreal, enum constants will be prefixed with an abbreviation of the enum they're in
	EMS_Normal UMETA(DisplayName = "Normal"), // If using an enum and marking it as UENUM, need to mark our constants with UMETA(DisplayName)
	// The DisplayName we can specify so it can be reflected in blueprints, basically a Category
	EMS_Sprinting UMETA(DisplayName = "Sprinting"),
	EMS_Dead UMETA(DisplayName = "Dead"),

	EMS_MAX UMETA(DisplayName = "DefaultMAX") // Generally a good idea to put a MAX at the end of your enums to represent a maximum for the enum constant your working with
};

UENUM(BlueprintType)
enum class EStaminaStatus : uint8
{
	ESS_Normal UMETA(DisplayName = "Normal"),
	ESS_BelowMinimum UMETA(DisplayName = "BelowMinimum"),
	ESS_Exhausted UMETA(DisplayName = "Exhausted"),
	ESS_ExhaustedRecovering UMETA(DisplayName = "ExhaustedRecovering"),

	ESS_MAX UMETA(DisplayName = "DefaultMAX")
};

UCLASS()
class MYPROJECT_API AMain : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AMain();

	UPROPERTY(EditDefaultsOnly, Category = "SavedData")
	TSubclassOf<class AItemStorage> WeaponStorage;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	bool bHasCombatTarget; // This will be for the enemy health bar. When its true, enemy health bar will show up, when false it won't
	// Basically, if we have a target then that targets health bar will show up

	FORCEINLINE void SetHasCombatTarget(bool HasTarget) { bHasCombatTarget = HasTarget; }

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Combat")
	FVector CombatTargetLocation; // Get the location of the enemy we're targetting so we can draw their health bar above them

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Controller")
	class AMainPlayerController* MainPlayerController;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	class UParticleSystem* HitParticles;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	class USoundCue* HitSound; // Sound for when player gets hit

	// TArray is a template container, stores typically pointers to objects but we can store other things like FVectors too
	TArray<FVector> PickupLocations; // Gonna use this to store the locations of pickups as we pick them up

	UFUNCTION(BlueprintCallable)
	void ShowPickupLocations();

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Enums")
	EMovementStatus MovementStatus;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Enums")
	EStaminaStatus StaminaStatus;

	FORCEINLINE void SetStaminaStatus(EStaminaStatus Status) { StaminaStatus = Status; }

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float StaminaDrainRate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float MinSprintStamina;

	/** Below three are for attempting to make it easier for the character to actually hit the enemy, turning him towards the enemy */
	float InterpSpeed;
	bool bInterpToEnemy;
	void SetInterpToEnemy(bool Interp); // Set interp to whatever boolean we pass in
	// Not exposing it to blueprints so no macros

	// Need to have a target to interp to, so we'll add a combat target of type AEnemy
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	class AEnemy* CombatTarget;

	// SETTER FOR ABOVE
	FORCEINLINE void SetCombatTarget(AEnemy* Target) { CombatTarget = Target; }

	FRotator GetLookAtRotationYaw(FVector Target); // Location of combat target
	
	/** Set movement status and running speed */
	void SetMovementStatus(EMovementStatus Status);

	// For SetMovementStatus we're going to change the running speed between running and sprinting
	// So we'll want those as parameters
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Running")
	float RunningSpeed;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Running")
	float SprintingSpeed;

	// These functions will just set Shift keypress to false (so we can see when it's pressed to know when to sprint)
	bool bShiftKeyDown;

	/** Press down to enable sprinting */
	void ShiftKeyDown();

	/** Release to stop sprinting */
	void ShiftKeyUp();

	/** Camera boom positioning the camera behind the player */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true")) 
	class USpringArmComponent* CameraBoom; // Above will make this accessable by the BP that contains it but not outside of it in blueprints

	/** Follow Camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;

	/** Base turn rates to scale turning functions for the camera */
	// Speeds at which we will turn when we hit the left and right arrow keys
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	float BaseTurnRate;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	float BaseLookUpRate;

	/** 
	/* 
	/* Player Stats
	/*
	*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Player Stats")
	float MaxHealth;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Stats")
	float Health;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Player Stats")
	float MaxStamina;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Stats")
	float Stamina;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Stats")
	int32 Coins;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// Called for forwards/backwards input
	void MoveForward(float Value);

	// Called for side to side input
	void MoveRight(float Value);

	/** Called for Yaw Rotation */
	void Turn(float Value);

	/** Called for Pitch Rotation */
	void LookUp(float Value);

	// These bools will be to check if we're moving at all
	// If we're not, then the character shouldn't be able to sprint
	bool bMovingForward;
	bool bMovingRight;

	bool CanMove(float Value);

	/** Called via input to turn at a given rate 
	* @param Rate This is a normalized rate, ie 1.0 means 100% of desired turn rate
	*/
	void TurnAtRate(float Rate);

	/** Called via input to look up/down at a given rate 
	* @param Rate This is a normalized rate, ie 1.0 means 100% of desired look up/down rate
	*/
	void LookUpAtRate(float Rate);

	bool bLMBDown;
	void LMBDown();
	void LMBUp();

	bool bESCDown;
	void ESCDown();
	void ESCUp();

	void DecrementHealth(float Amount);

	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const & DamageEvent, class AController * EventInstigator, AActor * DamageCauser) override; 
	// We inherit this from parent components
	// This will help simplify us actually taking damage and reducing our health using our DecrementHealth function
	
	UFUNCTION(BlueprintCallable)
	void IncrementCoins(int32 Amount);
	
	UFUNCTION(BlueprintCallable)
	void IncrementHealth(float Amount);

	void Die();

	virtual void Jump() override;

	// Getter for CameraBoom and FollowCamera
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	// Check to see if we have an equipped weapon
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Items")
	class AWeapon* EquippedWeapon;
	// Set to EditDefaultOnly because we want to edit it on the default, but don't want to be able to set it on each individual instance of the character

	// Below is how we'll make it so that if the player already has a weapon, they don't have to swap it out without any choice
	// We'll make it so they can click to pickup the weapon, rather than automatically grabbing whatever they run over so they can keep any weapons they like
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Items")
	class AItem* ActiveOverlappingItem; 

	// Setter for EquippedWeapon
	void SetEquippedWeapon(AWeapon* WeaponToSet);
	FORCEINLINE AWeapon* GetEquippedWeapon() { return EquippedWeapon; }
	FORCEINLINE void SetActiveOverlappingItem(AItem* Item) { ActiveOverlappingItem = Item; }

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anims")
	bool bAttacking;

	void Attack();

	UFUNCTION(BlueprintCallable)
	void AttackEnd();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anims")
	class UAnimMontage* CombatMontage;

	UFUNCTION(BlueprintCallable)
	void PlaySwingSound();

	UFUNCTION(BlueprintCallable)
	void DeathEnd();

	void UpdateCombatTarget();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	TSubclassOf<AEnemy> EnemyFilter;

	void SwitchLevel(FName LevelName); // Switch level to the specified level

	// Put all the functionality for saving the game inside the MainCharacter since they'll have access to all their information already
	// No need to make a child that looks up to a parent, so to speak
	UFUNCTION(BlueprintCallable)
	void SaveGame();

	UFUNCTION(BlueprintCallable)
	void LoadGame(bool bSetPosition);

	void LoadGameNoSwitch(); // Meant for when we're switching levels, not actually loading the game
	
};
