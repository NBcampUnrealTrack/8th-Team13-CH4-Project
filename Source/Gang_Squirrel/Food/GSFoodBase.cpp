// Fill out your copyright notice in the Description page of Project Settings.


#include "GSFoodBase.h"

#include "NaniteSceneProxy.h"
#include "SNegativeActionButton.h"
#include "Gang_Squirrel/DataAsset/GSFoodPrimaryDataAsset.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Net/UnrealNetwork.h"
#include "Gang_Squirrel/Food/GSFoodWidgetComponent.h"
#include "Gang_Squirrel/Character/GSCharacter.h"
#include "Gang_Squirrel/Food/GSFoodWidget.h"
#include "Misc/OutputDeviceNull.h"

// Sets default values
AGSFoodBase::AGSFoodBase()
{
	bReplicates = true;
	SetReplicateMovement(true);
    
	// 🎯 1. 아무 스케일 값도 없는 순수한 SceneComponent를 최상위 루트로 삼습니다!
	USceneComponent* DummyRoot = CreateDefaultSubobject<USceneComponent>("DefaultSceneRoot");
	RootComponent = DummyRoot;
    
	// 🎯 2. 크루아상 메쉬와 위젯, 콜리전은 전부 이 순수한 루트의 '동등한 자식'으로 붙입니다.
	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>("StaticMeshComponent");
	StaticMeshComponent->SetupAttachment(RootComponent); 
    
	FoodWidgetComponent = CreateDefaultSubobject<UGSFoodWidgetComponent>("FoodWidgetComponent");
	FoodWidgetComponent->SetupAttachment(RootComponent); // 👈 이제 메쉬 스케일 5배 영향을 안 받습니다!
    
	SphereComponent = CreateDefaultSubobject<USphereComponent>("OverlapCollision");
	SphereComponent->SetupAttachment(RootComponent);
    
	SphereComponent->OnComponentBeginOverlap.AddDynamic(this, &AGSFoodBase::OnBeginOverlap);
	SphereComponent->OnComponentEndOverlap.AddDynamic(this, &AGSFoodBase::OnEndOverlap);
    
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;
	
	

}

void AGSFoodBase::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(AGSFoodBase, bIsActive);
	DOREPLIFETIME(AGSFoodBase, FoodData);
}

// Called when the game starts or when spawned
void AGSFoodBase::BeginPlay()
{
	Super::BeginPlay();
	
	UE_LOG(LogTemp, Warning, TEXT("[Before] WidgetClass = %s"), *GetNameSafe(FoodWidgetComponent->GetWidgetClass()));
    
	if (FoodWidgetComponent)
	{
		// 엔진 버그로 위젯 클래스가 None이라면, 경로를 통해 강제로 직접 로드합니다.
		if (FoodWidgetComponent->GetWidgetClass() == nullptr)
		{
			// ⚠️ [주의] 콘텐츠 브라우저에서 WBP_FoodWidget 우클릭 -> '레퍼런스 복사' 한 뒤, 
			// 아래 경로를 지우고 붙여넣으세요. 그리고 맨 끝에 꼭 '_C'를 붙여야 합니다!
			// 예시: "/Game/UI/WBP_FoodWidget.WBP_FoodWidget_C"
			FString WidgetPath = TEXT("/Game/YJW/Food/FoodUI/WBP_FoodWidget.WBP_FoodWidget_C"); 
            
			UClass* LoadedClass = StaticLoadClass(UUserWidget::StaticClass(), nullptr, *WidgetPath);
			if (LoadedClass)
			{
				FoodWidgetComponent->SetWidgetClass(LoadedClass);
			}
		}
		
		FoodWidgetComponent->InitWidget();
		
		if (SphereComponent)
		{
			SphereComponent->SetRelativeScale3D(FVector(5.3f, 5.3f, 5.3f));
			SphereComponent->InitSphereRadius(180.f);
		}
	}
	
	bIsFilling = false;
	
	if (HasAuthority()) Activate();
}

