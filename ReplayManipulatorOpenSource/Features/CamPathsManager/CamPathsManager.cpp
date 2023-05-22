#include "pch.h"
#include "CamPathsManager.h"
#include "ImguiUtils.h"

size_t FindCaseInsensitive(std::string data, std::string to_search, const size_t pos = 0);

CamPathsManager::CamPathsManager(std::shared_ptr<GameWrapper> gw, std::shared_ptr<CVarManagerWrapper> cw,
                                 std::filesystem::path campaths_folder
    )
    : GuiFeatureBase(std::move(gw), "Load DollyCam Paths", DefaultVisibility::kSettings),
      campaths_folder_(std::move(campaths_folder)),
      campaths_(GetJsonsFilesInFolder(campaths_folder_)),
      cw_(std::move(cw)) {}

void CamPathsManager::Render()
{
    static std::string search;
    ImGui::InputText("Filter by name", &search);

    for (auto& path_name : campaths_)
    {
        if (!search.empty() && FindCaseInsensitive(path_name, search, 0) == std::string::npos)
            continue;

        ImGui::ScopeId const scope_id(path_name);
        if (ImGui::Button(path_name.c_str()))
        {
            cw_->executeCommand(std::format("dolly_path_load {}", path_name));
        }
    }

    ImGui::Separator();
    if (ImGui::Button("Refresh paths"))
    {
        campaths_ = GetJsonsFilesInFolder(campaths_folder_);
    }
}

std::vector<std::string> CamPathsManager::GetJsonsFilesInFolder(const std::filesystem::path& folder) const
{
    if (!exists(folder))
    {
        LOG("CamPaths folder doesn't exist");
        return {};
    }
    std::vector<std::string> paths_in_folder;
    for (auto& dir : std::filesystem::directory_iterator(folder))
    {
        if (!dir.is_regular_file())
            continue;
        auto& path = dir.path();
        if (path.extension() != ".json")
            continue;
        try
        {
            //auto ifs = std::ifstream(path);
            //json jf = json::parse(ifs);
            //if parsing didn't throw. it's most likely a valid campath. good enough for now
            paths_in_folder.push_back(path.filename().string());
        }
        catch (std::exception& e)
        {
            LOG("{}", e.what());
        }
    }
    return paths_in_folder;
}
