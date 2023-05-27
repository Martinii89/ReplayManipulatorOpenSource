#include "pch.h"
#include "Credits.h"
#include "ImguiUtils.h"

CreditsInSettings::CreditsInSettings(std::shared_ptr<GameWrapper> gw)
    : GuiFeatureBase{std::move(gw), "About", DefaultVisibility::kSettings} {}

void CreditsInSettings::Render()
{
    constexpr auto github_url = """https://github.com/Martinii89/ReplayManipulatorOpenSource""";

    ImGui::TextUnformatted("This plugin is now open source and the code is available on github");

    ImGui::TextWrapped("If there are any issues or feature requests you can report them on github");
    ImGui::NewLine();
    ImGui::TextUrl("Github", github_url);

}
