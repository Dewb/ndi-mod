local mod = require 'core/mods'

local this_name = mod.this_name

mod.hook.register("system_post_startup", "ndi", function()
  package.cpath = package.cpath .. ";" .. paths.code .. this_name .. "/lib/?.so"
  ndi_mod = require 'ndi_mod'

  -- replace the default update function
  screen.update_default = function()
    _norns.screen_update()
    ndi_mod.update()
  end

  -- patch screensaver metro event handler to continue
  -- updating NDI after screensaver activates
  local original_ss_event = metro[36].event
  metro[36].event = function()
    original_ss_event()
    screen.update = function()
      ndi_mod.update()
    end
  end

  -- clients get confused if norns starts up NDI too quickly
  -- after a restart, so delay it until the first screen update
  screen.update = function()
    ndi_mod.init()
    ndi_mod.start()
    screen.update = screen.update_default
    screen.update()
  end

end)

mod.hook.register("system_pre_shutdown", "ndi", function()
  ndi_mod.cleanup()
end)