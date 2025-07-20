#include "pch.h"
#include "APropertiesResourceView.h"
#include "ImGui/imgui.h"

#include <Plugins/PPropertyParser/PPropertiesWriter.h>
#include <imgui/imgui_internal.h>

void APropertiesResourceView::ShowProperties(PBProperties* a_pProperties)
{
	static constexpr int s_NodeFlags = ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_FramePadding;

	TBOOL bUpdatedValue = TFALSE;
	auto fnUpdateValue = [&](PBProperties::PBProperty* a_pProperty, PBPropertyValue* a_pValue)
	{
		bUpdatedValue = m_pSelectedProperty != a_pProperty || m_pSelectedValue != a_pValue;
		m_pSelectedProperty = a_pProperty;
		m_pSelectedValue = a_pValue;
	};

	for (auto it = a_pProperties->Begin(); it != a_pProperties->End(); it++)
	{
		auto propName = it->GetName().GetString();
		const int iFlags = (m_pSelectedProperty == it) ? ImGuiTreeNodeFlags_Selected : 0;

		switch (it->GetValue()->GetType())
		{
		case PBPropertyValue::Type::Int:
		case PBPropertyValue::Type::String:
		case PBPropertyValue::Type::Float:
		case PBPropertyValue::Type::Bool:
		case PBPropertyValue::Type::UInt32:
			ImGui::TreeNodeEx(it, s_NodeFlags | iFlags | ImGuiTreeNodeFlags_Leaf, propName);
			ImGui::TreePop();

			if (ImGui::IsItemFocused())
				fnUpdateValue(it, it->GetValue());

			break;
		case PBPropertyValue::Type::Array:
			if (ImGui::TreeNodeEx(it, iFlags | s_NodeFlags, propName))
			{
				if (ImGui::IsItemFocused())
					fnUpdateValue(it, TNULL);

				auto pArray = it->GetValue()->GetArray();

				for (size_t i = 0; i < pArray->GetSize(); i++)
				{
					auto pItem = pArray->GetValue(i);

					const int iItemFlags = (m_pSelectedValue == pItem) ? ImGuiTreeNodeFlags_Selected : 0;

					switch (pItem->GetType())
					{
					case PBPropertyValue::Type::Int:
						ImGui::TreeNodeEx(pItem, s_NodeFlags | iItemFlags | ImGuiTreeNodeFlags_Leaf, "%d", pItem->GetInteger());
						break;
					case PBPropertyValue::Type::String:
						ImGui::TreeNodeEx(pItem, s_NodeFlags | iItemFlags | ImGuiTreeNodeFlags_Leaf, "%s", pItem->GetString());
						break;
					case PBPropertyValue::Type::Float:
						ImGui::TreeNodeEx(pItem, s_NodeFlags | iItemFlags | ImGuiTreeNodeFlags_Leaf, "%f", pItem->GetFloat());
						break;
					case PBPropertyValue::Type::Bool:
						ImGui::TreeNodeEx(pItem, s_NodeFlags | iItemFlags | ImGuiTreeNodeFlags_Leaf, "%s", pItem->GetBoolean() ? "true" : "false");
						break;
					case PBPropertyValue::Type::UInt32:
						ImGui::TreeNodeEx(pItem, s_NodeFlags | iItemFlags | ImGuiTreeNodeFlags_Leaf, "%u", pItem->GetUINT32());
						break;
					}

					if (ImGui::IsItemFocused())
						fnUpdateValue(TNULL, pItem);

					ImGui::TreePop();
				}

				ImGui::TreePop();
			}
			else
			{
				if (ImGui::IsItemFocused())
					fnUpdateValue(it, TNULL);
			}

			break;
		case PBPropertyValue::Type::Properties:
			if (ImGui::TreeNodeEx(it, s_NodeFlags | iFlags, propName))
			{
				if (ImGui::IsItemFocused())
					fnUpdateValue(it, TNULL);

				ShowProperties(it->GetValue()->GetProperties());
				ImGui::TreePop();
			}
			else
			{
				if (ImGui::IsItemFocused())
					fnUpdateValue(it, TNULL);
			}


			break;
		}
	}

	if (bUpdatedValue)
	{
		if (m_pSelectedProperty)
		{
			Toshi::TStringManager::String8CopySafe(m_PropertyNameBuffer, m_pSelectedProperty->GetName().GetString());
		}

		if (m_pSelectedValue)
		{
			switch (m_pSelectedValue->GetType())
			{
			case PBPropertyValue::Type::Int:
				Toshi::TStringManager::String8Format(m_PropertyValueBuffer, sizeof(m_PropertyValueBuffer), "%d", m_pSelectedValue->GetInteger());
				break;
			case PBPropertyValue::Type::String:
				Toshi::TStringManager::String8Format(m_PropertyValueBuffer, sizeof(m_PropertyValueBuffer), "%s", m_pSelectedValue->GetString());
				break;
			case PBPropertyValue::Type::Float:
				Toshi::TStringManager::String8Format(m_PropertyValueBuffer, sizeof(m_PropertyValueBuffer), "%f", m_pSelectedValue->GetFloat());
				break;
			case PBPropertyValue::Type::Bool:
				Toshi::TStringManager::String8Format(m_PropertyValueBuffer, sizeof(m_PropertyValueBuffer), "%s", m_pSelectedValue->GetBoolean() ? "true" : "false");
				break;
			case PBPropertyValue::Type::UInt32:
				Toshi::TStringManager::String8Format(m_PropertyValueBuffer, sizeof(m_PropertyValueBuffer), "%u", m_pSelectedValue->GetUINT32());
				break;
			}
		}
	}
}

