#!/usr/bin/lua

mq = require "mq"

q, err = mq.create("/nownext", "wo", "rw-rw----")
if q == nil then
   print(err)
   return 1
end
print("q created successfully")


result, err = mq.send(q, "hello world1")
if result == nil then
   print(err)
   return 1
end
print("message sent successfully")


result, err = mq.close(q)
if result == nil then
   print(err)
   return 1
end
print("q closed successfully")




q, err = mq.open("/nownext", "rw")
if q == nil then
   print(err)
   return 1
end
print("q opened successfully")


result, err = mq.send(q, "hello world2", 1)
if result == nil then
   print(err)
   return 1
end
print("message sent successfully")

msg, prio = mq.receive(q)
if msg == nil then
   print(prio)
   return 1
end
print("message '" .. msg .. "' received with prio " .. prio)


result, err = mq.close(q)
if result == nil then
   print(err)
   return 1
end
print("q closed successfully")


q, err = mq.open("/nownext", "ro")
if q == nil then
   print(err)
   return 1
end
print("q opened successfully")

msg, prio = mq.receive(q)
if msg == nil then
   print(prio)
   return 1
end
print("message '" .. msg .. "' received with prio " .. prio)


result, err = mq.unlink("/nownext")
if result == nil then
   print(err)
   return 1
end
print("q deleted successfully")


return 0


