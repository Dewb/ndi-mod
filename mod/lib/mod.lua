local mod = require 'core/mods'

local this_name = mod.this_name


function hook_norns_script_refresh()
  local original_refresh = norns.script.refresh
  norns.script.refresh = function()
    ndi_mod.update()
    original_refresh()
  end
end

function hook_menu_refresh()
  local original_refresh = _menu.refresh
  _menu.refresh = function()
    ndi_mod.update()
    original_refresh()
  end
end

function hook_global_refresh()
  local original_refresh = refresh
  refresh = function()
    ndi_mod.update()
    original_refresh()
  end
end

mod.hook.register("system_post_startup", "ndi", function()
  package.cpath = package.cpath .. ";" .. paths.code .. this_name .. "/lib/?.so"
  ndi_mod = require 'ndi_mod'

  clock.run(function()

    -- clients get confused if norns starts up NDI too quickly
    -- after a restart, so delay it
    clock.sleep(0.125)

    ndi_mod.init()
    ndi_mod.start()

    hook_global_refresh() -- for startup animation
    hook_menu_refresh() -- for menus (won't work without some core changes)

  end)
end)

mod.hook.register("script_post_init", "ndi", function()
  hook_norns_script_refresh()
end)

mod.hook.register("system_pre_shutdown", "ndi", function()
  ndi_mod.cleanup()
end)