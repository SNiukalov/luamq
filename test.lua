#!/usr/bin/lua

mq = require "luamq"

q, err = mq.create("/nownext", "wo")
if q == nil then
   print(err)
   return 1
end
print("q created successfully")


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

result, err = mq.close(q)
if result == nil then
   print(err)
   return 1
end
print("q closed successfully")





result, err = mq.unlink("/nownext")
if result == nil then
   print(err)
   return 1
end
print("q deleted successfully")


return 0


