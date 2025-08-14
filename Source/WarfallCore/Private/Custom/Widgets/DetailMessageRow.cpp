#include "Custom/Widgets/DetailMessageRow.h"

#include "DetailWidgetRow.h"
#include "IDetailChildrenBuilder.h"

void SDetailMessageBox::Construct(const FArguments& InArgs)
{
	DetailMessageRow = InArgs._DetailMessageRow;

	ChildSlot
	[
		SNew(SOverlay)
		.Visibility_Lambda([this]()
		{
			return Message.IsSet() ? EVisibility::Visible : EVisibility::Hidden;
		})
		+ SOverlay::Slot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Center)
		[
			SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
			.BorderBackgroundColor(this, &SDetailMessageBox::GetCurrentBoxColor)
			.Padding(12.f)
		]
		+ SOverlay::Slot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		[
			SNew(STextBlock)
			.Justification(ETextJustify::Center)
			.Text(this, &SDetailMessageBox::GetMessageText)
			.ColorAndOpacity(this, &SDetailMessageBox::GetMessageColor)
			.Font(FCoreStyle::GetDefaultFontStyle("Italic", 10))
			.ShadowOffset(FVector2D(1.f, 1.f))
			.ShadowColorAndOpacity(FLinearColor::Black)
		]
	];
}

void SDetailMessageBox::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);

	if (!Message.IsSet() || !DetailMessageRow.IsValid() || !DetailMessageRow->IsVisible())
	{
		return;
	}

	FMessageRow& CurrentMessage = *Message;

	constexpr float FadeSpeed = 2.f; // ajustable

	// FADE IN
	if (CurrentMessage.FadeAlpha < 1.f && !bIsFadingOut)
	{
		CurrentMessage.FadeAlpha = FMath::Clamp(CurrentMessage.FadeAlpha + InDeltaTime * FadeSpeed, 0.f, 1.f);
		Invalidate(EInvalidateWidget::LayoutAndVolatility);
		return;
	}

	// WAIT or COUNTDOWN
	if (!CurrentMessage.bWaitForChangement && !bIsFadingOut)
	{
		CurrentMessage.RemainingTime -= InDeltaTime;
		if (CurrentMessage.RemainingTime <= 0.f)
		{
			ClearMessage();
		}
	}

	// FADE OUT
	if (bIsFadingOut)
	{
		CurrentMessage.FadeAlpha = FMath::Clamp(CurrentMessage.FadeAlpha - InDeltaTime * FadeSpeed, 0.f, 1.f);
		if (CurrentMessage.FadeAlpha <= 0.f)
		{
			Message.Reset();
			bIsFadingOut = false;
		}
		Invalidate(EInvalidateWidget::LayoutAndVolatility);
	}
}

void SDetailMessageBox::NewMessage(const FString& InMessage, const EMessageType Type, const bool bWaitForChangement, const float RemainingTime, const int32 Priority)
{
	float CurrentAlpha = 0;
	if (Message.IsSet())
	{
		if (Message->Priority > Priority) return;
		CurrentAlpha = Message->FadeAlpha;
	}

	Message = FMessageRow{
		InMessage,
		Type,
		bWaitForChangement, 
		!bWaitForChangement ? RemainingTime : 0.f, 
		CurrentAlpha, 
		Priority};

	Invalidate(EInvalidateWidget::LayoutAndVolatility);
}

void SDetailMessageBox::ClearMessage()
{
	if (!Message.IsSet())
	{
		return;
	}

	if (Message->bWaitForChangement)
	{
		Message.Reset(); // direct
	}
	else
	{
		bIsFadingOut = true; // start to fade out
	}
	Invalidate(EInvalidateWidget::LayoutAndVolatility);
}

TSharedRef<FDetailMessageRow> FDetailMessageRow::Create(IDetailChildrenBuilder& ChildBuilder, const EVisibility Visibility)
{
	TSharedRef<FDetailMessageRow> Instance = MakeShareable(new FDetailMessageRow());
	Instance->Init(ChildBuilder, Visibility, Instance);
	return Instance;
}

void FDetailMessageRow::Init(IDetailChildrenBuilder& ChildBuilder, const EVisibility Visibility, const TSharedRef<FDetailMessageRow>& Self)
{
	RowVisibility = Visibility;
	ChildBuilder.AddCustomRow(FText::FromString("DetailMessageRow"))
	.Visibility(Visibility)
	.WholeRowContent()
	[
		SAssignNew(MessageBox, SDetailMessageBox)
		.DetailMessageRow(Self)
	];
}

void FDetailMessageRow::NewMessage(const FString& Message, const EMessageType Type, const bool bWaitForChangement, const float RemainingTime, const int32 Priority) const
{
	MessageBox->NewMessage(Message, Type, bWaitForChangement, RemainingTime, Priority);
}

void FDetailMessageRow::ClearMessage() const
{
	MessageBox->ClearMessage();
}

bool FDetailMessageRow::IsWaitingForChangement(const int32 Priority) const
{
	return MessageBox->IsWaitingForChangement(Priority);
}

