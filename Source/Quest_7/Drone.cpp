#include "Drone.h"

#include "EnhancedInputComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "PC_PlayerController.h"


ADrone::ADrone()
{
	PrimaryActorTick.bCanEverTick = true;

	CapsuleComp = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleComp"));
	SetRootComponent(CapsuleComp);

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(CapsuleComp);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshAsset(TEXT(
		"/Game/Island/Meshes/Interiors/SM_interiors_island_Spaceship_1.SM_interiors_island_Spaceship_1"));


	if (MeshAsset.Succeeded())
	{
		Mesh->SetStaticMesh(MeshAsset.Object);
	}
	Mesh->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
	Mesh->SetSimulatePhysics(false);

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(CapsuleComp);
	SpringArm->TargetArmLength = 500.0f;

	SpringArm->SocketOffset = FVector(0.0f, 0.0f, 50.0f);
	SpringArm->SetRelativeRotation(FRotator(-15.0f, 0.0f, 0.0f));
	SpringArm->bUsePawnControlRotation = false;
	SpringArm->bEnableCameraLag = true;
	SpringArm->CameraLagSpeed = 4.0f;
	SpringArm->CameraLagMaxDistance = 300.0f;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
	Camera->bUsePawnControlRotation = false;
	Camera->SetRelativeRotation(FRotator(FRotator::ZeroRotator));
}

void ADrone::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (APC_PlayerController* PlayerController = Cast<APC_PlayerController>(GetController()))
		{
			if (PlayerController->IA_Move)
			{
				EnhancedInput->BindAction(
					PlayerController->IA_Move,
					ETriggerEvent::Triggered,
					this,
					&ADrone::Move
				);
			}
			if (PlayerController->IA_Look != nullptr)
			{
				EnhancedInput->BindAction(
					PlayerController->IA_Look,
					ETriggerEvent::Triggered,
					this,
					&ADrone::Look
				);
			}
			if (PlayerController->IA_Fly != nullptr)
			{
				EnhancedInput->BindAction(
					PlayerController->IA_Fly,
					ETriggerEvent::Triggered,
					this,
					&ADrone::StartFly
				);
				EnhancedInput->BindAction(
					PlayerController->IA_Fly,
					ETriggerEvent::Completed,
					this,
					&ADrone::StopFly
				);
			}
			if (PlayerController->IA_Boost != nullptr)
			{
				EnhancedInput->BindAction(
					PlayerController->IA_Boost,
					ETriggerEvent::Triggered,
					this,
					&ADrone::StartBoost
				);
				EnhancedInput->BindAction(
					PlayerController->IA_Boost,
					ETriggerEvent::Completed,
					this,
					&ADrone::StopBoost
				);
			}
			if (PlayerController->IA_Hover != nullptr)
			{
				EnhancedInput->BindAction(
					PlayerController->IA_Hover,
					ETriggerEvent::Started,
					this,
					&ADrone::Hover
				);
			}
		}
	}
}

void ADrone::Move(const FInputActionValue& Value)
{
	if (!Controller)
	{
		return;
	}

	FVector2D RawInput = Value.Get<FVector2D>();
	float DeltaTime = GetWorld()->GetDeltaSeconds();

	// 지상에 있을 경우 속도 50프로 제한
	float TargetSpeed = bIsOnGround ? (CurrentSpeed * 0.5f) : CurrentSpeed;

	if (!RawInput.IsNearlyZero())
	{
		// 호버링이 꺼졌을 경우 가속 수치 제한 
		MoveInput = RawInput;
		float CurrentAcceleration = bIsFalling ? (Acceleration * 0.2f) : Acceleration;
		Velocity = FMath::FInterpTo(Velocity, TargetSpeed, DeltaTime, CurrentAcceleration / 100.0f);
	}
	else
	{
		Velocity = FMath::FInterpTo(Velocity, 0.0f, DeltaTime, Deceleration / 100.0f);
	}

	if (Velocity > 0.1f)
	{
		FVector DeltaLocation = FVector(MoveInput.X, MoveInput.Y, 0.0f) * Velocity * DeltaTime;
		AddActorLocalOffset(DeltaLocation, true);
	}
}

