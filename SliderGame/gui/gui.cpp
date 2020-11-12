#include "gui.h"
#include "../helpers/helpers.h"
#include "../core/core.h"

#include <windows.h>
#pragma comment(lib, "winmm.lib")

namespace gui
{
	LPCTSTR lpz_class = nullptr;
	HWND hwnd = nullptr;
	bool show_new_game_window = false;
	bool show_message_window = false;
	bool show_info_window = false;
	bool show_setting_window = false;
	float color_first_pl[4] = { 0.f, 0.f, 0.f, 1.f };
}

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

static bool onc = false;

void gui::main_window()
{
	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

	// Start the Dear ImGui frame
	ImGui_ImplDX9_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	//mciSendString("stop C:\\Users\\w0resT\\Desktop\\2.mp3", NULL, 0, NULL);
	//mciSendString("close C:\\Users\\w0resT\\Desktop\\2.mp3", NULL, 0, NULL);

	mciSendString("play C:\\Users\\w0resT\\Desktop\\1.mp3 repeat", NULL, 0, NULL);

	// Bar menu
	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("Game"))
		{
			if (ImGui::MenuItem("New game")) { show_new_game_window = true; core::reset_data(); }
			if (show_new_game_window) if (ImGui::MenuItem("Back to menu")) { show_new_game_window = false; core::reset_data(); }
			if (ImGui::MenuItem("Exit")) { globals::exit = true; }
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Settings"))
		{
			if (ImGui::MenuItem("Style settings")) { show_setting_window = true; show_info_window = false; }
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Info"))
		{
			if (ImGui::MenuItem("About game")) { show_info_window = true; show_setting_window = false; }
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
			show_new_game_window = true;
		}

		ImGui::SetCursorPosX(x);
		if (ImGui::Button("Exit", ImVec2(180, 40)))
		{
			globals::exit = true;
		}
	}
	ImGui::End();

	if (show_message_window)
		message_window(gui::msg_type::STATE_MSG);

	if (show_info_window)
		info_window();

	if (show_setting_window)
		setting_window();

	if (show_new_game_window)
	{
		//mciSendString("stop C:\\Users\\w0resT\\Desktop\\1.mp3", NULL, 0, NULL);
		//mciSendString("close C:\\Users\\w0resT\\Desktop\\1.mp3", NULL, 0, NULL);
		game_window();
	}

	onc = false;

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

void gui::game_window()
{
	static bool can_play = true;
	static bool move_did = false;

	// TODO: Make global window_flags
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar;

	ImGui::SetNextWindowPos(ImVec2(0, 19));
	ImGui::SetNextWindowSize(ImVec2(WINDOW_WIDTH - 16, WINDOW_HEIGHT - 58));

	
	if (!onc)
	{
		mciSendString("stop C:\\Users\\w0resT\\Desktop\\1.mp3", NULL, 0, NULL);
		mciSendString("close C:\\Users\\w0resT\\Desktop\\1.mp3", NULL, 0, NULL);
		onc = true;
	}

	// TODO: Make func CalculateScore for this
	if (move_did)
	{
		core::best_score_h = core::evaluation(core::headtail.second, 0, 0);
		core::best_score_t = core::evaluation(core::headtail.first, 0, 1);
		move_did = false;
		mciSendString("stop C:\\Users\\w0resT\\Desktop\\2.mp3", NULL, 0, NULL);
		mciSendString("close C:\\Users\\w0resT\\Desktop\\2.mp3", NULL, 0, NULL);
	}

	mciSendString("play C:\\Users\\w0resT\\Desktop\\2.mp3 repeat", NULL, 0, NULL);

	ImGui::Begin("#Game", NULL, window_flags);
	{
		static bool once = false;
		if (!once)
		{
			core::reset_data();
			once = true;
		}
		
		ImGui::Columns(1);
		ImGui::BeginChild("#ChildGame", ImVec2(236, 0), true);
		{
			ImDrawList* draw_list = ImGui::GetWindowDrawList();	

			ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.00f, 0.00f, 0.00f, 0.52f));
			ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(0.00f, 0.00f, 0.00f, 0.52f));
			ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0.00f, 0.00f, 0.00f, 0.72f));
			for (int n = 0; n < 25; ++n)
			{
				ImGui::PushID(n);
				if ((n % 5) != 0)
					ImGui::SameLine(0.f, 30);

				if (n == 5)
					ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 29.f);

				if (n == 10)
					ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 29.f);

				if (n == 15)
					ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 29.f);

				if (n == 20)
					ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 29.f);

				if (n == 25)
					ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 29.f);

				core::map_dot_pos[n] = ImGui::GetCursorPos();
				core::map_dot_pos[n].x += 18.0f;
				core::map_dot_pos[n].y += 31.f;

				ImGui::RadioButton("", core::map_dot[n]);

				if (!can_play)
					goto out_first;

				// It is Human. We need do same for PC
				if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
				{
					ImGui::SetDragDropPayload("DND_GAME_CELL", &n, sizeof(int));
					ImGui::Text("%d", n);
					ImGui::EndDragDropSource();
				}
				if (ImGui::BeginDragDropTarget())
				{
					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_GAME_CELL"))
					{
						IM_ASSERT(payload->DataSize == sizeof(int));
						int payload_n = *(const int*)payload->Data;

						static bool first_ = false;

						if(!core::message.empty())
							core::message.clear();

						if (core::headtail.first == -1)
						{
							core::headtail.first = payload_n;
							core::headtail.second = n;
							first_ = true;
						}

						if (core::is_possible_move(payload_n, n))
						{
							if (!core::check_state(payload_n, n))
							{
								core::message.append("You lose.");
								show_message_window = true;
								can_play = false;
							}

							if (!first_)
							{
								if (payload_n == core::headtail.first)
									core::headtail.first = n;

								if (payload_n == core::headtail.second)
									core::headtail.second = n;
							}

							first_ = false;
							move_did = true;

							core::map_dot[n] = true;
							core::map_dot[payload_n] = true;

							if (can_play)
								core::player = !core::player;

							std::pair<ImVec2, ImVec2> temp;
							temp.first = core::map_dot_pos[payload_n];
							temp.second = core::map_dot_pos[n];
							core::made_moves.push_back(temp);

							std::pair<int, int> temp_2;
							temp_2.first = payload_n;
							temp_2.second = n;
							core::made_moves_idx.push_back(temp_2);

							core::made_moves_player.push_back(can_play == true ? core::player : !core::player);
						}
						
					}
					ImGui::EndDragDropTarget();
				}
				out_first:

				ImGui::PopID();
			}
			ImGui::PopStyleColor(3);
			
			// TODO: Make ColorPicker for it
			// Drawing lines
			int k = 0;
			for (auto foo : core::made_moves)
			{
				draw_list->AddLine(foo.first, foo.second, core::made_moves_player[k] ? ImColor(80, 130, 0) : ImColor(0, 130, 255), 2.f);
				k++;
			}
		}
		ImGui::EndChild();

		ImGui::SameLine();
		ImGui::NextColumn();
		ImGui::BeginChild("#ChildGame2", ImVec2(0, 0), true);
		{
			ImGui::Text("Information");
			ImGui::Separator();

			ImGui::Text("Move:");
			ImGui::TextColored(ImColor(70, 150, 170), core::player ? "Second player" : "First player");

			if (can_play)
			{
				ImGui::Text("Head: %s", get_way_by_idx(core::best_way_h).c_str());
				ImGui::Text("Tail: %s", get_way_by_idx(core::best_way_t).c_str());
				ImGui::Text("Head score: %d", core::best_score_h);
				ImGui::Text("Tail score: %d", core::best_score_t);
			}

			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.549f, 0.078f, 0.f, 1.f));
			if (!core::message.empty())
				ImGui::TextWrapped("%s", core::message.c_str());
			ImGui::PopStyleColor();

			ImGui::Separator();
			ImGui::Text("Game");
			ImGui::Separator();
			if (ImGui::Button("Reset", ImVec2(-1, 20))) 
			{ 
				core::reset_data(); can_play = true; 
				mciSendString("stop C:\\Users\\w0resT\\Desktop\\2.mp3", NULL, 0, NULL);
				mciSendString("close C:\\Users\\w0resT\\Desktop\\2.mp3", NULL, 0, NULL);
			}
			if (ImGui::Button("Back to menu", ImVec2(-1, 20))) 
			{ 
				show_new_game_window = false; core::reset_data(); 
				mciSendString("stop C:\\Users\\w0resT\\Desktop\\2.mp3", NULL, 0, NULL);
				mciSendString("close C:\\Users\\w0resT\\Desktop\\2.mp3", NULL, 0, NULL);
			}

		}
		ImGui::EndChild();
	}
	ImGui::End();
}

