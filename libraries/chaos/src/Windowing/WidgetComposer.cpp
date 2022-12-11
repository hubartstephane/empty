#include "chaos/ChaosPCH.h"
#include "chaos/ChaosInternals.h"

namespace chaos
{

	// =====================================================================
	// LinearComposerWidgetInsertData implementation
	// =====================================================================

	LinearComposerWidgetInsertData::LinearComposerWidgetInsertData(LinearComposerWidgetInsertType in_type):
		type(in_type)
	{}

	LinearComposerWidgetInsertData::LinearComposerWidgetInsertData(size_t in_position):
		type(LinearComposerWidgetInsertType::POSITIONAL),
		position(in_position)
	{}

	// =====================================================================
	// LinearComposerWidget implementation
	// =====================================================================

	void LinearComposerWidget::AddChildWidget(Widget* widget, LinearComposerWidgetInsertData insert_data, bool immediate_update)
	{
		assert(widget != nullptr);
		assert(widget->GetParentWidget() == nullptr);
		assert(dynamic_cast<WindowRootWidget*>(widget) == nullptr);

		if (insert_data.type == LinearComposerWidgetInsertType::END)
		{
			child_widgets.push_back(widget);
		}
		else if (insert_data.type == LinearComposerWidgetInsertType::START)
		{
			child_widgets.insert(child_widgets.begin(), widget);
		}
		else
		{
			if (insert_data.position >= child_widgets.size())
			{
				child_widgets.push_back(widget);
			}
			else
			{
				if (insert_data.type == LinearComposerWidgetInsertType::POSITIONAL)
				{
					child_widgets.insert(child_widgets.begin() + insert_data.position, widget);
				}
				else if (insert_data.type == LinearComposerWidgetInsertType::POSITIONAL_REPLACE)
				{
					// we want to always keep list of children coherent
					// that's why we have a
					//   child_widgets[insert_data.position] = nullptr
					// before
					//   child_widgets[insert_data.position] = widget;
					shared_ptr<Widget> prevent_destruction = child_widgets[insert_data.position];
					child_widgets[insert_data.position] = nullptr;
					if (prevent_destruction != nullptr)
						DetachChild(prevent_destruction.get(), immediate_update);
					child_widgets[insert_data.position] = widget;
				}
			}
		}
		AttachChild(widget, immediate_update);
	}

	void LinearComposerWidget::RemoveChildWidget(Widget* widget, bool immediate_update)
	{
		assert(widget != nullptr);
		assert(widget->GetParentWidget() == this);

		shared_ptr<Widget> prevent_destruction = widget;

		auto it = std::find(child_widgets.begin(), child_widgets.end(), widget);
		child_widgets.erase(it);

		DetachChild(widget, immediate_update);
	}

	// =====================================================================
	// OverlayWidget implementation
	// =====================================================================

	void OverlayWidget::SetPlacement(aabox2 const& in_placement)
	{
		Widget::SetPlacement(in_placement);
		for (auto& child : child_widgets)
			if (child != nullptr)
				child->SetPlacement(placement); // XXX: use member instead of incoming parameter, because the member take into account the overlay layout
	}

	// =====================================================================
	// LinearComposerLayoutWidget implementation
	// =====================================================================

	void LinearComposerLayoutWidget::SetPlacement(aabox2 const& in_placement)
	{
		LinearComposerWidget::SetPlacement(in_placement);

		size_t count = child_widgets.size();
		if (count > 0)
		{
			LinearComposerLayout linear_layout = GetComposerLayout();
			for (size_t i = 0; i < count; ++i)
				if (Widget* child = child_widgets[i].get())
					child->SetPlacement(linear_layout.ComputePlacement(placement, i, count));
		}
	}

	// =====================================================================
	// LinearComposerLayoutWidget implementation
	// =====================================================================

	LinearComposerLayout HorizontalBoxWidget::GetComposerLayout()
	{
		LinearComposerLayout result;
		result.orientation = Orientation::HORIZONTAL;
		//result.horizontal_mode = LinearComposerLayoutMode::REVERSED;
		return result;
	}

}; // namespace chaos
