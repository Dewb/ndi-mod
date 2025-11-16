-- ndi-mod/examples/send_image
--
-- a demo of using ndi-mod
-- to share offscreen images
-- as additional NDI streams

image = nil
step = 1

-- draw a test pattern in the current context
function test_pattern()
  screen.clear()
  screen.line_width(1)
  screen.level(15)
  screen.aa(0)
  screen.circle(64,64,64)
  screen.stroke()
  screen.circle(64,64,32)
  screen.stroke()
  screen.move(0,64)
  screen.line(128,64)
  screen.stroke()
  screen.move(64,0)
  screen.line(64,128)
  screen.stroke()
  screen.circle(64,64,step)
  screen.stroke()
  screen.update()
end

function key(n,z)
  if n==2 and z==1 then
    if image==nil then
      -- create the offscreen image
      -- it can be larger than the 128x64 norns screen!
      image = screen.create_image(128, 128)
      -- create the NDI sender for our offscreen image
      ndi_mod.create_image_sender(image, "image test")
    else
      ndi_mod.destroy_image_sender(image)
      image = nil
    end
  end
end

function refresh()

  step = (step % 60) + 1

  -- draw the main screen

  screen.line_width(1)
  screen.aa(0)
  screen.level(15)
  screen.move(0,10)
  screen.clear()
  screen.text("main screen")

  screen.level(7)
  screen.move(6,40)
  screen.text("tap key2 to toggle sending")
  screen.move(6,50)
  screen.text("offscreen image over NDI")
  
  screen.level(15)
  screen.rect(115, 3, 128, 10)
  screen.fill()
  screen.level(0)
  screen.move(126, 10)
  screen.text_right(step)
  
  screen.update()
  
    -- recommend not trying to update large offscreen images at 60fps
  -- here, only draw every sixth frame
  if step % 6 == 0 and image ~= nil then
    screen.draw_to(image, test_pattern)
  end
  

end


function cleanup()
  -- when the script is unloaded the image will be deleted.
  -- don't leave the sender hanging around, make
  -- sure it's cleaned up before the image goes away
  ndi_mod.destroy_image_sender(image)
end