// Called every frame
void AGSFoodBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if (bIsFilling && FoodWidgetComponent)
	{
		CurrentEatenTime += DeltaTime;
		
		float FinalTime = FoodData->EatTime * 4;
		
		float Alpha = FMath::Clamp(CurrentEatenTime / FinalTime, 0.f, 1.f );
		
		UUserWidget* UserWidget = FoodWidgetComponent->GetUserWidgetObject();
		if (UserWidget)
		{
			FOutputDeviceNull ar;
			FString Cmd = FString::Printf(TEXT("UpdateProgress %f"), Alpha);
			UserWidget->CallFunctionByNameWithArguments(*Cmd, ar, nullptr, true);
		}
		
		if (Alpha >= 0.188f)
		{
			bIsFilling = false;
			SetActorTickEnabled(false);
			
			bIsFilling = false;
			
			APlayerController* PC = GetWorld()->GetFirstPlayerController();
			if (PC)
			{
				AGSCharacter* LocalChar = Cast<AGSCharacter>(PC->GetPawn());
				if (LocalChar)
				{
					LocalChar->Server_NotifyFoodEaten(this);
				}
			}
			
			UE_LOG(LogTemp, Warning, TEXT("Food Eat Progress Complete!!"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Init GSFoodBase failed %d"), bIsFilling);
	}
}

void AGSFoodBase::Init(UGSFoodPrimaryDataAsset* InData)
{
	UE_LOG(LogTemp, Warning, TEXT("Init GSFoodBase"));
	
	if (!IsValid(InData))
	{
		UE_LOG(LogTemp, Error, TEXT("Init GSFoodBase failed"));
		return;
	}
	FoodData = InData;
	
	if (!IsValid(StaticMeshComponent)) return;
	
	OnRep_FoodData();
}

void AGSFoodBase::OnRep_Activate()
{
	this->SetActorHiddenInGame(!bIsActive);
	this->SetActorEnableCollision(bIsActive);
}

void AGSFoodBase::Activate()
{
	if (!HasAuthority()) return;
	bIsActive = true;
	OnRep_Activate();
}

void AGSFoodBase::Deactivate()
{
	if (!HasAuthority())
	{
		UE_LOG(LogTemp, Error, TEXT("GSFoodBase deactivated failed"));
		return;
	}
	UE_LOG(LogTemp, Warning, TEXT("Deactivate GSFoodBase"));
	bIsActive = false;
	OnRep_Activate();
}

void AGSFoodBase::OnRep_FoodData() const
{
	if (!IsValid(FoodData)) return;
	
	StaticMeshComponent->SetStaticMesh(FoodData->FoodMesh);
	
	if (StaticMeshComponent)
	{
		StaticMeshComponent->SetRelativeScale3D(FVector(5.f,5.f,5.f));
	}
}

int32 AGSFoodBase::Eaten()
{
	Deactivate();
	
	return FoodData->ScoreAmount;
}

void AGSFoodBase::OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AGSCharacter* Character = Cast<AGSCharacter>(OtherActor);
	if (!Character) return;
	if (Character->IsLocallyControlled())
	{
		UE_LOG(LogTemp, Warning, TEXT("Local Player Overlap Begin!"));
		
		bIsFilling = true;
		CurrentEatenTime = 0.f;
		
		SetActorTickEnabled(true);
		
		if (FoodWidgetComponent)
		{
			FoodWidgetComponent->SetVisibility(true);
			UUserWidget* UserWidget = FoodWidgetComponent->GetUserWidgetObject();
			if (UserWidget)
			{
				FOutputDeviceNull ar;
				UserWidget->CallFunctionByNameWithArguments(TEXT("UpdateProgress 0.0"), ar, nullptr, true);
			}
		}
	}
}

void AGSFoodBase::OnEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	AGSCharacter* Character = Cast<AGSCharacter>(OtherActor);
	if (!Character) return;
	
	if (Character->IsLocallyControlled())
	{
		UE_LOG(LogTemp, Warning, TEXT("Local Player Overlap End!"));
		
		bIsFilling = false;
		CurrentEatenTime = 0.f;
		
		SetActorTickEnabled(false);
		
		if (FoodWidgetComponent)
		{
			FoodWidgetComponent->SetVisibility(false);
		}
	}
}

void AGSFoodBase::SetUIVisible(bool bShow)
{
	if (FoodWidgetComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("SetUI"));
		FoodWidgetComponent->SetVisibility(bShow);
	}
	
}

