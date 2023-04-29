//
// lager - library for functional interactive c++ programs
// Copyright (C) 2017 Juan Pedro Bolivar Puente
//
// This file is part of lager.
//
// lager is free software: you can redistribute it and/or modify
// it under the terms of the MIT License, as detailed in the LICENSE
// file located at the root of this source code distribution,
// or here: <https://github.com/arximboldi/lager/blob/master/LICENSE>
//

#include "../model.hpp"

#include <lager/debug/debugger.hpp>
#include <lager/debug/http_server.hpp>
#include <lager/event_loop/sdl.hpp>
#include <lager/store.hpp>
#include <lager/resources_path.hpp>

#include <zug/compose.hpp>

#include <cereal/types/complex.hpp>

#include <imgui.h>
#include <imgui_impl_sdlrenderer.h>
#include <imgui_impl_sdl2.h>

#include <SDL.h>
#include <SDL_opengl.h>

#include <iostream>

constexpr int window_padding = 48;
constexpr int window_width   = 800;
constexpr int window_height  = 600;

// Sadly, ImGui sometimes forces us to store transient state, like text inputs.
// We use this store this.
struct ui_state
{
    static constexpr std::size_t input_string_size = 1 << 10;

    std::array<char, input_string_size> new_todo_input{'\0'};
};

void line()
{
	ImVec2 mi = ImGui::GetItemRectMin();
	ImVec2 ma = ImGui::GetItemRectMax();

	mi.y = ma.y;

	ImGui::GetWindowDrawList()->AddLine(mi, ma, 0x5a, 1.0f);
}

void draw(lager::context<todo::item_action> ctx, const todo::item& i)
{
    auto checked = i.done;
    if (ImGui::Checkbox("##checked", &checked)) {
        ctx.dispatch(todo::toggle_item_action{});
    }

    ImGui::SameLine();
    ImGui::Text("%s", i.text.c_str());
    if (i.done) {
        line();
    }

    ImGui::SameLine();
    if (ImGui::Button("Delete")) {
        ctx.dispatch(todo::remove_item_action{});
    }
}

void draw(lager::context<todo::model_action> ctx,
          const todo::model& m,
          ui_state& s)
{
    ImGui::SetNextWindowPos({window_padding, window_padding}, ImGuiCond_Once);
    ImGui::SetNextWindowSize(
        {window_width - 2 * window_padding, window_height - 2 * window_padding},
        ImGuiCond_Once);
    ImGui::Begin("Todo app");

    if (ImGui::BeginPopup("not-implemented")) {
        ImGui::Text("Saving and loading have not been implemented!");
        ImGui::EndPopup();
    }

    if (ImGui::Button("Save"))
        ImGui::OpenPopup("not-implemented");
    ImGui::SameLine();
    if (ImGui::Button("Load"))
        ImGui::OpenPopup("not-implemented");

    ImGui::Separator();
    if (ImGui::IsWindowAppearing())
        ImGui::SetKeyboardFocusHere();
    ImGui::PushItemWidth(-0.1f);
    if (ImGui::InputTextWithHint("##input",
                                 "What do you want to do today?",
                                 s.new_todo_input.data(),
                                 s.input_string_size,
                                 ImGuiInputTextFlags_EnterReturnsTrue)) {
        ctx.dispatch(todo::add_todo_action{s.new_todo_input.data()});
        s.new_todo_input[0] = '\0';
        ImGui::SetKeyboardFocusHere(-1);
    }
    ImGui::PopItemWidth();
    ImGui::Separator();

    ImGui::BeginChild("##child");
    {
        auto idx = std::size_t{};
        for (auto item : m.todos) {
            ImGui::PushID(idx);
            auto with_idx = [idx](auto&& a) { return std::make_pair(idx, a); };
            draw({ctx, with_idx}, item);
            ImGui::PopID();
            ++idx;
        }
    }
    ImGui::EndChild();

    ImGui::End();
}

int main(int argc, const char *argv[])
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
        std::cerr << "Error initializing SDL: " << SDL_GetError() << std::endl;
        return -1;
    }

    // Create window with SDL_Renderer graphics context
    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    SDL_Window* window = SDL_CreateWindow("Todo ImGui", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, window_flags);
    if (!window) {
        std::cerr << "Error creating SDL window: " << SDL_GetError()
                  << std::endl;
        return -1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);
    if (renderer == nullptr)
    {
        std::cerr << "Error creating SDL renderer: " << SDL_GetError()
                  << std::endl;
        return -1;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    auto& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

    ImGui::StyleColorsDark();

    ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer_Init(renderer);

    auto clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

#ifdef DEBUGGER
    auto debugger =
        lager::http_debug_server{argc, argv, 8080, lager::resources_path()};
#endif
    auto loop  = lager::sdl_event_loop{};
    auto store = lager::make_store<todo::model_action>(
        todo::model{}, lager::with_sdl_event_loop{loop},
        zug::comp(
#ifdef DEBUGGER
            lager::with_debugger(debugger),
#endif
            lager::identity
        )
    );
    auto state = ui_state{};

    loop.run(
        [&](const SDL_Event& ev) {
            ImGui_ImplSDL2_ProcessEvent(&ev);
            return ev.type != SDL_QUIT;
        },
        [&](auto dt) {
            ImGui_ImplSDLRenderer_NewFrame();
            ImGui_ImplSDL2_NewFrame(window);
            ImGui::NewFrame();
            {
                draw(store, store.get(), state);
            }
            ImGui::Render();
            SDL_RenderSetScale(renderer, io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);
            SDL_SetRenderDrawColor(renderer, (Uint8)(clear_color.x * 255), (Uint8)(clear_color.y * 255), (Uint8)(clear_color.z * 255), (Uint8)(clear_color.w * 255));
            SDL_RenderClear(renderer);
            ImGui_ImplSDLRenderer_RenderDrawData(ImGui::GetDrawData());
            SDL_RenderPresent(renderer);
        });

    ImGui_ImplSDLRenderer_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
