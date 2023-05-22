#include "pch.h"
#include "LoadoutPresetIO.h"

#define J(var) j.at(#var).get_to(obj.var);
#define JOPTIONAL(var) if (j.find(#var) != j.end()){j.at(#var).get_to(obj.var);}
#define JJ(var) {#var, obj.var}

void to_json(json & j, const Item & obj)
{
	j = json{JJ(id)};
	if (obj.paint_id != 0)
	{
		j["paint_id"] = obj.paint_id;
	}
}

void from_json(const json& j, Item& obj)
{
	J(id)
	JOPTIONAL(paint_id)
}

void to_json(json& j, const LinearColor& obj)
{
	j = json {JJ(R), JJ(G), JJ(B), JJ(A)};
}

void from_json(const json& j, LinearColor& obj)
{
	J(R)
	J(G)
	J(B)
	J(A)
}

void to_json(json& j, const PaintOverride& obj)
{
	j = json{JJ(enabled), JJ(primary), JJ(secondary)};
}

void from_json(const json& j, PaintOverride& obj)
{
	J(enabled)
	J(primary)
	J(secondary)
}

void to_json(json& j, const Loadout& obj)
{
	j = json{JJ(custom_decal_name), JJ(esport_wheel_id), JJ(id_string), JJ(items), JJ(paint_override)};
}

void from_json(const json& j, Loadout& obj)
{
	J(custom_decal_name)
	J(esport_wheel_id)
	J(id_string)
	J(items)
	J(paint_override)
}

void to_json(json& j, const NamedLoadout& obj)
{
	j = json{JJ(loadout), JJ(name)};
}

void from_json(const json& j, NamedLoadout& obj)
{
	J(loadout)
	J(name)
}

void to_json(json& j, const ReplayLoadout& obj)
{
	j = json{JJ(loadouts), JJ(replay_id), JJ(replay_name)};
}

void from_json(const json& j, ReplayLoadout& obj)
{
	J(loadouts)
	J(replay_id)
	J(replay_name)
}

