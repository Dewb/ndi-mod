local mod = require 'core/mods'

local this_name = mod.this_name
local first_update = true

mod.hook.register("system_post_startup", "ndi", function()
  package.cpath = package.cpath .. ";" .. paths.code .. this_name .. "/lib/?.so"
  ndi_mod = require 'ndi_mod'

  screen.update_default = function()
    -- clients get confused if norns creates the NDI sender too quickly after a restart.
    -- delay it to the first screen update
    if first_update then ndi_mod.init(); first_update = false end

    _norns.screen_update()
    ndi_mod.update()
  end

  screen.update = screen.update_default
  ndi_mod.start()
end)

mod.hook.register("system_pre_shutdown", "ndi", function()
  ndi_mod.cleanup()
end)