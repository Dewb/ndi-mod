local mod = require 'core/mods'

local this_name = mod.this_name

mod.hook.register("system_post_startup", "ndi-system-post-startup", function()
  package.cpath = package.cpath .. ";" .. paths.code .. this_name .. "/lib/?.so"
  ndi_mod = require 'ndi_mod'

  ndi_mod.init()
  ndi_mod.init_audio()
  ndi_mod.start()
end)

mod.hook.register("system_pre_shutdown", "ndi-system-pre-shutdown", function()
  ndi_mod.cleanup_audio()
  ndi_mod.cleanup()
end)

mod.hook.register("script_post_init", "ndi-script-post-init", function()
  local script_refresh_fn = norns.script.refresh
  norns.script.refresh = function()
    ndi_mod.update()
    script_refresh_fn()
  end
end)
