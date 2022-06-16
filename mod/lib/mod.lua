local mod = require 'core/mods'

local this_name = mod.this_name

mod.hook.register("system_post_startup", "ndi", function()
  package.cpath = package.cpath .. ";" .. paths.code .. this_name .. "/lib/?.so"
  ndi_mod = require 'ndi_mod'
end)
