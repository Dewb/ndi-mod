-- example of using ndi-mod to send offscreen images as additional NDI sources

image = nil

function init()
  image = screen.create_image(256, 256)
end

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
    ndi_mod.create_image_sender(image, "image test")
    screen.draw_to(image, test_pattern)
  elseif n==3 and z==1 then
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
  ndi_mod.destroy_image_sender(image)
end