void APropertiesResourceView::Render()
{
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8.0f, 0.0f));
	
	auto szWinName = Toshi::TStringManager::GetTempString8();
	Toshi::TStringManager::String8Format(szWinName, 128, "Properties Window - %s", GetFile()->GetFileName());
	ImGui::SetNextWindowSize({ 1000, 580 }, ImGuiCond_FirstUseEver);
	ImGui::Begin(szWinName, TNULL, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoSavedSettings);
	ImGui::PopStyleVar();

	ImGui::BeginMenuBar();
	if (ImGui::BeginMenu("File"))
	{
		ImGui::MenuItem("New");
		ImGui::MenuItem("Open...");

		if (ImGui::MenuItem("Save"))
		{
			Save();
		}

		ImGui::MenuItem("Save as...");
		ImGui::Separator();
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 3);
		
		if (ImGui::MenuItem("Close"))
		{
			Destroy();
		}

		ImGui::EndMenu();
	}
	ImGui::EndMenuBar();

	ImGuiID dockspaceId = ImGui::GetID(this);
	ImGui::DockSpace(dockspaceId);

	if (!m_bInitialised)
	{
		ImGui::DockBuilderAddNode(dockspaceId, ImGuiDockNodeFlags_DockSpace);
		ImGui::DockBuilderSetNodeSize(dockspaceId, ImGui::GetContentRegionMaxAbs());

		ImGuiID dockIdEditor;
		ImGuiID dockIdList = ImGui::DockBuilderSplitNode(dockspaceId, ImGuiDir_Left, 0.6f, nullptr, &dockIdEditor);
		ImGui::DockBuilderGetNode(dockIdList)->LocalFlags = ImGuiDockNodeFlags_NoTabBar | ImGuiDockNodeFlags_NoDocking;
		ImGui::DockBuilderGetNode(dockIdEditor)->LocalFlags = ImGuiDockNodeFlags_NoTabBar | ImGuiDockNodeFlags_NoDocking;
		ImGui::DockBuilderDockWindow(m_szIdListWindowName, dockIdList);
		ImGui::DockBuilderDockWindow(m_szIdEditorWindowName, dockIdEditor);
		ImGui::DockBuilderFinish(dockspaceId);

		m_bInitialised = TTRUE;
	}

	{
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { 4, 2 });
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 0, 2 });
		ImGui::Begin(m_szIdListWindowName, TNULL, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar);
		if (ImGui::TreeNodeEx(&m_Properties, ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_CollapsingHeader, "Properties"))
		{
			ImGui::TreePush(&m_Properties);
			ShowProperties(&m_Properties);
			ImGui::TreePop();
		}
		ImGui::PopStyleVar(2);
		ImGui::End();

		ImGui::Begin(m_szIdEditorWindowName, TNULL, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar);

		if (m_pSelectedProperty)
		{
			ImGui::Text("Property Name:");
			ImGui::InputText(m_szIdPropertyName, m_PropertyNameBuffer, sizeof(m_PropertyNameBuffer));
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 4);
		}

		if (m_pSelectedValue)
		{
			switch (m_pSelectedValue->GetType())
			{
			case PBPropertyValue::Type::Int:
				ImGui::Text("Property Value (Int):");
				ImGui::InputInt(m_szIdPropertyValue, m_pSelectedValue->GetIntegerPointer());
				break;
			case PBPropertyValue::Type::String:
				ImGui::Text("Property Value (String):");
				ImGui::InputText(m_szIdPropertyValue, m_PropertyValueBuffer, sizeof(m_PropertyValueBuffer));
				ImGui::SameLine();

				if (ImGui::Button("Apply"))
				{
					m_pSelectedValue->SetString(m_PropertyValueBuffer);
				}

				break;
			case PBPropertyValue::Type::Float:
				ImGui::Text("Property Value (Float):");
				ImGui::InputFloat(m_szIdPropertyValue, m_pSelectedValue->GetFloatPointer());
				break;
			case PBPropertyValue::Type::Bool:
				ImGui::Text("Property Value (Boolean):");
				ImGui::SameLine();
				ImGui::Checkbox(m_szIdPropertyValue, m_pSelectedValue->GetBooleanPointer());
				break;
			case PBPropertyValue::Type::UInt32:
				ImGui::Text("Property Value (UInt32):");
				ImGui::DragInt(m_szIdPropertyValue, TREINTERPRETCAST(int*, m_pSelectedValue->GetUINT32Pointer()));
				break;
			case PBPropertyValue::Type::Array:
				ImGui::Text("Property Value (Array):");
				break;
			case PBPropertyValue::Type::Properties:
				ImGui::Text("Property Value (Properties):");
				break;
			}
		}

		ImGui::End();
	}

	ImGui::End();
}

void APropertiesResourceView::Save()
{
	PPropertiesWriter::WriteTRB(GetFile()->GetFileName(), m_Properties, TFALSE);
}
