#pragma once

struct PriUid
{
    explicit PriUid(PriWrapper& pri);
    explicit PriUid(std::string id_string);

    auto operator<=>(const PriUid&) const = default;
    bool operator==(const PriUid&) const = default;

    static std::string GetPriIdString(PriWrapper& pri);

    std::string pri_id_string;
};

// Custom hash makes it possible to use custom types in unordered containers.
// custom specialization of std::hash can be injected in namespace std
template <>
struct std::hash<PriUid>
{
    std::size_t operator()(PriUid const& s) const noexcept
    {
        return std::hash<std::string>{}(s.pri_id_string);
        //std::size_t h1 = std::hash<std::string>{}(s.first_name);
        //std::size_t h2 = std::hash<std::string>{}(s.last_name);
        //return h1 ^ (h2 << 1); // or use boost::hash_combine
    }
};
