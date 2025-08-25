#pragma once

#include "CoreMinimal.h"

class SDetailMessageBox;

/**
 * @enum EMessageType
 * @brief Enumerates the types of messages used in the system.
 *
 * This enumeration is used to classify messages based on their severity or purpose.
 * It includes the following types:
 * - Success: Represents a successful operation or state.
 * - Error: Denotes an error condition or failure.
 * - Warning: Indicates a warning that may require attention but is not an error.
 */
enum class EMessageType
{
	Success,
	Error,
	Warning
};

/**
 * @struct FMessageRow
 * @brief Represents a data structure for storing information about a message.
 *
 * This structure encapsulates the details of a message, which can include components
 * such as its content, type, or other metadata. It is typically used to define and
 * manage individual message entries within a system or user interface.
 */
struct FMessageRow
{
	FString Message;
	EMessageType Type = EMessageType::Success;
	bool bWaitForChangement = false;
	float RemainingTime = 0.f;
	float FadeAlpha = 0.f;
	int32 Priority = 0;
};

/**
 * @class FDetailMessageRow
 * @brief Represents a message row in the detail panel with customizable visibility and messaging functionality.
 *
 * This class is designed to manage individual message rows in a detail panel. It enables the addition,
 * display, and clearing of messages, including support for visual effects like fading. The visibility
 * of the row can also be configured.
 */
class FDetailMessageRow : public TSharedFromThis<FDetailMessageRow>
{
public:
	/**
	 * @brief Creates an instance of FDetailMessageRow and initializes it with the provided parameters.
	 *
	 * This method constructs a new FDetailMessageRow, initializes it with the specified child builder
	 * and visibility, and returns a shared reference to the created instance.
	 *
	 * @param ChildBuilder A detail children builder for adding custom UI elements.
	 * @param Visibility The visibility setting for the row, determining how it should be displayed.
	 * @return A shared reference to the newly created FDetailMessageRow instance.
	 */
	static TSharedRef<FDetailMessageRow> Create(IDetailChildrenBuilder& ChildBuilder, const EVisibility Visibility = EVisibility::Visible);
	/**
	 * @brief Adds a new message to the detail message row.
	 *
	 * This method creates a new message for the associated message box with the specified parameters.
	 * The message can have different types, a wait condition for changement, a specified time before expiration, and
	 * a priority that determines the message's significance compared to others.
	 *
	 * @param Message The content of the message to be displayed.
	 * @param Type The type of the message, indicating its severity or purpose (e.g., Success, Error, Warning).
	 * @param bWaitForChangement Whether the message should wait for a changement state before starting its timer.
	 * @param RemainingTime The time, in seconds, before the message automatically disappears. If `bWaitForChangement` is true, this value is ignored.
	 * @param Priority The priority of the message. Higher priority messages can overwrite lower priority ones.
	 */
	void NewMessage(const FString& Message, const EMessageType Type, const bool bWaitForChangement, const float RemainingTime = 3.f, const int32 Priority = 0) const;
	/**
	 * @brief Clears the current message in the associated message box.
	 *
	 * This method triggers the removal of the current message from the message box,
	 * if one is present. If the message has a wait condition, it resets the message directly.
	 * Otherwise, it initiates a fade-out process. Also ensures the invalidation
	 * of the widget's layout and volatility for proper updates.
	 */
	void ClearMessage() const;

	/**
	 * @brief Checks if the row is currently visible based on its visibility settings.
	 *
	 * This method determines whether the row should be considered visible, taking into
	 * account various visibility states such as `Visible`, `HitTestInvisible`, and `SelfHitTestInvisible`.
	 *
	 * @return True if the row is visible; otherwise, false.
	 */
	bool IsVisible() const { return
		RowVisibility == EVisibility::Visible ||
		RowVisibility == EVisibility::HitTestInvisible ||
		RowVisibility == EVisibility::SelfHitTestInvisible;	
	}
	bool IsWaitingForChangement(const int32 Priority) const;

private:
	void Init(IDetailChildrenBuilder& ChildBuilder, const EVisibility Visibility, const TSharedRef<FDetailMessageRow>& Self);
	TSharedPtr<SDetailMessageBox> MessageBox;
	EVisibility RowVisibility = EVisibility::Visible;
};

class SDetailMessageBox : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SDetailMessageBox) {}
	SLATE_ARGUMENT(TSharedPtr<FDetailMessageRow>, DetailMessageRow)	
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;
	void NewMessage(const FString& InMessage, const EMessageType Type, const bool bWaitForChangement, const float RemainingTime = 3.f, const int32 Priority = 0);
	void ClearMessage();

	bool IsWaitingForChangement(const int32 Priority) const { return Message.IsSet() && Message->bWaitForChangement && Message->Priority == Priority;}

	FText GetMessageText() const
	{
		if (Message.IsSet())
		{
			return FText::FromString(Message->Message);
		}
		return FText::GetEmpty();
	}

	FSlateColor GetMessageColor() const
	{
		if (Message.IsSet())
		{
			return FLinearColor(1.f, 1.f, 1.f, Message->FadeAlpha);
		}
		return FLinearColor::Transparent;
	}

	FSlateColor GetCurrentBoxColor() const
	{
		if (!Message.IsSet())
		{
			return FLinearColor::Transparent;
		}
		switch (Message->Type)
		{
		case EMessageType::Success: return FLinearColor(0.0f, 0.229167f, 0.0f, Message->FadeAlpha); // Vert
		case EMessageType::Warning: return FLinearColor(0.64f, 0.64f, 0.00f, Message->FadeAlpha); // Jaune
		case EMessageType::Error:   return FLinearColor(0.161458f, 0.0f, 0.0f, Message->FadeAlpha); // Rouge
		default: return FLinearColor::Transparent;
		}
	}
private:
	TOptional<FMessageRow> Message;
	TSharedPtr<FDetailMessageRow> DetailMessageRow;
	bool bIsFadingOut = false;
};