// TODO: FIXIT: We can click out of this window
void gui::message_window(msg_type msg_type)
{
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar;

	std::string msg;
	
	switch (msg_type)
	{
	case gui::STATE_MSG:
		msg = core::player ? "Second " : "First ";
		msg.append("player lose!");
		break;
	case gui::ERROR_MSG:
		msg = "Finish game first!";
		break;
	default:
		msg = "Error! Unknown type!";
		break;
	}
	
	ImGui::SetNextWindowPos(ImVec2(100, 100));
	ImGui::SetNextWindowSize(ImVec2(146, 50));

	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.85f, 0.85f, 0.85f, 1.00f));
	ImGui::Begin("#Message", NULL, window_flags);
	{
		ImGui::Text("%s", msg.c_str());
		if (ImGui::Button("Back", ImVec2(-1, -1)))
			show_message_window = false;
	}
	ImGui::End();
	ImGui::PopStyleColor();
}

void gui::info_window()
{
	std::string msg = "Two people play, taking horizontal or vertical 'unit' segments in turn. It is required that the resulting trajectory of the game be continuous, but you can attach a new segment to an existing polyline from either end. The one who is forced to close the trajectory under his own power loses.";

	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar;

	ImGui::SetNextWindowPos(ImVec2(0, 19));
	ImGui::SetNextWindowSize(ImVec2(WINDOW_WIDTH - 16, WINDOW_HEIGHT - 58));
	ImGui::Begin("#Info", NULL, window_flags);
	{
		ImGui::Text("Rules:");
		ImGui::Separator();
		ImGui::BeginChild("#ChildInfo", ImVec2(-1, 90), true);
		{
			ImGui::TextWrapped("%s", msg.c_str());
		}
		ImGui::EndChild();
		if (ImGui::Button("Back", ImVec2(0, 0)))
			show_info_window = false;
	}
	ImGui::End();
}

void gui::setting_window()
{
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar;

	ImGui::SetNextWindowPos(ImVec2(0, 19));
	ImGui::SetNextWindowSize(ImVec2(WINDOW_WIDTH - 16, WINDOW_HEIGHT - 58));
	ImGui::Begin("#Info", NULL, window_flags);
	{
		ImGui::Text("Settings:");
		ImGui::Separator();
		ImGui::BeginChild("#ChildSettings", ImVec2(-1, 90), true);
		{
			
		}
		ImGui::EndChild();

		if (ImGui::Button("Back", ImVec2(0, 0)))
			show_setting_window = false;
	}
	ImGui::End();
}

std::string gui::get_way_by_idx(int idx)
{
	std::string str;

	switch (idx)
	{
	case 1:
		str = "Up";
		break;
	case 2:
		str = "Down";
		break;
	case 3:
		str = "Right";
		break;
	case 4:
		str = "Left";
		break;
	default:
		str = "None";
		break;
	}

	return str;
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