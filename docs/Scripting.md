## Scripting ndi-mod

The mod hooks `screen.update()`, so call `screen.update()` as usual when you want to update a NDI stream. Streaming is enabled by default when the system starts.

| method            | description                  |
| ----------------- | ---------------------------- |
| `ndi_mod.stop()`  | stops updating NDI streams |
| `ndi_mod.start()` | resumes updating NDI streams |
| `ndi_mod.is_running()` | returns `true` if NDI streams are being updated, false otherwise |
| `ndi_mod.create_image_sender(image, name)` | create an additional NDI stream based on the offscreen image buffer `image` and name it `name`. See [examples/send_image.lua](../mod/examples/send_image.lua) for usage in context. |
| `ndi_mod.destroy_image_sender(image)` | destroy the NDI source that was created for `image`. See [examples/send_image.lua](../mod/examples/send_image.lua) for usage in context. |

