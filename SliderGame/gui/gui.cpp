#include "gui.h"
#include "../helpers/helpers.h"

LPCTSTR gui::lpz_class = nullptr;
HWND gui::hwnd = nullptr;

bool gui::setup_window(HINSTANCE m_hInstance)
{
	lpz_class = "Slider game";
	if (!helpers::register_window(m_hInstance, lpz_class))
		return false;

	RECT rect_screen;
	GetWindowRect(GetDesktopWindow(), &rect_screen);
	int x = rect_screen.right / 2 - 300;
	int y = rect_screen.bottom / 2 - 200;

	HWND hwnd = CreateWindow(lpz_class, lpz_class, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU, x, y, WINDOW_WIDTH, WINDOW_HEIGHT, NULL, NULL, m_hInstance, NULL);
	if (!hwnd)
		return false;

	if (!helpers::create_device_d3d(hwnd))
	{
		helpers::cleanup_device_d3d();
		UnregisterClass(lpz_class, m_hInstance);
		return false;
	}

	// Show the window
	ShowWindow(hwnd, SW_SHOWDEFAULT);
	UpdateWindow(hwnd);

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	// Setup Platform/Renderer backends
	ImGui_ImplWin32_Init(hwnd);
	ImGui_ImplDX9_Init(globals::d3d_device);

	load_style();

	return true;
}

void gui::release_window(HINSTANCE m_hInstance)
{
	ImGui_ImplDX9_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	helpers::cleanup_device_d3d();
	DestroyWindow(hwnd);
	UnregisterClass(lpz_class, m_hInstance);
}

void gui::main()
{
	// Main loop
	MSG msg;
	ZeroMemory(&msg, sizeof(msg));
	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);

			continue;
		}

		if (globals::exit)
			break;

		gui::main_window();
	}
}

void gui::main_window()
{
	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

	// Start the Dear ImGui frame
	ImGui_ImplDX9_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	// Bar menu
	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("Game"))
		{
			if (ImGui::MenuItem("New game")) { }
			if (ImGui::MenuItem("Exit")) { globals::exit = true; }
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Settings"))
		{
			if (ImGui::MenuItem("Style settings")) { }
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Info"))
		{
			if (ImGui::MenuItem("About game")) { }
			if (ImGui::MenuItem("About author")) { }
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}

	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar;

	ImGui::SetNextWindowPos(ImVec2(0, 19));
	ImGui::SetNextWindowSize(ImVec2(WINDOW_WIDTH - 16, WINDOW_HEIGHT - 58));

	ImGui::Begin("#Main", NULL, window_flags);
	{
		float x = ImGui::GetWindowWidth() / 2 - 90.f;
		float y = ImGui::GetWindowHeight() / 2 - 50.f;

		ImGui::SetCursorPos(ImVec2(x, y));
		if (ImGui::Button("New game", ImVec2(180, 40)))
		{
			// New game
		}

		ImGui::SetCursorPosX(x);
		if (ImGui::Button("Exit", ImVec2(180, 40)))
		{
			globals::exit = true;
		}

		//ImGui::Text("Testing...");
		//if (ImGui::Button("Exit"))
		//	globals::exit = true;

		//if(ImGui::Button("Style settings"))
			//ImGui::ShowStyleEditor();
	}
	ImGui::End();

	// Rendering
	ImGui::EndFrame();
	globals::d3d_device->SetRenderState(D3DRS_ZENABLE, FALSE);
	globals::d3d_device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	globals::d3d_device->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);

	D3DCOLOR clear_col_dx = D3DCOLOR_RGBA((int)(clear_color.x*255.0f), (int)(clear_color.y*255.0f), (int)(clear_color.z*255.0f), (int)(clear_color.w*255.0f));
	globals::d3d_device->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, clear_col_dx, 1.0f, 0);

	if (globals::d3d_device->BeginScene() >= 0)
	{
		ImGui::Render();
		ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
		globals::d3d_device->EndScene();
	}

	HRESULT result = globals::d3d_device->Present(NULL, NULL, NULL, NULL);

	// Handle loss of D3D9 device
	if (result == D3DERR_DEVICELOST && globals::d3d_device->TestCooperativeLevel() == D3DERR_DEVICENOTRESET)
		helpers::reset_device();
}

