local mod = require 'core/mods'

local this_name = mod.this_name

mod.hook.register("system_post_startup", "ndi", function()

  package.cpath = package.cpath .. ";" .. paths.code .. this_name .. "/lib/?.so"
  ndi_mod = require 'ndi_mod'

end)

mod.hook.register("script_pre_init", "ndi", function()

  screen.update_default = function()
    _norns.screen_update()
    ndi_mod.start()
    ndi_mod.update()
  end

  screen.update = screen.update_default

end)

mod.hook.register("system_pre_shutdown", "ndi", function()

  ndi_mod.stop()

end)