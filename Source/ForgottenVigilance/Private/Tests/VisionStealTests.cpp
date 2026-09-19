// VisionStealTests.cpp
#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "VisionStealComponent.h"
#include "OptimusPrimeCharacter.h"
#include "AIBaseCharacter.h"
#include "AIBossCharacter.h"
#include "AIController.h"
#include "HealthComponent.h"
#include "MeleeAttackComponent.h"
#include "Camera/CameraActor.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "GameFramework/WorldSettings.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/Composites/BTComposite_Sequence.h"
#include "BehaviorTree/Tasks/BTTask_Wait.h"
#include "BrainComponent.h"
#include "InputMappingContext.h"
#include "OptimusPrimePlayerController.h"
#include "InputActionValue.h"
#include "Components/CapsuleComponent.h"
#include "Components/BoxComponent.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FVisionStealInputAssetTest, "ForgottenVigilance.Recon.InputAssets",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FVisionStealInputAssetTest::RunTest(const FString& Parameters)
{
	UClass* ControllerClass = LoadClass<AOptimusPrimePlayerController>(nullptr,
		TEXT("/Game/Blueprints/BP_OptimusPrimePlayerController.BP_OptimusPrimePlayerController_C"));
	if (!TestNotNull(TEXT("Player controller BP loads"), ControllerClass)) return false;
	const auto* Defaults = ControllerClass->GetDefaultObject<AOptimusPrimePlayerController>();
	const UInputMappingContext* Mapping = Defaults->InputMappingContext;
	if (!TestNotNull(TEXT("Configured mapping context exists"), Mapping)) return false;
	for (const FEnhancedActionKeyMapping& Binding : Mapping->GetMappings())
	{
		TestTrue(TEXT("Q is not reserved by the existing mapping context"), Binding.Key != EKeys::Q);
	}
	UClass* PlayerClass = LoadClass<AOptimusPrimeCharacter>(nullptr,
		TEXT("/Game/Blueprints/BP_OptimusPrimeCharacter.BP_OptimusPrimeCharacter_C"));
	if (!TestNotNull(TEXT("Player BP loads"), PlayerClass)) return false;
	TestNotNull(TEXT("Existing player BP inherits recon component without asset edits"),
		PlayerClass->GetDefaultObject<AOptimusPrimeCharacter>()->FindComponentByClass<UVisionStealComponent>());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FVisionStealLifecycleTest, "ForgottenVigilance.Recon.Lifecycle",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FVisionStealLifecycleTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	if (!TestNotNull(TEXT("Test world"), World)) return false;
	GEngine->CreateNewWorldContext(EWorldType::Game).SetCurrentWorld(World);
	World->SetGameInstance(NewObject<UGameInstance>(GEngine));
	FURL TestURL;
	TestURL.AddOption(TEXT("game=/Script/Engine.GameModeBase"));
	World->SetGameMode(TestURL);
	World->CreateAISystem();
	World->InitializeActorsForPlay(TestURL);
	World->BeginPlay();
	FActorSpawnParameters Spawn;
	Spawn.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AOptimusPrimeCharacter* Player = World->SpawnActor<AOptimusPrimeCharacter>(FVector::ZeroVector, FRotator::ZeroRotator, Spawn);
	APlayerController* PC = World->SpawnActor<APlayerController>();
	AAIBaseCharacter* Enemy = World->SpawnActor<AAIBaseCharacter>(FVector(500, 0, 0), FRotator(0, 170, 0), Spawn);
	AAIController* AI = World->SpawnActor<AAIController>();
	PC->Possess(Player);
	AI->Possess(Enemy);
	Player->DispatchBeginPlay();
	Enemy->DispatchBeginPlay();
	PC->SetViewTarget(Player);
	Player->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	Enemy->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	UVisionStealComponent* Recon = Player->FindComponentByClass<UVisionStealComponent>();
	UHealthComponent* PlayerHealth = Player->FindComponentByClass<UHealthComponent>();
	UMeleeAttackComponent* Melee = NewObject<UMeleeAttackComponent>(Enemy);
	Melee->RegisterComponent();

	// 실행 중인 행동 트리를 중단/복구할 수 있는지 실제 AI 컴포넌트로 검증합니다.
	UBehaviorTree* Tree = NewObject<UBehaviorTree>(World);
	Tree->RootNode = NewObject<UBTComposite_Sequence>(Tree);
	FBTCompositeChild& Child = Tree->RootNode->Children.AddDefaulted_GetRef();
	Child.ChildTask = NewObject<UBTTask_Wait>(Tree);
	AI->RunBehaviorTree(Tree);
	UBrainComponent* Brain = AI->GetBrainComponent();
	TestTrue(TEXT("Behavior tree initially running"), Brain && Brain->IsRunning());

	TestFalse(TEXT("Empty target is rejected"), Recon->StartVisionSteal(nullptr, PC));
	AAIBossCharacter* Boss = World->SpawnActor<AAIBossCharacter>(FVector(1500, 0, 0), FRotator::ZeroRotator, Spawn);
	TestTrue(TEXT("Boss can be scouted"), Recon->StartVisionSteal(Boss, PC));
	TestTrue(TEXT("Boss suppressed"), Boss->IsReconSuppressed());
	TestEqual(TEXT("Boss cannot start attack during recon"), Boss->ExecuteAttackPattern(Player), 0.0f);
	Recon->EndVisionSteal();
	TestTrue(TEXT("Living enemy accepted"), Recon->StartVisionSteal(Enemy, PC));
	TestTrue(TEXT("Player remains possessed"), PC->GetPawn() == Player);
	TestEqual(TEXT("Enemy remains AI controlled"), Enemy->GetController(), static_cast<AController*>(AI));
	TestEqual(TEXT("View switched to recon camera"), PC->GetViewTarget(), static_cast<AActor*>(Recon->ReconCamera));
	TestTrue(TEXT("Enemy suppressed"), Enemy->IsReconSuppressed());
	TestTrue(TEXT("AI brain paused"), Brain && Brain->IsPaused());
	TestFalse(TEXT("AI body rotation tick stopped"), AI->IsActorTickEnabled());
	TestEqual(TEXT("Player movement remains enabled"), Player->GetCharacterMovement()->MovementMode.GetValue(), MOVE_Walking);
	TestEqual(TEXT("Enemy cannot move"), Enemy->GetCharacterMovement()->MovementMode.GetValue(), MOVE_None);
	TestFalse(TEXT("Melee attacks blocked"), Melee->CanAttack());
	const float HealthBefore = PlayerHealth->GetCurrentHealth();
	Melee->PerformAttack(Player);
	TestEqual(TEXT("Suppressed attack causes no damage"), PlayerHealth->GetCurrentHealth(), HealthBefore);
	Recon->UpdateReconLook(FVector2D(1000, 1000));
	TestTrue(TEXT("Right limit survives yaw wrap around 180"), FMath::IsNearlyEqual(
		FMath::FindDeltaAngleDegrees(170.0f, Recon->ReconCamera->GetActorRotation().Yaw), 45.0f));
	const double FirstPitch = Recon->ReconCamera->GetActorRotation().Pitch;
	TestTrue(TEXT("Vertical input reaches pitch limit"), FMath::IsNearlyEqual(FMath::Abs(FirstPitch), 60.0));
	Recon->UpdateReconLook(FVector2D(-2000, -2000));
	TestTrue(TEXT("Opposite vertical input reaches opposite pitch limit"), FMath::IsNearlyEqual(Recon->ReconCamera->GetActorRotation().Pitch, -FirstPitch));
	TestTrue(TEXT("Left limit"), FMath::IsNearlyEqual(
		FMath::FindDeltaAngleDegrees(170.0f, Recon->ReconCamera->GetActorRotation().Yaw), -45.0f));
	TestEqual(TEXT("Enemy body yaw unchanged"), Enemy->GetActorRotation().Yaw, 170.0);
	Player->ConsumeMovementInputVector();
	Player->Move(FInputActionValue(FVector2D(1.0, 0.0)));
	const FVector ExpectedMove = FRotator(0.0, Recon->GetReconViewRotation().Yaw, 0.0).Vector();
	TestTrue(TEXT("W moves own body relative to recon camera yaw"), Player->GetPendingMovementInputVector().Equals(ExpectedMove, 0.001));
	// 실제 이동 컴포넌트가 입력을 소비해 내 몸을 움직이는지 확인합니다.
	Player->GetCharacterMovement()->SetMovementMode(MOVE_Flying);
	const FVector BeforeMove = Player->GetActorLocation();
	Player->GetCharacterMovement()->TickComponent(0.1f, LEVELTICK_All, nullptr);
	TestTrue(TEXT("Own body position changes during recon"), !Player->GetActorLocation().Equals(BeforeMove, 0.01));
	TestTrue(TEXT("Own body displacement follows recon view"), FVector::DotProduct(Player->GetActorLocation() - BeforeMove, ExpectedMove) > 0.0f);

	Recon->EndVisionSteal();
	Recon->EndVisionSteal();
	TestEqual(TEXT("Original view restored"), PC->GetViewTarget(), static_cast<AActor*>(Player));
	TestTrue(TEXT("AI brain resumed"), Brain && Brain->IsRunning());
	TestTrue(TEXT("AI controller tick restored"), AI->IsActorTickEnabled());
	TestEqual(TEXT("Exit preserves current movement mode"), Player->GetCharacterMovement()->MovementMode.GetValue(), MOVE_Flying);
	TestFalse(TEXT("Own body orientation setting restored"), Player->GetCharacterMovement()->bOrientRotationToMovement);
	Player->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	TestFalse(TEXT("Enemy visible again"), PC->HiddenActors.Contains(Enemy));
	TestTrue(TEXT("Attacks allowed again"), Melee->CanAttack());
	TestNull(TEXT("Temporary camera cleared"), Recon->ReconCamera.Get());

	// 다른 시스템이 이미 정지한 AI는 정찰 종료 때 임의로 재개하지 않습니다.
	Brain->PauseLogic(TEXT("Other system"));
	AI->SetActorTickEnabled(false);
	Recon->StartVisionSteal(Enemy, PC);
	Recon->EndVisionSteal();
	TestTrue(TEXT("Pre-existing brain pause preserved"), Brain->IsPaused());
	TestFalse(TEXT("Pre-existing disabled tick preserved"), AI->IsActorTickEnabled());
	Brain->ResumeLogic(TEXT("Test reset"));
	AI->SetActorTickEnabled(true);

	// 대상 사망은 실제 HealthComponent의 피해 처리 경로로 검증합니다.
	Recon->StartVisionSteal(Enemy, PC);
	UGameplayStatics::ApplyDamage(Enemy, 1000.0f, PC, Player, nullptr);
	TestFalse(TEXT("Target death ends recon"), Recon->IsReconActive());
	TestEqual(TEXT("Dead enemy movement not restored"), Enemy->GetCharacterMovement()->MovementMode.GetValue(), MOVE_None);
	TestFalse(TEXT("Dead target rejected"), Recon->StartVisionSteal(Enemy, PC));

	AAIBaseCharacter* DestroyedEnemy = World->SpawnActor<AAIBaseCharacter>(FVector(800, 0, 0), FRotator::ZeroRotator, Spawn);
	DestroyedEnemy->DispatchBeginPlay();
	Recon->StartVisionSteal(DestroyedEnemy, PC);
	DestroyedEnemy->Destroy();
	TestFalse(TEXT("Target destruction ends recon"), Recon->IsReconActive());
	TestTrue(TEXT("No hidden target references remain"), PC->HiddenActors.IsEmpty());

	AAIBaseCharacter* LastEnemy = World->SpawnActor<AAIBaseCharacter>(FVector(1000, 0, 0), FRotator::ZeroRotator, Spawn);
	LastEnemy->DispatchBeginPlay();
	Recon->StartVisionSteal(LastEnemy, PC);
	UGameplayStatics::ApplyDamage(Player, 1000.0f, AI, LastEnemy, nullptr);
	TestFalse(TEXT("Player death ends recon"), Recon->IsReconActive());
	TestEqual(TEXT("Dead player movement not restored"), Player->GetCharacterMovement()->MovementMode.GetValue(), MOVE_None);
	TestFalse(TEXT("Surviving enemy released"), LastEnemy->IsReconSuppressed());
	TestEqual(TEXT("View returns to player on death"), PC->GetViewTarget(), static_cast<AActor*>(Player));

	World->EndPlay(EEndPlayReason::Quit);
	World->DestroyWorld(false);
	GEngine->DestroyWorldContext(World);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FVisionStealTargetingTest, "ForgottenVigilance.Recon.Targeting",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FVisionStealTargetingTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	GEngine->CreateNewWorldContext(EWorldType::Game).SetCurrentWorld(World);
	World->SetGameInstance(NewObject<UGameInstance>(GEngine));
	FURL URL;
	URL.AddOption(TEXT("game=/Script/Engine.GameModeBase"));
	World->SetGameMode(URL);
	World->CreateAISystem();
	World->InitializeActorsForPlay(URL);
	World->BeginPlay();
	FActorSpawnParameters Spawn;
	Spawn.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AOptimusPrimeCharacter* Player = World->SpawnActor<AOptimusPrimeCharacter>(FVector(-500, 0, 0), FRotator::ZeroRotator, Spawn);
	UVisionStealComponent* Recon = Player->FindComponentByClass<UVisionStealComponent>();
	AAIBaseCharacter* Enemy = World->SpawnActor<AAIBaseCharacter>(FVector(500, 0, 0), FRotator::ZeroRotator, Spawn);
	// 실제 적 BP의 캡슐 설정으로 이전 Visibility 검사와 새 선택 검사를 비교합니다.
	const TCHAR* EnemyClasses[] = {
		TEXT("/Game/Blueprints/BP_AI/BP_AIBaseCharacter.BP_AIBaseCharacter_C"),
		TEXT("/Game/Blueprints/BP_AI/BP_AIMeleeCharacter.BP_AIMeleeCharacter_C"),
		TEXT("/Game/Blueprints/BP_AI/BP_AIRangeCharacter.BP_AIRangeCharacter_C"),
		TEXT("/Game/Blueprints/BP_AI/AISecialCharacter.AISecialCharacter_C"),
		TEXT("/Game/Blueprints/Boss/BP_AIBossCharacter.BP_AIBossCharacter_C")
	};
	for (const TCHAR* Path : EnemyClasses)
	{
		UClass* Class = LoadClass<AAIBaseCharacter>(nullptr, Path);
		if (!TestNotNull(FString::Printf(TEXT("Enemy BP loads: %s"), Path), Class)) continue;
		const UCapsuleComponent* Defaults = Class->GetDefaultObject<AAIBaseCharacter>()->GetCapsuleComponent();
		Enemy->GetCapsuleComponent()->SetCollisionObjectType(Defaults->GetCollisionObjectType());
		Enemy->GetCapsuleComponent()->SetCollisionResponseToChannels(Defaults->GetCollisionResponseToChannels());
		Enemy->GetCapsuleComponent()->SetCollisionEnabled(Defaults->GetCollisionEnabled());
		TestTrue(FString::Printf(TEXT("Select enemy using BP collision: %s"), Path), Recon->FindReconTarget(FVector::ZeroVector, FVector::ForwardVector) == Enemy);
	}
	// Pawn 전용 검사로 바꿔도 벽 너머/거리 밖의 대상을 고르지 않아야 합니다.
	AActor* Wall = World->SpawnActor<AActor>();
	UBoxComponent* Box = NewObject<UBoxComponent>(Wall);
	Wall->SetRootComponent(Box);
	Box->SetBoxExtent(FVector(10, 100, 100));
	Box->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Box->SetCollisionObjectType(ECC_WorldStatic);
	Box->SetCollisionResponseToAllChannels(ECR_Block);
	Box->RegisterComponent();
	Wall->SetActorLocation(FVector(250, 0, 0));
	TestNull(TEXT("Wall blocks recon selection"), Recon->FindReconTarget(FVector::ZeroVector, FVector::ForwardVector));
	Wall->Destroy();
	Enemy->SetActorLocation(FVector(4000, 0, 0));
	TestNull(TEXT("Target beyond range rejected"), Recon->FindReconTarget(FVector::ZeroVector, FVector::ForwardVector));
	World->EndPlay(EEndPlayReason::Quit);
	World->DestroyWorld(false);
	GEngine->DestroyWorldContext(World);
	return true;
}
#endif