void gui::load_style()
{
	ImGui::StyleColorsDark();

	ImGuiStyle& style = ImGui::GetStyle();

	style.WindowRounding = 0.f;
	style.WindowPadding.x = 8.f;
	style.WindowPadding.y = 3.f;
	style.FramePadding.x = 5.f;
	style.FramePadding.y = 3.f;
	style.FrameRounding = 0.f;
	style.ItemSpacing.x = 5.f;
	style.ItemSpacing.y = 4.f;
	style.ItemInnerSpacing.x = 5.f;
	style.ItemInnerSpacing.y = 4.f;
	style.TabRounding = 4.f;

	ImGui::GetStyle() = style;

	ImVec4* colors = ImGui::GetStyle().Colors;
	colors[ImGuiCol_Text] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
	colors[ImGuiCol_TextDisabled] = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
	colors[ImGuiCol_WindowBg] = ImVec4(0.94f, 0.94f, 0.94f, 1.00f);
	colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	colors[ImGuiCol_PopupBg] = ImVec4(1.00f, 1.00f, 1.00f, 0.98f);
	colors[ImGuiCol_Border] = ImVec4(0.00f, 0.00f, 0.00f, 0.31f);
	colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
	colors[ImGuiCol_FrameBg] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
	colors[ImGuiCol_FrameBgHovered] = ImVec4(0.00f, 0.00f, 0.00f, 0.05f);
	colors[ImGuiCol_FrameBgActive] = ImVec4(0.00f, 0.00f, 0.00f, 0.18f);
	colors[ImGuiCol_TitleBg] = ImVec4(0.96f, 0.96f, 0.96f, 1.00f);
	colors[ImGuiCol_TitleBgActive] = ImVec4(0.82f, 0.82f, 0.82f, 1.00f);
	colors[ImGuiCol_TitleBgCollapsed] = ImVec4(1.00f, 1.00f, 1.00f, 0.51f);
	colors[ImGuiCol_MenuBarBg] = ImVec4(0.86f, 0.86f, 0.86f, 1.00f);
	colors[ImGuiCol_ScrollbarBg] = ImVec4(0.98f, 0.98f, 0.98f, 0.53f);
	colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.69f, 0.69f, 0.69f, 0.80f);
	colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.49f, 0.49f, 0.49f, 0.80f);
	colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.49f, 0.49f, 0.49f, 1.00f);
	colors[ImGuiCol_CheckMark] = ImVec4(0.00f, 0.00f, 0.00f, 0.64f);
	colors[ImGuiCol_SliderGrab] = ImVec4(0.00f, 0.00f, 0.00f, 0.64f);
	colors[ImGuiCol_SliderGrabActive] = ImVec4(0.46f, 0.54f, 0.80f, 0.60f);
	colors[ImGuiCol_Button] = ImVec4(0.00f, 0.00f, 0.02f, 0.08f);
	colors[ImGuiCol_ButtonHovered] = ImVec4(0.00f, 0.00f, 0.00f, 0.05f);
	colors[ImGuiCol_ButtonActive] = ImVec4(0.00f, 0.00f, 0.02f, 0.08f);
	colors[ImGuiCol_Header] = ImVec4(0.00f, 0.00f, 0.00f, 0.16f);
	colors[ImGuiCol_HeaderHovered] = ImVec4(0.00f, 0.00f, 0.00f, 0.19f);
	colors[ImGuiCol_HeaderActive] = ImVec4(0.00f, 0.00f, 0.00f, 0.24f);
	colors[ImGuiCol_Separator] = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
	colors[ImGuiCol_SeparatorHovered] = ImVec4(0.00f, 0.00f, 0.00f, 0.19f);
	colors[ImGuiCol_SeparatorActive] = ImVec4(0.00f, 0.00f, 0.00f, 0.24f);
	colors[ImGuiCol_ResizeGrip] = ImVec4(0.80f, 0.80f, 0.80f, 0.56f);
	colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.00f, 0.00f, 0.00f, 0.19f);
	colors[ImGuiCol_ResizeGripActive] = ImVec4(0.00f, 0.00f, 0.00f, 0.38f);
	colors[ImGuiCol_PlotLines] = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
	colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
	colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
	colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.45f, 0.00f, 1.00f);
	colors[ImGuiCol_TextSelectedBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.19f);
	colors[ImGuiCol_ModalWindowDarkening] = ImVec4(0.20f, 0.20f, 0.20f, 0.35f);
	colors[ImGuiCol_DragDropTarget] = ImVec4(0.00f, 0.00f, 0.00f, 0.19f);

}

