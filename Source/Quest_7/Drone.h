#pragma once

#include "CoreMinimal.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Pawn.h"
#include "Drone.generated.h"

class UCapsuleComponent;
class UCameraComponent;
class USpringArmComponent;
class UStaticMeshComponent;

struct FInputActionValue;

UCLASS()
class QUEST_7_API ADrone : public APawn
{
	GENERATED_BODY()

public:
	ADrone();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UCapsuleComponent* CapsuleComp;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	UStaticMeshComponent* Mesh;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Camera")
	UCameraComponent* Camera;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Camera")
	USpringArmComponent* SpringArm;

	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float BaseSpeed = 1000.0f;;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float BoostSpeed = 2000.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float CurrentSpeed = BaseSpeed;

protected:
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION()
	void Move(const FInputActionValue& Value);
	UFUNCTION()
	void StartFly(const FInputActionValue& Value);
	UFUNCTION()
	void StopFly(const FInputActionValue& Value);
	UFUNCTION()
	void Look(const FInputActionValue& Value);
	UFUNCTION()
	void StartBoost(const FInputActionValue& Value);
	UFUNCTION()
	void StopBoost(const FInputActionValue& Value);
	UFUNCTION()
	void Hover(const FInputActionValue& Value);

private:
	float Velocity = 0.0f;
	float Acceleration = 500.0f;
	float Deceleration = 800.0f;
	float MaxTiltAngle = 10.0f;
	FVector2D CurrentMoveDirection;
	const float TiltInterpSpeed = 5.0f;
	FVector2D MoveInput;

	float VerticalVelocity = 0.0f;
	const float Gravity = -980.0f;
	bool bIsFalling = false;
	bool bIsOnGround = true;
};
