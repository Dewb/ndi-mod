-- ndi-mod/examples/send_image
--
-- a demo of using ndi-mod
-- to share offscreen images
-- as additional NDI streams

image = nil

function init()
  -- create the offscreen image
  -- it can be larger than the 128x64 norns screen!
  image = screen.create_image(256, 256)
end

-- draw a test pattern in the current context
function test_pattern()
  screen.clear()
  screen.line_width(1)
  screen.level(15)
  screen.aa(0)
  screen.circle(128,128,128)
  screen.stroke()
  screen.circle(128,128,64)
  screen.stroke()
  screen.move(0,128)
  screen.line(256,128)
  screen.stroke()
  screen.move(128,0)
  screen.line(128,256)
  screen.stroke()
  screen.update()
end

function key(n,z)
  if n==2 and z==1 then

    -- create the NDI sender for our offscreen image
    ndi_mod.create_image_sender(image, "image test")
    -- draw to it once. for a useful script, you
    -- probably want to do this repeatedly
    screen.draw_to(image, test_pattern)

  elseif n==3 and z==1 then

    -- send a blank image as the final frame and
    -- then destroy the NDI sender
    screen.draw_to(image, function()
      screen.clear()
      screen.update()
    end)
    ndi_mod.destroy_image_sender(image)

  end
end

function redraw()
  screen.clear()
  screen.line_width(1)
  screen.aa(0)
  screen.move(0,20)
  screen.text("main screen")
  screen.move(0,40)
  screen.text("key2 to send offscreen image")
  screen.move(0,50)
  screen.text("over NDI, key3 to stop")
  screen.update()
end

function cleanup()
  -- when the script is unloaded the image will be deleted.
  -- don't leave the sender hanging around, make
  -- sure it's cleaned up before the image goes away
  ndi_mod.destroy_image_sender(image)
end