void ADrone::StartFly(const FInputActionValue& Value)
{
	if (!Controller)
	{
		return;
	}

	const float FlyDirection = Value.Get<float>();
	float DeltaTime = GetWorld()->GetDeltaSeconds();

	if (!FMath::IsNearlyZero(FlyDirection))
	{
		if (FlyDirection > 0.0f)
		{
			bIsOnGround = false;

			if (bIsFalling)
			{
				VerticalVelocity += 3000.0f * DeltaTime;
				if (VerticalVelocity > 0.0f)
				{
					bIsFalling = false;
				}
			}
		}
		FVector DeltaLocation = FVector(0.0f, 0.0f, FlyDirection) * CurrentSpeed * DeltaTime;
		AddActorLocalOffset(DeltaLocation, true);
	}
}

void ADrone::StopFly(const FInputActionValue& Value)
{
	if (!Controller)
	{
	}
}

void ADrone::Look(const FInputActionValue& Value)
{
	if (!Controller)
	{
		return;
	}

	const FVector2D LookInput = Value.Get<FVector2D>();
	float DeltaTime = GetWorld()->GetDeltaSeconds();

	if (!LookInput.IsNearlyZero())
	{
		float RotationSpeed = 100.0f;

		FRotator DeltaYaw = FRotator(0.0f, LookInput.X * RotationSpeed * DeltaTime, 0.0f);
		AddActorLocalRotation(DeltaYaw);

		FRotator NewSpringArmRotation = SpringArm->GetRelativeRotation();
		NewSpringArmRotation.Pitch = FMath::Clamp(
			NewSpringArmRotation.Pitch + (LookInput.Y * RotationSpeed * DeltaTime), -40.0f, 0.0f);
		SpringArm->SetRelativeRotation(NewSpringArmRotation);
	}
}

void ADrone::StartBoost(const FInputActionValue& Value)
{
	CurrentSpeed = BoostSpeed;
	MaxTiltAngle = 20.0f;
}

void ADrone::StopBoost(const FInputActionValue& value)
{
	CurrentSpeed = BaseSpeed;
	MaxTiltAngle = 10.0f;
}

void ADrone::Hover(const FInputActionValue& Value)
{
	if (!Controller)
	{
		return;
	}

	bIsFalling = !bIsFalling;

	if (!bIsFalling)
	{
		VerticalVelocity = 0.0f;
		Velocity = 0.0f;
	}
}

void ADrone::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 중력 영향
	if (bIsFalling)
	{
		VerticalVelocity += Gravity * DeltaTime;
		FVector GravityVector = FVector(0.0f, 0.0f, VerticalVelocity * DeltaTime);

		FHitResult HitResult;
		AddActorLocalOffset(GravityVector, true, &HitResult);

		// 지면 감지
		if (HitResult.IsValidBlockingHit())
		{
			VerticalVelocity = 0.0f;
			bIsFalling = false;
			bIsOnGround = true;
		}
	}

	// 기울기
	float SpeedRatio = Velocity / CurrentSpeed;
	float TargetRoll = MoveInput.X * MaxTiltAngle * SpeedRatio;
	float TargetPitch = MoveInput.Y * MaxTiltAngle * SpeedRatio;
	FRotator CurrentMeshRot = Mesh->GetRelativeRotation();
	FRotator TargetMeshRot = FRotator(TargetPitch, -90.0f, TargetRoll);

	FRotator NewMeshRot = FMath::RInterpTo(CurrentMeshRot, TargetMeshRot, DeltaTime, TiltInterpSpeed);
	Mesh->SetRelativeRotation(NewMeshRot);

	MoveInput = FVector2D::ZeroVector;
}