static void HelpMarker(const char* desc)
{
	ImGui::TextDisabled("(?)");
	if (ImGui::IsItemHovered())
	{
		ImGui::BeginTooltip();
		ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
		ImGui::TextUnformatted(desc);
		ImGui::PopTextWrapPos();
		ImGui::EndTooltip();
	}
}

bool ImGui::ShowStyleSelector(const char* label)
{
	static int style_idx = -1;
	if (ImGui::Combo(label, &style_idx, "Classic\0Dark\0Light\0"))
	{
		switch (style_idx)
		{
		case 0: ImGui::StyleColorsClassic(); break;
		case 1: ImGui::StyleColorsDark(); break;
		case 2: ImGui::StyleColorsLight(); break;
		}
		return true;
	}
	return false;
}

// Demo helper function to select among loaded fonts.
// Here we use the regular BeginCombo()/EndCombo() api which is more the more flexible one.
void ImGui::ShowFontSelector(const char* label)
{
	ImGuiIO& io = ImGui::GetIO();
	ImFont* font_current = ImGui::GetFont();
	if (ImGui::BeginCombo(label, font_current->GetDebugName()))
	{
		for (int n = 0; n < io.Fonts->Fonts.Size; n++)
		{
			ImFont* font = io.Fonts->Fonts[n];
			ImGui::PushID((void*)font);
			if (ImGui::Selectable(font->GetDebugName(), font == font_current))
				io.FontDefault = font;
			ImGui::PopID();
		}
		ImGui::EndCombo();
	}
	ImGui::SameLine();
	HelpMarker(
		"- Load additional fonts with io.Fonts->AddFontFromFileTTF().\n"
		"- The font atlas is built when calling io.Fonts->GetTexDataAsXXXX() or io.Fonts->Build().\n"
		"- Read FAQ and docs/FONTS.md for more details.\n"
		"- If you need to add/remove fonts at runtime (e.g. for DPI change), do it before calling NewFrame().");
}

#define IM_NEWLINE  "\r\n"

