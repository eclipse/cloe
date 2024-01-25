#pragma once

#include <cloe/sync.hpp>
#include <sol/table.hpp>

namespace engine {
void register_usertype_coordinator(sol::table& lua, const cloe::Sync& sync);
}
