local mod = require 'core/mods'

local this_name = mod.this_name

mod.hook.register("system_post_startup", "ndi", function()
  package.cpath = package.cpath .. ";" .. paths.code .. this_name .. "/lib/?.so"
  ndi_mod = require 'ndi_mod'

  screen.update_default = function()
    _norns.screen_update()
    ndi_mod.update()
  end
  screen.update = screen.update_default

  ndi_mod.start()
end)

mod.hook.register("system_pre_shutdown", "ndi", function()
  ndi_mod.cleanup()
end)