void ImGui::ShowStyleEditor(ImGuiStyle* ref)
{
	// You can pass in a reference ImGuiStyle structure to compare to, revert to and save to
	// (without a reference style pointer, we will use one compared locally as a reference)
	ImGuiStyle& style = ImGui::GetStyle();
	static ImGuiStyle ref_saved_style;

	// Default to using internal storage as reference
	static bool init = true;
	if (init && ref == NULL)
		ref_saved_style = style;
	init = false;
	if (ref == NULL)
		ref = &ref_saved_style;

	ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.50f);

	if (ImGui::ShowStyleSelector("Colors##Selector"))
		ref_saved_style = style;
	ImGui::ShowFontSelector("Fonts##Selector");

	// Simplified Settings (expose floating-pointer border sizes as boolean representing 0.0f or 1.0f)
	if (ImGui::SliderFloat("FrameRounding", &style.FrameRounding, 0.0f, 12.0f, "%.0f"))
		style.GrabRounding = style.FrameRounding; // Make GrabRounding always the same value as FrameRounding
	{ bool border = (style.WindowBorderSize > 0.0f); if (ImGui::Checkbox("WindowBorder", &border)) { style.WindowBorderSize = border ? 1.0f : 0.0f; } }
	ImGui::SameLine();
	{ bool border = (style.FrameBorderSize > 0.0f);  if (ImGui::Checkbox("FrameBorder", &border)) { style.FrameBorderSize = border ? 1.0f : 0.0f; } }
	ImGui::SameLine();
	{ bool border = (style.PopupBorderSize > 0.0f);  if (ImGui::Checkbox("PopupBorder", &border)) { style.PopupBorderSize = border ? 1.0f : 0.0f; } }

	// Save/Revert button
	if (ImGui::Button("Save Ref"))
		*ref = ref_saved_style = style;
	ImGui::SameLine();
	if (ImGui::Button("Revert Ref"))
		style = *ref;
	ImGui::SameLine();
	HelpMarker(
		"Save/Revert in local non-persistent storage. Default Colors definition are not affected. "
		"Use \"Export\" below to save them somewhere.");

	ImGui::Separator();

	if (ImGui::BeginTabBar("##tabs", ImGuiTabBarFlags_None))
	{
		if (ImGui::BeginTabItem("Sizes"))
		{
			ImGui::Text("Main");
			ImGui::SliderFloat2("WindowPadding", (float*)&style.WindowPadding, 0.0f, 20.0f, "%.0f");
			ImGui::SliderFloat2("FramePadding", (float*)&style.FramePadding, 0.0f, 20.0f, "%.0f");
			ImGui::SliderFloat2("ItemSpacing", (float*)&style.ItemSpacing, 0.0f, 20.0f, "%.0f");
			ImGui::SliderFloat2("ItemInnerSpacing", (float*)&style.ItemInnerSpacing, 0.0f, 20.0f, "%.0f");
			ImGui::SliderFloat2("TouchExtraPadding", (float*)&style.TouchExtraPadding, 0.0f, 10.0f, "%.0f");
			ImGui::SliderFloat("IndentSpacing", &style.IndentSpacing, 0.0f, 30.0f, "%.0f");
			ImGui::SliderFloat("ScrollbarSize", &style.ScrollbarSize, 1.0f, 20.0f, "%.0f");
			ImGui::SliderFloat("GrabMinSize", &style.GrabMinSize, 1.0f, 20.0f, "%.0f");
			ImGui::Text("Borders");
			ImGui::SliderFloat("WindowBorderSize", &style.WindowBorderSize, 0.0f, 1.0f, "%.0f");
			ImGui::SliderFloat("ChildBorderSize", &style.ChildBorderSize, 0.0f, 1.0f, "%.0f");
			ImGui::SliderFloat("PopupBorderSize", &style.PopupBorderSize, 0.0f, 1.0f, "%.0f");
			ImGui::SliderFloat("FrameBorderSize", &style.FrameBorderSize, 0.0f, 1.0f, "%.0f");
			ImGui::SliderFloat("TabBorderSize", &style.TabBorderSize, 0.0f, 1.0f, "%.0f");
			ImGui::Text("Rounding");
			ImGui::SliderFloat("WindowRounding", &style.WindowRounding, 0.0f, 12.0f, "%.0f");
			ImGui::SliderFloat("ChildRounding", &style.ChildRounding, 0.0f, 12.0f, "%.0f");
			ImGui::SliderFloat("FrameRounding", &style.FrameRounding, 0.0f, 12.0f, "%.0f");
			ImGui::SliderFloat("PopupRounding", &style.PopupRounding, 0.0f, 12.0f, "%.0f");
			ImGui::SliderFloat("ScrollbarRounding", &style.ScrollbarRounding, 0.0f, 12.0f, "%.0f");
			ImGui::SliderFloat("GrabRounding", &style.GrabRounding, 0.0f, 12.0f, "%.0f");
			ImGui::SliderFloat("LogSliderDeadzone", &style.LogSliderDeadzone, 0.0f, 12.0f, "%.0f");
			ImGui::SliderFloat("TabRounding", &style.TabRounding, 0.0f, 12.0f, "%.0f");
			ImGui::Text("Alignment");
			ImGui::SliderFloat2("WindowTitleAlign", (float*)&style.WindowTitleAlign, 0.0f, 1.0f, "%.2f");
			int window_menu_button_position = style.WindowMenuButtonPosition + 1;
			if (ImGui::Combo("WindowMenuButtonPosition", (int*)&window_menu_button_position, "None\0Left\0Right\0"))
				style.WindowMenuButtonPosition = window_menu_button_position - 1;
			ImGui::Combo("ColorButtonPosition", (int*)&style.ColorButtonPosition, "Left\0Right\0");
			ImGui::SliderFloat2("ButtonTextAlign", (float*)&style.ButtonTextAlign, 0.0f, 1.0f, "%.2f");
			ImGui::SameLine(); HelpMarker("Alignment applies when a button is larger than its text content.");
			ImGui::SliderFloat2("SelectableTextAlign", (float*)&style.SelectableTextAlign, 0.0f, 1.0f, "%.2f");
			ImGui::SameLine(); HelpMarker("Alignment applies when a selectable is larger than its text content.");
			ImGui::Text("Safe Area Padding");
			ImGui::SameLine(); HelpMarker("Adjust if you cannot see the edges of your screen (e.g. on a TV where scaling has not been configured).");
			ImGui::SliderFloat2("DisplaySafeAreaPadding", (float*)&style.DisplaySafeAreaPadding, 0.0f, 30.0f, "%.0f");
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Colors"))
		{
			static int output_dest = 0;
			static bool output_only_modified = true;
			if (ImGui::Button("Export"))
			{
				if (output_dest == 0)
					ImGui::LogToClipboard();
				else
					ImGui::LogToTTY();
				ImGui::LogText("ImVec4* colors = ImGui::GetStyle().Colors;" IM_NEWLINE);
				for (int i = 0; i < ImGuiCol_COUNT; i++)
				{
					const ImVec4& col = style.Colors[i];
					const char* name = ImGui::GetStyleColorName(i);
					if (!output_only_modified || memcmp(&col, &ref->Colors[i], sizeof(ImVec4)) != 0)
						ImGui::LogText("colors[ImGuiCol_%s]%*s= ImVec4(%.2ff, %.2ff, %.2ff, %.2ff);" IM_NEWLINE,
							name, 23 - (int)strlen(name), "", col.x, col.y, col.z, col.w);
				}
				ImGui::LogFinish();
			}
			ImGui::SameLine(); ImGui::SetNextItemWidth(120); ImGui::Combo("##output_type", &output_dest, "To Clipboard\0To TTY\0");
			ImGui::SameLine(); ImGui::Checkbox("Only Modified Colors", &output_only_modified);

			static ImGuiTextFilter filter;
			filter.Draw("Filter colors", ImGui::GetFontSize() * 16);

			static ImGuiColorEditFlags alpha_flags = 0;
			if (ImGui::RadioButton("Opaque", alpha_flags == ImGuiColorEditFlags_None)) { alpha_flags = ImGuiColorEditFlags_None; } ImGui::SameLine();
			if (ImGui::RadioButton("Alpha", alpha_flags == ImGuiColorEditFlags_AlphaPreview)) { alpha_flags = ImGuiColorEditFlags_AlphaPreview; } ImGui::SameLine();
			if (ImGui::RadioButton("Both", alpha_flags == ImGuiColorEditFlags_AlphaPreviewHalf)) { alpha_flags = ImGuiColorEditFlags_AlphaPreviewHalf; } ImGui::SameLine();
			HelpMarker(
				"In the color list:\n"
				"Left-click on color square to open color picker,\n"
				"Right-click to open edit options menu.");

			ImGui::BeginChild("##colors", ImVec2(0, 0), true, ImGuiWindowFlags_AlwaysVerticalScrollbar | ImGuiWindowFlags_AlwaysHorizontalScrollbar | ImGuiWindowFlags_NavFlattened);
			ImGui::PushItemWidth(-160);
			for (int i = 0; i < ImGuiCol_COUNT; i++)
			{
				const char* name = ImGui::GetStyleColorName(i);
				if (!filter.PassFilter(name))
					continue;
				ImGui::PushID(i);
				ImGui::ColorEdit4("##color", (float*)&style.Colors[i], ImGuiColorEditFlags_AlphaBar | alpha_flags);
				if (memcmp(&style.Colors[i], &ref->Colors[i], sizeof(ImVec4)) != 0)
				{
					// Tips: in a real user application, you may want to merge and use an icon font into the main font,
					// so instead of "Save"/"Revert" you'd use icons!
					// Read the FAQ and docs/FONTS.md about using icon fonts. It's really easy and super convenient!
					ImGui::SameLine(0.0f, style.ItemInnerSpacing.x); if (ImGui::Button("Save")) { ref->Colors[i] = style.Colors[i]; }
					ImGui::SameLine(0.0f, style.ItemInnerSpacing.x); if (ImGui::Button("Revert")) { style.Colors[i] = ref->Colors[i]; }
				}
				ImGui::SameLine(0.0f, style.ItemInnerSpacing.x);
				ImGui::TextUnformatted(name);
				ImGui::PopID();
			}
			ImGui::PopItemWidth();
			ImGui::EndChild();

			ImGui::EndTabItem();
		}

		/*if (ImGui::BeginTabItem("Fonts"))
		{
			ImGuiIO& io = ImGui::GetIO();
			ImFontAtlas* atlas = io.Fonts;
			HelpMarker("Read FAQ and docs/FONTS.md for details on font loading.");
			ImGui::PushItemWidth(120);
			for (int i = 0; i < atlas->Fonts.Size; i++)
			{
				ImFont* font = atlas->Fonts[i];
				ImGui::PushID(font);
				NodeFont(font);
				ImGui::PopID();
			}
			if (ImGui::TreeNode("Atlas texture", "Atlas texture (%dx%d pixels)", atlas->TexWidth, atlas->TexHeight))
			{
				ImVec4 tint_col = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
				ImVec4 border_col = ImVec4(1.0f, 1.0f, 1.0f, 0.5f);
				ImGui::Image(atlas->TexID, ImVec2((float)atlas->TexWidth, (float)atlas->TexHeight), ImVec2(0, 0), ImVec2(1, 1), tint_col, border_col);
				ImGui::TreePop();
			}

			// Post-baking font scaling. Note that this is NOT the nice way of scaling fonts, read below.
			// (we enforce hard clamping manually as by default DragFloat/SliderFloat allows CTRL+Click text to get out of bounds).
			const float MIN_SCALE = 0.3f;
			const float MAX_SCALE = 2.0f;
			HelpMarker(
				"Those are old settings provided for convenience.\n"
				"However, the _correct_ way of scaling your UI is currently to reload your font at the designed size, "
				"rebuild the font atlas, and call style.ScaleAllSizes() on a reference ImGuiStyle structure.\n"
				"Using those settings here will give you poor quality results.");
			static float window_scale = 1.0f;
			if (ImGui::DragFloat("window scale", &window_scale, 0.005f, MIN_SCALE, MAX_SCALE, "%.2f", ImGuiSliderFlags_AlwaysClamp)) // Scale only this window
				ImGui::SetWindowFontScale(window_scale);
			ImGui::DragFloat("global scale", &io.FontGlobalScale, 0.005f, MIN_SCALE, MAX_SCALE, "%.2f", ImGuiSliderFlags_AlwaysClamp); // Scale everything
			ImGui::PopItemWidth();

			ImGui::EndTabItem();
		}*/

		if (ImGui::BeginTabItem("Rendering"))
		{
			ImGui::Checkbox("Anti-aliased lines", &style.AntiAliasedLines);
			ImGui::SameLine();
			HelpMarker("When disabling anti-aliasing lines, you'll probably want to disable borders in your style as well.");

			ImGui::Checkbox("Anti-aliased lines use texture", &style.AntiAliasedLinesUseTex);
			ImGui::SameLine();
			HelpMarker("Faster lines using texture data. Require backend to render with bilinear filtering (not point/nearest filtering).");

			ImGui::Checkbox("Anti-aliased fill", &style.AntiAliasedFill);
			ImGui::PushItemWidth(100);
			ImGui::DragFloat("Curve Tessellation Tolerance", &style.CurveTessellationTol, 0.02f, 0.10f, 10.0f, "%.2f");
			if (style.CurveTessellationTol < 0.10f) style.CurveTessellationTol = 0.10f;

			// When editing the "Circle Segment Max Error" value, draw a preview of its effect on auto-tessellated circles.
			ImGui::DragFloat("Circle Segment Max Error", &style.CircleSegmentMaxError, 0.01f, 0.10f, 10.0f, "%.2f");
			if (ImGui::IsItemActive())
			{
				ImGui::SetNextWindowPos(ImGui::GetCursorScreenPos());
				ImGui::BeginTooltip();
				ImVec2 p = ImGui::GetCursorScreenPos();
				ImDrawList* draw_list = ImGui::GetWindowDrawList();
				float RAD_MIN = 10.0f, RAD_MAX = 80.0f;
				float off_x = 10.0f;
				for (int n = 0; n < 7; n++)
				{
					const float rad = RAD_MIN + (RAD_MAX - RAD_MIN) * (float)n / (7.0f - 1.0f);
					draw_list->AddCircle(ImVec2(p.x + off_x + rad, p.y + RAD_MAX), rad, ImGui::GetColorU32(ImGuiCol_Text), 0);
					off_x += 10.0f + rad * 2.0f;
				}
				ImGui::Dummy(ImVec2(off_x, RAD_MAX * 2.0f));
				ImGui::EndTooltip();
			}
			ImGui::SameLine();
			HelpMarker("When drawing circle primitives with \"num_segments == 0\" tesselation will be calculated automatically.");

			ImGui::DragFloat("Global Alpha", &style.Alpha, 0.005f, 0.20f, 1.0f, "%.2f"); // Not exposing zero here so user doesn't "lose" the UI (zero alpha clips all widgets). But application code could have a toggle to switch between zero and non-zero.
			ImGui::PopItemWidth();

			ImGui::EndTabItem();
		}

		ImGui::EndTabBar();
	}

	ImGui::PopItemWidth();
}