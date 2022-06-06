#pragma once
#include "config-utils/shared/config-utils.hpp"

DECLARE_CONFIG(ModConfig,

    CONFIG_VALUE(Channel, std::string, "Channel Name", "darknight1050");
    CONFIG_VALUE(OAuth, std::string, "OAuth Token", "ENTER_TOKEN_HERE");

    CONFIG_INIT_FUNCTION(
        CONFIG_INIT_VALUE(Channel);
        CONFIG_INIT_VALUE(OAuth);
    )
)